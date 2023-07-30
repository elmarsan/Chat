#include <iostream>

#include "server.h"
int main() {
  Server server(63333);
  server.Run();
}
