#include "VolumeTexture.h"
#include <gl/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Manager.h"

#define idx(x, y, z) (x + y*8 + z*8*8)
#define uint32_t unsigned __int32
#define uint16_t unsigned __int16

// http://graphics.stanford.edu/~seander/bithacks.html
uint32_t calcZOrder(uint16_t xPos, uint16_t yPos, uint16_t zPos)
{
    static const uint32_t MASKS[] =
    {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
    static const uint32_t SHIFTS[] = {1, 2, 4, 8};
    uint32_t x = xPos; 
    uint32_t y = yPos; 
    uint32_t z = zPos;
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x <<  8)) & 0x0300F00F;
    x = (x | (x <<  4)) & 0x030C30C3;
    x = (x | (x <<  2)) & 0x09249249;
    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y <<  8)) & 0x0300F00F;
    y = (y | (y <<  4)) & 0x030C30C3;
    y = (y | (y <<  2)) & 0x09249249;
    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z <<  8)) & 0x0300F00F;
    z = (z | (z <<  4)) & 0x030C30C3;
    z = (z | (z <<  2)) & 0x09249249;
    const uint32_t result = x | (y << 1) | (z << 2);
    return result;
}

VolumeTexture * VolumeTexture::New() {
  return new VolumeTexture();
}

void VolumeTexture::ReadFromFile(std::string _fileName, int _bits, int _dim) {

  _dim = 8;
  
  std::cout << "Checking errors..." << std::endl;
  Manager::Instance().CheckGLErrors();

  int bytes = _bits/8;
  int nrVoxelsBaseLevel = _dim*_dim*_dim;
  int nrLevels = log(_dim)/log(2) + 1;
  int nrVoxels = ((pow(8, nrLevels) - 1) / 7);

  int maxSize;
  glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxSize);
  std::cout << "GL_MAX_TEXTURE_BUFFER_SIZE: " << maxSize << "\n";
	if (nrVoxels*2 > maxSize) {
    std::cout << "Data is too big for texture buffer\n";
    exit(1);
  }

  // Tree levels start with 0 in the shader, so max depth is one less
  maxDepth_ = nrLevels - 1;

  std::cout << "Creating octree texture\n"
    << "Base level dimensions: " << _dim << "\n" 
    << "Nr of voxels in base level: " << nrVoxelsBaseLevel << "\n"
    << "Nr of levels in octree: " << nrLevels << "\n"
    << "Nr of voxels in whole tree: " << nrVoxels << "\n";

  // Allocate a vector to hold the whole octree data before creating texture
  std::vector<double> hostData;
  hostData.resize(nrVoxels * 2);
  std::cout << "hostData size: " << hostData.size() << "\n";

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


   
    // The data is sorted geometrically, but we want it sorted to that the 
    // octree nodes are laid out together. Therefore, we reshuffle the data
    // and put it in its place in the octree structure.


    // Copy the base level data, starting from the position where the last
    // level begins.
    int currentLevel = maxDepth_;
    int levelAbove = maxDepth_-1;
    int currentLevelStart = ((int)(pow(8, (levelAbove)+1) - 1) / 7) * 2;
    std::cout << "First iteration currentLevelStart: " << currentLevelStart << "\n";

    std::vector<double> controlData;
    controlData.resize(nrVoxelsBaseLevel);
    int index = 0;
    for (int z=0; z<_dim; z++) {
      for (int y=0; y<_dim; y++) {
        for (int x=0; x<_dim; x++) {
          controlData[index++] = (0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0)));
        }
      }
    }

    // Make an array with Morton (Z-order curve) numbers
    std::vector<uint32_t> morton;
    morton.resize(nrVoxelsBaseLevel);
    uint16_t x, y, z;
    for (z=0; z<(uint32_t)_dim; z++) {
      for (y=0; y<(uint32_t)_dim; y++) {
        for (x=0; x<(uint32_t)_dim; x++) {
          morton.at(idx(x, y, z)) = calcZOrder(x, y, z);
          if (morton.at(idx(x, y, z)) > (unsigned int)nrVoxelsBaseLevel) {
            std::cout << "FETFEL";
            exit(1);
          }
        }
      }
    }
    
    // Use the morton array to sort the host data
    for (int i=0; i<nrVoxelsBaseLevel; i++) {
      hostData[currentLevelStart+morton[i]*2] = controlData[i];
      hostData[currentLevelStart+morton[i]*2+1] = -1.f;
    }

    /*
    
    
    int x = 0;
    int y = 0;
    int z = 0;
    int value;
    int index = currentLevelStart;
    while (index < nrVoxels*2)
    {
      for (int j=0; j<8; j++)
      {
       for (int i=0; i<8; i++)
       {
          
          // (0, 0, 0)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;
  
          x++; 
          // (1, 0, 0)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;
  
          x--;
          y++;
          // (0, 1, 0)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;
  
          x++;
          // (1, 1, 0)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;
  
          x--;
          y--;
          z++;
          // (0, 0, 1)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;

          x++;
          // (1, 0, 1)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;

          x--;
          y++;
          // (0, 1, 1)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;

          x++;
          // (1, 1, 1)
          value = (int)(buffer[bytes*idx(x,y,z)]);
          hostData.at(index++) = 0.57*sqrt(pow((double)x/(double)(_dim-1), 2.0) + pow((double)y/(double)(_dim-1), 2.0) + pow((double)z/(double)(_dim-1), 2.0));//(double)value/(double)_dim;
          hostData.at(index++) = -1.f;

          // (0, 0, 0)
          x--; 
          y--;
          z--;

          // (2, 0, 0)
          x+=2;
          
          if (x > 3) { x -= 4; y += 2; }
          if (y > 3) { y -= 3; z += 2; }
        }
       
       x -= 4;
        
      }
      x=0;
      y++;
      z=0;
    }

    std::cout << "Last position written to by first iteration: " << index-1 << std::endl;

    
   */

    /*

    // Where to put the data
    int toFill=currentLevelStart; 
    // Read positions in original array
    int front = 0;
    int back = _dim;
    int frontZ = _dim*_dim;
    int backZ = _dim*_dim + _dim;
    int value;
    do
    {
      for (int i=0; i<_dim/2; i++)
      {
        for (int j=0; j<_dim/2; j++)
        {
          //value = static_cast<int>(buffer[bytes*front++]);
          hostData.at(toFill++) = controlData.at(front++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
          //value = static_cast<int>(buffer[bytes*front++]);
          hostData.at(toFill++) = controlData.at(front++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
          //value = static_cast<int>(buffer[bytes*back++]);
          hostData.at(toFill++) = controlData.at(back++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
         // value = static_cast<int>(buffer[bytes*back++]);
          hostData.at(toFill++) = controlData.at(back++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;

         // value = static_cast<int>(buffer[bytes*frontZ++]);
          hostData.at(toFill++) = controlData.at(frontZ++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
         // value = static_cast<int>(buffer[bytes*frontZ++]);
          hostData.at(toFill++) = controlData.at(frontZ++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
          //value = static_cast<int>(buffer[bytes*backZ++]);
          hostData.at(toFill++) = controlData.at(backZ++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
          //value = static_cast<int>(buffer[bytes*backZ++]);
          hostData.at(toFill++) = controlData.at(backZ++);//static_cast<float>(value)/256.f;
          hostData.at(toFill++) = -1.f;
        }
        front += _dim;
        back += _dim;
        frontZ += _dim;
        backZ += _dim;
      }
      front += _dim*_dim;
      back += _dim*_dim;
      frontZ += _dim*_dim;
      backZ += _dim*_dim;
    } 
    while (toFill < nrVoxels*2);
    
    */

   // std::cout << "Last position written to: " << toFill-1 << "\n";
    delete buffer;

    // Construct the higher levels in the tree by averaging the children
    
    int currentLevelDim = _dim;
    do
    {
      currentLevel--;
      std::cout << "\ncurrentLevel: " << currentLevel << std::endl;
      levelAbove--;
      std::cout << "levelAbove: " << levelAbove << std::endl;
      currentLevelDim /= 2;
      std::cout << "currentLevelDim: " << currentLevelDim << std::endl;
      currentLevelStart = ((pow(8, (levelAbove)+1) - 1) / 7) * 2;
      std::cout << "currentLevelStart: " << currentLevelStart << "\n";
      int toFill = currentLevelStart;
      int firstChild = ((pow(8, (currentLevel)+1) - 1) / 7) * 2;
      std::cout << "first child: " << firstChild << "\n";
      int child = firstChild;
      for (int i=0; i<currentLevelDim*currentLevelDim*currentLevelDim; i++)
      {
        double data = 0.0;
        for (int j=0; j<8; j++)
        {
          data += hostData.at(child);
          child += 2;
        }
        hostData.at(toFill++) = data/8.0;
        hostData.at(toFill++) = static_cast<double>(child-16);
      }
      std::cout << "Last child written: " << child-16 << std::endl;
      std::cout << "Last pos written to: " << toFill - 1 << "\n";
    }
    while (currentLevel != 0);

    std::vector<float> gpuData;
    gpuData.resize(hostData.size());
    for (unsigned int i=0; i<hostData.size(); i++)
    {
      gpuData.at(i) = (float)hostData.at(i);
    }
    std::cout << "gpuData.size() = " << gpuData.size() << std::endl;
    for (unsigned int i=0; i<146; i+=2)
    {
      std::cout << i << ": " << gpuData.at(i) << " " << gpuData.at(i+1) << std::endl;
    }
    /*
    // Level to be written to, starting at second to lowest
    int level = nrLevels - 1;
    // Dimensions for level to be read from
    int levelDim = _dim;
    // Offset into index array for level to be written to
    int offsetWrite = _dim*_dim*_dim;;
    // Offset into index array for level to be read from
    int offsetRead = 0;

    while (level>0) {

      std::cout 
        << "\n"
        << "level: " << level << "\n"
        << "levelDim: " << levelDim << "\n"
        << "offsetWrite: " << offsetWrite << "\n"
        << "offsetRead: " << offsetRead << "\n";

      for (int z=0; z<levelDim; z+=2) {
        for (int y=0; y<levelDim; y+=2) {
          for (int x=0; x<levelDim; x+=2) {
            // 8 sample points in 
            int idx[8];
            idx[0] = x     + y    *levelDim + z*    levelDim*levelDim;
            idx[1] = (x+1) + y    *levelDim + z*    levelDim*levelDim;
            idx[2] = x     + (y+1)*levelDim + z*    levelDim*levelDim;
            idx[3] = (x+1) + (y+1)*levelDim + z*    levelDim*levelDim;
            idx[4] = x     + y    *levelDim + (z+1)*levelDim*levelDim;
            idx[5] = (x+1) + y    *levelDim + (z+1)*levelDim*levelDim;
            idx[6] = x     + (y+1)*levelDim + (z+1)*levelDim*levelDim;
            idx[7] = (x+1) + (y+1)*levelDim + (z+1)*levelDim*levelDim;
            // Sample level below and average
            float sum = 0.f;
            for (int i=0; i<8; i++) {
              sum += hostData.at(offsetRead+idx[i]);
            }
            // Write to current level and step offset forward
            hostData.at(offsetWrite) = sum/8.f;
            offsetWrite++;
          } // z
        } // y     
      } // x

      // Add current level's size to next read level
      offsetRead += levelDim*levelDim*levelDim;
      // Calculate dimensions for next level to read from
      levelDim /= 2;
      // Start again
      level--;

      

    } // while

    */

    // TODO remove this temporary hack
    //std::reverse(hostData.begin(), hostData.end());

    std::cout << "Created octree structure on host\n";
    //std::cout << "Last postion written to: " << offsetWrite-1 << "\n\n";
    std::cout << "Creating texture buffer object and array...\n";

    // Create a buffer object for the data
    unsigned int dataBuffer;
    glGenBuffers(1, &dataBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, dataBuffer);
    glBufferData(GL_ARRAY_BUFFER, 
                 gpuData.size()*sizeof(float), // WAVE
                 static_cast<GLvoid*>(&gpuData[0]),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Construct 1D texture array, no filtering to make things easier and clearer
    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_BUFFER, handle_);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, dataBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    Manager::Instance().CheckGLErrors("Bound texture buffer");

    std::cout << "Finished creating texture buffer object and array\n";

    std::cout << "Finished creating volume buffer texture\n\n";

  } else {
    std::cout << _fileName << " could not be opened." << std::endl;
  }

}