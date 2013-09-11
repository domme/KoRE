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

#ifndef CORE_INCLUDE_CORE_RENDERMANAGER_H_
#define CORE_INCLUDE_CORE_RENDERMANAGER_H_

#include <list>
#include "KoRE/Common.h"
#include "KoRE/Operations/Operation.h"
#include "KoRE/Components/MeshComponent.h"
#include "KoRE/Components/Camera.h"
#include "KoRE/ShaderProgram.h"
#include "KoRE/Passes/FrameBufferStage.h"
#include "KoRE/Optimization/Optimizer.h"
#include "KoRE/GPUtimer.h"

namespace kore {
  enum EOpInsertPos {
    INSERT_BEFORE,
    INSERT_AFTER
  };

  // Usage of this enum (dlazarek):
  // Add new texture-targets whenever the OpenGL API updates so that 
  // NUM_TEXTURE_TARGETS always holds the value of the number of available
  // texture targets on the compiled system. 
  // Use preprocessor #ifdefs to conditionally include targets on machines
  // with more advanced OpenGL-versions in the future to maintain backwards-
  // compatibility.
  namespace TextureTargets {
    enum ETextureTargets {
      TEXTURE_1D = 0,
      TEXTURE_2D,
      TEXTURE_3D,
      TEXTURE_1D_ARRAY,
      TEXTURE_2D_ARRAY,
      TEXTURE_RECTANGLE,
      TEXTURE_CUBE_MAP,
      TEXTURE_CUBE_MAP_ARRAY,
      TEXTURE_BUFFER,
      TEXTURE_2D_MULTISAMPLE,
      TEXTURE_2D_MULTISAMPLE_ARRAY,

      NUM_TEXTURE_TARGETS
    };
  }

  namespace BufferTargets {
    enum EBufferTargets {
       ARRAY_BUFFER = 0,
       ATOMIC_COUNTER_BUFFER,
       COPY_READ_BUFFER,
       COPY_WRITE_BUFFER,
       DRAW_INDIRECT_BUFFER,
       DISPATCH_INDIRECT_BUFFER,
       ELEMENT_ARRAY_BUFFER,
       PIXEL_PACK_BUFFER,
       PIXEL_UNPACK_BUFFER,
       SHADER_STORAGE_BUFFER,
       TEXTURE_BUFFER,
       TRANSFORM_FEEDBACK_BUFFER,
       UNIFORM_BUFFER,

       NUM_BUFFER_TARGETS
    };
  }

  enum EFrameBufferTargets {
    DRAW_FRAMEBUFFER = 0,
    READ_FRAMEBUFFER = 1
  };


  //////////////////////////////////////////////////////////////////////////

  class RenderManager {
  public:
    static RenderManager *getInstance(void);
    virtual ~RenderManager(void);
    
    /*! \brief Returns the current screen-resolution.
     * This value is the current pixel-size of the output-window and is
     * independent of the current viewport used for rendering. */
    inline const glm::ivec2& getScreenResolution() const {return _screenRes;}

    /*! \brief Sets the screen-resolution.
    * This value is the current pixel-size of the output-window and is
    * independent of the current viewport used for rendering.
    * Note that this method has to be called manually from the UI.*/
    inline void setScreenResolution(const glm::ivec2& screenRes)
    {_screenRes = screenRes;}

    inline std::vector<FrameBufferStage*>&
      getFrameBufferStages() {return _frameBufferStages;}

    inline ShaderData* getShdScreenRes(){return &_shdScreenRes;}

    glm::ivec2 getRenderResolution() const;
    const glm::ivec4& getViewport() const;
    void setViewport(const glm::ivec4& newViewport);
    void setOptimizer(const Optimizer* optimizer);
    void renderFrame(void);
    
    void addFramebufferStage(FrameBufferStage* stage);
    void swapFramebufferStage(FrameBufferStage* which,
                              FrameBufferStage* towhere);
    void onRemoveComponent(const SceneNodeComponent* comp);

    // The OpenGL-State wrapper functions go here:
    void bindVAO(const GLuint vao);
    void bindVBO(const GLuint vbo);
    void bindIBO(const GLuint ibo);
    void useShaderProgram(const GLuint shaderProgram);

    void setGLcapability(GLuint cap, bool enable);
    void setColorMask(bool red, bool green, bool blue, bool alpha);

    void bindTexture(const GLuint textureUnit,
                     const GLuint textureTarget,
                     const GLuint textureHandle);

    void bindTexture(const GLuint textureTarget,
                     const GLuint textureHandle);

    void bindSampler(const GLuint textureUnit,
                     const GLuint samplerHandle);

    void bindFrameBuffer(const GLuint fboTarget,
                         const GLuint fboHandle);

    void bindBuffer(const GLenum bufferTarget,
                    const GLuint bufferHandle);

    void bindBufferBase(const GLenum indexedBufferTarget,
                        const uint bindingPoint,
                        const GLuint bufferHandle);

   /* void removeOperation(const Operation* operation);
    void removeShaderProgramPass(const ShaderProgramPass* progPass);
    void removeNodePass(const NodePass* nodePass);
    */
    void removeFrameBufferStage(const FrameBufferStage* fboStage);

    /**
      Sets the active texture unit(glActiveTexture).
      Note that the argument is an index to a texture unit (e.g. the "i" in
      glActiveTexture(GL_TEXTURE0 + i)
    */
    void activeTexture(const GLuint activeTextureUnitIndex);
    //////////////////////////////////////////////////////////////////////////

    inline void setUseGPUprofiling(const bool useProfiling) {
      _useGPUprofiling = useProfiling;
    }

    inline bool getUseGPUpfofiling() const {
      return _useGPUprofiling;
    }

  private:
    RenderManager(void);
    
    void resolutionChanged();

    glm::ivec2 _screenRes;
    glm::ivec4 _viewport;
    const Optimizer* _optimizer;

    typedef std::list<const Operation*> OperationList;
    OperationList _operations;
    std::vector<FrameBufferStage*> _frameBufferStages;

    // OpenGL-States:
    GLuint _activeTextureUnitIndex;
    GLuint _vao;
    GLuint _vbo;
    GLuint _ibo;
    GLuint _shaderProgram;
    GLuint _boundAtomicBuffers[GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS];
    GLuint _boundTextures[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS]
                         [TextureTargets::NUM_TEXTURE_TARGETS];
    GLuint _boundSamplers[GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
    GLuint _boundBuffers[BufferTargets::NUM_BUFFER_TARGETS];
    GLuint _boundFrameBuffers[2];
    bool   _drawBuffers[KORE_MAX_FRAMEBUFFER_COUNT]
                       [GL_MAX_DRAW_BUFFERS];

                       
    glm::bvec4 _colorMask;
    std::map<GLuint, uint> _vTexTargetMap;
    std::map<GLuint, uint> _vBufferTargetMap;
    kore::ShaderData _shdScreenRes;

    bool _useGPUprofiling;
    //////////////////////////////////////////////////////////////////////////
  };
};
#endif  // CORE_INCLUDE_CORE_RENDERMANAGER_H_
