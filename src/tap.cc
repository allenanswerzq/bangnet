// TUN/TAP provides packet reception and transmission for user space programs. 
// It can be seen as a simple Point-to-Point or Ethernet device, which,
// instead of receiving packets from physical media, receives them from 
// user space program and instead of sending packets via physical media 
// writes them to the user space program. 

// In order to use the driver a program has to open /dev/net/tun and issue a
// corresponding ioctl() to register a network device with the kernel. A network
// device will appear as tunXX or tapXX, depending on the options chosen. When
// the program closes the file descriptor, the network device and all
// corresponding routes will disappear.

// Depending on the type of device chosen the userspace program has to 
// read/write IP packets (with tun) or ethernet frames (with tap). 
// Which one is being used depends on the flags given with the ioctl().

// More details can be found on this link:
// https://github.com/torvalds/linux/blob/master/Documentation/networking/tuntap.txt

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>

#include "tap.h"
#include "common.h"

#define IP_COMMAND "/sbin/ip"
#define SYSCTL_COMMAND "/sbin/sysctl"

namespace bangnet {

Tap::Tap(const MacAddress& mac)
    : mac_(mac), 
      put_buff_((unsigned char*)0), 
      get_buff_((unsigned char*)0),
      fd_(0),
      mtu_(2800) {
  
  fd_ = open("/dev/net/tun", O_RDWR);
  CHECK_GT(fd_, 0) << "Could not open TAP device";

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  // Get a device name for this inteface.
  int devno = 0;
  struct stat sbuf;
  char procpath[128];
  do {
    sprintf(ifr.ifr_name, "bg%d", devno++);
    sprintf(procpath, "/proc/sys/net/ipv4/conf/%s", ifr.ifr_name);
  } while (stat(procpath, &sbuf) == 0);

  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(fd_, TUNSETIFF, (void*)&ifr) < 0) {
    ::close(fd_);
    LOG(FATAL) << "Unable to configure TAP device";
  }

  // Now, we have a name for this interface.
  strcpy(dev_, ifr.ifr_name); 
  
  // Dont know what this does, Leave it now.
  ioctl(fd_, TUNSETPERSIST, 0);

  // Open an any sockset
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock <= 0) {
    ::close(fd_);
    LOG(FATAL) << "Unable to open socket";
  }

  // Set MacAddress address.
  ifr.ifr_ifru.ifru_hwaddr.sa_family = ARPHRD_ETHER;
  memcpy(ifr.ifr_ifru.ifru_hwaddr.sa_data, mac_.data(), 6);
  if (ioctl(sock,SIOCSIFHWADDR, (void *)&ifr) < 0) {
    ::close(fd_);
    ::close(sock);
    LOG(FATAL) << "Unable to configure interface";
  }

  // Set MTU.
  ifr.ifr_ifru.ifru_mtu = (int)mtu_;
  if (ioctl(sock, SIOCSIFMTU, (void *)&ifr) < 0) {
    ::close(fd_);
    ::close(sock);
    LOG(FATAL) << "Unable to configure interface";
  }

  if (fcntl(fd_, F_SETFL, fcntl(fd_,F_GETFL) & ~O_NONBLOCK) == -1) {
    ::close(fd_);
    LOG(FATAL) << "Unable to configure interface";
  }

  // Bring interface up.
  if (ioctl(sock, SIOCGIFFLAGS, (void *)&ifr) < 0) {
    ::close(fd_);
    ::close(sock);
    LOG(FATAL) << "Unable to get tap interface flags";
  }
  ifr.ifr_flags |= IFF_UP;
  if (ioctl(sock, SIOCSIFFLAGS, (void*)&ifr < 0)) {
    ::close(fd_);
    ::close(sock);
    LOG(FATAL) << "Unable to get tap interfaces flags";
  }

  ::close(sock);
  put_buff_ = new unsigned char[(mtu_ + 16) * 2];
  get_buff_ = put_buff_ + (mtu_ + 16);

  LOG(INFO) << "Tap " << device_name() << " created";
}

Tap::~Tap() {
  this->close();
  delete put_buff_;
}

void Tap::put(const MacAddress& from, const MacAddress& to, unsigned int type, 
              const void* data, unsigned int len) {
  // Constructs ethernet a frame payload
  if (fd_ > 0 && len <= mtu_) {
    for (int i=0; i<6; ++i) {
      put_buff_[i] = to.data(i);
      put_buff_[i+6] = from.data(i);
    }
    *(uint16_t*)(put_buff_ + 12) = htons((uint16_t)type);
    memcpy(put_buff_ + 14, data, len);
    ::write(fd_, put_buff_, len + 14);
  }
}

unsigned int Tap::get(MacAddress& from, MacAddress& to, unsigned int type, void *buf) {
  // Simply just read a ethernet frame
  if (fd_ > 0) {
    unsigned int n = ::read(fd_, get_buff_, mtu_ + 14);
    if (n > 14) {
      for (int i=0; i<6; ++i) {
        to.set_data(i, get_buff_[i]);
        from.set_data(i, get_buff_[i+6]);
      }
      type = ntohs(((uint16_t *)get_buff_)[6]);
      memcpy(buf, get_buff_ + 14, n);
      return n;
    }
  }
}

bool Tap::IsOpen() const {
  return fd_ > 0;
}

void Tap::close() {
  if (fd_ > 0) {
    int f = fd_;
    fd_ = 0;
    ::close(f);
  }
}

bool Tap::remove_ip(const char *dev_, set<InetAddress>& ips_, 
               const InetAddress& ip) {
  int cpid;
  if ((cpid = fork()) == 0) {
    execl(IP_COMMAND, IP_COMMAND, "addr", "del", ip.ToIpString().c_str(), 
          "dev", dev_, (const char *)0);
    exit(1);
  } else {
    int exit_code = 1;
    waitpid(cpid, &exit_code, 0);
    if (exit_code == 0) {
      ips_.erase(ip);
      return true;
    }
    return false;
  }

  return false;
}

bool Tap::AddIP(const InetAddress& ip) {
  // If it's not a internet address.
  if (!ip)
    return false;

  if (ips_.count(ip))
    return true;
  
  for (auto it = ips_.begin(); it != ips_.end(); ++it) {
    // If we already has this address, remove first.
    if (*it == ip) {
      remove_ip(dev_, ips_, *it);
      break;
    }
  }

  int cpid;
  if ((cpid = fork()) == 0) {
    // Child process.
    execl(IP_COMMAND, IP_COMMAND, "addr", "add", ip.ToString().c_str(),
          "dev", dev_, (const char *)0);
    exit(-1);
  } else {
    int exit_code = -1;
    waitpid(cpid, &exit_code, 0);
    if (exit_code == 0) {
      ips_.insert(ip);
      return true;
    }
    return false;
  }

  return false;
}

bool Tap::RemoveIP(const InetAddress& ip) {
  if (ips_.count(ip))
    return remove_ip(dev_, ips_, ip);
  return false;
}

}  // namespace bangnet