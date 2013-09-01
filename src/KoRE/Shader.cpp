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

#include <fstream>
#include <sstream>
#include <vector>

#include "KoRE/Common.h"
#include "KoRE/Shader.h"
#include "KoRE/ResourceManager.h"



kore::Shader::Shader(void) : _handle(KORE_GLUINT_HANDLE_INVALID),
                             _code(""),
                             _shadertype(KORE_GLUINT_HANDLE_INVALID),
                             BaseResource() {
}

kore::Shader::~Shader(void) {
}


bool Shader::readTextFileLines(const std::string& szFileName,
                       std::vector<std::string>& rvLines) {
  std::ifstream fileStream;
  fileStream.open(szFileName.c_str());
  
  if(fileStream.good()) {
    while(!fileStream.eof()) {
      std::string newLine;
      std::getline(fileStream, newLine);
      rvLines.push_back( newLine );
    }
    fileStream.close();
    return true;
  }

  fileStream.close();
  return false;
}

void kore::Shader::loadShaderCode(const std::string& file, GLenum shadertype) {
  std::string shaderCode = "";
  std::vector<std::string> vLines;
  if(!readTextFileLines(file, vLines)) {
    Log::getInstance()->write("[ERROR] Could not read shader file %s\n", file.c_str());
  }

  std::vector<std::string>::iterator iterIncludeLine = vLines.end();
  int iFoundPos = -1;
  int iFirstQuotationPos = -1;
  int iSecondQutationPos = -1;
  bool bFound = false;

  do
  {
    iterIncludeLine = vLines.end();
    uint uIdx = 0;
    for( std::vector<std::string>::iterator iter = vLines.begin(); iter != vLines.end(); ++iter)
    {
      iFoundPos = iter->find( "#include" );
      if( iFoundPos != std::string::npos ) {  //#include found!
        iFirstQuotationPos = iter->find_first_of( "\"" );
        iSecondQutationPos = iter->find_last_of( "\"" );
        iterIncludeLine = iter;
        break;
      }
      uIdx++;
    }

    if( iterIncludeLine != vLines.end() && iFoundPos > -1 && iFirstQuotationPos > -1 && iSecondQutationPos > -1 && ( iSecondQutationPos > iFirstQuotationPos ) )
    {
      bFound = true;
    }

    else
    {
      bFound = false;
    }

    if( bFound )
    {
      std::string szSubStr = iterIncludeLine->substr( iFirstQuotationPos + 1, iSecondQutationPos - ( iFirstQuotationPos + 1 ) );
      //clear the "#include..." line
      vLines[ uIdx ] = "";

      //szSubStr = "shader/" + szSubStr;

      std::vector<std::string> vInsertLines;
      if (!readTextFileLines(szSubStr, vInsertLines)) {
        Log::getInstance()->write("[ERROR] Could not read #include-shader file %s\n", szSubStr.c_str());
      }

      if(vInsertLines.size() > 0) {
        vLines.insert(iterIncludeLine, vInsertLines.begin(), vInsertLines.end());
      }
    }

    
    shaderCode.clear();
    for(uint uIdx = 0; uIdx < vLines.size(); ++uIdx) {
      shaderCode += vLines[uIdx] + "\n";
    }

  } while(bFound);


  _handle = glCreateShader(shadertype);
  _shadertype = shadertype;
  const char* szShaderSource = shaderCode.c_str();
  glShaderSource(_handle, 1, &szShaderSource, 0);
  glCompileShader(_handle);

  bool bSuccess = checkShaderCompileStatus(_handle, file);
  if (!bSuccess) {
    glDeleteShader(_handle);
  }
  _name = file.substr(file.find_last_of("/")+1);
  kore::ResourceManager::getInstance()->addShader(this);
}

bool kore::Shader::checkShaderCompileStatus(const GLuint shaderHandle,
                                                   const std::string& name) {
  GLint success;
  GLint infologLen;
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
  glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infologLen);
  
  if (infologLen > 1) {
    GLchar * infoLog = new GLchar[infologLen];
    if (infoLog == NULL) {
      kore::Log::getInstance()->write(
        "[ERROR] Could not allocate ShaderInfoLog buffer from '%s'\n",
        name.c_str());
    }
    int charsWritten = 0;
    glGetShaderInfoLog(shaderHandle, infologLen, &charsWritten, infoLog);
    std::string shaderlog = infoLog;
    kore::Log::getInstance()->write(
      "[DEBUG] '%s' shader Log %s\n", name.c_str(), shaderlog.c_str());
    KORE_SAFE_DELETE_ARR(infoLog);
  } else {
    /*kore::Log::getInstance()->write(
      "[DEBUG] Shader '%s' compiled\n", name.c_str());*/
  }
  return success == GL_TRUE;
}
