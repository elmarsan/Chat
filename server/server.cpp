#include "server.h"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamclientpublic.h>
#include <steam/steamnetworkingtypes.h>

#include <ctime>
#include <iostream>
#include <string>

#include "message.h"

static Server* s_Instance = nullptr;

Server::Server(int port) : port(port) {
  socketInterface = nullptr;
  networkingUtils = nullptr;
  listenSocket = 0u;
  pollGroup = 0u;
  running = false;
}

Server::~Server() {
  if (thread.joinable()) thread.join();
}

void Server::Run() {
  if (running) return;

  thread = std::thread([this]() {
    s_Instance = this;
    running = true;

    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
      Stop(errMsg);
      return;
    }
    // TODO: error handling here
    socketInterface = SteamNetworkingSockets();
    networkingUtils = SteamNetworkingUtils();

    if (!networkingUtils) {
      Stop("Unable to init networkingUtils");
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
      Stop("Unable to create listen socket");
      return;
    }

    pollGroup = socketInterface->CreatePollGroup();
    if (pollGroup == k_HSteamNetPollGroup_Invalid) {
      Stop("Unable to create poll group");
      return;
    }

    std::cout << "Server listening on port " << port << std::endl;
    while (running) {
      PollIncomingMessages();
      PollConnectionStateChanges();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    Clean();
  });
}

void Server::Stop(std::optional<std::string> reason) {
  if (reason.has_value()) {
    std::cout << reason.value() << std::endl;
  }
  running = false;
  Clean();
}

void Server::Clean() {
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
    if (msgCount <= 0) break;
    // FYI in c++20 you have .contains().
    auto itClient = connectedClients.find(incomingMsg->m_conn);
    if (itClient == connectedClients.end()) {
      continue;
    }
    HandleIncomingSteamMessage(incomingMsg);
  }
}

void Server::HandleIncomingSteamMessage(ISteamNetworkingMessage* steamMessage) {
  if (!steamMessage->m_cbSize) {
    steamMessage->Release();
    return;
  }
  std::string serialized;
  serialized.assign((const char*)steamMessage->m_pData, steamMessage->m_cbSize);
  Message message;
  message.deserialize(serialized);
  auto broadcastConnections = std::vector<HSteamNetConnection>();
  for (const auto& pair : connectedClients) {
    message.connectedUsers.push_back(pair.second.name);

    if (pair.first != steamMessage->GetConnection()) {
      broadcastConnections.push_back(pair.first);
    }
  }
  steamMessage->Release();
  BroadcastMessage(message, broadcastConnections);
}

void Server::ConnectionStatusChangedCallback(
    SteamNetConnectionStatusChangedCallback_t* info) {
  s_Instance->OnConnectionStatusChanged(info);
}

void Server::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* status) {
  switch (status->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting: {
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
    };
    case k_ESteamNetworkingConnectionState_Connected:
      std::cout << "Client connected" << std::endl;
      break;
    case k_ESteamNetworkingConnectionState_ClosedByPeer:
      std::cout << "Client closed connection" << std::endl;
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      std::cout << "Network problem detected" << std::endl;
      socketInterface->CloseConnection(status->m_hConn, 0, nullptr, false);
      break;
    };
    default:
      break;
  }
}

void Server::PollConnectionStateChanges() { socketInterface->RunCallbacks(); }

void Server::SendMessage(Message& message,
                         const HSteamNetConnection clientConnection) {
  std::string serialized = message.serialize();
  auto result = socketInterface->SendMessageToConnection(
      clientConnection, serialized.data(), serialized.size(),
      k_nSteamNetworkingSend_Reliable, nullptr);
  std::cout << "SendMessage result: " << result << std::endl;
}

void Server::BroadcastMessage(Message& message,
                              std::vector<HSteamNetConnection> connections) {
  for (const auto& connection : connections) {
    SendMessage(message, connection);
  }
}
