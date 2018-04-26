#ifndef BANGNET_IP_H_
#define BANGNET_IP_H_

#include <cstring>
#include <netinet/in.h>

#include "src/common.h"

namespace bangnet {

class InetAddress {
public:
  // IP address type.
  enum AddressType {
   IP_TYPE_NULL = 0,
   IP_TYPE_IPV4,
   IP_TYPE_IPV6
  };

  // 
  InetAddress() {
    memset(&sa_, 0, sizeof(sa_));
  }

  InetAddress(const InetAddress& ip) {
    memcpy(&sa_, &ip.sa_, sizeof(sa_));
  }

  // InetAddress(const struct sockaddr *sa) {
  //   SetInetAddr(sa);
  // }

  InetAddress(const string& ip, unsigned int port) {
    SetInetAddr(ip, port);
  }

  // Set Inet address according to different input arguments.
  inline void SetInetAddr(const struct sockaddr sa) {
    switch(sa.sa_family) {
      case AF_INET:
        memcpy(&sa_, &sa, sizeof(struct sockaddr_in));
        break;
      case AF_INET6:
        // TODO
      default:
        sa_.saddr.sa_family = 0;
        break;
    }
  } 

  inline void SetInetAddr(const void *ip, unsigned int len, 
                             unsigned int port) {
    sa_.saddr.sa_family = 0;
    // IPv4 case
    if (len == 4) {
      SetTypeV4();
      memcpy(raw_ip_addr(), ip, 4);
      SetPort(port);
    } else {
      SetTypeV6();
      memcpy(raw_ip_addr(), ip, 16);
      SetPort(port);
    }
  }

  void SetInetAddr(const string& ip_salsh_port);

  void SetInetAddr(const string& ip, unsigned int port);

  // Set port.
  inline void SetPort(unsigned int port) {
    if (family() == AF_INET)
      sa_.sin.sin_port = htons((uint16_t)port);
    else if (family() == AF_INET6)
      sa_.sin6.sin6_port = htons((uint16_t)port);
  }

  // Set family.
  inline void SetTypeV4() {sa_.saddr.sa_family = AF_INET; }
  inline void SetTypeV6() {sa_.saddr.sa_family = AF_INET6; }
  inline void SetFamilyInV4() { sa_.sin.sin_family = AF_INET; }
  inline void SetFamilyInV6() { sa_.sin6.sin6_family = AF_INET6; }
  inline void SetFamilyZero() { sa_.saddr.sa_family = 0; }

  //
  inline bool IsV4() const { return sa_.saddr.sa_family == AF_INET; } 
  inline bool IsV6() const { return sa_.saddr.sa_family == AF_INET6; }

  // Returns family of this address to determin whether its ipv4 or ipv6.
  inline unsigned short family() const { return sa_.saddr.sa_family; }

  inline struct sockaddr* saddr() { return &(sa_.saddr); } 
  inline const struct sockaddr* saddr() const { return &(sa_.saddr); } 

  //
  inline unsigned int saddr_len () const {
    return IsV4() ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  }

  inline unsigned int saddr_space_len() const { return sizeof(sa_); }

  // Returns pointer which point to raw ip data.
  inline void *raw_ip_addr() {
    return sa_.saddr.sa_family == AF_INET ? (void*)&sa_.sin.sin_addr.s_addr :
      (void*)&sa_.sin6.sin6_addr.s6_addr;
  } 
  inline const void *raw_ip_addr() const {
    return sa_.saddr.sa_family == AF_INET ? (void*)&sa_.sin.sin_addr.s_addr :
      (void*)&sa_.sin6.sin6_addr.s6_addr;
  } 

  // Returns port bound with this address.
  inline unsigned int port() const {
    switch(family()) {
      case AF_INET:
        return ntohs(sa_.sin.sin_port);
        break;
      case AF_INET6:
        return ntohs(sa_.sin6.sin6_port);
        break;
    }
    return 0;
  }

  bool operator==(const InetAddress &a) const;

  // Returns true if this address is a internet style address.
  // Caller should call it like this `if (ip)`.
  inline operator bool() const {
    return sa_.saddr.sa_family == AF_INET || sa_.saddr.sa_family == AF_INET6;
  }

  // Returns a representation of a inetnet address.
  string ToString() const;

  // Returns only ip part, ignore port.
  string ToIpString() const;

private:
  union {
    // Generic socket address.
    struct sockaddr saddr;

    // IPV4 socket address. 
    struct sockaddr_in sin;
    
    // IPV6 socket address. 
    struct sockaddr_in6 sin6;
  } sa_;
};

}  // namespace bangnet
#endif  // IP_H_