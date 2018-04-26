#ifndef __SHADOWSOCKS_SERVER_INCLUDED__
#define __SHADOWSOCKS_SERVER_INCLUDED__
#pragma once


/* other shadowsocks headers */
#include "shadowsocks/ss_network.h"


/* `Ss_Server` class */
class Ss_Server {
    public:
        Ss_Server();
        ~Ss_Server();

    public:
        bool createDaemon();

    private:
        Ss_Network *_network;
};


#endif // __SHADOWSOCKS_SERVER_INCLUDED__
