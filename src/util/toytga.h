/*
 * A toy TGA parser written for Quarke.
 * At some point, this should be replaced with something of greater sanity.
 */

#ifndef QUARKE_SRC_UTIL_TOYTGA_H_
#define QUARKE_SRC_UTIL_TOYTGA_H_

namespace quarke {
namespace util {

namespace TGA {

struct Descriptor {
  char* data;
  int length;

  int width;
  int height;

  enum {
    TGA_RGBA32,
    TGA_RGB24
  } format;
};

// Loads the TGA file at the given path into RGBA data.
// Only true-color TGAs are supported.
bool LoadTGA(const char* path, Descriptor& out_descriptor);

}  // namespace TGA

}  // namespace util
}  // namespace quarke

#endif  // QUARKE_SRC_UTIL_TOYTGA_H_
