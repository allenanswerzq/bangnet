#ifndef BANGNET_TAP_H_
#define BANGNET_TAP_H_

#include <string>
#include "src/mac.h"
#include "src/inet_addr.h"
#include "src/common.h"

namespace bangnet {

class Tap {
public:
  // Constructs a tap device with a mac address.
  explicit Tap(const MacAddress &mac); 
  ~Tap();

  // Access to the mac address of this device.
  const MacAddress& mac() const { return mac_; }

  // Returns device's name
  string device_name(const char *dev) {
    return string(dev);
  }

  // Packets sent by an OS to user-space program which attaches itself
  // to the device. Also a user-space program can pass packets into tap 
  // device.
  // 
  // Put a frame, making it apparent for the os.
  void put(const MacAddress& from, const MacAddress& to, unsigned int type, 
           const void* data, unsigned int len); 

  unsigned int get(MacAddress& from, MacAddress& to, unsigned int type, void* buf);

  bool IsOpen() const;

  void close();

  bool AddIP(const InetAddress& ip);

  bool RemoveIP(const InetAddress& ip);

  inline set<InetAddress> IPSet() {
    return ips_;
  }
  
private:
  // Mac address of this tap device.
  const MacAddress mac_;

  // Mtu number of this tap device. 
  const unsigned int mtu_;

  // Device name.
  char dev_[16];
  
  // 
  unsigned char *put_buff_;

  // 
  unsigned char *get_buff_;

  // File descriptor associated with this interface.
  int fd_;

  // Bind ip addresses.
  set<InetAddress> ips_;

  // BN_DISALLOW_COPY_AND_ASSIGN(Tap);
};

}  // namespace bangnet

#endif  // BANGNET_TAP_H_