#include "app.h"

#include <GLFW/glfw3.h>

#include <cfloat>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void App::imguiStyle() {}

void App::run() {
  if (!running) setup();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    frameTick();
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
}

void App::frameTick() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("##", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
  const ImVec2 screenSize = ImGui::GetIO().DisplaySize;

  auto const renderColumn = [this](ImVec2 size, ImVec2* siblingSize,
                                   std::function<void()> renderFunc) {
    ImGui::SetNextWindowSize(size);
    if (siblingSize != nullptr) {
      ImGui::SetNextWindowPos(ImVec2(siblingSize->x, 0));
    } else {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
    }
    renderFunc();
  };

  ImVec2 leftCol(screenSize.x * 0.7f, screenSize.y);
  renderColumn(leftCol, nullptr, [this]() {
    ImGui::Begin("Messages", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
    if (ImGui::BeginListBox(
            "##", ImVec2(contentRegionAvail.x, contentRegionAvail.y - 30))) {
      for (const auto& message : client->messages) {
        std::time_t result = std::time(nullptr);
        ImGui::TextWrapped("%s %s", std::asctime(std::localtime(&result)),
                           message.body.c_str());
      }
      ImGui::EndListBox();
    }

    // Input Text for sending messages
    static char buf1[64] = "";
    if (ImGui::InputText("Send", buf1, 64,
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      std::cout << "Enter key pressed" << std::endl;
      client->SendMessage(Message("you", buf1));
      strncpy(buf1, "", 64);
    }
    ImGui::End();
  });

  ImVec2 rightCol(screenSize.x * 0.3f, screenSize.y);
  renderColumn(rightCol, &leftCol, []() {
    // Step 6: Begin the second element's ImGui window
    ImGui::Begin("Users", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    // Add your ImGui code for the second element here...
    ImGui::Text("Second Element (30%% width)");
    ImGui::End();  // End the second element's window
  });

  ImGui::End();
}

App::~App() { glfwDestroyWindow(window); }

void App::clean() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void App::stop() {
  std::cout << "Closing chat...";
  running = false;
  clean();
}
static void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void App::setup() {
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    std::cerr << "Unable to init glfw" << std::endl;
    running = false;
    return;
  }

  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(1280, 720, "Chat", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Unable to create glfw window" << std::endl;
    running = false;
    return;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Styles
  auto textColor = ImVec4(236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 1.0f);
  auto bodyColor = ImVec4(44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 1.0f);
  auto areaColor = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  ImGuiStyle& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_Text] = textColor;
  style.Colors[ImGuiCol_Border] = bodyColor;
  style.Colors[ImGuiCol_FrameBg] = areaColor;

  running = true;
}
