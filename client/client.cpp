#include "client.h"

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "message.h"

Client::Client(std::string serverAddress, std::string name)
    : name(name), serverAddress(serverAddress) {
  socketInterface = nullptr;
  connection = 0;
}

static Client* s_Instance = nullptr;
Client::~Client() {
  if (thread.joinable()) thread.join();
  Clean();
}

void Client::Disconnect() {
  running = false;
  if (thread.joinable()) thread.join();
}

void Client::Connect() {
  if (running) return;
  thread = std::thread([this]() { Run(); });
}

void Client::Run() {
  s_Instance = this;
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
    Stop("Unable to initialize GameNetworkingSockets");
    return;
  }

  socketInterface = SteamNetworkingSockets();

  SteamNetworkingIPAddr address;
  if (!address.ParseString(serverAddress.c_str())) {
    Stop("Invalid server address");
    return;
  }

  SteamNetworkingConfigValue_t options;
  options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                 (void*)ConnectionStatusChangedCallback);

  std::cout << "Connecting to server " << serverAddress << std::endl;
  connection = socketInterface->ConnectByIPAddress(address, 1, &options);
  if (connection == k_HSteamNetConnection_Invalid) {
    Stop("Invalid server address");
    return;
  }

  running = true;
  while (running) {
    PollIncomingMessages();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
  Clean();
}

void Client::Clean() {
  socketInterface->CloseConnection(connection, 0, nullptr, false);
}

void Client::Stop(std::optional<std::string> reason) {
  if (reason.has_value()) {
    std::cout << reason.value() << std::endl;
  }
  running = false;
  Clean();
}

void Client::SendMessage(Message message) {
  std::string payload = message.serialize();
  auto res = socketInterface->SendMessageToConnection(
      connection, payload.data(), payload.size(),
      k_nSteamNetworkingSend_Reliable, nullptr);
  messages.push_back(message);
  std::cout << "Sending Message: " << payload << std::endl;
  std::cout << "SendMessage result: " << res << std::endl;
}

void Client::PollIncomingMessages() {
  while (running) {
    ISteamNetworkingMessage* incomingMessage = nullptr;
    int messageCount = socketInterface->ReceiveMessagesOnConnection(
        connection, &incomingMessage, 1);
    if (messageCount <= 0) break;
    HandleIncomingSteamMessage(incomingMessage);
  }
}

void Client::HandleIncomingSteamMessage(ISteamNetworkingMessage* steamMessage) {
  if (!steamMessage->m_cbSize) {
    steamMessage->Release();
    return;
  }
  std::string serialized;
  serialized.assign((const char*)steamMessage->m_pData, steamMessage->m_cbSize);
  Message message;
  message.deserialize(serialized);
  messages.push_back(message);
  connectedUsers = message.connectedUsers;
  steamMessage->Release();
}

void Client::PollConnectionStateChanges() { socketInterface->RunCallbacks(); }

void Client::ConnectionStatusChangedCallback(
    SteamNetConnectionStatusChangedCallback_t* info) {
  s_Instance->OnConnectionStatusChanged(info);
}

void Client::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* status) {
  switch (status->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
      break;
    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      Stop("Network problem detected");
      break;
    }
    case k_ESteamNetworkingConnectionState_Connecting:
      break;
    case k_ESteamNetworkingConnectionState_Connected:
      std::cout << "Connected to server" << std::endl;
      break;
    default:
      break;
  }
}
