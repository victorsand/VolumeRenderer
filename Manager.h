#ifndef MANAGER_H
#define MANAGER_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class ShaderProgram;
class Texture2D;
class VolumeTexture;

class Manager {
public:
  static Manager& Instance();
  void SetWinDimensions(unsigned int _width, unsigned int _height);
  // Initializes glew and the GLUT window
  void InitWindow(int &_argc, char **_argv);
  // Initializes cube position buffers
  void InitCubePositionBuffer();
  // Initializes renderbuffer and framebuffer objects
  void StartLoop();
  void InitFramebuffer();
  void InitMatrices();
  void InitCallbacks();
  void SetCubeShaderProgram(ShaderProgram *_program);
  void SetVolumeShaderProgram(ShaderProgram *_program);
  void SetCubeFrontTexture(Texture2D *_texture);
  void SetCubeBackTexture(Texture2D *_texture);
  void SetVolumeTexture(VolumeTexture *_texture);

private:
  // Checks for OpenGL errors and prints them if present
  static unsigned int CheckGLErrors();
  // Update matrices with current view params
  static void UpdateMatrices();
  // For clarity, functions that sets the culling mode
  static void CullFrontFace();
  static void CullBackFace();
  // Helper to bind transformation matrices
  static void BindTransformationMatrices(ShaderProgram * _program);

  // Callback functions for rendering loop
  static void RenderScene();
  static void ChangeSize(int _width, int _height);
  static void Keyboard(unsigned char _key, int _x, int _y);
  static void MouseButtons(int _button, int _state, int _x, int _y);
  static void MouseMotion(int _x, int _y);

  // Transformation matrices
  static glm::mat4 model_;
  static glm::mat4 view_;
  static glm::mat4 proj_;

  // Variables keeping track of view parameters
  static float pitch_;
  static float yaw_;
  static float roll_;
  static bool mouseDown_;
  static int lastMouseX_;
  static int lastMouseY_;

  // Window size
  unsigned int width_;
  unsigned int height_;

  // Fixed buffer objects
  static unsigned int cubePositionAttrib_;
  static unsigned int cubeBackFBO_;
  static unsigned int cubeFrontFBO_;
  static unsigned int renderbufferObject_;
  static unsigned int cubePositionBufferObject_;
  // Fixed shaders and textures
  static ShaderProgram *cubeShaderProg_;
  static ShaderProgram *volumeShaderProg_;
  static Texture2D *cubeFrontTex_;
  static Texture2D *cubeBackTex_;
  static VolumeTexture *volumeTex_;
};

#endif