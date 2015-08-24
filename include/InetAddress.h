#pragma once

/**
 * InetAddress just support ipv4
 * @author xiao
 */

#include <cstdint>

namespace ECor
{

class InetAddress
{
public:
    enum NetType { IPV4 };
private:
    NetType type;
    //All in network byte order
    union
    {
        struct
        {
            uint16_t port;
            uint32_t ip;
        };
    };

public:

    InetAddress();
    InetAddress(uint32_t _ip, uint16_t _port);
    InetAddress(char* _host, uint16_t _port, NetType _type);

    NetType getType() const;
    uint16_t getPort() const;
    uint32_t getIp() const;
};

}
