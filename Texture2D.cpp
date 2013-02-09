#include "Texture2D.h"
#include <iostream>
#include <gl/glew.h>

Texture2D * Texture2D::New(unsigned int _width, unsigned int _height) {
  return new Texture2D(_width, _height);
}

Texture2D::Texture2D(unsigned int _width, unsigned int _height) 
  : width_(_width), height_(_height), initialized_(false) {}

void Texture2D::Init() {
  if (initialized_) {
    std::cout << "Warning: Texture2D already initialized\n";
    return;
  }

  glGenTextures(1, &handle_);
  glBindTexture(GL_TEXTURE_2D, handle_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,     // target
               0,                 // level
               GL_RGB8,           // internal format
               width_,            // width
               height_,           // height
               0,                 // border
               GL_RGBA,           // format
               GL_UNSIGNED_BYTE,  // type
               0);                // data
  glBindTexture(GL_TEXTURE_2D, 0);
  initialized_ = true;
}