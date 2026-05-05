#include "Codec.h"

#include <arpa/inet.h>

#include <cstring>

void Codec::encode(uint8_t type, const std::string& value, std::string& out)
{
    out.clear();
    out.reserve(kHeaderSize + value.size());

    out.push_back(static_cast<char>(type));

    uint32_t length = htonl(static_cast<uint32_t>(value.size()));
    out.append(reinterpret_cast<const char*>(&length), sizeof(length));
    out.append(value);
}

bool Codec::tryDecode(muduo::net::Buffer* buffer, Message& msg)
{
    if (buffer->readableBytes() < kHeaderSize) {
        return false;
    }

    const char* data = buffer->peek();
    const uint8_t type = static_cast<uint8_t>(data[0]);

    uint32_t networkLength = 0;
    std::memcpy(&networkLength, data + sizeof(uint8_t), sizeof(networkLength));
    const uint32_t length = ntohl(networkLength);

    if (buffer->readableBytes() < kHeaderSize + length) {
        return false;
    }

    buffer->retrieve(kHeaderSize);

    msg.type = type;
    msg.value = buffer->retrieveAsString(length);
    return true;
}
