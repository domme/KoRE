/*
  Copyright (c) 2012 The KoRE Project

  This file is part of KoRE.

  KoRE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  KoRE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KoRE.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "KoRE/Operations/BindOperations/BindImageTexture.h"
#include "KoRE/GLerror.h"
#include "KoRE/RenderManager.h"

kore::BindImageTexture::BindImageTexture() : BindOperation() {
  init();
}

kore::BindImageTexture::
  BindImageTexture(const ShaderData* texData, const ShaderInput* shaderInput, GLuint access /* = GL_READ_WRITE */)
  : BindOperation() {
    init();
    connect(texData, shaderInput, access);
}

void kore::BindImageTexture::init() {
  _type = OP_BINDIMAGETEXTURE;
}

kore::BindImageTexture::~BindImageTexture() {
  if (!isValid()) {
    return;
  }

  // Try to invalidate the binding to prevent bugs when the program closes
  STextureInfo* pTexInfo = static_cast<STextureInfo*>(_componentUniform->data); 

  glBindImageTexture(_shaderUniform->imgUnit,
                      0,
                      0,
                      GL_TRUE,
                      0,
                      GL_READ_WRITE,
                      pTexInfo->internalFormat);
}

void kore::BindImageTexture::
  connect(const ShaderData* texData, const ShaderInput* shaderInput, GLuint access) {
    if (!texData || !shaderInput) {
      // Make invalid:
      _shaderUniform = NULL;
      _componentUniform = NULL;
      return;
    }

  _access = access;
    _componentUniform = texData;
    _shaderUniform = shaderInput;
}

void kore::BindImageTexture::doExecute(void) const {
  _renderManager->
    useShaderProgram(_shaderUniform->shader->getProgramLocation());

  GLerror::gl_ErrorCheckStart();
  glUniform1i(_shaderUniform->location, _shaderUniform->imgUnit);
  GLerror::gl_ErrorCheckFinish("BindImageTexture::execute");
  STextureInfo* pTexInfo = static_cast<STextureInfo*>(_componentUniform->data); 

 // Log::getInstance()->write("ImageUnit = %u\n", _shaderUniform->imgUnit);

  glBindImageTexture(_shaderUniform->imgUnit,
                     pTexInfo->texLocation,
                     0,
                     GL_TRUE,
                     0,
                     _access,
                     pTexInfo->internalFormat);
}

void kore::BindImageTexture::update(void) {

}

void kore::BindImageTexture::reset(void) {

}

GLuint kore::BindImageTexture::
  internalFormatToImageFormat(const GLuint internalFormat) const {
    // Note(dlazarek) For some reason, image-uniforms only take 1- 2- or
    // 4-component texture images (but not 3-component texture images).
    // So for now, just convert every internal format of base format GL_RGB
    // to the corresponding sized internal format with base format GL_RGBA.
    switch (internalFormat) {
        case GL_RGB:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
          return GL_RGBA8;

        case GL_RGB8_SNORM:
          return GL_RGBA8_SNORM;

        case GL_RGB10:
        case GL_RGB12:
          return GL_RGBA12;

        case GL_RGB16:
        case GL_R11F_G11F_B10F:
          return GL_RGBA16;
        
        case GL_RGB16_SNORM:
          return GL_RGBA16_SNORM;

        case GL_RGB32F:
          return GL_RGBA32F;
        
        case GL_RGB8I:
          return GL_RGBA8I;

        case GL_RGB8UI:
          return GL_RGBA8UI;

        case GL_RGB16I:
          return GL_RGBA16I;

        case GL_RGB16UI:
          return GL_RGBA16UI;

        case GL_RGB32I:
          return GL_RGBA32I;

        case GL_RGB32UI:
          return GL_RGBA32UI;

        default:
          return internalFormat;
    }
}
