#include "Manager.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "VolumeTexture.h"
#include <gl\glew.h>
#include <gl\glut.h>
#include <iostream>

// Static definitions
unsigned int Manager::cubePositionBufferObject_;
unsigned int Manager::renderbufferObject_;
unsigned int Manager::cubeFrontFBO_;
unsigned int Manager::cubeBackFBO_;
unsigned int Manager::cubePositionAttrib_;
glm::mat4 Manager::model_;
glm::mat4 Manager::view_;
glm::mat4 Manager::proj_;
float Manager::pitch_ = -30.f;
float Manager::yaw_ = 0.f;
float Manager::roll_ = 30.f;
bool Manager::mouseDown_ = false;
int Manager::lastMouseX_ = 0;
int Manager::lastMouseY_ = 0;
ShaderProgram *Manager::cubeShaderProg_;
ShaderProgram *Manager::volumeShaderProg_;
Texture2D *Manager::cubeFrontTex_;
Texture2D *Manager::cubeBackTex_;
VolumeTexture *Manager::volumeTex_;

Manager& Manager::Instance() {
  static Manager instance;
  return instance;
}

void Manager::SetWinDimensions(unsigned int _width, unsigned int _height) {
  width_ = _width;
  height_ = _height;
}

void Manager::InitWindow(int &_argc, char **_argv) {
  std::cout << "Initializing GLEW and GLUT\n";
  glutInit(&_argc, _argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(width_, height_);
  glutCreateWindow("Volume renderer - Victor Sand");
  std::cout << "Window initialized\n\n";

  char* version = (char*)glGetString(GL_VERSION);
  if (version) {
    std::cout << "OpenGL version: " << version << "\n";
  } else {
    std::cout << "Failed to get OpenGL version\n";
  }

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cout << "GLEW error: " << glewGetErrorString(err) << std::endl;
  } else {
    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  }

  glClearColor(.3f, .3f, .3f, 1.f);
  CheckGLErrors();
}

void Manager::InitMatrices() {
  // Set perspective, flip Z axis and set up camera
  proj_ = glm::perspective(40.f, 
                          (float)width_/(float)height_,
                          0.1f,
                          100.f);
  view_ = glm::rotate(glm::mat4(1.f), 180.f, glm::vec3(1.f, 0.f, 0.f));
  view_ = glm::translate(view_, glm::vec3(-0.5f, -0.5f, 2.2f));
}

void Manager::InitCallbacks() {
  glutDisplayFunc(RenderScene);
  glutReshapeFunc(ChangeSize);
  glutKeyboardFunc(Keyboard);
  glutMouseFunc(MouseButtons);
  glutMotionFunc(MouseMotion);
  CheckGLErrors();
}

void Manager::InitCubePositionBuffer() {
  float v[] = {
    // front
    1.f, 0.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 1.f,
    0.f, 0.f, 0.f, 1.f,
    1.f, 0.f, 0.f, 1.f,
    1.f, 1.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 1.f,
    // right
    1.f, 0.f, 0.f, 1.f,
    1.f, 0.f, 1.f, 1.f,
    1.f, 1.f, 0.f, 1.f,
    1.f, 0.f, 1.f, 1.f,
    1.f, 1.f, 1.f, 1.f,
    1.f, 1.f, 0.f, 1.f,
    // back
    1.f, 1.f, 1.f, 1.f,
    0.f, 0.f, 1.f, 1.f,
    0.f, 1.f, 1.f, 1.f,
    1.f, 1.f, 1.f, 1.f,
    1.f, 0.f, 1.f, 1.f,
    0.f, 0.f, 1.f, 1.f,
    // left
    0.f, 0.f, 1.f, 1.f,
    0.f, 0.f, 0.f, 1.f,
    0.f, 1.f, 1.f, 1.f,
    0.f, 0.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 1.f,
    0.f, 1.f, 1.f, 1.f,
    // top
    0.f, 1.f, 0.f, 1.f,
    1.f, 1.f, 0.f, 1.f,
    0.f, 1.f, 1.f, 1.f,
    0.f, 1.f, 1.f, 1.f,
    1.f, 1.f, 0.f, 1.f,
    1.f, 1.f, 1.f, 1.f,
    // bottom
    0.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 1.f,
    1.f, 0.f, 1.f, 1.f,
    0.f, 0.f, 0.f, 1.f,
    1.f, 0.f, 1.f, 1.f,
    1.f, 0.f, 0.f, 1.f
  };
  glGenBuffers(1, &cubePositionBufferObject_);
  glBindBuffer(GL_ARRAY_BUFFER, cubePositionBufferObject_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*144, v, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  CheckGLErrors();
}

void Manager::InitFramebuffer() {
  // Renderbuffer for depth component
  glGenRenderbuffers(1, &renderbufferObject_);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject_);
  glRenderbufferStorage(GL_RENDERBUFFER,
                        GL_DEPTH_COMPONENT,
                        width_, 
                        height_);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Front and back cube textures to different FBO's
  glGenFramebuffers(1, &cubeFrontFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, cubeFrontFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, 
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, 
                         cubeFrontTex_->Handle(),
                         0);  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
                            GL_DEPTH_ATTACHMENT, 
                            GL_RENDERBUFFER,
                            renderbufferObject_);
  GLenum status;
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Error: Framebuffer not complete" << std::endl;
    exit(1);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &cubeBackFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, cubeBackFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, 
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, 
                         cubeBackTex_->Handle(),
                         0);  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
                            GL_DEPTH_ATTACHMENT, 
                            GL_RENDERBUFFER,
                            renderbufferObject_);

  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Error: Framebuffer not complete" << std::endl;
    exit(1);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  CheckGLErrors();
}

void Manager::UpdateMatrices() {
  model_ = glm::mat4(1.f);
  model_ = glm::translate(model_, glm::vec3(0.5f, 0.5f, 0.5f));
  model_ = glm::rotate(model_, roll_, glm::vec3(1.f, 0.f, 0.0));
  model_ = glm::rotate(model_, -pitch_, glm::vec3(0.f, 1.f, 0.0));
  model_ = glm::rotate(model_, yaw_, glm::vec3(0.f, 0.f, 1.f));
  model_ = glm::translate(model_, glm::vec3(-0.5f, -0.5f, -0.5f));
  CheckGLErrors();
}

void Manager::BindTransformationMatrices(ShaderProgram * _program) {
  _program->BindMatrix4fv("modelMatrix", &model_[0][0]);
  _program->BindMatrix4fv("viewMatrix", &view_[0][0]);
  _program->BindMatrix4fv("projMatrix", &proj_[0][0]);
}

void Manager::RenderScene() {
  UpdateMatrices();
  BindTransformationMatrices(cubeShaderProg_);
  BindTransformationMatrices(volumeShaderProg_);

  glUseProgram(cubeShaderProg_->Handle());

  
  
  // Render cube front
  glBindFramebuffer(GL_FRAMEBUFFER, cubeFrontFBO_);
  CullBackFace();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  cubePositionAttrib_ = cubeShaderProg_->GetAttribLocation("position");
  glBindBuffer(GL_ARRAY_BUFFER, cubePositionBufferObject_);
  glEnableVertexAttribArray(cubePositionAttrib_);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  GLenum frontBuffer[1] = { GL_COLOR_ATTACHMENT0 };
  //glDrawBuffers(1, frontBuffer);
  glDrawArrays(GL_TRIANGLES, 0, 144);
  glDisableVertexAttribArray(cubePositionAttrib_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  // Render cube back
  glBindFramebuffer(GL_FRAMEBUFFER, cubeBackFBO_);
  CullFrontFace();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  cubePositionAttrib_ = cubeShaderProg_->GetAttribLocation("position");
  glBindBuffer(GL_ARRAY_BUFFER, cubePositionBufferObject_);
  glEnableVertexAttribArray(cubePositionAttrib_);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  GLenum backBuffer[1] = { GL_COLOR_ATTACHMENT1 };
  //glDrawBuffers(1, backBuffer);
  glDrawArrays(GL_TRIANGLES, 0, 144);
  glDisableVertexAttribArray(cubePositionAttrib_);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(0);

  // We have now rendered to the textures and can bind them
  volumeShaderProg_->BindTexture2D("cubeFrontTex", 
                                    GL_TEXTURE0,
                                    0,
                                    cubeFrontTex_);
  volumeShaderProg_->BindTexture2D("cubeBackTex",
                                    GL_TEXTURE1,
                                    1,
                                    cubeBackTex_);
  volumeShaderProg_->BindVolumeTexture("volumeTex",
                                       GL_TEXTURE2,
                                       2,
                                       volumeTex_);

  glUseProgram(volumeShaderProg_->Handle());
  
  // Render to screen
  CullBackFace();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  cubePositionAttrib_ = volumeShaderProg_->GetAttribLocation("position");
  glBindBuffer(GL_ARRAY_BUFFER, cubePositionBufferObject_);
  glEnableVertexAttribArray(cubePositionAttrib_);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLES, 0, 144);
  glDisableVertexAttribArray(cubePositionAttrib_);
 
  glUseProgram(0);
  glutSwapBuffers();
  glutPostRedisplay();
  CheckGLErrors();
}

void Manager::SetCubeShaderProgram(ShaderProgram *_program) {
  cubeShaderProg_ = _program;
}

void Manager::SetVolumeShaderProgram(ShaderProgram *_program) {
  volumeShaderProg_ = _program;
}

void Manager::SetCubeFrontTexture(Texture2D *_texture) {
  cubeFrontTex_ = _texture;
}

void Manager::SetCubeBackTexture(Texture2D *_texture) {
  cubeBackTex_ = _texture;
}

void Manager::SetVolumeTexture(VolumeTexture *_texture) {
  volumeTex_ = _texture;
}

unsigned int Manager::CheckGLErrors() {
  unsigned int error = glGetError();
  switch (error) {
  case GL_NO_ERROR:
    break;
  case GL_INVALID_ENUM:
    std::cout << "GL_INVALID_ENUM\n";
    break;
  case GL_INVALID_VALUE:
      std::cout << "GL_INVALID_VALUE\n";
      break;
  case GL_INVALID_OPERATION:
    std::cout << "GL_INVALID_OPERATION\n";
      break;
  case GL_STACK_OVERFLOW:
    std::cout << "GL_STACK_OVERFLOW\n";
    break;
  case GL_STACK_UNDERFLOW:
    std::cout << "GL_STACK_UNDERFLOW\n";
    break;
  case GL_OUT_OF_MEMORY:
    std::cout << "GL_OUT_OF_MEMORY\n";
    break;
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION\n";
    break;
  case GL_TABLE_TOO_LARGE:
    std::cout << "GL_TABLE_TOO_LARGE\n";
    break;
  }
  return error;
}

void Manager::ChangeSize(int _width, int _height) {
  glViewport(0, 0, (GLsizei)_width, (GLsizei)_height);
}

void Manager::Keyboard(unsigned char _key, int _x, int _y) {
}

void Manager::MouseButtons(int _button, int _state, int _x, int _y) {
  if(_button == GLUT_LEFT_BUTTON) {
    if (_state == GLUT_DOWN) {
      mouseDown_ = true;
      lastMouseX_ = _x;
      lastMouseY_ = _y;
    } else if (_state == GLUT_UP) {
      mouseDown_ = false;
    }
  }
}

void Manager::MouseMotion(int _x, int _y) {
   if (mouseDown_) {
     pitch_ += 0.3f*(float)(_x - lastMouseX_);
     lastMouseX_ = _x;
     roll_ += 0.3f*(float)(_y - lastMouseY_);
     lastMouseY_ = _y;
   }
}

void Manager::StartLoop() {
  std::cout << "Starting main loop...\n";
  glutMainLoop();
}

void Manager::CullBackFace() {
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
}

void Manager::CullFrontFace() {
  glFrontFace(GL_CW);
  glCullFace(GL_FRONT);
  glEnable(GL_CULL_FACE);
}