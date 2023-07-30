#ifndef MESSAGE_H
#define MESSAGE_H

#include <ctime>
#include <string>
#include <vector>

class Message {
 public:
  Message() = default;
  Message(const std::string username, const std::string body);
  ~Message() = default;

  std::string username;
  std::string body;
  std::time_t time;
  std::vector<std::string> connectedUsers;

  std::string serialize();
  void deserialize(std::string data);
};

#endif
