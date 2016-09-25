#include "util/toytga.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

namespace quarke {
namespace util {
namespace TGA {

#pragma pack(push,1)
struct Header {
  uint8_t id_len;
  uint8_t color_map_type;
  uint8_t image_type;
  uint8_t color_map_spec[5];
  uint16_t x_origin;
  uint16_t y_origin;
  uint16_t width;
  uint16_t height;
  uint8_t depth;
  uint8_t image_descriptor;
};
#pragma pack(pop)

static const uint8_t IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR = 2;
static const uint8_t IMAGE_TYPE_COMPRESSED_TRUE_COLOR = 10;

bool LoadTGA(const char* path, Descriptor& out_descriptor) {
  std::ifstream file(path);
  if (!file) {
    std::cerr << "[tga] Failed to open " << path << std::endl;
    return false;
  }

  Header header;
  file.read((char*)&header, sizeof(header));
  if (header.image_type != IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR &&
      header.image_type != IMAGE_TYPE_COMPRESSED_TRUE_COLOR) {
    std::cerr << "[tga] Unsupported format.";
    file.close();
    return false;
  }

  bool rle = header.image_type == IMAGE_TYPE_COMPRESSED_TRUE_COLOR;
  uint8_t depth = header.depth;
  if (depth != 24 && depth != 32) {
    std::cerr << "[tga] Unsupported depth " << header.depth << std::endl;
    file.close();
    return false;
  }

  if (header.width <= 0 || header.height <= 0) {
    std::cerr << "[tga] Invalid dimensions." << std::endl;
    file.close();
    return false;

  }

  int pixel_size = depth / 8;
  int length = header.width * header.height * pixel_size;
  char* data = (char*) malloc(length);

  // Following the header is the image ID, followed by the color map size (zero
  // for our impl's supported formats).
  file.seekg(header.id_len, std::fstream::cur);

  bool error = false;
  if (rle) {
    int offset = 0;
    while (offset < length) {
      char pcount;
      if (!file.get(pcount)) {
        std::cerr << "[tga] Failed to read packet pixel count." << std::endl;
        error = true;
        break;
      }

      int num_pixels = (pcount & 0x7F) + 1; // repeat count is minimally 1
      // Upper bit is always set to 1 for RLE.
      if (pcount & 0x80) {
        char val[pixel_size];
        if (!file.read(val, pixel_size)) {
          std::cerr << "[tga] Failed to read pixel data." << std::endl;
          error = true;
          break;
        }
        for (int i = 0; i < num_pixels; i++) {
          // assume little endian.
          memcpy(data + offset, val, pixel_size);
          offset += pixel_size;
        }
      } else {
        if (!file.read(data + offset, num_pixels * pixel_size)) {
          std::cerr << "[tga] Failed to read pixel data." << std::endl;
          error = true;
          break;
        }
        offset += num_pixels * pixel_size;
      }
    }
  } else {
    if (!file.read(data, length)) {
      std::cerr << "[tga] Failed to read raw pixel data." << std::endl;
      error = true;
    }
  }

  if (error) {
    free(data);
    file.close();
    return false;
  }

  out_descriptor.data = data;
  out_descriptor.length = length;
  out_descriptor.width = header.width;
  out_descriptor.height = header.height;
  if (depth == 32) {
    out_descriptor.format = Descriptor::TGA_RGBA32;
  } else if (depth == 24) {
    out_descriptor.format = Descriptor::TGA_RGB24;
  } else {
    assert(true);
  }

  file.close();
  return true;
}

}  // namespace TGA
}  // namespace util
}  // namespace quarke
