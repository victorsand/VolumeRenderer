#include "OctreeTexture.h"
#include <gl/glew.h>
#include <iostream>
#include <fstream>
#include <vector>

OctreeTexture * OctreeTexture::New() {
  return new OctreeTexture();
}

void OctreeTexture::ReadFromFile(std::string _fileName, int _bits, int _dim) {
  int bytes = _bits/8;
  int nrVoxelsBaseLevel = _dim*_dim*_dim;
  int nrLevels = log(_dim) + 1;
  int nrVoxels = 2*nrVoxelsBaseLevel + 1;
  std::cout << "Creating octree texture\n"
    << "Bytes per value in raw file: " << bytes << "\n" 
    << "Nr of voxels in base level: " << nrVoxelsBaseLevel << "\n"
    << "Nr of levels in octree: " << nrLevels << "\n"
    << "Nr of voxels in whole tree: " << nrVoxels << "\n";

  // Allocate a vector to hold the whole octree data before creating texture
  std::vector<float> hostData;
  hostData.resize(nrVoxels);

  // Read data from file. This will later be the base level data.
  std::ifstream inFileStream;
  char *buffer;
  inFileStream.open(_fileName.c_str(), std::ios::in|std::ios::binary);
  if (inFileStream.is_open()) {
    inFileStream.seekg(0, std::ios::end);
    std::cout << "File size: " << inFileStream.tellg() << std::endl;
    inFileStream.seekg(0, std::ios::beg);
    buffer = new char[bytes*nrVoxelsBaseLevel];
    inFileStream.read(buffer, bytes*nrVoxelsBaseLevel);
    inFileStream.close();

    // Copy the base level data
    for (int i=0; i<nrVoxelsBaseLevel; i++) {
      float value=static_cast<float>(static_cast<int>(buffer[bytes*i]))/256.f;
      hostData.at(i) = value;
    }
    delete buffer;


    // Level to be written to, starting at second to lowest
    int level = nrLevels - 1;
    // Dimensions for level to be read to
    int levelDim = _dim/2;
    // Offset into index array for level to be written to
    int offsetWrite = levelDim*levelDim*levelDim;;
    // Offset into index array for level to be read from
    int offsetRead = 0;
    while (level>0) {
      for (int x=0; x<levelDim; x++) {
        for (int y=0; y<levelDim; y++) {
          for (int z=0; z<levelDim; z++) {
            // 8 sample points in 
            int idx[8];
            idx[0] = x     + y    *levelDim + z*    levelDim*levelDim;
            idx[1] = (x+1) + y    *levelDim + z*    levelDim*levelDim;
            idx[2] = (x+1) + (y+1)*levelDim + z*    levelDim*levelDim;
            idx[3] = (x+1) + (y+1)*levelDim + (z+1)*levelDim*levelDim;
            idx[4] = x     + (y+1)*levelDim + z    *levelDim*levelDim;
            idx[5] = x     + (y+1)*levelDim + (z+1)*levelDim*levelDim;
            idx[6] = x     + y    *levelDim + (z+1)*levelDim*levelDim;
            idx[7] = (x+1) + y    *levelDim + (z+1)*levelDim*levelDim;
            // Sample level below and average
            float sum = 0.f;
            for (int i=0; i<8; i++) {
              sum += hostData.at(offsetRead+idx[i]);
            }
            // Write to current level and step offset forward
            hostData.at(offsetWrite) = sum/8.f;
            offsetWrite++;
          }
        }     
        // Add current level's size to next read level
        offsetRead += levelDim*levelDim*levelDim;
        // Calculate dimensions for next level to read from
        levelDim /= 2;
        // Start again
        level--;
      }
    }


    // Construct 3D texture
    // TODO For testing, only create one level at a time
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_3D, handle_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexImage3D(GL_TEXTURE_3D,                              // target
                 0,                                          // level
                 GL_R32F,                                    // internal format
                 _dim,                                       // width
                 _dim,                                       // height
                 _dim,                                       // depth  
                 0,                                          // border  
                 GL_RED,                                     // format
                 GL_FLOAT,                                   // type  
                 static_cast<GLvoid*>(&hostData[0]));     // data
    glBindTexture(GL_TEXTURE_3D, 0);
    
    std::cout << "Finished creating volume texture\n\n";

  } else {
    std::cout << _fileName << " could not be opened." << std::endl;
  }


}