#include "Manager.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "VolumeTexture.h"

int main(int _argc, char * _argv) {
  unsigned int width = 600;
  unsigned int height = 600;

  // Initialize 
  Manager::Instance().SetWinDimensions(width, height);
  Manager::Instance().InitWindow(_argc, &_argv);
  Manager::Instance().InitCubePositionBuffer();
  Manager::Instance().InitCallbacks();
  Manager::Instance().InitMatrices();

  // Create shader programs
  ShaderProgram *cubeShaderProg = ShaderProgram::New();
  cubeShaderProg->CreateShader(ShaderProgram::VERTEX, "cubeVert.glsl");
  cubeShaderProg->CreateShader(ShaderProgram::FRAGMENT, "cubeFrag.glsl");
  cubeShaderProg->CreateProgram();
  ShaderProgram *volumeShaderProg = ShaderProgram::New();
  volumeShaderProg->CreateShader(ShaderProgram::VERTEX, "octreeVert.glsl");
  volumeShaderProg->CreateShader(ShaderProgram::FRAGMENT, "octreeFrag.glsl");
  volumeShaderProg->CreateProgram();

  // Bind shader programs to manager
  Manager::Instance().SetCubeShaderProgram(cubeShaderProg);
  Manager::Instance().SetVolumeShaderProgram(volumeShaderProg);

  // Create textures to render to
  Texture2D *cubeFrontTex = Texture2D::New(width, height);
  cubeFrontTex->Init();
  Texture2D *cubeBackTex = Texture2D::New(width, height);
  cubeBackTex->Init();

  // Create 3D texture and populate it
  VolumeTexture *volTex = VolumeTexture::New();
  volTex->ReadFromFile("skull.raw", 8, 256);

  // Bind the textures to the manager
  // (The manager takes care of the FBO binding in the rendering loop)
  Manager::Instance().SetCubeFrontTexture(cubeFrontTex);
  Manager::Instance().SetCubeBackTexture(cubeBackTex);
  Manager::Instance().SetVolumeTexture(volTex);

  // Read constants from file
  Manager::Instance().SetConfigFileName("constants.txt");
  Manager::Instance().ReadConfigFile();

  // Now we have everything to fire up the buffers
  Manager::Instance().InitFramebuffer();

  // Let's go!
  Manager::Instance().StartLoop();
}

