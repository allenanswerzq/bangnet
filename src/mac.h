#ifndef BANGNET_MAC_H_
#define BANGNET_MAC_H_

#include <string>
#include <cstdio>

#include "utils.h"

namespace bangnet {

class MacAddress {
public:
  // Constructs a zero/null mac.
  MacAddress() {
    for (int i=0; i<6; ++i)
      mac_[i] = 0;
  }

  // Constructs a mac contains all same number.
  MacAddress(const unsigned char num) {
    for (int i=0; i<6; ++i)
      mac_[i] = num;
  }

  // Constructs a new mac from raw bits.
  MacAddress(const unsigned char *bits) {
    for (int i=0; i<6; ++i)
      mac_[i] = *(unsigned char*)(bits+i);
  }

  // Returns ture is this mac is not zero/null.
  inline bool IsZero() {
    for (int i=0; i<6; ++i)
      if (mac_[i])
        return true;
    return false;
  }

  // Returns ture is this mac is a broadcast mac address.
  inline bool IsBroadcast() {
    for (int i=0; i<6; ++i)
      if (mac_[i] != 0xff)
        return false;
    return true;
  }

  // Set this mac to zero/null.
  inline void SetZero() {
    for (int i=0; i<6; ++i)
      mac_[i] = 0;
  }

  // Constructs a mac address from a string that contains hex numbers.
  inline bool FromString(const char *s) {
    string b(utils::unhex(s));

    if (b.size() == 6) {
      for (int i=0; i<6; ++i) 
       mac_[i] = (unsigned char)b[i];
      return true;
    }  

    this->SetZero();
    return false;
  }

  const unsigned char* data() const {
    return mac_;
  }

  const unsigned char data(int i) const {
    return mac_[i];
  }

  void set_data(int i, unsigned char val) {
    mac_[i] = val;
  }

  // Convert mac address to a printable string
  inline string ToString() {
    char tmp[32];
    sprintf(tmp, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", (int)mac_[0],
      (int)mac_[1], (int)mac_[2], (int)mac_[3], (int)mac_[4], (int)mac_[5]);
    return string(tmp);
  }

private:
  unsigned char mac_[6];
};

}  // namespace bangnet
#endif  // BANGNET_MAC_H_