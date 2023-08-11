#ifndef APP_H
#define APP_H

#include <GLFW/glfw3.h>

#include <algorithm>
#include <memory>

#include "client.h"

class App {
 public:
     // Why is "client" a shared_ptr? Why not a unique_ptr? From looking at
     // main() it seems ownership is wholly transferred to app.
  App(std::unique_ptr<Client> client);
  ~App();
  void Run();
  void Stop();
  void TickFrame();

 private:
  GLFWwindow* window;
  std::shared_ptr<Client> client;
  bool running;
  void Setup();
  void Clean();
};

#endif
