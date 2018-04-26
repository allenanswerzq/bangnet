#include "src/inet_addr.h"
#include <arpa/inet.h>

namespace bangnet {

  void InetAddress::SetInetAddr(const string& ip, unsigned int port) {
    memset(&sa_, 0, sizeof(sa_));
    if (ip.find(':') != string::npos) {
      SetFamilyInV6();
      SetPort(port);
      if (inet_pton(AF_INET6, ip.c_str(), raw_ip_addr()) <= 0)
        SetFamilyZero();
    } else {
      SetFamilyInV4();
      SetPort(port);
      if (inet_pton(AF_INET, ip.c_str(), raw_ip_addr()) <= 0)
        SetFamilyZero();
    }
  }

  string InetAddress::ToString() const {
    char buf[128], buf2[128];

    if (family() == AF_INET) {
      if (inet_ntop(AF_INET, raw_ip_addr(), buf, sizeof(buf))) {
        sprintf(buf2, "%s/%u", buf, port());
        return string(buf);
      }
    } else  {
      if (inet_ntop(AF_INET6, raw_ip_addr(), buf, sizeof(buf))) {
        sprintf(buf2, "%s/%u", buf, port());
        return string(buf);
      }
    }
    return string();
  }

  string InetAddress::ToIpString() const {
    char buf[128];
    if(family() == AF_INET) {
      if (inet_ntop(AF_INET, raw_ip_addr(), buf, sizeof(128)))
        return string(buf);
    } else {
      if (inet_ntop(AF_INET6, raw_ip_addr(), buf, sizeof(128)))
        return string(buf);
    }
    return string();
  }

  void InetAddress::SetInetAddr(const string& ip_slash_port) {
    size_t slash_at = ip_slash_port.find('/');
    if (slash_at == string::npos || slash_at >= ip_slash_port.length()) {
      // No port specifies.
      SetInetAddr(ip_slash_port, 0);
    } else {
      int port = stoi(ip_slash_port.substr(slash_at + 1));
      if (port > 0 && port <= 65535)
        SetInetAddr(ip_slash_port.substr(0, slash_at), (unsigned int)port);
      else
        SetInetAddr(ip_slash_port.substr(0, slash_at), 0);
    }
  }

  bool InetAddress::operator==(const InetAddress &a) const {
    if (family() == AF_INET) {
      if (a.family() == AF_INET)
        return ((int*)raw_ip_addr() == (int*)a.raw_ip_addr()) && 
          (port() == a.port());
      return false;
    } else if (family() == AF_INET6) {
      // if (a.family() == AF_INET6)
      //   return (*raw_ip_addr() == *a.raw_ip_addr()) && (port() == a.port());
      return false;
    }
  }

}  // namespace bangnet
