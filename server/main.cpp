#include <iostream>

#include "server.h"

int main() {
  std::cout << "Server main.cpp" << std::endl;
  Server server(63333);
  server.start();
}
