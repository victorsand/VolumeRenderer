#ifndef VOLUMETEXTURE_H
#define VOLUMETEXTURE_H

#include <string>

class VolumeTexture {
public:
  static VolumeTexture * New();
  // Read voxel data from .raw file, create the 3D texture
  // Params: filename, bits per voxel in raw data, dimensions (assuming cube)
  void ReadFromFile(std::string _fileName, int _bits, int _dim);
  unsigned int Handle() { return handle_; }
private:
  VolumeTexture() {}
  VolumeTexture(const VolumeTexture&) {}
  unsigned int handle_;
};

#endif