#ifndef SERVER_H
#define SERVER_H

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
  // void SendMsgToClient(const char* str, HSteamNetConnection clientConn);

  // ////////////////////////////
  //        Messaging methods
  // ////////////////////////////
  void SendMessage(const char* str, const HSteamNetConnection clientConnection);
  void SendMessageToAll(const char* str);

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
