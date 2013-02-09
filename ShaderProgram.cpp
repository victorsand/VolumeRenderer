#define _CRT_SECURE_NO_DEPRECATE // Get rid of fopen warning
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "VolumeTexture.h"
#include <iostream>
#include <algorithm>

ShaderProgram * ShaderProgram::New() {
  return new ShaderProgram();
}

void ShaderProgram::CreateShader(ShaderType _type, std::string _fileName) {
  GLuint type;
  switch (_type) {
  case ShaderProgram::VERTEX:
    type = GL_VERTEX_SHADER;
    break;
  case ShaderProgram::FRAGMENT:
    type = GL_FRAGMENT_SHADER;
    break;
  default:
    std::cout << "Error: Shader type invalid\n";
    exit(1);
  }

  std::cout << "Creating shader. Filename: " << _fileName << "\n";
  unsigned int shaderHandle = glCreateShader(type);
  if (glIsShader(shaderHandle) == GL_FALSE) {
    std::cout << "Error: Failed to create shader\n" <<
      "OpenGL error code: " << glGetError() << "\n";
  }
  char *source = ReadTextFile(_fileName);
  // Convert source to const because the OpgenGL call requires it
  const char *constSource = source; 
  //std::cout << "\nShader source to read from:\n" << constSource << "\n\n";
  glShaderSource(shaderHandle, 1, &constSource, NULL);

  // Compile and verify result
  glCompileShader(shaderHandle);
  int shaderCompiled;
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &shaderCompiled);
  if (shaderCompiled == GL_FALSE) {
    std::cout << "Error: Shader compilation failed\n" <<
      "OpenGL error code: " << glGetError() << "\n";
    PrintLog(shaderHandle);
   exit(1);
  }
  shaderHandles_.push_back(shaderHandle);
  free(source);
}

void ShaderProgram::CreateProgram() {
  std::cout << "Creating shader program\n";
  programHandle_ = glCreateProgram();
  // Attach shaders
  std::vector<unsigned int>::iterator it;
  for (it=shaderHandles_.begin(); it!=shaderHandles_.end(); it++) {
    glAttachShader(programHandle_, *it);
  }

  // Link and verify result
  glLinkProgram(programHandle_);
  int programLinked;
  glGetProgramiv(programHandle_, GL_LINK_STATUS, &programLinked);
  if (programLinked == GL_FALSE) {
    std::cout << "Error: Program linking failed\n" <<
      "OpenGL error code: " << glGetError() << "\n";
    PrintLog(programHandle_);
    exit(1);
  }
  
  // Since we have linked the program, we can get rid of the shaders
  std::for_each(shaderHandles_.begin(), shaderHandles_.end(), glDeleteShader);
  std::cout << "Finished creating shader program\n\n";
}

void ShaderProgram::BindMatrix4fv(std::string _uniform, float *_matrix) {
  glUseProgram(programHandle_);
  int location = glGetUniformLocation(programHandle_, _uniform.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, _matrix);
  glUseProgram(0);
}

void ShaderProgram::BindTexture2D(std::string _uniform,
                                  GLenum _texUnit,
                                  unsigned int _unitNumber,
                                  Texture2D * _tex) {
  glUseProgram(programHandle_);
  glActiveTexture(_texUnit);
  glEnable(GL_TEXTURE_2D);
  int location = glGetUniformLocation(programHandle_, _uniform.c_str());
  glUniform1i(location, _unitNumber);
  glBindTexture(GL_TEXTURE_2D, _tex->Handle());
  glUseProgram(0);
}

void ShaderProgram::BindVolumeTexture(std::string _uniform,
                                      GLenum _texUnit,
                                      unsigned int _unitNumber,
                                      VolumeTexture *_tex) {
  glUseProgram(programHandle_);
  glActiveTexture(_texUnit);
  glEnable(GL_TEXTURE_3D);
  int location = glGetUniformLocation(programHandle_, _uniform.c_str());
  glUniform1i(location, _unitNumber);
  glBindTexture(GL_TEXTURE_3D, _tex->Handle());
  glUseProgram(0);                                    
}

unsigned int ShaderProgram::GetAttribLocation(std::string _attrib) {
  return glGetAttribLocation(programHandle_, _attrib.c_str());
}

char * ShaderProgram::ReadTextFile(std::string _fileName) {
  FILE * inFile;
  char * content = NULL;
  inFile = fopen(_fileName.c_str(), "r");
  if (inFile != NULL) {
    fseek(inFile, 0, SEEK_END);
    int count = ftell(inFile);
    rewind(inFile);
    content = (char*)malloc(sizeof(char)*(count+1));
    count = fread(content, sizeof(char), count, inFile);
    content[count] = '\0';
    fclose(inFile);
  } else {
    std::cout << "Error: Problems when reading " << _fileName << std::endl;
    exit(1);
  }
  return content;
}

void ShaderProgram::PrintLog(unsigned int _object) {
  int logLength, maxLength;
  // We need to handle shaders and programs differently
  if (glIsShader(_object)) {
    glGetShaderiv(_object, GL_INFO_LOG_LENGTH, &maxLength);
  } else {
    glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &maxLength);
  }
  std::vector<char> log(maxLength);
  if (glIsShader(_object)) {
    glGetShaderInfoLog(_object, maxLength, &logLength, (GLchar*)&log[0]);
  } else {
    glGetProgramInfoLog(_object, maxLength, &logLength, (GLchar*)&log[0]);
  }
  if (logLength > 0) {
    std::cout << "\nLog:\n" << &log[0] << "\n\n";
  } 
}