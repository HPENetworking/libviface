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
//#include <stdexcept>
//#include <sstream>

// Framework
#include "viface/viface.hpp"

using namespace std;
using namespace viface;

namespace viface
{
class VIfaceImpl
{
    private:

        string name;
        bool tap;
        uint id;
        string mac;
        string ipv4;
        string ipv6;
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

        void setMac(std::string mac);

        std::string getMac() const;

        void setIPv4(std::string ipv4);

        std::string getIPv4() const;

        void setIPv6(std::string ipv6);

        std::string getIPv6() const;

        void setMTU(uint mtu);

        uint getMTU() const;

        void up() const;

        void down() const;

        bool isUp() const;

        std::vector<uint8_t> receive(int timeout) const;

        void send(std::vector<uint8_t>& packet) const;
};
};
#endif // _VIFACE_PRIV_HPP