#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <vector>
#include <string>

#include <gl\glew.h>

class Texture2D;
class VolumeTexture;

class ShaderProgram {
public:
  enum ShaderType {
    VERTEX,
    FRAGMENT
  };
  static ShaderProgram * New();
  // Creates a vertex or fragment shader from file
  void CreateShader(ShaderType _type, std::string _fileName);
  // Attaches the created shaders and links the program
  void CreateProgram();
  // Returns the handle to the linked program
  unsigned int Handle() { return programHandle_; }
  // Binds a 4x4 float matrix to the shader program
  void BindMatrix4fv(std::string _uniform, float *_matrix);
  // Binds a 2D texture to the shader program
  void BindTexture2D(std::string _uniform,
                     GLenum _texUnit,
                     unsigned int _unitNumber,
                     Texture2D *_tex);
  // Binds a volume texture to the shader program
  void BindVolumeTexture(std::string _uniform,
                         GLenum _texUnit,
                         unsigned int _unitNumber,
                         VolumeTexture *_tex);
  // Binds a float uniform to the shader program
  void BindFloat(std::string _uniform, float _value);
  // Get location for named attribute
  unsigned int GetAttribLocation(std::string _attrib);

private:
  ShaderProgram() {}
  ShaderProgram(const ShaderProgram&) {}
  // Prints log for a shader or a program
  void PrintLog(unsigned int _object);
  // Reads shader source to a char * for use when creating shaders
  char * ReadTextFile(std::string _fileName);

  std::vector<unsigned int> shaderHandles_;
  unsigned int programHandle_;
};

#endif