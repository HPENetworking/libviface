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

/**
 * @file viface.hpp
 * Main libviface header file.
 * Define the public interface for libviface.
 */

#ifndef _VIFACE_HPP
#define _VIFACE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "viface/config.hpp"

namespace viface
{
/**
 * @defgroup libviface Public Interface
 * @{
 */

class VIfaceImpl;

/**
 * Virtual Interface object.
 *
 * Creates a virtual network interface.
 */
class VIface
{
    private:

        std::unique_ptr<VIfaceImpl> pimpl;
        VIface(const VIface& other) = delete;
        VIface& operator=(VIface rhs) = delete;

    public:

        /**
         * Create a VIface object with given name.
         *
         * @param[in]  name Name of the virtual interface.
         * @param[in]  tap Tap device (default, true) or Tun device (false).
         * @param[in]  id Optional numeric id. If given id < 0 a sequential
         *             number will be given.
         */
        VIface(std::string name = "viface%d", bool tap = true, int id = -1);
        ~VIface();

        /**
         * Getter method for virtual interface associated name.
         *
         * @return the name of the virtual interface.
         */
        std::string getName() const;

        /**
         * Getter method for virtual interface associated numeric id.
         *
         * @return the numeric id of the virtual interface.
         */
        uint getID() const;

        /**
         * Set the MAC address of the virtual interface.
         *
         * The format of the MAC address is verified, but is just until up()
         * is called that the library will try to attempt to write it.
         * If you don't provide a MAC address (the default) one will be
         * automatically assigned when bringing up the interface.
         *
         * @param[in]  mac New MAC address for this virtual interface in the
         *             form "d8:9d:67:d3:65:1f".
         *
         * @return always void.
         *         An exception is thrown in case of malformed argument.
         */
        void setMAC(std::string mac = "");

        /**
         * Getter method for virtual interface associated MAC Address.
         *
         * @return the current MAC address of the virtual interface.
         *         An empty string means no associated MAC address.
         */
        std::string getMAC() const;

        /**
         * Set the IPv4 address of the virtual interface.
         *
         * The format of the IPv4 address is verified, but is just until up()
         * is called that the library will try to attempt to write it.
         *
         * @param[in]  ipv4 New IPv4 address for this virtual interface in the
         *             form "172.17.42.1".
         *
         * @return always void.
         *         An exception is thrown in case of malformed argument.
         */
        void setIPv4(std::string ipv4 = "");

        /**
         * Getter method for virtual interface associated IPv4 Address.
         *
         * @return the current IPv4 address of the virtual interface.
         *         An empty string means no associated IPv4 address.
         */
        std::string getIPv4() const;

        /**
         * Set the IPv6 address of the virtual interface.
         *
         * The format of the IPv4 address is verified, but is just until up()
         * is called that the library will try to attempt to write it.
         * If you don't provide a IPv6 address (the default) one may be
         * automatically assigned when bringing up the interface
         * (depending on your system configuration).
         *
         * @param[in]  ipv6 New IPv6 address for this virtual interface in the
         *             form "2001:0DB8:AC10:FE01::".
         *
         * @return always void.
         *         An exception is thrown in case of malformed argument.
         */
        void setIPv6(std::string ipv6 = "");

        /**
         * Getter method for virtual interface associated IPv6 Address.
         *
         * @return the current IPv6 address of the virtual interface.
         *         An empty string means no associated IPv6 address.
         */
        std::string getIPv6() const;

        /**
         * Set the MTU of the virtual interface.
         *
         * The range of the MTU is verified, but is just until up() is called
         * that the library will try to attempt to write it.
         *
         * @param[in]  mtu New MTU for this virtual interface.
         *
         * @return always void.
         *         An exception is thrown in case of bad range MTU
         *         ([68, 65536]).
         */
        void setMTU(uint mtu = 1500);

        /**
         * Getter method for virtual interface associated maximum transmission
         * unit (MTU).
         *
         * MTU is the size of the largest packet or frame that can be sent
         * using this interface.
         *
         * @return the current MTU of the virtual interface.
         */
        uint getMTU() const;

        /**
         * Bring up the virtual interface.
         *
         * This call will configure and bring up the interface.
         *
         * @return always void.
         *         Exceptions are thrown in case of configuration or bring-up
         *         failures.
         */
        void up();

        /**
         * Bring down the virtual interface.
         *
         * @return always void.
         *         Exceptions are thrown in case of misbehaviours.
         */
        void down() const;

        /**
         * Getter method for virtual interface associated maximum transmission
         * unit (MTU).
         *
         * MTU is the size of the largest packet or frame that can be sent
         * using this interface.
         *
         * @return the current MTU of the virtual interface.
         */
        bool isUp() const;

        /**
         * Receive a packet from the virtual interface.
         *
         * Note: Receive a packet from a virtual interface means that another
         * userspace program (or the kernel) sent a packet to the network
         * interface with the name of the instance of this class.
         *
         * @param[in]  timeout Operation timeout. If timeout < 0, wait
         *             indefinitely; if timeout == 0 do not block and return
         *             immediately, in such case, the vector will be empty if
         *             no packet were available. Finally, if timeout > 0 wait
         *             up to that amount of milliseconds has passed. In this
         *             case, if the operation timed-out, the vector will also
         *             be empty.
         *
         * @return the packet (if tun) or frame (if tap) as a binary blob
         *         (array of bytes).
         */
        std::vector<uint8_t> receive(int timeout = 0) const;

        /**
         * Send a packet from this virtual interface.
         *
         * Note: Sending a packet from this virtual interface means that it
         * will reach any userspace program (or the kernel) listening for
         * packets in the interface with the name of the instance of this
         * class.
         *
         * @param[in]  packet Packet (if tun) or frame (if tap) to send as a
         *             binary blob (array of bytes).
         *
         * @return always void.
         *         Exceptions are thrown in case of misbehaviours. For example,
         *         size of the packet will be checked against MTU and minimal
         *         Ethernet 2 size. Another case can be write errors.
         */
        void send(std::vector<uint8_t>& packet) const;
};

/** @} */ // End of libviface
};
#endif // _VIFACE_HPP