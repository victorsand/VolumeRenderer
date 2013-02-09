#include "VolumeTexture.h"
#include <gl/glew.h>
#include <iostream>
#include <fstream>
#include <vector>

VolumeTexture * VolumeTexture::New() {
  return new VolumeTexture();
}

void VolumeTexture::ReadFromFile(std::string _fileName, int _bits, int _dim) {
  int bytes = _bits/8;
  int nrVoxels = _dim*_dim*_dim;
  std::ifstream inFileStream;

  std::cout << "Opening " << _fileName << std::endl;
  std::cout << "Bytes per voxel: " << bytes << std::endl;
  std::cout << "Dimensions: " << _dim<< "^3" << std::endl;
  std::cout << "Voxels: " << nrVoxels  << std::endl;

  // Read raw data
  char *buffer;
  inFileStream.open(_fileName.c_str(), std::ios::in);
  if (inFileStream.is_open()) {
    inFileStream.seekg(0, std::ios::end);
    std::cout << "File size: " << inFileStream.tellg() << std::endl;
    inFileStream.seekg(0, std::ios::beg);
    buffer = new char[bytes*nrVoxels];
    inFileStream.read(buffer, bytes*nrVoxels);
    inFileStream.close();

    std::vector<float> floatBuffer;
    for (int i=0; i<bytes*nrVoxels; i+=bytes) {
      floatBuffer.push_back(static_cast<float>(buffer[bytes*i]));
    }
    delete buffer;
    std::cout << "Amount of read values: " << floatBuffer.size() << "\n";
  
    // Construct 3D texture
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_3D, handle_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexImage3D(GL_TEXTURE_3D,                             // target
                0,                                          // level
                GL_R8,                                      // internal format
                _dim,                                       // width
                _dim,                                       // height
                _dim,                                       // depth  
                0,                                          // border  
                GL_RED,                                     // format
                GL_FLOAT,                                   // type  
                static_cast<GLvoid*>(&floatBuffer[0]));     // data
    glBindTexture(GL_TEXTURE_3D, 0);

    std::cout << "Finished creating volume texture\n\n";
  } else {
    std::cout << _fileName << " could not be opened." << std::endl;
  }
  
}