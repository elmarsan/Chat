#include <iostream>
#include "app.h"
#include "client.h"

int main() {
  std::cout << "Client main.cpp" << std::endl;
  Client client;
  client.Connect("127.0.0.1:63333");

  App app;
  app.client = &client;
  app.start();
}
