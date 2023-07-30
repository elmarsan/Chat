#include "message.h"

#include <gtest/gtest.h>

TEST(MessageTest, Serialize) {
  Message message("Bob", "Hello chat!");
  std::time_t timestamp = 1690718901;
  message.time = timestamp;
  message.connectedUsers = {"Alice", "Paco"};
  auto serialized = message.serialize();
  EXPECT_STREQ(serialized.c_str(),
               "{\"body\":\"Hello "
               "chat!\",\"connected_users\":[\"Alice\",\"Paco\"],\"time\":"
               "1690718901,\"username\":\"Bob\"}");
}

TEST(MessageTest, Deserialize) {
  std::string serialized =
      "{\"body\":\"Hello "
      "chat!\",\"connected_users\":[\"Alice\",\"Paco\"],\"time\":1690718901,"
      "\"username\":\"Bob\"}";
  Message message;
  message.deserialize(serialized);
  EXPECT_STREQ("Bob", message.username.c_str());
  EXPECT_STREQ("Hello chat!", message.body.c_str());
  EXPECT_EQ(message.time, 1690718901);
  EXPECT_EQ(message.connectedUsers.size(), 2);
  EXPECT_STREQ(message.connectedUsers[0].c_str(), "Alice");
  EXPECT_STREQ(message.connectedUsers[1].c_str(), "Paco");
}
