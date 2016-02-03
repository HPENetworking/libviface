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
 * @file viface.h
 * Main libviface header file.
 * Define the public interface for libviface.
 */

#ifndef _VIFACE_H
#define _VIFACE_H

#ifdef __cplusplus
extern "C" {
#endif

// Posix
#include <unistd.h>    // open(), close()
#include <fcntl.h>     // O_RDWR
#include <dirent.h>    // opendir, readdir, closedir

// Network
#include <arpa/inet.h> // inet_ntop()

// Linux TUN/TAP includes
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>

// Interfaces
#include <ifaddrs.h>   // getifaddrs

// Standard
#include <stdbool.h>   // bool
#include <stdlib.h>    // EXIT_FAILURE

// APR
#include <apr_hash.h>  // apr hash
#include <ctype.h>       // isprint
#include <apr_ring.h>    // apr ring
#include <apr_strings.h> // apr_cpystrn
#define APR_WANT_STDIO
#include <apr_want.h>


/*= Structs Declarations =====================================================*/

struct in6_ifreq
{
    struct in6_addr ifr6_addr;
    uint32_t ifr6_prefixlen;
    int ifr6_ifindex;
};

struct viface_queues
{
    int rx;
    int tx;
};

struct viface
{
    struct viface_queues queues;
    int kernel_socket;
    int kernel_socket_ipv6;
    uint id;
    uint mtu;
    uint8_t* pktbuff;
    char broadcast[INET_ADDRSTRLEN];
    char ipv4[INET_ADDRSTRLEN];
    char name[IFNAMSIZ];
    char netmask[INET_ADDRSTRLEN];
    char mac[18];
    char** ipv6s;
    apr_hash_t* stats_cache;
    apr_hash_t* stats_keys_cache;
    apr_pool_t* viface_pool;
};

// Vifaces sequencial identifier number
static uint ID_SEQ = 0;

// APR parent pool
static apr_pool_t* PARENT_POOL = NULL;

// APR temporal pool
static apr_pool_t* TEMPORAL_POOL = NULL;

/**
 * Dispatch callback type to handle packet reception.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  name Name of the virtual interface that received the packet.
 * @param[in]  id Numeric ID assigned to the virtual interface.
 * @param[in]  packet Packet (if tun) or frame (if tap) as a binary blob
 *             (array of bytes).
 * @param[out] true if the dispatcher should continue processing or false to stop.
 *
 * @return status indicating if an error happened or not.
 */
typedef int (*dispatcher_cb)(struct viface* self, char* name, uint id,
                             uint8_t* packet, bool* result);

/*= Utilities ================================================================*/

/**
 * Parse a MAC address in string format.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  mac MAC address to parse in format "56:84:7a:fe:97:99".
 * @param[out] A vector of 6 bytes with the parsed MAC address.
 *
 * @return status indicating if an error happened or not.
 */
int  viface_parse_mac(struct viface* self, char* mac, uint8_t** result);

/**
 * Build a hexdump representation of a binary blob.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  bytes Binary blob (array of bytes) with the data to build
 *             representation.
 * @param[out] string with the hexdump representation of the input binary blob.
 *
 * @return status indicating if an error happened or not.
 */
int viface_hex_dump(struct viface* self, uint8_t* bytes, char** result);

/**
 * Calculate the 32 bit CRC of the given binary blob.
 *
 * @param[in]  bytes Binary blob (array of bytes) with the data to calculate
 *             the 32 bit CRC.
 * @param[out] string with the hexdump representation of the input binary blob.
 *
 * @return status indicating if an error happened or not.
 */
int viface_crc_32(uint8_t* bytes, uint32_t* result);

/*= Virtual Interface Implementation =========================================*/

/**
 * Create an APR parent pool for memory allocation.
 *
 * @return status indicating if an error happened or not.
 */
int viface_create_global_pool();

/**
 * Create a viface struct and an APR subpool from a parent pool.
 *
 * @param[in]  name Name of the virtual interface. The placeholder %d
 *             can be used and a number will be assigned to it.
 * @param[in]  tap Tap device (default, true) or Tun device (false).
 * @param[in]  id Optional numeric id. If given id < 0 a sequential
 *             number will be given.
 * @param[out] viface struct created
 *
 * @return status indicating if an error happened or not.
 */
int viface_create_viface(char* name, bool tap, int id, struct viface** result);

/**
 * Destroy the static global APR parent pool.
 *
 * @return status indicating if an error happened or not.
 */
int viface_destroy_global_pool();

/**
 * Destroy the static temporal APR pool.
 *
 * @return status indicating if an error happened or not.
 */
int viface_destroy_temporal_pool();

/**
 * Destroy a viface struct and free all allocated memory from the APR subpool
 *
 * @param[in]  viface struct containing its state.
 *
 * @return status indicating if an error happened or not.
 */
int viface_destroy_viface(struct viface** self);

/**
 * Create a viface object with given name.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  name Name of the virtual interface. The placeholder %d
 *             can be used and a number will be assigned to it.
 * @param[in]  tap Tap device (default, true) or Tun device (false).
 * @param[in]  id Optional numeric id. If given id < 0 a sequential
 *             number will be given.
 *
 * @return status indicating if an error happened or not.
 */
int viface_initialization_viface(struct viface* self, char* name, bool tap,
                                 int id);

/**
 * Getter method for virtual interface associated name.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] name of the virtual interface.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_name(struct viface* self, char** result);

/**
 * Getter method for virtual interface associated numeric id.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] numeric id of the virtual interface.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_id(struct viface* self, uint* result);

/**
 * Getter method for virtual interface associated
 * transmission file descriptor.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] transmission file descriptor number.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_tx(struct viface* self, int* result);

/**
 * Getter method for virtual interface associated
 * reception file descriptor.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] reception file descriptor number.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_rx(struct viface* self, int* result);

/**
 * Set the MAC address of the virtual interface.
 *
 * The format of the MAC address is verified, but is just until up()
 * is called that the library will try to attempt to write it.
 * If you don't provide a MAC address (the default) one will be
 * automatically assigned when bringing up the interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  mac New MAC address for this virtual interface in the
 *             form "d8:9d:67:d3:65:1f".
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_mac(struct viface* self, char* mac);

/**
 * Getter method for virtual interface associated MAC Address.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current MAC address of the virtual interface.
 *             An empty string means no associated MAC address.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_mac(struct viface* self, char** result);

/**
 * Auxiliar method to get IPv4 Address, Netmask and Broadcast
 * of the virtual interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  code indicating which IPv4 value to read
 *             (Address, Netmask, Broadcast)
 * @param[out] current IPv4 address, netmask or broadcast of the virtual
 *             interface depending on the request value.
 *             An empty string means no associated IPv4 address.
 * @return status indicating if an error happened or not.
 */
int viface_ioctl_get_ipv4(struct viface* self, unsigned long request,
                          char** result);

/**
 * Set the IPv4 address of the virtual interface.
 *
 * The format of the IPv4 address is verified, but is just until up()
 * is called that the library will try to attempt to write it.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  ipv4 New IPv4 address for this virtual interface in the
 *             form "172.17.42.1".
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_ipv4(struct viface* self, char* ipv4);

/**
 * Getter method for virtual interface associated IPv4 Address.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current IPv4 address of the virtual interface.
 *             An empty string means no associated IPv4 address.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_ipv4(struct viface* self, char** result);

/**
 * Set the IPv4 netmask of the virtual interface.
 *
 * The format of the IPv4 netmask is verified, but is just until up()
 * is called that the library will try to attempt to write it.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  netmask New IPv4 netmask for this virtual interface in
 *             the form "255.255.255.0".
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_ipv4_netmask(struct viface* self, char* netmask);

/**
 * Getter method for virtual interface associated IPv4 netmask.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current IPv4 netmask of the virtual interface.
 *             An empty string means no associated IPv4 netmask.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_ipv4_netmask(struct viface* self, char** result);

/**
 * Set the IPv4 broadcast address of the virtual interface.
 *
 * The format of the IPv4 broadcast address is verified, but is just
 * until up() is called that the library will try to attempt to write
 * it.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  broadcast New IPv4 broadcast address for this virtual
 *             interface in the form "172.17.42.255".
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_ipv4_broadcast(struct viface* self, char* broadcast);

/**
 * Getter method for virtual interface associated IPv4 broadcast
 * address.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current IPv4 broadcast address of the virtual interface.
 *             An empty string means no associated IPv4 broadcast address.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_ipv4_broadcast(struct viface* self, char** result);

/**
 * Set the IPv6 addresses of the virtual interface.
 *
 * The format of the IPv6 addresses are verified, but is just until
 * up() is called that the library will try to attempt to write them.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  number of IPv6s to set
 * @param[in]  ipv6s New IPv6 addresses for this virtual interface in
 *             the form "::FFFF:204.152.189.116".
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_ipv6(struct viface* self, int num_ipv6s, char* ipv6s[]);

/**
 * Getter method for virtual interface associated IPv6 Addresses
 * (note the plural).
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current IPv6 addresses of the virtual interface.
 *             An empty set means no associated IPv6 addresses.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_ipv6(struct viface* self, char** result[]);

/**
 * Set the MTU of the virtual interface.
 *
 * The range of the MTU is verified, but is just until up() is called
 * that the library will try to attempt to write it.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  mtu New MTU for this virtual interface.
 *
 * @return status indicating if an error happened or not.
 */
int viface_set_mtu(struct viface* self, uint mtu);

/**
 * Getter method for virtual interface associated maximum transmission
 * unit (MTU).
 *
 * MTU is the size of the largest packet or frame that can be sent
 * using this interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] current MTU of the virtual interface.
 *
 * @return status indicating if an error happened or not.
 */
int viface_get_mtu(struct viface* self, uint* mtu);

/**
 * Bring up the virtual interface.
 *
 * This call will configure and bring up the interface.
 *
 * @param[in]  viface struct containing its state.
 *
 * @return status indicating if an error happened or not.
 */
int viface_up(struct viface* self);

/**
 * Bring down the virtual interface.
 *
 *
 * @param[in]  viface struct containing its state.
 *
 * @return status indicating if an error happened or not.
 */
int viface_down(struct viface* self);

/**
 * Indicate if the virtual interface is up.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] value indicating if the virtual interface
 *             is up or not.
 *
 * @return status indicating if an error happened or not.
 */
int viface_is_up(struct viface* self, bool* result);

/**
 * Receive a packet from the virtual interface.
 *
 * Note: Receive a packet from a virtual interface means that another
 * userspace program (or the kernel) sent a packet to the network
 * interface with the name of the instance of this class. If not packet
 * was available, and empty vector is returned.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] packet (if tun) or frame (if tap) as a binary blob
 *             (array of bytes).
 *
 * @return status indicating if an error happened or not.
 */
int viface_receive(struct viface* self, uint8_t** result);

/**
 * Send a packet to this virtual interface.
 *
 * Note: Sending a packet to this virtual interface means that it
 * will reach any userspace program (or the kernel) listening for
 * packets in the interface with the name of the instance of this
 * class.
 *
 * @param[in]  viface struct containing its state.
 * @param[in] packet (if tun) or frame (if tap) as a binary blob
 *            (array of bytes).
 *
 * @return status indicating if an error happened or not.
 */
int viface_send(struct viface* self, uint8_t* packet);

/**
 * List available statistics for this interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[out] set of statistics names.
 *             For example, {"rx_packets", "tx_bytes", ...}
 *             Exceptions can be thrown in case of generic IO errors.
 *
 * @return status indicating if an error happened or not.
 */
int viface_list_stats(struct viface* self, char** result[]);

/**
 * Read given statistic for this interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  stat statistic name. See listStats().
 * @param[out] current value of the given statistic.
 *
 * @return status indicating if an error happened or not.
 */
int viface_read_stat_file(struct viface* self, char* stat, uint64_t* result);

/**
 * Read given statistic for this interface.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  stat statistic name. See listStats().
 * @param[out] current value of the given statistic.
 *
 * @return status indicating if an error happened or not.
 */
int viface_read_stat(struct viface* self, char* stat, uint64_t* result);

/**
 * Clear given statistic for this interface.
 *
 * Please note that this feature is implemented in library as there is
 * currently no way to clear them (except by destroying the interface).
 * If you clear the statistics using this method, subsequent calls to
 * readStat() results will differ from, for example, those reported
 * by tools like ifconfig.
 *
 * @param[in]  viface struct containing its state.
 * @param[in]  stat statistic name. See listStats().
 *
 * @return status indicating if an error happened or not.
 */
int viface_clear_stat(struct viface* self, char* stat);

#ifdef __cplusplus
}
#endif

#endif // _VIFACE_H