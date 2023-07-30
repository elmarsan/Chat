#include "client.h"

#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include "message.h"

static Client* s_Instance = nullptr;
Client::~Client() {
  if (m_NetworkThread.joinable()) m_NetworkThread.join();
}

void Client::Disconnect() {
  running = false;
  if (m_NetworkThread.joinable()) m_NetworkThread.join();
}

void Client::Connect(const std::string& serverAddress) {
  if (running) return;

  m_ServerAddress = serverAddress;
  m_NetworkThread = std::thread([this]() { Run(); });
}

void Client::Run() {
  s_Instance = this;

  std::cout << "Run started\n";
  ;

  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
    std::cerr << "Could not initialize GameNetworkingSockets\n";
    running = false;
    return;
  }

  // Select instance to use.  For now we'll always use the default.
  m_Interface = SteamNetworkingSockets();

  // Start connecting
  SteamNetworkingIPAddr address;
  if (!address.ParseString(m_ServerAddress.c_str())) {
    std::cerr << "Invalid IP address - could not parse " << m_ServerAddress
              << std::endl;
    running = false;
    return;
  }

  SteamNetworkingConfigValue_t options;
  options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                 (void*)ConnectionStatusChangedCallback);

  std::cout << "Connecting to server" << std::endl;

  m_Connection = m_Interface->ConnectByIPAddress(address, 1, &options);
  if (m_Connection == k_HSteamNetConnection_Invalid) {
    std::cerr << "Invalid IP address - could not parse " << m_ServerAddress
              << std::endl;
    running = false;
    return;
  }

  std::cout << "Client running" << std::endl;

  running = true;
  while (running) {
    PollIncomingMessages();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Clean up
  m_Interface->CloseConnection(m_Connection, 0, nullptr, false);
}

void Client::SendMessage(Message message) {
  std::string payload = message.serialize();
  auto res = m_Interface->SendMessageToConnection(
      m_Connection, payload.data(), payload.size(),
      k_nSteamNetworkingSend_Reliable, nullptr);
  messages.push_back(message);
  std::cout << "SendMsg result: " << res << std::endl;
}

void Client::PollIncomingMessages() {
  // Process all messages
  while (running) {
    ISteamNetworkingMessage* incomingMessage = nullptr;
    int messageCount = m_Interface->ReceiveMessagesOnConnection(
        m_Connection, &incomingMessage, 1);
    if (messageCount == 0) break;

    if (messageCount < 0) {
      // messageCount < 0 means critical error?
      running = false;
      return;
    }

    if (incomingMessage->m_cbSize) {
      std::string payload;
      payload.assign((const char*)incomingMessage->m_pData,
                     incomingMessage->m_cbSize);
      Message message;
      message.deserialize(payload);
      messages.push_back(message);
    }

    incomingMessage->Release();
  }
}

void Client::PollConnectionStateChanges() { m_Interface->RunCallbacks(); }

void Client::ConnectionStatusChangedCallback(
    SteamNetConnectionStatusChangedCallback_t* info) {
  s_Instance->OnConnectionStatusChanged(info);
}

void Client::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* status) {
  std::cout << "OnConnectionStatusChanged" << status->m_info.m_eState
            << std::endl;
}
