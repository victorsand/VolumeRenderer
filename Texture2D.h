#ifndef TEXTURE2D_H
#define TEXTURE2D_H

class Texture2D {
public:
  static Texture2D * New(unsigned int _width, unsigned int _height);
  // Init an empty texture
  void Init();
  unsigned int Handle() { return handle_; }
  unsigned int Width() { return width_; }
  unsigned int Height() { return height_; }

private:
  Texture2D(unsigned int _width, unsigned int _height);
  Texture2D(const Texture2D&) {}

  bool initialized_;
  unsigned int width_;
  unsigned int height_;
  unsigned int handle_;
};

#endif