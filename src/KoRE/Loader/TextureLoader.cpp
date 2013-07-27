/*
  Copyright © 2012 The KoRE Project

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

#include "KoRE/Loader/TextureLoader.h"
#include "KoRE/ResourceManager.h"
#include "KoRE/IDManager.h"
#include "KoRE/Loader/lodepng.h"
#include "KoRE/Log.h"
#include "KoRE/Texture.h"

kore::TextureLoader::TextureLoader(void) {
}

kore::TextureLoader::~TextureLoader(void) {
}

kore::TextureLoader* kore::TextureLoader::getInstance() {
  static TextureLoader instance;
  return &instance;
}

kore::Texture*
  kore::TextureLoader::loadTexture(const std::string& filepath) {
  
  std::vector<unsigned char> imageData;
  std::vector<unsigned char> buffer;

  std::string loadFilePath = filepath;

  lodepng::load_file(buffer, loadFilePath);

  if (buffer.size() == 0) {
    // Texture does not exist in the provided path -> try once again in the textures-folder
    loadFilePath = "assets/textures/" + filepath;
    lodepng::load_file(buffer, loadFilePath);
  }

  uint width, height;
  lodepng::State pngState;
  pngState.decoder.color_convert = false;

  uint err = lodepng::decode(imageData, width, height, pngState, buffer);

  if ( err != 0) {
    kore::Log::getInstance()->write("[ERROR] Failed to load texture '%s' :\n"
                                    "\t%s\n",
                                    loadFilePath.c_str(),
                                    lodepng_error_text(err));
    return NULL;
  } else {
    kore::Texture* tex = new Texture();
    LodePNGColorMode& color = pngState.info_raw;
    
    STextureProperties texProperties;
    texProperties.targetType = GL_TEXTURE_2D;
    texProperties.border = 0;
    texProperties.width = width;
    texProperties.height = height;

    //Pass the actual Texture Data
    if(color.colortype == LCT_RGB) {
      texProperties.internalFormat = GL_RGB8;
      texProperties.format = GL_RGB;
      texProperties.pixelType = GL_UNSIGNED_BYTE; 
    } else if(color.colortype == LCT_RGBA) {
      texProperties.internalFormat = GL_RGBA8;
      texProperties.format = GL_RGBA;
      texProperties.pixelType = GL_UNSIGNED_BYTE;
    }

    std::string name = filepath.substr(filepath.find_last_of('/')+1);

    if (tex->init(texProperties, name, TEXSEMANTICS_UNKNOWN, &imageData[0])) {
      ResourceManager::getInstance()->addTexture(tex);
      std::string url = IDManager::getInstance()->genURL("", filepath, 0);
      IDManager::getInstance()->registerURL(tex->getID(), url);
      kore::Log::getInstance()
        ->write("[DEBUG] Texture '%s' successfully loaded.\n",
        filepath.c_str());
      tex->genMipmapHierarchy();
      return tex;
    } else {
      kore::Log::getInstance()
        ->write("[ERROR] Texture '%s' could not be loaded.\n",
        filepath.c_str());
      KORE_SAFE_DELETE(tex);
      return NULL;
    }
  }
}
