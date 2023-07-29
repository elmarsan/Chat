#include "client.h"

#include <iostream>
#include <iterator>
#include <string>
#include <thread>

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

void Client::SendMsg(const char* str) {
  auto res = m_Interface->SendMessageToConnection(
      m_Connection, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable,
      nullptr);
  messages.push_back(std::string(str));
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
      std::string msgStr;
      msgStr.assign((const char*)incomingMessage->m_pData,
                    incomingMessage->m_cbSize);
      std::cout << "Received msg: " << msgStr.c_str() << "\n";

      // std::string msgStr;
      // msgStr.assign((const char*)incomingMessage->m_pData,
      //               incomingMessage->m_cbSize);
      // std::cout << "Receive msg: " << msgStr << "\n";
      messages.push_back(msgStr);
    }
    // Release when done
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

// void Client::SendMsg2(const Message& msg) {
// std::cout <<"Sender: " << msg.sender << std::endl;
// std::cout << "Body: " << msg.body << std::endl;

// auto* data = msg.serialize();

// const void* pData = reinterpret_cast<const void*>(&msg);
//  auto res = m_Interface->SendMessageToConnection(
//      m_Connection, data, (uint32)sizeof(data),
//      k_nSteamNetworkingSend_Reliable, nullptr);

// delete[] static_cast<char*>(data);

//  std::cout << "SendMsg result: " << res << std::endl;
// }
