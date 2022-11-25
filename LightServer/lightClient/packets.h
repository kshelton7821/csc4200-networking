#ifndef PACKETS_H
#define PACKETS_H

#include <stdio.h>
#include <string.h>
#include <iostream>

struct Packet
{
    uint32_t version;
    uint32_t type;
    uint32_t length;
    uint8_t message[10];
};

#endif // PACKETS_H