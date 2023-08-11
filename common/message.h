#ifndef MESSAGE_H
#define MESSAGE_H

#include <ctime>
#include <string>
#include <vector>

class Message {
 public:
  Message() = default;
  // Use non-const arguments so that you can then std::move() the strings
  // into the member variables. See: https://youtu.be/xnqTKD8uD64
  Message(std::string username, std::string body);
  ~Message() = default;

  std::string username;
  std::string body;
  std::time_t time;
  std::vector<std::string> connectedUsers;

  std::string serialize() const;  // declare a method as "const" when it doesn't
                                  // modify any of the class's member variables.
  void deserialize(std::string data);
};

#endif
