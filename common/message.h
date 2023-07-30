#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

class Message {
 public:
  Message() = default;
  Message(const std::string username, const std::string body);
  ~Message() = default;

  std::string username;
  std::string body;

  std::string serialize();
  void deserialize(std::string data);
};

#endif
