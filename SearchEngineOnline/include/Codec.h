#pragma once

#include "Message.h"

#include <muduo/net/Buffer.h>

#include <cstdint>
#include <string>

class Codec {
public:
    static void encode(uint8_t type, const std::string& value, std::string& out);
    static bool tryDecode(muduo::net::Buffer* buffer, Message& msg);

private:
    static constexpr std::size_t kHeaderSize = sizeof(uint8_t) + sizeof(uint32_t);
};
