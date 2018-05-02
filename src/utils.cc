#include "utils.h"

namespace bangnet {
namespace utils {

  string unhex(const char *hex) {
    int n = 1;
    unsigned char ch, b = 0;
    string r = string();

    while ((ch = *hex++)) {
      if ('0'<=ch && ch <='9') {
        if (n ^= 1) r.push_back((char)(b | ch-'0'));
        else b = (ch-'0') << 4;
      } else if ('a'<=ch && ch<='f') {
        if (n ^= 1) r.push_back((char)(b | ch-'a'+10));
        else b = (ch-'a') << 4;
      } else if ('A'<=ch && ch<='F') {
        if (n ^= 1) r.push_back((char)(b | ch-'A'+10));
        else b = (ch-'A') << 4;
      }
    }
    return r;
  }

}  // namespace utils
}  // namespace bangnet