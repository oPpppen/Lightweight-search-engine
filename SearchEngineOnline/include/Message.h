#pragma once

#include <cstdint>
#include <string>

struct Message {
    uint8_t type = 0;
    std::string value;
};
