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

#ifndef KORE_SHADERPROGRAMPASS_H_
#define KORE_SHADERPROGRAMPASS_H_

#include <vector>
#include "KoRE/ShaderProgram.h"
#include "KoRE/Passes/NodePass.h"
#include "Kore/FrameBuffer.h"

namespace kore {
  class ShaderProgramPass {
  public:
    explicit ShaderProgramPass(ShaderProgram* prog);
    ShaderProgramPass(void);
    virtual ~ShaderProgramPass(void);

    inline std::vector<NodePass*>&
      getNodePasses() {return _nodePasses;}

    inline std::vector<Operation*>&
      getStartupOperations() {return _startupOperations;}
    inline std::vector<Operation*>&
      getFinishOperations() {return _finishOperations;}
    inline std::vector<Operation*>&
      getInternalStartupOperations() {return _internalStartup;}
    inline std::vector<Operation*>&
      getInternalFinishOperations() {return _internalFinish;}

    inline ShaderProgram* getShaderProgram() {return _program;}
    
    void setShaderProgram(ShaderProgram* program);

    void addNodePass(NodePass* pass);
    void removeNodePass(NodePass* pass);
    void swapNodePass(NodePass* which, NodePass* towhere);

    void addStartupOperation(Operation* op);
    void removeStartupOperation(Operation* op);

    void addFinishOperation(Operation* op);
    void removeFinishOperation(Operation* op);

    inline const EOperationExecutionType getExecutionType() const {return _executionType;}
    inline void setExecutionType(EOperationExecutionType exType) {_executionType = exType;}
    inline void setExecuted(bool executed) {_executed = executed;}
    inline const bool getExecuted() const {return _executed;}

    inline const std::string& getName() const {return _name;}
    
  protected:
    uint64 _id;
    ShaderProgram* _program;

    std::vector<Operation*> _startupOperations;
    std::vector<Operation*> _finishOperations;
    std::vector<Operation*> _internalStartup;
    std::vector<Operation*> _internalFinish;
    std::vector<NodePass*> _nodePasses;

    EOperationExecutionType _executionType;
    bool _executed;

    std::string _name;
    
    GLuint _timerQuery;
    void startQuery();
    void endQuery();
    bool _useGPUProfiling;

  };

}
#endif  // KORE_SHADERPROGRAMPASS_H_
