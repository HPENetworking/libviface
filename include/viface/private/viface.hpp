/**
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _VIFACE_PRIV_HPP
#define _VIFACE_PRIV_HPP

// Standard
#include <stdexcept>   // Exceptions
#include <sstream>     // ostringstream
#include <iomanip>     // setw
#include <map>         // map

// C
#include <cstring>     // memset
#include <cerrno>      // EAGAIN

// Posix
#include <unistd.h>    // open(), close()
#include <fcntl.h>     // O_RDWR

// Network
#include <arpa/inet.h> // inet_ntop()

// Linux TUN/TAP includes
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>

// Framework
#include "viface/viface.hpp"

using namespace std;
using namespace viface;

namespace viface
{
typedef struct
{
    int rx;
    int tx;
} viface_queues_t;

class VIfaceImpl
{
    private:

        viface_queues_t queues;
        int kernel_socket;
        vector<uint8_t> pktbuff;

        string name;
        uint id;
        string mac;
        string ipv4;
        uint mtu;

        static uint idseq;

    public:

        VIfaceImpl(string name, bool tap, int id);

        string getName() const
        {
            return this->name;
        }

        uint getID() const
        {
            return this->id;
        }

        int getTX() const
        {
            return this->queues.tx;
        }

        int getRX() const
        {
            return this->queues.rx;
        }

        void setMAC(string mac);

        string getMAC() const;

        void setIPv4(string ipv4);

        string getIPv4() const;

        void setMTU(uint mtu);

        uint getMTU() const;

        void up();

        void down() const;

        bool isUp() const;

        vector<uint8_t> receive();

        void send(vector<uint8_t>& packet) const;
};
};
#endif // _VIFACE_PRIV_HPP