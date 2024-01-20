
// clang-format off

#include "base/debug/logging.h"
#include "base/bind/callback.h"
#include "base/memory/ref_counted.h"

#include "renderer/context/gles2_context.h"

#include "SDL.h"
#include "SDL_opengles2.h"
#include "SDL_video.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "SDL_stdinc.h"

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "content/worker/content_compositor.h"
#include "content/public/bitmap.h"
#include "content/public/sprite.h"
#include "content/public/plane.h"
#include "content/public/window2.h"
#include "content/public/tilemap2.h"

#include "physfs.h"

// clang-format on

const int kTestMapData[][4] = {
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0xb22, 0x0, 0x0, 0x1},   {0xb14, 0x0, 0x0, 0x2},
    {0xb14, 0x0, 0x0, 0x4},   {0xb14, 0x0, 0x0, 0x8},
    {0xb14, 0x0, 0x0, 0x3},   {0xb14, 0x0, 0x0, 0xc},
    {0xb14, 0x0, 0x0, 0x5},   {0xb24, 0x0, 0x0, 0xa},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x10cb, 0x0, 0x0},
    {0x0, 0x10c1, 0x0, 0x0},  {0x0, 0x10cd, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0xb12, 0x0, 0x0, 0x0},
    {0xb1c, 0xc7b, 0x0, 0x0}, {0xb1c, 0xc71, 0x0, 0x0},
    {0xb0c, 0xc71, 0x0, 0x0}, {0xb1c, 0xc71, 0x0, 0x0},
    {0xb1c, 0xc71, 0x0, 0x0}, {0xb1c, 0xc7d, 0x0, 0x0},
    {0xb19, 0x0, 0x0, 0x0},   {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x10cb, 0x0, 0x0},  {0x0, 0x10cd, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0xb20, 0x0, 0x0, 0x0},   {0x1abe, 0x0, 0x0, 0x0},
    {0x187e, 0x0, 0x0, 0x0},  {0xb20, 0x106e, 0x0, 0x0},
    {0x601, 0x0, 0x0, 0x0},   {0x60a, 0x0, 0x0, 0x0},
    {0x60c, 0x0, 0x0, 0x0},   {0xb20, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x32, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x32, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x32, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0xb2c, 0x0, 0x0, 0x0},
    {0x1c13, 0x0, 0x0, 0x0},  {0x19d6, 0x0, 0x0, 0x0},
    {0xb29, 0x0, 0x0, 0x5},   {0xb21, 0x0, 0x0, 0x0},
    {0xb21, 0x0, 0x0, 0x0},   {0xb21, 0x0, 0x0, 0x0},
    {0xb27, 0x0, 0x0, 0x0},   {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x3a, 0x0},    {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x3a, 0x0},    {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x3a, 0x0},    {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x619, 0x0, 0x0, 0x0},   {0x1c11, 0x0, 0x0, 0x0},
    {0x19d4, 0x0, 0x0, 0x0},  {0x619, 0x0, 0x0, 0x5},
    {0x618, 0x0, 0x0, 0x0},   {0x618, 0x0, 0x0, 0x0},
    {0x618, 0x0, 0x0, 0x0},   {0x618, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x1c19, 0x0, 0x0, 0x0},  {0x19dc, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x82b, 0x8bb, 0x0, 0x0}, {0x825, 0x8b5, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x82b, 0x88b, 0x0, 0x0},
    {0x816, 0x876, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x82c, 0x8bc, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x828, 0x888, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x82e, 0x8be, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x80f, 0x89f, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x822, 0x8b2, 0x0, 0x0},
    {0x824, 0x8b4, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x82e, 0x88e, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x80f, 0x86f, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x822, 0x882, 0x0, 0x0},
    {0x824, 0x884, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x828, 0x8b8, 0x0, 0x0}, {0x826, 0x8b6, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0x828, 0x888, 0x0, 0x0}, {0x826, 0x886, 0x0, 0x0},
    {0x0, 0x0, 0x0, 0x0},     {0x0, 0x0, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0xbea, 0x0, 0x0}, {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0xbeb, 0x0, 0x0}, {0xb14, 0xbed, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb14, 0xd6b, 0x0, 0x0},
    {0xb14, 0xd6d, 0x0, 0x0}, {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb14, 0x0, 0x0, 0x0},
    {0xb14, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x7},   {0xb00, 0x0, 0x0, 0xb},
    {0xb00, 0x0, 0x0, 0xd},   {0xb00, 0x0, 0x0, 0xe},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0xbec, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x1},   {0xb00, 0x0, 0x0, 0x2},
    {0xb00, 0x0, 0x0, 0xa},   {0xb00, 0x0, 0x0, 0x4},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},   {0xb00, 0x0, 0x0, 0x0},
    {0xb00, 0x0, 0x0, 0x0},
};

scoped_refptr<content::Table> GetTestMapData() {
  scoped_refptr<content::Table> t = new content::Table(17, 13, 4);

  for (int z = 0; z < 4; ++z) {
    for (int y = 0; y < 13; ++y) {
      for (int x = 0; x < 17; ++x) {
        t->Set(kTestMapData[x + y * 17][4], x, y, z);
      }
    }
  }

  return t;
}

SDL_EGLAttrib kAttrib[] = {
    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
    EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE,
    // EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE,
    EGL_NONE,
};

SDL_EGLAttrib* SDLCALL GetAttribArray() {
  return kAttrib;
}

int main(int argc, char* argv[]) {
  PHYSFS_init(argv[0]);

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_EGL_PLATFORM, EGL_PLATFORM_ANGLE_ANGLE);
  SDL_EGL_SetEGLAttributeCallbacks(GetAttribArray, nullptr, nullptr);

  SDL_Window* win = SDL_CreateWindow("RGU Window", 800, 600,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  content::WorkerTreeCompositor cc;
  content::WorkerTreeCompositor::InitParams params;

  params.binding_params.binding_boot =
      base::BindRepeating([](scoped_refptr<content::BindingRunner> binding) {
        scoped_refptr<content::Graphics> screen = binding->graphics();

        screen->ResizeScreen(base::Vec2i(1024, 768));

        scoped_refptr<content::Bitmap> bmp = new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\example.png");

        scoped_refptr<content::Bitmap> sampler = new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\test.png");

        bmp->ClearRect(base::Rect(20, 20, 50, 50));
        bmp->Blt(100, 100, sampler, sampler->GetRect()->AsBase(), 255);

        bmp->GradientFillRect(base::Rect(50, 100, 100, 50),
                              new content::Color(0, 255, 0, 125),
                              new content::Color(0, 0, 255, 125));

        /* Sync method test */
        // SDL_Surface* surf = bmp->SurfaceRequired();
        // IMG_SavePNG(surf, "D:\\Desktop\\snap.png");

        scoped_refptr<content::Sprite> sp = new content::Sprite(screen);
        sp->SetBitmap(bmp);
        sp->GetTransform().SetOrigin(
            base::Vec2i(sp->GetWidth() / 2, sp->GetHeight() / 2));
        sp->GetTransform().SetPosition(
            base::Vec2i(screen->GetWidth() / 2, screen->GetHeight() / 2));

        sp->SetBushDepth(100);
        sp->SetBushOpacity(128);

        sp->SetWaveAmp(20);
        sp->GetSrcRect()->Set(base::Rect(100, 100, 100, 100));

        scoped_refptr<content::Tilemap2> tilemap =
            new content::Tilemap2(screen);
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileA1,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_A1.png"));
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileA2,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_A2.png"));
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileA4,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_A4.png"));
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileA5,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_A5.png"));
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileB,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_B.png"));
        tilemap->SetBitmap(
            content::Tilemap2::TilemapBitmapID::TileC,
            new content::Bitmap(screen,
                                "D:\\Desktop\\rgu\\app\\test\\Inside_C.png"));

        tilemap->SetMapData(GetTestMapData());

        /*scoped_refptr<content::Plane> pl = new content::Plane(screen);
        pl->SetBitmap(
            new content::Bitmap(screen,
        "D:\\Desktop\\rgu\\app\\test\\bg.png"));*/

        for (int i = 0; i < 120; ++i) {
          screen->Update();
        }

        screen->Freeze();

        scoped_refptr<content::Viewport> viewp =
            new content::Viewport(screen, base::Rect(0, 0, 300, 300));
        viewp->SetZ(100);

        scoped_refptr<content::Window2> vx_win =
            new content::Window2(screen, 100, 100, 300, 300);

        vx_win->SetViewport(viewp);

        vx_win->SetWindowskin(new content::Bitmap(
            screen, "D:\\Desktop\\rgu\\app\\test\\Window.png"));
        vx_win->SetZ(100);
        vx_win->GetTone()->Set(-68, -68, 68, 0);
        vx_win->SetPause(true);

        vx_win->SetContents(bmp);

        vx_win->SetActive(true);
        vx_win->SetCursorRect(
            new content::Rect(base::Rect(110, 110, 100, 100)));

        vx_win->SetArrowsVisible(true);

        viewp->SetTone(new content::Tone(68, 68, 0, 0));

        screen->SetBrightness(125);

        scoped_refptr<content::Bitmap> snapshot = screen->SnapToBitmap();
        auto* surf = snapshot->SurfaceRequired();
        IMG_SavePNG(surf, "D:\\Desktop\\snap.png");

        screen->Transition(
            120, new content::Bitmap(
                     screen, "D:\\Desktop\\rgu\\app\\test\\BattleStart.png"));

        scoped_refptr<content::Bitmap> snapshot2 =
            new content::Bitmap(screen, 800, 600);
        viewp->SnapToBitmap(snapshot2);
        auto* surf2 = snapshot2->SurfaceRequired();
        IMG_SavePNG(surf2, "D:\\Desktop\\snap2.png");

        float xxx = 0;
        while (!binding->quit_required()) {
          tilemap->Update();

          sp->Update();
          sp->GetTransform().SetRotation(++xxx);

          screen->Update();

          if (!vx_win->IsDisposed())
            vx_win->Update();
        }
      });

  params.binding_params.initial_resolution = base::Vec2i(800, 600);
  params.renderer_params.target_window = win;

  cc.InitCC(params);
  cc.ContentMain();

  SDL_DestroyWindow(win);

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  PHYSFS_deinit();

  return 0;
}
