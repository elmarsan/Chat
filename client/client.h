#ifndef CLIENT_H
#define CLIENT_H

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "message.h"

class Client {
 public:
  Client(std::string serverAddress, std::string name);
  ~Client();
  void Connect();
  void Disconnect();
  void SendMessage(Message message);

  std::vector<std::string> connectedUsers;
  std::vector<Message> messages;
  std::string name;

 private:
  void Run();
  void Clean();
  void Stop(std::optional<std::string> reason);
  static void ConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t *info);
  void OnConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *info);
  void HandleIncomingSteamMessage(ISteamNetworkingMessage *steamMessage);
  void PollIncomingMessages();
  void PollConnectionStateChanges();

  std::thread thread;
  std::string serverAddress;
  bool running = false;
  ISteamNetworkingSockets *socketInterface;
  HSteamNetConnection connection;
};

#endif
