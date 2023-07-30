#include "server.h"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamclientpublic.h>
#include <steam/steamnetworkingtypes.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include "message.h"

static Server* s_Instance = nullptr;

Server::Server(int port) : port(port) {}

Server::~Server() {
  if (thread.joinable()) thread.join();
}

void Server::start() {
  if (running) return;

  thread = std::thread([this]() { run(); });
}

void Server::stop() { running = false; }

void Server::run() {
  s_Instance = this;
  running = true;

  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
    std::cout << errMsg << std::endl;
    running = false;
    return;
  }
  // TODO: error handling here
  socketInterface = SteamNetworkingSockets();
  networkingUtils = SteamNetworkingUtils();

  if (!networkingUtils) {
    std::cout << "Unable to init networkingUtils" << std::endl;
    running = false;
    return;
  }

  SteamNetworkingIPAddr serverLocalAddress;
  serverLocalAddress.Clear();
  serverLocalAddress.m_port = port;

  SteamNetworkingConfigValue_t options;
  options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                 (void*)Server::ConnectionStatusChangedCallback);

  listenSocket =
      socketInterface->CreateListenSocketIP(serverLocalAddress, 1, &options);

  if (listenSocket == k_HSteamListenSocket_Invalid) {
    std::cout << "Fatal error: Failed to listen on port " << port;
    running = false;
    return;
  }

  pollGroup = socketInterface->CreatePollGroup();
  if (pollGroup == k_HSteamNetPollGroup_Invalid) {
    std::cout << "Fatal error: Failed to create poll group on port " << port;
    running = false;
    return;
  }

  std::cout << "Server listening on port " << port << std::endl;

  while (running) {
    PollIncomingMessages();
    PollConnectionStateChanges();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Clear
  connectedClients.clear();
  socketInterface->CloseListenSocket(listenSocket);
  listenSocket = k_HSteamListenSocket_Invalid;
  socketInterface->DestroyPollGroup(pollGroup);
  pollGroup = k_HSteamNetPollGroup_Invalid;
}

void Server::PollIncomingMessages() {
  while (running) {
    ISteamNetworkingMessage* incomingMsg = nullptr;
    int msgCount =
        socketInterface->ReceiveMessagesOnPollGroup(pollGroup, &incomingMsg, 1);
    if (msgCount == 0) break;

    if (msgCount < 0) {
      std::cerr << "While polling incoming messages\n";
      running = false;
      return;
    }

    std::cout << "Received " << msgCount << " messages\n";

    auto itClient = connectedClients.find(incomingMsg->m_conn);
    if (itClient == connectedClients.end()) {
      std::cout << "ERROR: Received data from unregistered client\n";
      continue;
    }

    // TODO: data received callback
    if (incomingMsg->m_cbSize) {
      std::string msgStr;
      msgStr.assign((const char*)incomingMsg->m_pData, incomingMsg->m_cbSize);
      std::cout << "Received msg: " << msgStr.c_str() << "\n";
      Message message("server", "thanks for your message bro");
      SendMessage(message, incomingMsg->GetConnection());
    }

    incomingMsg->Release();
  }
}

void Server::ConnectionStatusChangedCallback(
    SteamNetConnectionStatusChangedCallback_t* info) {
  s_Instance->OnConnectionStatusChanged(info);
}

void Server::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* status) {
  switch (status->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting: {
      std::cout << "Status changed Connecting" << std::endl;

      // Accept connection
      if (socketInterface->AcceptConnection(status->m_hConn) != k_EResultOK) {
        socketInterface->CloseConnection(status->m_hConn, 0, nullptr, false);
        std::cout << "Unable to accept connection" << std::endl;
      }

      // Assing to poll group
      if (!socketInterface->SetConnectionPollGroup(status->m_hConn,
                                                   pollGroup)) {
        socketInterface->CloseConnection(status->m_hConn, 0, nullptr, false);
        std::cout << "Unable to set poll group" << std::endl;
      }

      // Retrieve connection info
      SteamNetConnectionInfo_t connectionInfo;
      socketInterface->GetConnectionInfo(status->m_hConn, &connectionInfo);

      // Register connected client
      auto& client = connectedClients[status->m_hConn];
      client.id = connectionInfo.m_szConnectionDescription;
      client.name = "Client " + std::to_string(connectedClients.size() + 1);

      // TODO: client connected callback
      break;
    }

    case k_ESteamNetworkingConnectionState_Connected:
      std::cout << "Status changed Connected" << std::endl;
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
      std::cout << "Status changed ClosedByPeer" << std::endl;
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      std::cout << "Status changed ProblemDetectedLocally" << std::endl;
      socketInterface->CloseConnection(status->m_hConn, 0, nullptr, false);
      break;
    };
  }
}

void Server::PollConnectionStateChanges() { socketInterface->RunCallbacks(); }

void Server::SendMessage(Message& message,
                           const HSteamNetConnection clientConnection) {
  std::string payload = message.serialize();

  std::cout << "Sending: " << payload << std::endl;

  auto result = socketInterface->SendMessageToConnection(
      clientConnection, payload.data(), payload.size(),
      k_nSteamNetworkingSend_Reliable, nullptr);

  std::cout << "SendMessageV2 result: " << result << std::endl;
}

void Server::BroadcastMessage(Message& message) {
  for (const auto& pair : connectedClients) {
    SendMessage(message, pair.first);
  }
}
