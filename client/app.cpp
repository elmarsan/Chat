#include "app.h"

#include <GLFW/glfw3.h>

#include <cfloat>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

App::App(std::unique_ptr<Client> client) : client(std::move(client)) { running = false; }

void App::Run() {
  if (!running) Setup();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    TickFrame();
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
}

void App::TickFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("##", nullptr, ImGuiWindowFlags_NoDecoration);
  // You probably don't want to copy the whole struct, so just take a const ref.
  const ImVec2& screenSize = ImGui::GetIO().DisplaySize;

  auto const columnFlags =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
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
    ImGui::Begin("Messages", nullptr, columnFlags);
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
    if (ImGui::BeginListBox(
            "##", ImVec2(contentRegionAvail.x, contentRegionAvail.y - 30))) {
      for (const auto& message : client->messages) {
        ImGui::TextWrapped("%s %s", std::asctime(std::localtime(&message.time)),
                           message.body.c_str());
      }
      ImGui::EndListBox();
    }

    static char textBuffer[64] = "";
    if (ImGui::InputText("Send", textBuffer, 64,
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      std::cout << "Enter key pressed" << std::endl;
      client->SendMessage(Message(client->name, textBuffer));
      strncpy(textBuffer, "", 64);
    }
    ImGui::End();
  });

  ImVec2 rightCol(screenSize.x * 0.3f, screenSize.y);
  renderColumn(rightCol, &leftCol, [this]() {
    std::string title =
        std::to_string(client->connectedUsers.size() + 1) + " Users";
    ImGui::Begin(title.c_str(), nullptr, columnFlags);
    for (const auto& user : client->connectedUsers) {
      ImGui::Text(user.c_str());
    }
    ImGui::Text("You");
    ImGui::End();
  });

  ImGui::End();
}

App::~App() { Clean(); }

void App::Clean() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
}

void App::Stop() {
  std::cout << "Closing chat...";
  running = false;
  Clean();
}

static void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void App::Setup() {
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

  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowMenuButtonPosition = ImGuiDir_None;

  running = true;
}
