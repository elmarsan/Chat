#include <cstdlib>
#include <iostream>
#include <memory>

#include "app.h"
#include "client.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Client name is required" << std::endl;
    return EXIT_FAILURE;
  }
  std::string name = argv[1];
  if (name.size() == 0) {
    std::cerr << "Client name cannot be empty" << std::endl;
    return EXIT_FAILURE;
  }
  const auto client = std::make_shared<Client>("127.0.0.1:63333", name);
  client->Connect();
  App app(client);
  app.Run();
}
