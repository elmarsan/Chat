#include "server.h"

#include <iostream>
int main() {
  Server server(63333);
  server.Run();
}
