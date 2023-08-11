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
     // this is probably overly pedantic, but you could use uint16_t, as a part
     // is always an unsigned 16 bit int.
     // Also, most socket libs would allow you to use 0 as the value, and then
     // the OS would choose an available port dynamically. 
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
  // the std::map is pretty shit in general. One property that is sometimes
  // useful, but oftentimes not, is that it's ordered by key (it's implemented
  // using a tree). If you don't care about ordering, you probably want
  // std::unordered_map instead.
  std::map<HSteamNetConnection, Client> connectedClients;
  std::thread thread;
  int port;
  bool running;
};

#endif
