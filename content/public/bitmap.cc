// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/bitmap.h"

#include "SDL_image.h"

#include <array>

#include "base/exceptions/exception.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "content/public/font.h"
#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"

#define OutlineSize 1

namespace content {

namespace {

uint16_t utf8_to_ucs2(const char* _input, const char** end_ptr) {
  const unsigned char* input = reinterpret_cast<const unsigned char*>(_input);
  *end_ptr = _input;

  if (input[0] == 0)
    return -1;

  if (input[0] < 0x80) {
    *end_ptr = _input + 1;

    return input[0];
  }

  if ((input[0] & 0xE0) == 0xE0) {
    if (input[1] == 0 || input[2] == 0)
      return -1;

    *end_ptr = _input + 3;

    return (input[0] & 0x0F) << 12 | (input[1] & 0x3F) << 6 | (input[2] & 0x3F);
  }

  if ((input[0] & 0xC0) == 0xC0) {
    if (input[1] == 0)
      return -1;

    *end_ptr = _input + 2;

    return (input[0] & 0x1F) << 6 | (input[1] & 0x3F);
  }

  return -1;
}

void RenderShadowSurface(SDL_Surface*& in, const SDL_Color& color) {
  SDL_Surface* out =
      SDL_CreateSurface(in->w + 1, in->h + 1, in->format->format);
  float fr = color.r / 255.0f, fg = color.g / 255.0f, fb = color.b / 255.0f;

  for (int y = 0; y < in->h + 1; ++y) {
    for (int x = 0; x < in->w + 1; ++x) {
      uint32_t src = 0, shd = 0,
               *outP = (uint32_t*)((uint8_t*)out->pixels + y * out->pitch) + x;

      if (y < in->h && x < in->w)
        src = ((uint32_t*)((uint8_t*)in->pixels + y * in->pitch))[x];
      if (y > 0 && x > 0)
        shd = ((uint32_t*)((uint8_t*)in->pixels + (y - 1) * in->pitch))[x - 1] &
              in->format->Amask;

      if (x == 0 || y == 0 || src & in->format->Amask) {
        *outP = (x == in->w || y == in->h) ? shd : src;
        continue;
      }

      uint8_t srcA = (src & in->format->Amask) >> in->format->Ashift;
      float fSrcA = srcA / 255.0f,
            fShdA = ((shd & in->format->Amask) >> in->format->Ashift) / 255.0f;
      float fa = fSrcA + fShdA * (1.0f - fSrcA), co3 = fSrcA / fa;

      *outP = SDL_MapRGBA(
          in->format,
          static_cast<uint8_t>(std::clamp(fr * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fg * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fb * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fa, 0.0f, 1.0f) * 255));
    }
  }

  SDL_DestroySurface(in);
  in = out;
}

std::string FixupString(const std::string& text) {
  std::string str(text);

  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == '\r' || str[i] == '\n')
      str[i] = ' ';

  return str;
}

SDL_Surface* RenderText(const std::string& text,
                        uint8_t* font_opacity,
                        TTF_Font* font,
                        bool is_bold,
                        bool is_italic,
                        bool has_shadow,
                        bool has_outline,
                        const SDL_Color& color,
                        const SDL_Color& out_color) {
  if (!font)
    return nullptr;

  int font_style = TTF_STYLE_NORMAL;
  if (is_bold)
    font_style |= TTF_STYLE_BOLD;
  if (is_italic)
    font_style |= TTF_STYLE_ITALIC;
  TTF_SetFontStyle(font, font_style);

  auto ensure_format = [](SDL_Surface*& surf) {
    if (!surf)
      return;

    SDL_Surface* format_surf = surf;
    if (surf->format->format != SDL_PIXELFORMAT_ABGR8888) {
      format_surf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888);
      SDL_DestroySurface(surf);
      surf = format_surf;
    }
  };

  std::string src_text = FixupString(text);
  if (src_text.empty() || src_text == " ")
    return nullptr;

  SDL_Color font_color = color;
  SDL_Color outline_color = out_color;
  if (font_opacity)
    *font_opacity = font_color.a;

  font_color.a = 255;
  outline_color.a = 255;

  SDL_Surface* raw_surf =
      TTF_RenderUTF8_Blended(font, src_text.c_str(), font_color);
  if (!raw_surf)
    return nullptr;
  ensure_format(raw_surf);

  if (has_shadow)
    RenderShadowSurface(raw_surf, font_color);

  if (has_outline) {
    SDL_Surface* outline = nullptr;
    TTF_SetFontOutline(font, OutlineSize);
    outline = TTF_RenderUTF8_Blended(font, src_text.c_str(), outline_color);
    if (!outline) {
      SDL_DestroySurface(raw_surf);
      return nullptr;
    }

    ensure_format(outline);
    SDL_Rect outRect = {OutlineSize, OutlineSize, raw_surf->w, raw_surf->h};
    SDL_SetSurfaceBlendMode(raw_surf, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(raw_surf, NULL, outline, &outRect);
    SDL_DestroySurface(raw_surf);
    raw_surf = outline;
    TTF_SetFontOutline(font, 0);
  }

  return raw_surf;
}

}  // namespace

Bitmap::Bitmap(scoped_refptr<Graphics> host, int width, int height)
    : GraphicElement(host),
      Disposable(host),
      font_(new Font()),
      surface_cache_(nullptr) {
  if (width <= 0 || height <= 0)
    throw base::Exception(base::Exception::ContentError,
                          "Invalid bitmap create size: (%dx%d)", width, height);

  if (width > screen()->renderer()->max_texture_size() ||
      height > screen()->renderer()->max_texture_size())
    throw base::Exception(base::Exception::OpenGLError,
                          "Unable to create large bitmap: (%dx%d)", width,
                          height);

  size_ = base::Vec2i(width, height);
  host->renderer()->PostTask(base::BindOnce(
      &Bitmap::InitBitmapInternal, reinterpret_cast<uint64_t>(this), size_));
}

Bitmap::Bitmap(scoped_refptr<Graphics> host, const std::string& filename)
    : GraphicElement(host),
      Disposable(host),
      font_(new Font()),
      surface_cache_(nullptr) {
  SDL_Surface* surf = nullptr;

  auto file_handler = base::BindRepeating(
      [](SDL_Surface** surf, SDL_RWops* ops, const std::string& ext) {
        *surf = IMG_LoadTyped_RW(ops, SDL_TRUE, ext.c_str());

        return !!*surf;
      },
      &surf);

  host->filesystem()->OpenRead(filename, file_handler);

  if (!surf)
    throw base::Exception(base::Exception::ContentError,
                          "Failed to load image: '%s': %s", filename.c_str(),
                          SDL_GetError());

  if (surf->w + surf->h > screen()->renderer()->max_texture_size() * 2)
    throw base::Exception(base::Exception::OpenGLError,
                          "Unable to load large image: (%dx%d)", surf->w,
                          surf->h);

  size_ = base::Vec2i(surf->w, surf->h);

  if (surf->format->format != SDL_PIXELFORMAT_ABGR8888) {
    SDL_Surface* conv =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888);
    SDL_DestroySurface(surf);
    surf = conv;
  }

  host->renderer()->PostTask(base::BindOnce(
      &Bitmap::InitBitmapInternal, reinterpret_cast<uint64_t>(this), surf));
}

Bitmap::~Bitmap() {
  Dispose();
}

scoped_refptr<Bitmap> Bitmap::Clone() {
  CheckIsDisposed();

  scoped_refptr<Bitmap> new_bitmap = new Bitmap(screen(), size_.x, size_.y);
  new_bitmap->Blt(0, 0, this, size_);

  return new_bitmap;
}

void Bitmap::Blt(int x,
                 int y,
                 scoped_refptr<Bitmap> src_bitmap,
                 const base::Rect& src_rect,
                 int opacity) {
  CheckIsDisposed();

  if (src_rect.width <= 0 || src_rect.height <= 0)
    return;
  if (src_bitmap->IsDisposed() || !opacity)
    return;

  base::Rect rect = src_rect;

  if (rect.x + rect.width > src_bitmap->GetWidth())
    rect.width = src_bitmap->GetWidth() - rect.x;

  if (rect.y + rect.height > src_bitmap->GetHeight())
    rect.height = src_bitmap->GetHeight() - rect.y;

  rect.width = std::max(0, rect.width);
  rect.height = std::max(0, rect.height);

  StretchBlt(base::Rect(x, y, rect.width, rect.height), src_bitmap, rect,
             opacity);
}

void Bitmap::StretchBlt(const base::Rect& dest_rect,
                        scoped_refptr<Bitmap> src_bitmap,
                        const base::Rect& src_rect,
                        int opacity) {
  CheckIsDisposed();

  if (dest_rect.width <= 0 || dest_rect.height <= 0)
    return;
  if (src_rect.width <= 0 || src_rect.height <= 0)
    return;

  opacity = std::clamp(opacity, 0, 255);

  if (src_bitmap->IsDisposed() || !opacity)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::StretchBltInternal, reinterpret_cast<uint64_t>(this), dest_rect,
      reinterpret_cast<uint64_t>(src_bitmap.get()), src_rect,
      opacity / 255.0f));

  NeedUpdateSurface();
}

void Bitmap::FillRect(const base::Rect& rect, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal,
                     reinterpret_cast<uint64_t>(this), rect, color->AsBase()));

  NeedUpdateSurface();
}

void Bitmap::GradientFillRect(const base::Rect& rect,
                              scoped_refptr<Color> color1,
                              scoped_refptr<Color> color2,
                              bool vertical) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::GradientFillRectInternal, reinterpret_cast<uint64_t>(this), rect,
      color1->AsBase(), color2->AsBase(), vertical));

  NeedUpdateSurface();
}

void Bitmap::Clear() {
  CheckIsDisposed();

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal,
                     reinterpret_cast<uint64_t>(this), size_, base::Vec4()));

  NeedUpdateSurface();
}

void Bitmap::ClearRect(const base::Rect& rect) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal,
                     reinterpret_cast<uint64_t>(this), rect, base::Vec4()));

  NeedUpdateSurface();
}

scoped_refptr<Color> Bitmap::GetPixel(int x, int y) {
  CheckIsDisposed();

  if (x < 0 || x >= size_.x || y < 0 || y >= size_.y)
    return nullptr;

  SurfaceRequired();
  int bpp = surface_cache_->format->bytes_per_pixel;
  uint8_t* pixel = static_cast<uint8_t*>(surface_cache_->pixels) +
                   y * surface_cache_->pitch + x * bpp;

  uint8_t color[4];
  SDL_GetRGBA(*reinterpret_cast<uint32_t*>(pixel), surface_cache_->format,
              &color[0], &color[1], &color[2], &color[3]);

  return new Color(color[0], color[1], color[2], color[3]);
}

void Bitmap::SetPixel(int x, int y, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (x < 0 || x >= size_.x || y < 0 || y >= size_.y)
    return;

  auto data = color->AsNormal();
  if (surface_cache_) {
    int bpp = surface_cache_->format->bytes_per_pixel;
    uint8_t* pixel = static_cast<uint8_t*>(surface_cache_->pixels) +
                     y * surface_cache_->pitch + x * bpp;
    *reinterpret_cast<uint32_t*>(pixel) =
        SDL_MapRGBA(surface_cache_->format, static_cast<uint8_t>(data.x),
                    static_cast<uint8_t>(data.y), static_cast<uint8_t>(data.z),
                    static_cast<uint8_t>(data.w));
  }

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::SetPixelInternal, reinterpret_cast<uint64_t>(this), x, y, data));

  NeedUpdateSurface();
}

void Bitmap::HueChange(int hue) {
  CheckIsDisposed();

  if (hue % 360 == 0)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::HueChangeInternal, reinterpret_cast<uint64_t>(this), hue));

  NeedUpdateSurface();
}

void Bitmap::Blur() {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

void Bitmap::RadialBlur(int angle, int division) {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

void Bitmap::DrawText(const base::Rect& rect,
                      const std::string& str,
                      TextAlign align) {
  CheckIsDisposed();

  const int font_id = font_->font_id();
  const int font_size = font_->GetSize();
  const bool is_bold = font_->GetBold();
  const bool is_italic = font_->GetItalic();
  const bool has_shadow = font_->GetShadow();
  const bool has_outline = font_->GetOutline();
  const SDL_Color font_color = font_->GetColor()->AsSDLColor();
  const SDL_Color out_color = font_->GetOutColor()->AsSDLColor();

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::DrawTextInternal, reinterpret_cast<uint64_t>(this), rect, str,
      align, font_id, font_size, is_bold, is_italic, has_shadow, has_outline,
      font_color, out_color));

  NeedUpdateSurface();
}

scoped_refptr<Rect> Bitmap::TextSize(const std::string& str) {
  CheckIsDisposed();

  int w = 0, h = 0;
  TTF_Font* font = font_->AsSDLFont();
  if (font && !str.empty()) {
    std::string src_text = FixupString(str);
    TTF_SizeUTF8(font, src_text.c_str(), &w, &h);
    const char* end_char = nullptr;
    uint16_t ucs2 = utf8_to_ucs2(str.c_str(), &end_char);
    if (font_->GetItalic() && *end_char == '\0')
      TTF_GlyphMetrics(font, ucs2, 0, 0, 0, 0, &w);
  }

  return new Rect(base::Rect(0, 0, w, h));
}

scoped_refptr<Font> Bitmap::GetFont() const {
  CheckIsDisposed();
  return font_;
}

void Bitmap::SetFont(scoped_refptr<Font> font) {
  CheckIsDisposed();
  *font_ = *font;
}

SDL_Surface* Bitmap::SurfaceRequired() {
  CheckIsDisposed();

  if (surface_cache_)
    return surface_cache_;
  surface_cache_ =
      SDL_CreateSurface(size_.x, size_.y, SDL_PIXELFORMAT_ABGR8888);

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::GetSurfaceInternal,
                     reinterpret_cast<uint64_t>(this), surface_cache_));
  screen()->renderer()->WaitForSync();

  return surface_cache_;
}

void Bitmap::UpdateSurface() {
  CheckIsDisposed();

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::UpdateSurfaceInternal,
                     reinterpret_cast<uint64_t>(this), surface_cache_));
  screen()->renderer()->WaitForSync();

  NeedUpdateSurface();
}

void Bitmap::OnObjectDisposed() {
  // Dispose notify
  observers_.Notify();

  if (surface_cache_) {
    SDL_DestroySurface(surface_cache_);
    surface_cache_ = nullptr;
  }

  screen()->renderer()->PostTask(base::BindOnce(
      [](uint64_t self) {
        auto it = Graphics::texture_pool().find(self);
        renderer::TextureFrameBuffer::Del(it->second);
        Graphics::texture_pool().erase(it);
      },
      reinterpret_cast<uint64_t>(this)));
}

void Bitmap::InitBitmapInternal(
    uint64_t self,
    const std::variant<base::Vec2i, SDL_Surface*>& initial_data) {
  // Alloc new texture memory
  auto tex_fbo = renderer::TextureFrameBuffer::Gen();
  bool need_clear = false;

  if (std::holds_alternative<base::Vec2i>(initial_data)) {
    auto size = std::get<base::Vec2i>(initial_data);
    renderer::TextureFrameBuffer::Alloc(tex_fbo, size.x, size.y);
    // Clear texture cache
    need_clear = true;
  } else if (std::holds_alternative<SDL_Surface*>(initial_data)) {
    SDL_Surface* surf = std::get<SDL_Surface*>(initial_data);
    renderer::TextureFrameBuffer::Alloc(tex_fbo, surf->w, surf->h);
    renderer::Texture::TexImage2D(surf->w, surf->h, GL_RGBA, surf->pixels);
    SDL_DestroySurface(surf);
  } else {
    NOTREACHED();
  }

  // Link framebuffer
  renderer::TextureFrameBuffer::LinkFrameBuffer(tex_fbo);
  if (need_clear)
    renderer::FrameBuffer::Clear();

  // Emplace in graphics context
  Graphics::texture_pool().emplace(self, tex_fbo);
}

void Bitmap::StretchBltInternal(uint64_t self,
                                const base::Rect& dest_rect,
                                uint64_t src,
                                const base::Rect& src_rect,
                                float opacity) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  auto& src_tex = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(src));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);
  base::Vec2i src_size(src_tex.width, src_tex.height);

  auto& dst_tex =
      renderer::GSM.EnsureCommonTFB(dest_rect.width, dest_rect.height);

  renderer::Blt::BeginDraw(dst_tex);
  renderer::Blt::TexSource(tex_fbo);
  renderer::Blt::BltDraw(dest_rect, dest_rect.Size());
  renderer::Blt::EndDraw();

  /*
   * (texCoord - src_offset) * src_dst_factor
   */
  base::Vec4 offset_scale;
  offset_scale.x = static_cast<float>(src_rect.x) / src_size.x;
  offset_scale.y = static_cast<float>(src_rect.y) / src_size.y;
  offset_scale.z = (static_cast<float>(src_size.x) / src_rect.width) *
                   (static_cast<float>(dest_rect.width) / dst_tex.width);
  offset_scale.w = (static_cast<float>(src_size.y) / src_rect.height) *
                   (static_cast<float>(dest_rect.height) / dst_tex.height);

  auto& shader = renderer::GSM.shaders()->texblt;

  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  renderer::FrameBuffer::Bind(tex_fbo.fbo);

  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetSrcTexture(src_tex.tex);
  shader.SetTextureSize(src_size);
  shader.SetDstTexture(dst_tex.tex);
  shader.SetOffsetScale(offset_scale);
  shader.SetOpacity(opacity);

  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::FillRectInternal(uint64_t self,
                              const base::Rect& rect,
                              const base::Vec4& color) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));

  renderer::FrameBuffer::Bind(tex_fbo.fbo);
  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(rect);

  renderer::GSM.states.clear_color.Push(color);
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Bitmap::GradientFillRectInternal(uint64_t self,
                                      const base::Rect& rect,
                                      const base::Vec4& color1,
                                      const base::Vec4& color2,
                                      bool vertical) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);

  renderer::FrameBuffer::Bind(tex_fbo.fbo);
  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  auto& shader = renderer::GSM.shaders()->color;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());

  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(rect);

  if (vertical) {
    quad->SetColor(0, color1);
    quad->SetColor(1, color1);
    quad->SetColor(2, color2);
    quad->SetColor(3, color2);
  } else {
    quad->SetColor(0, color1);
    quad->SetColor(1, color2);
    quad->SetColor(2, color2);
    quad->SetColor(3, color1);
  }

  quad->Draw();

  renderer::GSM.states.viewport.Pop();
  renderer::GSM.states.blend.Pop();
}

void Bitmap::SetPixelInternal(uint64_t self,
                              int x,
                              int y,
                              const base::Vec4& color) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));

  std::array<uint8_t, 4> pixel = {
      static_cast<uint8_t>(color.x), static_cast<uint8_t>(color.y),
      static_cast<uint8_t>(color.z), static_cast<uint8_t>(color.w)};

  renderer::Texture::Bind(tex_fbo.tex);
  renderer::Texture::TexSubImage2D(x, y, 1, 1, GL_RGBA, pixel.data());
}

void Bitmap::HueChangeInternal(uint64_t self, int hue) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);

  auto& dst_tex = renderer::GSM.EnsureCommonTFB(size.x, size.y);

  while (hue < 0)
    hue += 359;
  hue %= 359;

  renderer::FrameBuffer::Bind(dst_tex.fbo);
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.viewport.Push(size);
  auto& shader = renderer::GSM.shaders()->hue;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTexture(tex_fbo.tex);
  shader.SetTextureSize(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetHueAdjustValue(static_cast<float>(hue) / 360.0f);

  auto* quad = renderer::GSM.common_quad();
  quad->SetTexCoordRect(base::Vec2(size));
  quad->SetPositionRect(base::Vec2(size));
  quad->Draw();
  renderer::GSM.states.viewport.Pop();

  renderer::Blt::BeginDraw(tex_fbo);
  renderer::Blt::TexSource(dst_tex);
  renderer::Blt::BltDraw(size, size);
  renderer::Blt::EndDraw();
}

void Bitmap::DrawTextInternal(uint64_t self,
                              const base::Rect& rect,
                              const std::string& str,
                              TextAlign align,
                              int font_id,
                              int font_size,
                              bool is_bold,
                              bool is_italic,
                              bool has_shadow,
                              bool has_outline,
                              const SDL_Color& color,
                              const SDL_Color& out_color) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);

  std::mutex* lock = nullptr;
  TTF_Font* font_obj = Font::AsSDLFont(font_id, font_size);

  uint8_t fopacity;
  SDL_Surface* txt_surf =
      RenderText(str, &fopacity, font_obj, is_bold, is_italic, has_shadow,
                 has_outline, color, out_color);
  if (!txt_surf)
    return;

  int align_x = rect.x, align_y = rect.y + (rect.height - txt_surf->h) / 2;
  switch (align) {
    default:
    case TextAlign::Left:
      break;
    case TextAlign::Center:
      align_x += (rect.width - txt_surf->w) / 2;
      break;
    case TextAlign::Right:
      align_x += rect.width - txt_surf->w;
      break;
  }

  float zoom_x = static_cast<float>(rect.width) / txt_surf->w;
  zoom_x = std::min(zoom_x, 1.0f);
  base::Rect pos(align_x, align_y, txt_surf->w * zoom_x, txt_surf->h);

  auto& common_frame_buffer =
      renderer::GSM.EnsureCommonTFB(pos.width, pos.height);
  base::Vec2i origin_size =
      base::Vec2i(common_frame_buffer.width, common_frame_buffer.height);

  renderer::Blt::BeginDraw(common_frame_buffer);
  renderer::Blt::TexSource(tex_fbo);
  renderer::Blt::BltDraw(pos, pos.Size());
  renderer::Blt::EndDraw();

  base::Vec2i rendered_text_size;
  auto& generic_tex = renderer::GSM.EnsureGenericTex(txt_surf->w, txt_surf->h,
                                                     rendered_text_size);

  renderer::Texture::Bind(generic_tex);
  renderer::Texture::TexSubImage2D(0, 0, txt_surf->w, txt_surf->h, GL_RGBA,
                                   txt_surf->pixels);

  base::Vec4 offset_scale(
      0, 0, static_cast<float>(rendered_text_size.x * zoom_x) / origin_size.x,
      static_cast<float>(rendered_text_size.y) / origin_size.y);

  base::Vec2 text_surf_size = base::Vec2i(txt_surf->w, txt_surf->h);

  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  renderer::FrameBuffer::Bind(tex_fbo.fbo);

  auto& shader = renderer::GSM.shaders()->texblt;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetSrcTexture(generic_tex);
  shader.SetTextureSize(rendered_text_size);
  shader.SetDstTexture(common_frame_buffer.tex);
  shader.SetOffsetScale(offset_scale);
  shader.SetOpacity(fopacity / 255.0f);

  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(pos);
  quad->SetTexCoordRect(text_surf_size);
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::GetSurfaceInternal(uint64_t self, SDL_Surface* output) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);

  renderer::GSM.states.viewport.Push(size);
  renderer::FrameBuffer::Bind(tex_fbo.fbo);
  renderer::GL.ReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                          output->pixels);
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::UpdateSurfaceInternal(uint64_t self, SDL_Surface* output) {
  auto& tex_fbo = Graphics::texture_pool().at(reinterpret_cast<uint64_t>(self));
  base::Vec2i size(tex_fbo.width, tex_fbo.height);

  renderer::Texture::Bind(tex_fbo.tex);
  renderer::Texture::TexImage2D(size.x, size.y, GL_RGBA, output->pixels);
}

void Bitmap::NeedUpdateSurface() {
  // Clear pixel cache
  if (surface_cache_) {
    SDL_DestroySurface(surface_cache_);
    surface_cache_ = nullptr;
  }

  observers_.Notify();
}

}  // namespace content
