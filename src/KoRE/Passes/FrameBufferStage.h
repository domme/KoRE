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

#ifndef KORE_FRAMEBUFFERSTAGE_H_
#define KORE_FRAMEBUFFERSTAGE_H_

#include <vector>
#include "KoRE/FrameBuffer.h"
#include "KoRE/Passes/ShaderProgramPass.h"
#include "KoRE/Operations/Operation.h"

namespace kore {
  class FrameBufferStage {
  public:
    FrameBufferStage(void);
    virtual ~FrameBufferStage(void);

    inline std::vector<ShaderProgramPass*>&
      getShaderProgramPasses() {return _programPasses;}

    inline std::vector<Operation*>&
      getStartupOperations() {return _startupOperations;}
    inline std::vector<Operation*>&
      getFinishOperations() {return _finishOperations;}
    inline std::vector<Operation*>&
      getInternalStartupOperations() {return _internalStartup;}
    inline std::vector<Operation*>&
      getInternalFinishOperations() {return _internalFinish;}
    inline FrameBuffer* getFrameBuffer() {return _frameBuffer;}
    inline const std::vector<GLenum>& getActiveAttachments() {return _activeBuffers;}

    void setActiveAttachments(const std::vector<GLenum>& activeBuffers);

    void setFrameBuffer(kore::FrameBuffer* frameBuffer);

    void addProgramPass(ShaderProgramPass* progPass);

    void removeProgramPass(ShaderProgramPass* progPass);

    void swapPasses(ShaderProgramPass* which,
                    ShaderProgramPass* towhere);

    void addStartupOperation(Operation* op);
    void removeStartupOperation(Operation* op);

    void addFinishOperation(Operation* op);
    void removeFinishOperation(Operation* op);

    inline const EOperationExecutionType getExecutionType() const {return _executionType;}
    inline void setExecutionType(EOperationExecutionType exType) {_executionType = exType;}
    inline void setExecuted(bool executed) {_executed = executed;}
    inline const bool getExecuted() const {return _executed;}

  protected:
    FrameBuffer* _frameBuffer;
    std::vector<Operation*> _startupOperations;
    std::vector<Operation*> _finishOperations;
    std::vector<Operation*> _internalStartup;
    std::vector<Operation*> _internalFinish;
    std::vector<ShaderProgramPass*> _programPasses;
    uint64 _id;
    std::vector<GLenum> _activeBuffers;

    EOperationExecutionType _executionType;
    bool _executed;
  };
}
#endif  // KORE_FRAMEBUFFERSTAGE_H_
