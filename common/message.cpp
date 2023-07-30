#include "message.h"

#include "../vendor/nlohmann/json.hpp"

#include <string>

Message::Message(const std::string username, const std::string body)
    : username(username), body(body) {}

std::string Message::serialize() {
  nlohmann::json j;
  j["username"] = username;
  j["body"] = body;
  return j.dump();
}

void Message::deserialize(std::string data) {
  auto jsonData = nlohmann::json::parse(data);
  username = jsonData["username"];
  body = jsonData["body"];
}
