#ifndef SERVER_H
#define SERVER_H

// #include "../message.h"
#include "message.h"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>

#include <functional>
#include <map>
#include <string>
#include <thread>

#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

struct Client {
  std::string name;
  std::string id;
};

class Server {
 public:
  Server(int port);
  ~Server();

  void start();
  void stop();

 private:
  void run();

  static void ConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t* info);
  void OnConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* info);
  void PollIncomingMessages();
  void PollConnectionStateChanges();

  void BroadcastMessage(Message& message);
  void SendMessage(Message& message, const HSteamNetConnection clientConnection);

 private:
  ISteamNetworkingSockets* socketInterface = nullptr;
  ISteamNetworkingUtils* networkingUtils = nullptr;
  HSteamListenSocket listenSocket = 0u;
  HSteamNetPollGroup pollGroup = 0u;

  std::map<HSteamNetConnection, Client> connectedClients;
  std::thread thread;
  int port;
  bool running = false;
};

#endif
