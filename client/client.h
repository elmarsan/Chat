#ifndef CLIENT_H
#define CLIENT_H

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include <string>
#include <thread>
#include <vector>

class Client {
 public:
  Client() = default;
  ~Client();

  void Connect(const std::string &serverAddress);
  void Disconnect();
  void SendMsg(const char *str);

  std::vector<std::string> messages;

 private:
  void Run();
  static void ConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t *info);
  void OnConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *info);

  void PollIncomingMessages();
  void PollConnectionStateChanges();

 private:
  std::thread m_NetworkThread;
  std::string m_ServerAddress;
  bool running = false;

  ISteamNetworkingSockets *m_Interface = nullptr;
  HSteamNetConnection m_Connection = 0;
};

#endif
