#ifndef OCTREETEXTURE_H  
#define OCTREETEXTURE_H

#include <string>

class OctreeTexture {
public:
  static OctreeTexture * New();
  // Read voxel data from .raw file, create the 3D texture
  // Create the octree structure
  // Params: filename, bits per voxel in raw data, dimensions (assuming cube)
  // Dimensions needs to be a power of 2
  void ReadFromFile(std::string _fileName, int _bits, int _dim);
  unsigned int Handle() { return handle_; }
private:
  OctreeTexture() {}
  OctreeTexture(const OctreeTexture&) {}
  unsigned int handle_;
};

#endif