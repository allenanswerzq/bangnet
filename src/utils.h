#ifndef BANGNET_UTILIS_H_
#define BANGNET_UTILIS_H_

#include "common.h"

namespace bangnet {
  namespace utils {

    // Unhex a string, combine two chars to from a hex number. ignore
    // other chars.
    string unhex(const char* hex);
    string unhex(const string& hex) { unhex(hex.c_str()); };

  }  // namespace utils
}  // namespace bangnet
#endif  // BANGNET_UTILIS_H_