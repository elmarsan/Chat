#ifndef APP_H
#define APP_H

#include <GLFW/glfw3.h>

#include "client.h"

class App {
 public:
  App() = default;
  ~App();
  void run();
  void clean();
  void stop();
  void frameTick();
  Client* client;

 private:
  GLFWwindow* window;
  bool running = false;
  void setup();
  void imguiStyle();
};


#endif
