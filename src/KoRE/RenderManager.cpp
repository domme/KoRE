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

#include <vector>
#include <algorithm>

#include "KoRE/RenderManager.h"
#include "KoRE/Log.h"
#include "KoRE/GLerror.h"
#include "KoRE/Optimization/SimpleOptimizer.h"

kore::RenderManager* kore::RenderManager::getInstance(void) {
  static kore::RenderManager theInstance;
  return &theInstance;
}

kore::RenderManager::RenderManager(void)
  : _optimizer(NULL),
    _colorMask(true, true, true, true),
    _ibo(0),
    _vbo(0),
    _vao(0),
    _viewport(0,0,0,0),
    _activeTextureUnitIndex(0),
    _screenRes(0,0),
    _shaderProgram(KORE_GLUINT_HANDLE_INVALID){

  //sync internal states with opengl-states:
  
  _vBufferTargetMap[GL_ARRAY_BUFFER]              = BufferTargets::ARRAY_BUFFER,
  _vBufferTargetMap[GL_ATOMIC_COUNTER_BUFFER]     = BufferTargets::ATOMIC_COUNTER_BUFFER;
  _vBufferTargetMap[GL_COPY_READ_BUFFER]          = BufferTargets::COPY_READ_BUFFER;
  _vBufferTargetMap[GL_COPY_WRITE_BUFFER]         = BufferTargets::COPY_WRITE_BUFFER;
  _vBufferTargetMap[GL_DRAW_INDIRECT_BUFFER]      = BufferTargets::DRAW_INDIRECT_BUFFER;
  _vBufferTargetMap[GL_DISPATCH_INDIRECT_BUFFER]  = BufferTargets::DISPATCH_INDIRECT_BUFFER;
  _vBufferTargetMap[GL_ELEMENT_ARRAY_BUFFER]      = BufferTargets::ELEMENT_ARRAY_BUFFER;
  _vBufferTargetMap[GL_PIXEL_PACK_BUFFER]         = BufferTargets::PIXEL_PACK_BUFFER;
  _vBufferTargetMap[GL_PIXEL_UNPACK_BUFFER]       = BufferTargets::PIXEL_UNPACK_BUFFER;
  _vBufferTargetMap[GL_SHADER_STORAGE_BUFFER]     = BufferTargets::SHADER_STORAGE_BUFFER;
  _vBufferTargetMap[GL_TEXTURE_BUFFER]            = BufferTargets::TEXTURE_BUFFER;
  _vBufferTargetMap[GL_TRANSFORM_FEEDBACK_BUFFER] = BufferTargets::TRANSFORM_FEEDBACK_BUFFER;
  _vBufferTargetMap[GL_UNIFORM_BUFFER]            = BufferTargets::UNIFORM_BUFFER;

  _vTexTargetMap[GL_TEXTURE_1D] =                   TextureTargets::TEXTURE_1D;
  _vTexTargetMap[GL_TEXTURE_2D] =                   TextureTargets::TEXTURE_2D;
  _vTexTargetMap[GL_TEXTURE_3D] =                   TextureTargets::TEXTURE_3D;
  _vTexTargetMap[GL_TEXTURE_1D_ARRAY] =             TextureTargets::TEXTURE_1D_ARRAY;
  _vTexTargetMap[GL_TEXTURE_2D_ARRAY] =             TextureTargets::TEXTURE_2D_ARRAY;
  _vTexTargetMap[GL_TEXTURE_RECTANGLE] =            TextureTargets::TEXTURE_RECTANGLE;
  _vTexTargetMap[GL_TEXTURE_CUBE_MAP] =             TextureTargets::TEXTURE_CUBE_MAP;
  _vTexTargetMap[GL_TEXTURE_CUBE_MAP_ARRAY] =       TextureTargets::TEXTURE_CUBE_MAP_ARRAY;
  _vTexTargetMap[GL_TEXTURE_BUFFER] =               TextureTargets::TEXTURE_BUFFER;
  _vTexTargetMap[GL_TEXTURE_2D_MULTISAMPLE] =       TextureTargets::TEXTURE_2D_MULTISAMPLE;
  _vTexTargetMap[GL_TEXTURE_2D_MULTISAMPLE_ARRAY] =
                                                TextureTargets::TEXTURE_2D_MULTISAMPLE_ARRAY;

  if (_vTexTargetMap.size() != TextureTargets::NUM_TEXTURE_TARGETS) {
    Log::getInstance()->write("[ERROR] Not all texture targets where"
                              " added into the textureTargetMap");
  }

  if (_vBufferTargetMap.size() != BufferTargets::NUM_BUFFER_TARGETS) {
    Log::getInstance()->write("[ERROR] Not all buffer targets where"
                              " added into the bufferTargetMap");
  }

  memset(_boundTextures, 0, sizeof(GLuint) *
                            GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS *
                            TextureTargets::NUM_TEXTURE_TARGETS);

  memset(_boundBuffers, 0, sizeof(GLuint) * BufferTargets::NUM_BUFFER_TARGETS);

  memset(_boundSamplers, 0, sizeof(GLuint) * 
                            GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

  memset(_boundFrameBuffers, 0, sizeof(GLuint) * 2);

  memset(_drawBuffers, 0, sizeof(bool) * 
                          KORE_MAX_FRAMEBUFFER_COUNT *
                          GL_MAX_DRAW_BUFFERS);

  memset(_boundAtomicBuffers, 0, sizeof(GLuint) *
                                 GL_MAX_COMBINED_ATOMIC_COUNTERS);

  activeTexture(0);  // Activate texture unit 0 by default

  _shdScreenRes.data = &_screenRes;
  _shdScreenRes.name = "screenRes";
  _shdScreenRes.type = GL_INT_VEC2;
  _shdScreenRes.size = 1;
}

kore::RenderManager::~RenderManager(void) {
  KORE_SAFE_DELETE(_optimizer);

  for (uint i = 0; i < _frameBufferStages.size(); ++i) {
    KORE_SAFE_DELETE(_frameBufferStages[i]);
  }
}

glm::ivec2 kore::RenderManager::getRenderResolution() const {
    return glm::ivec2(_viewport.z,_viewport.w);
}

const glm::ivec4& kore::RenderManager::getViewport() const {
    return _viewport;
}

void kore::RenderManager::setViewport(const glm::ivec4& newViewport) {
   if(newViewport == _viewport) {
     return; 
   } else {
     _viewport = newViewport;
     glViewport(_viewport.x,_viewport.y,_viewport.z,_viewport.w);
     resolutionChanged();
   }
}

void kore::RenderManager::renderFrame(void) {
  if (_optimizer == NULL) {
    setOptimizer(new SimpleOptimizer);
  }

  // For now, just optimize every frame... later do this only on changes
  // in operations.
  _optimizer->optimize(_frameBufferStages, _operations);

    for (auto it = _operations.begin(); it != _operations.end(); ++it) {
        (*it)->execute();
    }
}

void kore::RenderManager::resolutionChanged() {
    // Update all resolution-dependant resources here
    // (e.g. GBuffer-Textures...)
}


void kore::RenderManager::setOptimizer(const Optimizer* optimizer) {
  if (_optimizer != NULL) {
    KORE_SAFE_DELETE(_optimizer);
  }
  _optimizer = optimizer;
}


void kore::RenderManager::onRemoveComponent(const SceneNodeComponent* comp) {
  for (auto iter = _operations.begin(); iter != _operations.end(); ++iter) {
    if ((*iter)->dependsOn(static_cast<const void*>(comp))) {
      _operations.erase(iter);
    }
  }
}

// OpenGL-Wrappers:
void kore::RenderManager::bindVBO(const GLuint vbo) {
  if (_vbo != vbo) {
    _vbo = vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
  }
}

void kore::RenderManager::bindVAO(const GLuint vao) {
  if (_vao != vao) {
      _vao = vao;
      glBindVertexArray(vao);
  }
}

void kore::RenderManager::bindIBO( const GLuint ibo ) {
  if (_ibo != ibo) {
    _ibo = ibo;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  }
}

void kore::RenderManager::useShaderProgram(const GLuint shaderProgram) {
  if (_shaderProgram != shaderProgram) {
    _shaderProgram = shaderProgram;
    glUseProgram(shaderProgram);
  }
}

void kore::RenderManager::bindTexture(const GLuint textureUnit,
                                      const GLuint textureTarget,
                                      const GLuint textureHandle) {
  uint uTexTargetIndex = _vTexTargetMap[textureTarget];
  if (_boundTextures[textureUnit][uTexTargetIndex] != textureHandle) {
    activeTexture(textureUnit);
    glBindTexture(textureTarget, textureHandle);
    _boundTextures[textureUnit][uTexTargetIndex] = textureHandle;
  }
}

void kore::RenderManager::bindTexture(const GLuint textureTarget,
                                      const GLuint textureHandle) {
  bindTexture(_activeTextureUnitIndex, textureTarget, textureHandle);
}

void kore::RenderManager::bindSampler(const GLuint textureUnit,
                                      const GLuint samplerHandle) {
  //if (_boundSamplers[textureUnit] != samplerHandle) {
  //  activeTexture(textureUnit);
  //  glBindSampler(textureUnit, samplerHandle);
  //  _boundSamplers[textureUnit] = samplerHandle;
  //}
  activeTexture(textureUnit);
  glBindSampler(textureUnit, samplerHandle);
}

void kore::RenderManager::activeTexture(const GLuint activeTextureUnitIndex) {
  if(_activeTextureUnitIndex != activeTextureUnitIndex) {
    _activeTextureUnitIndex = activeTextureUnitIndex;
    glActiveTexture(GL_TEXTURE0 + activeTextureUnitIndex);
  }
}

void kore::RenderManager::bindFrameBuffer(const GLuint fboTarget,
                                          const GLuint fboHandle) {
  if (fboTarget == GL_FRAMEBUFFER) {
    if (_boundFrameBuffers[READ_FRAMEBUFFER] != fboHandle ||
        _boundFrameBuffers[DRAW_FRAMEBUFFER] != fboHandle) {
      _boundFrameBuffers[READ_FRAMEBUFFER] = fboHandle;
      _boundFrameBuffers[DRAW_FRAMEBUFFER] = fboHandle;
      glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
      _shaderProgram = KORE_GLUINT_HANDLE_INVALID;
    } else if (fboTarget == GL_READ_FRAMEBUFFER) {
      if (_boundFrameBuffers[READ_FRAMEBUFFER] != fboHandle) {
        _boundFrameBuffers[READ_FRAMEBUFFER] = fboHandle;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboHandle);
        _shaderProgram = KORE_GLUINT_HANDLE_INVALID;
      } 
    } else if (fboHandle == GL_DRAW_FRAMEBUFFER) {
      if (_boundFrameBuffers[DRAW_FRAMEBUFFER]) {
        _boundFrameBuffers[DRAW_FRAMEBUFFER] = fboHandle;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboHandle);
        _shaderProgram = KORE_GLUINT_HANDLE_INVALID;
      }
    }
  }
}

void kore::RenderManager::addFramebufferStage(FrameBufferStage* stage) {
  _frameBufferStages.push_back(stage);
}

void kore::RenderManager::swapFramebufferStage(FrameBufferStage* which,
                                               FrameBufferStage* towhere) {
  auto it = std::find(_frameBufferStages.begin(),
                      _frameBufferStages.end(), which);
  auto it2 = std::find(_frameBufferStages.begin(),
                       _frameBufferStages.end(), towhere);
  if(it != _frameBufferStages.end() && it2 != _frameBufferStages.end()) {
    std::iter_swap(it,it2);
  }
}

/*
void kore::RenderManager::removeOperation(const Operation* operation) {
  for (uint ifbo = 0; ifbo < _frameBufferStages.size(); ++ifbo) {
    std::vector<ShaderProgramPass*>& progPasses =
      _frameBufferStages[ifbo]->getShaderProgramPasses();
    for (uint iProg = 0; iProg < progPasses.size(); ++iProg) {
      std::vector<NodePass*>& nodePasses =
        progPasses[iProg]->getNodePasses();
      for (uint iNode = 0; iNode < nodePasses.size(); ++iNode) {
        std::vector<Operation*>& operations =
          nodePasses[iNode]->getOperations();
        auto it = std::find(operations.begin(), operations.end(), operation);
        if (it != operations.end()) {
          Operation* pOp = (*it);
          //KORE_SAFE_DELETE(pOp);
          operations.erase(it);
        }
      }  // Node Passes
    }  // program passes
  }  // fbo Passes
}

void kore::RenderManager::
  removeShaderProgramPass(const ShaderProgramPass* progPass) {
    for (uint ifbo = 0; ifbo < _frameBufferStages.size(); ++ifbo) {
      std::vector<ShaderProgramPass*>& progPasses =
        _frameBufferStages[ifbo]->getShaderProgramPasses();

      auto it = std::find(progPasses.begin(), progPasses.end(), progPass);
      if (it != progPasses.end()) {
        ShaderProgramPass* pProgPass = (*it);
        //KORE_SAFE_DELETE(pProgPass);
        progPasses.erase(it);
      }
    }
}

void kore::RenderManager::removeNodePass(const NodePass* nodePass) {
  for (uint ifbo = 0; ifbo < _frameBufferStages.size(); ++ifbo) {
    std::vector<ShaderProgramPass*>& progPasses =
      _frameBufferStages[ifbo]->getShaderProgramPasses();
    for (uint iProg = 0; iProg < progPasses.size(); ++iProg) {
      std::vector<NodePass*>& nodePasses =
        progPasses[iProg]->getNodePasses();

      auto it = std::find(nodePasses.begin(), nodePasses.end(), nodePass);
      if (it != nodePasses.end()) {
        NodePass* pNodePass = (*it);
        //KORE_SAFE_DELETE(pNodePass);
        nodePasses.erase(it);
      }
    }
  }
}
*/

void kore::RenderManager::
  removeFrameBufferStage(const FrameBufferStage* fboStage) {
   auto it =
    std::find(_frameBufferStages.begin(), _frameBufferStages.end(), fboStage);
      if (it != _frameBufferStages.end()) {
        _frameBufferStages.erase(it);
      }
}

void kore::RenderManager::bindBuffer(const GLenum bufferTarget,
                                     const GLuint bufferHandle) {
  auto buf = _vBufferTargetMap.find(bufferTarget);
  
  if (buf == _vBufferTargetMap.end()) {
    Log::getInstance()->write("[ERROR] RenderManager::bindbuffer(): "
                              "Buffer-target is invalid");
    return;
  }

  if (_boundBuffers[buf->second] != bufferHandle) {
    _boundBuffers[buf->second] = bufferHandle;
    glBindBuffer(bufferTarget, bufferHandle);
  }
}

void kore::RenderManager::bindBufferBase(const GLenum indexedBufferTarget,
                                         const uint bindingPoint,
                                         const GLuint bufferHandle) {
  switch (indexedBufferTarget) {
    case GL_ATOMIC_COUNTER_BUFFER: 
      if (bindingPoint < GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS) {
        if (_boundAtomicBuffers[bindingPoint] != bufferHandle) {
          _boundAtomicBuffers[bindingPoint] = bufferHandle;
          glBindBufferBase(indexedBufferTarget, bindingPoint, bufferHandle);
        }
      }
    break;

    // TODO(dlazarek): Implement for GL_UNIFORM_BUFFER,
    //                               GL_TRANSFORM_FEEDBACK_BUFFER, etc...

    default:
      Log::getInstance()->write("[ERROR] RenderManager::bindBufferBase - "
        "The requested indexedBufferTarget is not implemented or is invalid");
    break;
  }
}

void kore::RenderManager::setColorMask(bool red,
                                       bool green,
                                       bool blue,
                                       bool alpha) {
  if (_colorMask.r == red
      && _colorMask.g == green
      && _colorMask.b == blue
      && _colorMask.a == alpha) {
    return;
  }

  _colorMask = glm::bvec4(red, green, blue, alpha);
  glColorMask(red, green, blue, alpha);
}

void kore::RenderManager::setGLcapability(GLuint cap, bool enable) {
  if (glIsEnabled(cap) == static_cast<GLboolean>(enable)) {
   // return;
  }

  if (enable) {
    glEnable(cap);
  } else {
    glDisable(cap);
  }

}
