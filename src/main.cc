#include <iostream>

#include "src/common.h"
#include "src/tap.h"
#include "src/mac.h"

using namespace bangnet;

int main(int argc, char** argv) {
  MacAddress mac;
  mac.FromString("12:34:56:78");
  Tap tap(mac);

  return 0;
}