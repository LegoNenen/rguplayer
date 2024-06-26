// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef IMGUI_IMGUI_IMPL_OPENGL3_LOADER_H_
#define IMGUI_IMGUI_IMPL_OPENGL3_LOADER_H_

#include "renderer/context/gles2_context.h"

#define glActiveTexture renderer::GL.ActiveTexture
#define glAttachShader renderer::GL.AttachShader
#define glBindAttribLocation renderer::GL.BindAttribLocation
#define glBindBuffer renderer::GL.BindBuffer
#define glBindFramebuffer renderer::GL.BindFramebuffer
#define glBindRenderbuffer renderer::GL.BindRenderbuffer
#define glBindTexture renderer::GL.BindTexture
#define glBlendColor renderer::GL.BlendColor
#define glBlendEquation renderer::GL.BlendEquation
#define glBlendEquationSeparate renderer::GL.BlendEquationSeparate
#define glBlendFunc renderer::GL.BlendFunc
#define glBlendFuncSeparate renderer::GL.BlendFuncSeparate
#define glBufferData renderer::GL.BufferData
#define glBufferSubData renderer::GL.BufferSubData
#define glCheckFramebufferStatus renderer::GL.CheckFramebufferStatus
#define glClear renderer::GL.Clear
#define glClearColor renderer::GL.ClearColor
#define glClearDepthf renderer::GL.ClearDepthf
#define glClearStencil renderer::GL.ClearStencil
#define glColorMask renderer::GL.ColorMask
#define glCompileShader renderer::GL.CompileShader
#define glCompressedTexImage2D renderer::GL.CompressedTexImage2D
#define glCompressedTexSubImage2D renderer::GL.CompressedTexSubImage2D
#define glCopyTexImage2D renderer::GL.CopyTexImage2D
#define glCopyTexSubImage2D renderer::GL.CopyTexSubImage2D
#define glCreateProgram renderer::GL.CreateProgram
#define glCreateShader renderer::GL.CreateShader
#define glCullFace renderer::GL.CullFace
#define glDeleteBuffers renderer::GL.DeleteBuffers
#define glDeleteFramebuffers renderer::GL.DeleteFramebuffers
#define glDeleteProgram renderer::GL.DeleteProgram
#define glDeleteRenderbuffers renderer::GL.DeleteRenderbuffers
#define glDeleteShader renderer::GL.DeleteShader
#define glDeleteTextures renderer::GL.DeleteTextures
#define glDepthFunc renderer::GL.DepthFunc
#define glDepthMask renderer::GL.DepthMask
#define glDepthRangef renderer::GL.DepthRangef
#define glDetachShader renderer::GL.DetachShader
#define glDisable renderer::GL.Disable
#define glDisableVertexAttribArray renderer::GL.DisableVertexAttribArray
#define glDrawArrays renderer::GL.DrawArrays
#define glDrawElements renderer::GL.DrawElements
#define glEnable renderer::GL.Enable
#define glEnableVertexAttribArray renderer::GL.EnableVertexAttribArray
#define glFinish renderer::GL.Finish
#define glFlush renderer::GL.Flush
#define glFramebufferRenderbuffer renderer::GL.FramebufferRenderbuffer
#define glFramebufferTexture2D renderer::GL.FramebufferTexture2D
#define glFrontFace renderer::GL.FrontFace
#define glGenBuffers renderer::GL.GenBuffers
#define glGenerateMipmap renderer::GL.GenerateMipmap
#define glGenFramebuffers renderer::GL.GenFramebuffers
#define glGenRenderbuffers renderer::GL.GenRenderbuffers
#define glGenTextures renderer::GL.GenTextures
#define glGetActiveAttrib renderer::GL.GetActiveAttrib
#define glGetActiveUniform renderer::GL.GetActiveUniform
#define glGetAttachedShaders renderer::GL.GetAttachedShaders
#define glGetAttribLocation renderer::GL.GetAttribLocation
#define glGetBooleanv renderer::GL.GetBooleanv
#define glGetBufferParameteriv renderer::GL.GetBufferParameteriv
#define glGetError renderer::GL.GetError
#define glGetFloatv renderer::GL.GetFloatv
#define glGetFramebufferAttachmentParameteriv \
  renderer::GL.GetFramebufferAttachmentParameteriv
#define glGetIntegerv renderer::GL.GetIntegerv
#define glGetProgramiv renderer::GL.GetProgramiv
#define glGetProgramInfoLog renderer::GL.GetProgramInfoLog
#define glGetRenderbufferParameteriv renderer::GL.GetRenderbufferParameteriv
#define glGetShaderiv renderer::GL.GetShaderiv
#define glGetShaderInfoLog renderer::GL.GetShaderInfoLog
#define glGetShaderPrecisionFormat renderer::GL.GetShaderPrecisionFormat
#define glGetShaderSource renderer::GL.GetShaderSource
#define glGetString renderer::GL.GetString
#define glGetTexParameterfv renderer::GL.GetTexParameterfv
#define glGetTexParameteriv renderer::GL.GetTexParameteriv
#define glGetUniformfv renderer::GL.GetUniformfv
#define glGetUniformiv renderer::GL.GetUniformiv
#define glGetUniformLocation renderer::GL.GetUniformLocation
#define glGetVertexAttribfv renderer::GL.GetVertexAttribfv
#define glGetVertexAttribiv renderer::GL.GetVertexAttribiv
#define glGetVertexAttribPointerv renderer::GL.GetVertexAttribPointerv
#define glHint renderer::GL.Hint
#define glIsBuffer renderer::GL.IsBuffer
#define glIsEnabled renderer::GL.IsEnabled
#define glIsFramebuffer renderer::GL.IsFramebuffer
#define glIsProgram renderer::GL.IsProgram
#define glIsRenderbuffer renderer::GL.IsRenderbuffer
#define glIsShader renderer::GL.IsShader
#define glIsTexture renderer::GL.IsTexture
#define glLineWidth renderer::GL.LineWidth
#define glLinkProgram renderer::GL.LinkProgram
#define glPixelStorei renderer::GL.PixelStorei
#define glPolygonOffset renderer::GL.PolygonOffset
#define glReadPixels renderer::GL.ReadPixels
#define glReleaseShaderCompiler renderer::GL.ReleaseShaderCompiler
#define glRenderbufferStorage renderer::GL.RenderbufferStorage
#define glSampleCoverage renderer::GL.SampleCoverage
#define glScissor renderer::GL.Scissor
#define glShaderBinary renderer::GL.ShaderBinary
#define glShaderSource renderer::GL.ShaderSource
#define glStencilFunc renderer::GL.StencilFunc
#define glStencilFuncSeparate renderer::GL.StencilFuncSeparate
#define glStencilMask renderer::GL.StencilMask
#define glStencilMaskSeparate renderer::GL.StencilMaskSeparate
#define glStencilOp renderer::GL.StencilOp
#define glStencilOpSeparate renderer::GL.StencilOpSeparate
#define glTexImage2D renderer::GL.TexImage2D
#define glTexParameterf renderer::GL.TexParameterf
#define glTexParameterfv renderer::GL.TexParameterfv
#define glTexParameteri renderer::GL.TexParameteri
#define glTexParameteriv renderer::GL.TexParameteriv
#define glTexSubImage2D renderer::GL.TexSubImage2D
#define glUniform1f renderer::GL.Uniform1f
#define glUniform1fv renderer::GL.Uniform1fv
#define glUniform1i renderer::GL.Uniform1i
#define glUniform1iv renderer::GL.Uniform1iv
#define glUniform2f renderer::GL.Uniform2f
#define glUniform2fv renderer::GL.Uniform2fv
#define glUniform2i renderer::GL.Uniform2i
#define glUniform2iv renderer::GL.Uniform2iv
#define glUniform3f renderer::GL.Uniform3f
#define glUniform3fv renderer::GL.Uniform3fv
#define glUniform3i renderer::GL.Uniform3i
#define glUniform3iv renderer::GL.Uniform3iv
#define glUniform4f renderer::GL.Uniform4f
#define glUniform4fv renderer::GL.Uniform4fv
#define glUniform4i renderer::GL.Uniform4i
#define glUniform4iv renderer::GL.Uniform4iv
#define glUniformMatrix2fv renderer::GL.UniformMatrix2fv
#define glUniformMatrix3fv renderer::GL.UniformMatrix3fv
#define glUniformMatrix4fv renderer::GL.UniformMatrix4fv
#define glUseProgram renderer::GL.UseProgram
#define glValidateProgram renderer::GL.ValidateProgram
#define glVertexAttrib1f renderer::GL.VertexAttrib1f
#define glVertexAttrib1fv renderer::GL.VertexAttrib1fv
#define glVertexAttrib2f renderer::GL.VertexAttrib2f
#define glVertexAttrib2fv renderer::GL.VertexAttrib2fv
#define glVertexAttrib3f renderer::GL.VertexAttrib3f
#define glVertexAttrib3fv renderer::GL.VertexAttrib3fv
#define glVertexAttrib4f renderer::GL.VertexAttrib4f
#define glVertexAttrib4fv renderer::GL.VertexAttrib4fv
#define glVertexAttribPointer renderer::GL.VertexAttribPointer
#define glViewport renderer::GL.Viewport

#endif  //! IMGUI_IMGUI_IMPL_OPENGL3_LOADER_H_
