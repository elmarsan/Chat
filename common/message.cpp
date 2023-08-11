#include "message.h"

#include <ctime>
#include <string>

#include "../vendor/nlohmann/json.hpp"

Message::Message(std::string username, std::string body)
    : username(std::move(username)), body(std::move(body)) {
  time = std::time(NULL);
}

std::string Message::serialize() const {
  nlohmann::json j;
  j["username"] = username;
  j["body"] = body;
  j["connected_users"] = connectedUsers;
  j["time"] = time;
  return j.dump();
}

void Message::deserialize(std::string data) {
  auto jsonData = nlohmann::json::parse(data);
  username = jsonData["username"];
  body = jsonData["body"];
  connectedUsers = jsonData["connected_users"];
  time = jsonData["time"];
}
