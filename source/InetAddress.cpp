#include "InetAddress.h"

#include <cstring>

#include <arpa/inet.h>

namespace ECor
{

InetAddress::InetAddress()
{
}

InetAddress::InetAddress(uint32_t _ip, uint16_t _port)
    : type(InetAddress::IPV4), port(_port), ip(_ip)
{
}

InetAddress::InetAddress(char* _ip, uint16_t _port, NetType _type)
    : type(_type), port(_port)
{
    inet_pton(AF_INET, _ip, &ip);
}

InetAddress::NetType InetAddress::getType() const
{
    return type;
}

uint16_t InetAddress::getPort() const
{
    return port;
}

uint32_t InetAddress::getIp() const
{
    return ip;
}

}
