#ifndef SERVER_H
#define SERVER_H

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>

#include <map>
#include <optional>
#include <string>
#include <thread>

#include "message.h"

struct Client {
  std::string name;
  std::string id;
};

class Server {
 public:
  Server(int port);
  ~Server();
  void Run();

 private:
  void Stop(std::optional<std::string> reason);
  void Clean();
  static void ConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t* info);
  void OnConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* info);
  void PollIncomingMessages();
  void PollConnectionStateChanges();

  void HandleIncomingSteamMessage(ISteamNetworkingMessage* steamMessage);
  void BroadcastMessage(Message& message,
                        std::vector<HSteamNetConnection> connections);
  void SendMessage(Message& message,
                   const HSteamNetConnection clientConnection);

  ISteamNetworkingSockets* socketInterface;
  ISteamNetworkingUtils* networkingUtils;
  HSteamListenSocket listenSocket;
  HSteamNetPollGroup pollGroup;
  std::map<HSteamNetConnection, Client> connectedClients;
  std::thread thread;
  int port;
  bool running;
};

#endif
