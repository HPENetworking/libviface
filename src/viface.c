/**
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use self file except in compliance with the License.
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

#include "viface/viface.h"

/*= Utilities ================================================================*/

int  viface_parse_mac(struct viface* self, char* mac, uint8_t** result)
{
    unsigned int bytes[6];
    int scans = sscanf(
        mac,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]
        );

    if (scans != 6) {
        fprintf(stdout, "---- Invalid MAC address %s.\n", mac);
        return EXIT_FAILURE;
    }

    int i = 0;
    uint8_t *parsed = apr_pcalloc(TEMPORAL_POOL, 6);

    if (parsed == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " parsing mac address.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    for (i = 0; i < 6; i++) {
        parsed[i] = bytes[i];
    }

    *result = parsed;
    return EXIT_SUCCESS;
}

int viface_hex_dump(struct viface* self, uint8_t* bytes, char** result)
{
    const int BYTES_PER_LINE = 72;

    char buffer[10];
    int number_lines = (bytes[0] / 16) + 1;

    *result = apr_pcalloc(TEMPORAL_POOL, BYTES_PER_LINE * number_lines);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'viface_hex_dump' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    memset(*result, '\0', BYTES_PER_LINE * number_lines);

    int i;
    int j;
    char c;

    for (i = 0, j = 0; i < bytes[0]; i += 16) {
        // Print offset
        sprintf(buffer, "%.4d  ", i);
        strcat(*result, buffer);

        for (j = 0; j < 16; j++) {
            if (i + j < bytes[0]) {
                c = bytes[i + j + 1];
                sprintf(buffer, "%.2x ", ((int)c & 0xFF));
                strcat(*result, buffer);
            } else {
                strcat(*result, "   ");
            }
        }
        strcat(*result, " ");

        // Print printable characters
        for (j = 0; j < 16; j++) {
            if (i + j < bytes[0]) {
                c = bytes[i + j + 1];
                if (isprint(c)) {
                    sprintf(buffer, "%c", c);
                    strcat(*result, buffer);
                } else {
                    strcat(*result, ".");
                }
            }
        }
        strcat(*result, "\n");
    }
    return EXIT_SUCCESS;
}

int viface_crc_32(uint8_t* bytes, uint32_t* result)
{
    static uint32_t crc_table[] = {
        0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
        0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
        0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
        0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
    };

    uint32_t crc = 0;
    uint32_t i = 0;
    const uint8_t* data = &bytes[1];

    for (i = 0; i < bytes[0]; ++i) {
        crc = (crc >> 4) ^ crc_table[(crc ^ data[i]) & 0x0F];
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[i] >> 4)) & 0x0F];
    }
    *result = crc;
    return EXIT_SUCCESS;
}

/*= Helpers ==================================================================*/

static int viface_read_flags(int sock_fd, char* name, struct ifreq* ifr)
{
    // Prepare communication structure
    memset(ifr, 0, sizeof(struct ifreq));

    // Set interface name
    apr_cpystrn(ifr->ifr_name, name, IFNAMSIZ - 1);

    // Read interface flags
    if (ioctl(sock_fd, SIOCGIFFLAGS, ifr) != 0) {
        fprintf(stdout, "--- Unable to read %s flags.\n", name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


static int viface_read_mtu(char* name, size_t size_bytes, uint* result)
{
    const int SIZE_BYTES_PATH = 19;

    int fd = -1;
    int nread = -1;
    int size_bytes_mtu_path = SIZE_BYTES_PATH + strlen(name);
    char buffer[size_bytes + 1];

    char mtu_path[size_bytes_mtu_path + 1];
    memset(&mtu_path, 0, size_bytes_mtu_path);

    snprintf(mtu_path, sizeof(mtu_path), "%s%s%s",
             "/sys/class/net/", name, "/mtu");

    mtu_path[size_bytes_mtu_path] = '\0';

    // Opens MTU file
    fd = open(mtu_path,
              O_RDONLY | O_NONBLOCK);

    if (fd < 0) {
        fprintf(stdout, "--- Unable to open MTU file for");
        fprintf(stdout, " '%s' network interface.\n", name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        goto err;
    }

    // Reads MTU value
    nread = read(fd, &buffer, size_bytes);
    buffer[size_bytes] = '\0';

    // Handles errors
    if (nread == -1) {
        fprintf(stdout, "--- Unable to read MTU for");
        fprintf(stdout, " '%s' network interface.\n", name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        goto err;
    }

    if (close(fd) < 0) {
        fprintf(stdout, "--- Unable to close MTU file for");
        fprintf(stdout, " '%s' network interface.\n", name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        goto err;
    }

    *result = strtoul(buffer, NULL, 10);
    return EXIT_SUCCESS;

err:
    // Rollback close file descriptor
    close(fd);
    return EXIT_FAILURE;
}

static int viface_alloc_viface(struct viface* self, char* name, bool tap,
                               struct viface_queues* queues, char* result)
{
    int i = 0;
    int fd = -1;

    /* Create structure for ioctl call
     *
     * Flags: IFF_TAP   - TAP device (layer 2, ethernet frame)
     *        IFF_TUN   - TUN device (layer 3, IP packet)
     *        IFF_NO_PI - Do not provide packet information
     *        IFF_MULTI_QUEUE - Create a queue of multiqueue device
     */
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_NO_PI | IFF_MULTI_QUEUE;
    if (tap) {
        ifr.ifr_flags |= IFF_TAP;
    } else {
        ifr.ifr_flags |= IFF_TUN;
    }

    apr_cpystrn(ifr.ifr_name, name, IFNAMSIZ - 1);

    // Allocate queues
    for (i = 0; i < 2; i++) {
        // Open TUN/TAP device
        fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            fprintf(stdout, "--- Unable to open TUN/TAP device.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            goto err;
        }

        // Register a network device with the kernel
        if (ioctl(fd, TUNSETIFF, (void* )&ifr) != 0) {
            fprintf(stdout, "--- Unable to register a TUN/TAP device.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            goto err;

            if (close(fd) < 0) {
                fprintf(stdout, "--- Unable to close a TUN/TAP device.\n");
                fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
                goto err;
            }
            goto err;
        }

        ((int* )queues)[i] = fd;
    }

    apr_cpystrn(result, ifr.ifr_name, strlen(ifr.ifr_name) + 1);
    return EXIT_SUCCESS;

err:
    // Rollback close file descriptors
    for (--i; i >= 0; i--) {
        if (close(((int* )queues)[i]) < 0) {
            fprintf(stdout, "--- Unable to close a TUN/TAP device.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
    }
    return EXIT_FAILURE;
}

static int viface_hook_viface(char* name, struct viface_queues* queues)
{
    int i = 0;
    int fd = -1;

    // Creates Tx/Rx sockets and allocates queues
    for (i = 0; i < 2; i++) {
        // Creates the socket
        fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if (fd < 0) {
            fprintf(stdout, "--- Unable to create the Tx/Rx socket channel.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            goto err;
        }

        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));

        apr_cpystrn(ifr.ifr_name, name, IFNAMSIZ - 1);

        // Obtains the network index number
        if (ioctl(fd, SIOCGIFINDEX, &ifr) != 0) {
            fprintf(stdout, "--- Unable to get network index number.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            goto err;
        }

        struct sockaddr_ll socket_addr;
        memset(&socket_addr, 0, sizeof(struct sockaddr_ll));

        socket_addr.sll_family = AF_PACKET;
        socket_addr.sll_protocol = htons(ETH_P_ALL);
        socket_addr.sll_ifindex = ifr.ifr_ifindex;

        // Binds the socket to the 'socket_addr' address
        if (bind(fd, (struct sockaddr*) &socket_addr,
                 sizeof(socket_addr)) != 0) {
            fprintf(stdout, "--- Unable to bind the Tx/Rx socket channel to");
            fprintf(stdout, " the '%s' network interface.\n", name);
            fprintf(stdout, "    Queue: %d.\n", i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            goto err;
        }

        ((int* )queues)[i] = fd;
    }
    return EXIT_SUCCESS;

err:
    // Rollback close file descriptors
    for (--i; i >= 0; i--) {
        if (close(((int* )queues)[i]) < 0) {
            fprintf(stdout, "--- Unable to close a Rx/Tx socket.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
    }
    return EXIT_FAILURE;
}

static int viface_is_empty(char* buffer, bool* result)
{
    *result = true;

    if ((buffer != NULL) && (buffer[0] != '\0')) {
        *result = false;
    }
    return EXIT_SUCCESS;
}

/*= Virtual Interface Implementation =========================================*/

int viface_create_global_pool()
{
    apr_initialize();

    // Creates parent and temporal pools
    apr_pool_create(&PARENT_POOL, NULL);
    apr_pool_create(&TEMPORAL_POOL, PARENT_POOL);
}

int viface_create_viface(char* name, bool tap, int id, struct viface** result)
{
    apr_pool_t* viface_pool = NULL;
    apr_pool_create(&viface_pool, PARENT_POOL);

    struct viface* self = apr_pcalloc(viface_pool, sizeof(struct viface));

    if (self == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " 'viface' struct.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        viface_destroy_viface(&self);
        return EXIT_FAILURE;
    }

    self->viface_pool = viface_pool;

    self->stats_keys_cache = apr_hash_make(self->viface_pool);
    self->stats_cache = apr_hash_make(self->viface_pool);

    if ((self->stats_keys_cache == NULL) ||
        (self->stats_cache == NULL)) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " APR hash table.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        viface_destroy_viface(&self);
        return EXIT_FAILURE;
    }
    *result = self;

    if (viface_initialization_viface(self, name, tap, id) == EXIT_FAILURE) {
        viface_destroy_viface(&self);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int viface_destroy_global_pool()
{
    apr_pool_destroy(PARENT_POOL);
    apr_terminate();
    return EXIT_SUCCESS;
}

int viface_destroy_temporal_pool()
{
    apr_pool_destroy(TEMPORAL_POOL);
    return EXIT_SUCCESS;
}

int viface_destroy_viface(struct viface** self)
{
    if (*self == NULL) {
        fprintf(stdout, "--- Error destroying viface struct");
        return EXIT_FAILURE;
    }

    if ((close((*self)->queues.rx)) ||
        (close((*self)->queues.tx)) ||
        (close((*self)->kernel_socket)) ||
        (close((*self)->kernel_socket_ipv6))) {
        fprintf(stdout, "--- Unable to close file descriptors for");
        fprintf(stdout, " interface %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    apr_pool_destroy((*self)->viface_pool);
    *self = NULL;

    return EXIT_SUCCESS;
}

int viface_initialization_viface(struct viface* self, char* name,
                                 bool tap, int id)
{
    // Check name length
    if (strlen(name) >= IFNAMSIZ) {
        fprintf(stdout, "--- Virtual interface name too long.\n");
        return EXIT_FAILURE;
    }

    // Create queues
    struct viface_queues queues;
    memset(&queues, 0, sizeof(struct viface_queues));

    int size_bytes_viface_path = strlen("/sys/class/net/") + strlen(name);

    char viface_path[size_bytes_viface_path + 1];
    memset(&viface_path, 0, size_bytes_viface_path);

    snprintf(viface_path, sizeof(viface_path), "%s%s",
             "/sys/class/net/", name);

    /* Checks if the path name can be accessed. If so,
     * it means that the network interface is already defined.
     */
    if (access(viface_path, F_OK) == 0) {
        if (viface_hook_viface(name, &queues) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        apr_cpystrn(self->name, name, strlen(name) + 1);

        // Read MTU value and resize buffer
        if (viface_read_mtu(name, sizeof(self->mtu),
                            &self->mtu) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        self->pktbuff = apr_pcalloc(self->viface_pool, sizeof(self->mtu));

        if (self->pktbuff == NULL) {
            fprintf(stdout, "--- Memory could not be allocated for");
            fprintf(stdout, " pktbuff in %s interface.\n", name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    } else {
        char viface_name[IFNAMSIZ];
        if (viface_alloc_viface(self, name, tap, &queues,
                                viface_name) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        apr_cpystrn(self->name, viface_name, strlen(viface_name) + 1);

        // Other defaults
        self->mtu = 1500;
    }

    self->queues = queues;

    // Create socket channels to the NET kernel for later ioctl
    self->kernel_socket = -1;
    self->kernel_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (self->kernel_socket < 0) {
        fprintf(stdout, "--- Unable to create IPv4 socket channel to the");
        fprintf(stdout, " NET kernel.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    self->kernel_socket_ipv6 = -1;
    self->kernel_socket_ipv6 = socket(AF_INET6, SOCK_STREAM, 0);
    if (self->kernel_socket_ipv6 < 0) {
        fprintf(stdout, "--- Unable to create IPv6 socket channel to the");
        fprintf(stdout, " NET kernel.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Set id
    if (id < 0) {
        self->id = ID_SEQ;
    } else {
        self->id = id;
    }

    ID_SEQ++;
    return EXIT_SUCCESS;
}

int viface_get_name(struct viface* self, char** result)
{
    *result = apr_pcalloc(TEMPORAL_POOL, IFNAMSIZ);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'viface_get_name' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = self->name;
    return EXIT_SUCCESS;
}

int viface_get_id(struct viface* self, uint* result)
{
    *result = self->id;
    return EXIT_SUCCESS;
}

int viface_get_tx(struct viface* self, int* result)
{
    *result = self->queues.tx;
    return EXIT_SUCCESS;
}

int viface_get_rx(struct viface* self, int* result)
{
    *result = self->queues.rx;
    return EXIT_SUCCESS;
}


int viface_set_mac(struct viface* self, char* mac)
{
    uint8_t* mac_bin;
    if (viface_parse_mac(self, mac, &mac_bin) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    apr_cpystrn(self->mac, mac, strlen(mac) + 1);
    return EXIT_SUCCESS;
}

int viface_get_mac(struct viface* self, char** result)
{
    // Read interface flags
    struct ifreq ifr;
    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl(self->kernel_socket, SIOCGIFHWADDR, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get MAC addr for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = apr_pcalloc(TEMPORAL_POOL, 18);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'viface_get_mac' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Convert binary MAC address to string
    int i = 0;
    for (i = 0; i < 6; i++) {
        char value[2];
        snprintf(value, 3, "%.02x",
                 (unsigned int) (0xFF & ifr.ifr_hwaddr.sa_data[i]));
        strcat(*result, value);

        if (i != 5) {
            strcat(*result, ":");
        }
    }
    return EXIT_SUCCESS;
}

int viface_set_ipv4(struct viface* self, char* ipv4)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, ipv4, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 address (%s) for", ipv4);
        fprintf(stdout, " %s.\n", self->name);
        return EXIT_FAILURE;
    }

    apr_cpystrn(self->ipv4, ipv4, strlen(ipv4) + 1);
    return EXIT_SUCCESS;
}

int viface_ioctl_get_ipv4(struct viface* self, unsigned long request,
                          char** result)
{
    // Read interface flags
    struct ifreq ifr;
    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl(self->kernel_socket, request, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get IPv4 for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Convert binary IP address to string
    char addr[INET_ADDRSTRLEN];
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in* ipaddr = (struct sockaddr_in*) &ifr.ifr_addr;
    if (inet_ntop(AF_INET, &(ipaddr->sin_addr), addr, sizeof(addr)) == NULL) {
        fprintf(stdout, "--- Unable to convert IPv4 for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = apr_pcalloc(TEMPORAL_POOL, INET_ADDRSTRLEN);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'viface_ioctlGetIPv4' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    apr_cpystrn(*result, addr, strlen(addr) + 1);
    return EXIT_SUCCESS;
}

int viface_get_ipv4(struct viface* self, char** result)
{
    return viface_ioctl_get_ipv4(self, SIOCGIFADDR, result);
}

int viface_set_ipv4_netmask(struct viface* self, char* netmask)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, netmask, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 netmask (%s) for", netmask);
        fprintf(stdout, " %s.\n", self->name);
        return EXIT_FAILURE;
    }

    apr_cpystrn(self->netmask, netmask, strlen(netmask) + 1);
    return EXIT_SUCCESS;
}

int viface_get_ipv4_netmask(struct viface* self, char** result)
{
    return viface_ioctl_get_ipv4(self, SIOCGIFNETMASK, result);
}

int viface_set_ipv4_broadcast(struct viface* self, char* broadcast)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, broadcast, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 address (%s) for", broadcast);
        fprintf(stdout, " %s.\n", self->name);
        return EXIT_FAILURE;
    }

    apr_cpystrn(self->broadcast, broadcast, strlen(broadcast) + 1);
    return EXIT_SUCCESS;
}

int viface_get_ipv4_broadcast(struct viface* self, char** result)
{
    return viface_ioctl_get_ipv4(self, SIOCGIFBRDADDR, result);
}

int viface_set_mtu(struct viface* self, uint mtu)
{
    // RFC 791, p. 24: "Every internet module must be able to forward a
    // datagram of 68 octets without further fragmentation."
    if (mtu < 68) {
        fprintf(stdout, "--- MTU %d too small (< 68).\n", mtu);
        return EXIT_FAILURE;
    }

    // Are we sure about self upper validation?
    // lo interface reports self number for its MTU
    if (mtu > 65536) {
        fprintf(stdout, "--- MTU %d too large (> 65536).\n", mtu);
        return EXIT_FAILURE;
    }

    self->mtu = mtu;
    return EXIT_SUCCESS;
}

int viface_get_mtu(struct viface* self, uint* mtu)
{
    // Read interface flags
    struct ifreq ifr;
    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl(self->kernel_socket, SIOCGIFMTU, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get MTU for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *mtu = ifr.ifr_mtu;
    return EXIT_SUCCESS;
}

int viface_set_ipv6(struct viface* self, int num_ipv6s, char* ipv6s[])
{
    // Validate format
    struct in6_addr addr6;

    int i = 0;
    for (i = 0; i < num_ipv6s; i++) {
        if (!inet_pton(AF_INET6, ipv6s[i], &addr6)) {
            fprintf(stdout, "--- Invalid IPv6 address");
            fprintf(stdout, " (%s) for %s.\n", ipv6s[i], self->name);
            return EXIT_FAILURE;
        }
    }

    self->ipv6s = apr_pcalloc(self->viface_pool, sizeof(char) * num_ipv6s + 1);

    if (self->ipv6s == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " ipv6s buffer at 'viface_set_ipv6' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    self->ipv6s = ipv6s;
    return EXIT_SUCCESS;
}

int viface_get_ipv6(struct viface* self, char** result[])
{
    int i = 1;

    // Creates hash table
    char** ipv6s =
        apr_pcalloc(TEMPORAL_POOL, sizeof(char*) * INET6_ADDRSTRLEN + 1);

    if (ipv6s == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " getting IPv6s for %s interface.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Buffer to store string representation of the address
    char buff[INET6_ADDRSTRLEN];
    memset(&buff, 0, sizeof(buff));

    // Cast pointer to ipv6 address
    struct sockaddr_in6* addr;

    // Pointers to list head and current node
    struct ifaddrs *head;
    struct ifaddrs *node;

    // Get list of interfaces
    if (getifaddrs(&head) == -1) {
        fprintf(stdout, "--- Failed to get list of interface addresses.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Iterate list
    for (node = head; node != NULL; node = node->ifa_next) {
        if (node->ifa_addr == NULL) {
            continue;
        }
        if (node->ifa_addr->sa_family != AF_INET6) {
            continue;
        }

        if (strcmp(node->ifa_name, self->name) != 0) {
            continue;
        }

        // Convert IPv6 address to string representation
        addr = (struct sockaddr_in6*) node->ifa_addr;
        if (inet_ntop(AF_INET6, &(addr->sin6_addr), buff,
                      sizeof(buff)) == NULL) {
            fprintf(stdout, "--- Unable to convert IPv6 for");
            fprintf(stdout, " %s.\n", self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }

        ipv6s[i] = apr_pcalloc(TEMPORAL_POOL, sizeof(buff) + 1);
        if (ipv6s[i] == NULL) {
            fprintf(stdout, "--- Memory could not be allocated for");
            fprintf(stdout, " getting IPv6 addresses.\n");
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
        apr_cpystrn(ipv6s[i], buff, sizeof(buff));
        i++;
    }

    char *number_ipv6s = apr_pcalloc(TEMPORAL_POOL, sizeof(int));

    if (number_ipv6s == NULL) {
        fprintf(stdout, "--- Memory could not be allocated");
        fprintf(stdout, " getting Ipv6s for %s interface.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    sprintf(number_ipv6s,"%d", i - 1);
    ipv6s[0] = number_ipv6s;

    freeifaddrs(head);
    *result = ipv6s;
    return EXIT_SUCCESS;
}

int viface_is_up(struct viface* self, bool* result)
{
    // Read interface flags
    struct ifreq ifr;

    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    *result = (ifr.ifr_flags & IFF_UP) != 0;
    return EXIT_SUCCESS;
}

int viface_up(struct viface* self)
{
    bool is_viface_up;
    if (viface_is_up(self, &is_viface_up) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (is_viface_up) {
        fprintf(stdout, "--- Virtual interface %s", self->name);
        fprintf(stdout, " is already up.\n");
        fprintf(stdout, "    up() Operation not permitted.\n");
        return EXIT_FAILURE;
    }

    // Read interface flags
    struct ifreq ifr;
    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Set MAC address
    bool is_mac_empty;
    viface_is_empty(self->mac, &is_mac_empty);
    if (!is_mac_empty) {
        uint8_t* mac_bin;

        if (viface_parse_mac(self, self->mac, &mac_bin) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        int i = 0;
        ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
        for (i = 0; i < 6; i++) {
            ifr.ifr_hwaddr.sa_data[i] = mac_bin[i];
        }

        if (ioctl(self->kernel_socket, SIOCSIFHWADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set MAC Address (%s)", self->mac);
            fprintf(stdout, " for %s.\n", self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Set IPv4 related
    // FIXME: Refactor self, it's ugly :/
    struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;
    addr->sin_family = AF_INET;

    // Address
    bool is_ipv4_empty;
    viface_is_empty(self->ipv4, &is_ipv4_empty);
    if (!is_ipv4_empty) {
        if (!inet_pton(AF_INET, self->ipv4, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 address");
            fprintf(stdout, " (%s) for %s.\n", self->ipv4, self->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl(self->kernel_socket, SIOCSIFADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4");
            fprintf(stdout, " (%s) for %s.\n", self->ipv4, self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Netmask
    bool is_netmask_empty;
    viface_is_empty(self->netmask, &is_netmask_empty);
    if (!is_netmask_empty) {
        if (!inet_pton(AF_INET, self->netmask, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 netmask");
            fprintf(stdout, " (%s) for %s.\n", self->netmask, self->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl(self->kernel_socket, SIOCSIFNETMASK, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4 netmask");
            fprintf(stdout, " (%s) for %s.\n", self->netmask, self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Broadcast
    bool is_broadcast_empty;
    viface_is_empty(self->broadcast, &is_broadcast_empty);
    if (!is_broadcast_empty) {
        if (!inet_pton(AF_INET, self->broadcast, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 broadcast");
            fprintf(stdout, " (%s) for", self->broadcast);
            fprintf(stdout, " %s.\n", self->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl(self->kernel_socket, SIOCSIFBRDADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4 broadcast");
            fprintf(stdout, " (%s) for", self->broadcast);
            fprintf(stdout, " %s.\n", self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Set IPv6 related
    // FIXME: Refactor self, it's ugly :/
    if (self->ipv6s != NULL) {
        struct in6_ifreq ifr6;
        memset(&ifr6, 0, sizeof(struct in6_ifreq));

        // Get interface index
        if (ioctl(self->kernel_socket, SIOGIFINDEX, &ifr) < 0) {
            fprintf(stdout, "--- Unable to get interface index for");
            fprintf(stdout, " (%s).\n", self->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }
        ifr6.ifr6_ifindex = ifr.ifr_ifindex;
        ifr6.ifr6_prefixlen = 64;

        int i = 0;
        for (i = 0; i < 2; i++) {
            // Parse IPv6 address into IPv6 address structure
            if (!inet_pton(AF_INET6, self->ipv6s[i], &ifr6.ifr6_addr)) {
                fprintf(stdout, "--- Invalid cached IPv6 address");
                fprintf(stdout, " (%s) for %s.\n", self->ipv6s[i], self->name);
                fprintf(stdout, "    Something really bad happened :/\n");
                return EXIT_FAILURE;
            }

            // Set IPv6 address
            if (ioctl(self->kernel_socket_ipv6, SIOCSIFADDR, &ifr6) < 0) {
                fprintf(stdout, "--- Unable to set IPv6 address");
                fprintf(stdout, " (%s) for %s.\n", self->ipv6s[i], self->name);
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
                return EXIT_FAILURE;
            }
        }
    }

    // Set MTU
    ifr.ifr_mtu = self->mtu;
    if (ioctl(self->kernel_socket, SIOCSIFMTU, &ifr) != 0) {
        fprintf(stdout, "--- Unable to set MTU");
        fprintf(stdout, " (%d) for %s.\n", self->mtu, self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    self->pktbuff = apr_pcalloc(self->viface_pool, sizeof(self->mtu));

    if (self->pktbuff == NULL) {
        fprintf(stdout, "--- Memory could not be reallocated for pktbuff");
        fprintf(stdout, " for %s network interface.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Bring-up interface
    ifr.ifr_flags |= IFF_UP;
    if (ioctl(self->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        fprintf(stdout, "--- Unable to bring-up interface");
        fprintf(stdout, " %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int viface_down(struct viface* self)
{
    // Read interface flags
    struct ifreq ifr;
    if (viface_read_flags(self->kernel_socket, self->name,
                          &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Bring-down interface
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(self->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        fprintf(stdout, "--- Unable to bring-down interface");
        fprintf(stdout, " %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int viface_receive(struct viface* self, uint8_t** result)
{
    // Read packet into our buffer
    int nread = read(self->queues.rx, &(self->pktbuff[0]), self->mtu);

    // Handle errors
    if (nread == -1) {
        // Nothing was read for this fd (non-blocking).
        // This could happen, as http://linux.die.net/man/2/select states:
        //
        //    Under Linux, select() may report a socket file descriptor as
        //    "ready for reading", while nevertheless a subsequent read
        //    blocks. This could for example happen when data has arrived
        //    but upon examination has wrong checksum and is discarded.
        //    There may be other circumstances in which a file descriptor
        //    is spuriously reported as ready. Thus it may be safer to
        //    use O_NONBLOCK on sockets that should not block.
        //
        // I know this is not a socket, but the "There may be other
        // circumstances in which a file descriptor is spuriously reported
        // as ready" warns it, and so, it better to do this that to have
        // an application that frozes for no apparent reason.
        //
        if (errno == EAGAIN) {
            char *packet = apr_pcalloc(TEMPORAL_POOL, sizeof(char) * (3));

            if (packet == NULL) {
                fprintf(stdout, "--- Memory could not be allocated for the");
                fprintf(stdout, " received packet for %s", self->name);
                fprintf(stdout, " network interface.\n");
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
                return EXIT_FAILURE;
            }

            memcpy(&packet[1], "0", 1);
            packet[0] = 1;
            packet[1] = '\0';
            *result = (uint8_t*)packet;
            return EXIT_SUCCESS;
        }

        // Something bad happened
        fprintf(stdout, "--- IO error while reading from %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Copy packet from buffer and return
    char *packet = apr_pcalloc(TEMPORAL_POOL, sizeof(char) * (nread + 2));

    if (packet == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for the received");
        fprintf(stdout, " packet for %s network interface.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }


    memcpy(&packet[1], self->pktbuff, nread);
    packet[0] = nread;
    packet[nread + 1] = '\0';
    *result = (uint8_t*)packet;
    return EXIT_SUCCESS;
}

int viface_send(struct viface* self, uint8_t* packet)
{
    int size = packet[0];

    // RFC 791, p. 24: "Every internet module must be able to forward a
    // datagram of 68 octets without further fragmentation."
    if (size < 68) {
        fprintf(stdout, "--- Packet too small (%d)", size);
        fprintf(stdout, " too small (< 68).\n");
        return EXIT_FAILURE;
    }

    if (size > self->mtu) {
        fprintf(stdout, "--- Packet too large (%d)", size);
        fprintf(stdout, " for current MTU (> %d).\n", self->mtu);
        return EXIT_FAILURE;
    }

    // Write packet to TX queue
    int written = write(self->queues.tx, &packet[1], size);

    if (written != size) {
        fprintf(stdout, "--- IO error while writting to %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int viface_list_stats(struct viface* self, char** result[])
{
    const int NUMBER_STATS = 23;

    DIR* dir;
    struct dirent* ent;
    int i = 1;

    char** stats_names =
        apr_pcalloc(TEMPORAL_POOL, sizeof(char*) * NUMBER_STATS + 1);
    apr_hash_t *hash_stats = apr_hash_make(self->viface_pool);

    if ((stats_names == NULL) ||
        (hash_stats == NULL)) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " network interface statistics.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    int size_bytes_stats_path = strlen("/sys/class/net/") +
                                strlen(self->name) + strlen("/statistics/");

    char stats_path[size_bytes_stats_path + 1];
    memset(&stats_path, 0, size_bytes_stats_path);

    snprintf(stats_path, sizeof(stats_path), "%s%s%s",
             "/sys/class/net/", self->name, "/statistics/");

    // Open directory
    if ((dir = opendir(stats_path)) == NULL) {
        fprintf(stdout, "--- Unable to open statistics folder for interface");
        fprintf(stdout, " %s:\n", self->name);
        fprintf(stdout, "    %s.\n", stats_path);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    char *number_stats = apr_pcalloc(TEMPORAL_POOL, sizeof(int));

    if (number_stats == NULL) {
        fprintf(stdout, "--- Memory could not be allocated");
        fprintf(stdout, " for statistic size number.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    sprintf(number_stats,"%d", NUMBER_STATS);
    stats_names[0] = number_stats;

    // List files
    while ((ent = readdir(dir)) != NULL) {
        //char entry = ent->d_name;
        char* entry = ent->d_name;

        // Ignore current, parent and hidden files
        if (entry[0] != '.') {
            char *key = apr_pcalloc(TEMPORAL_POOL, strlen(entry) + 1);

            if (key == NULL) {
                fprintf(stdout, "--- Memory could not be allocated");
                fprintf(stdout, " for statistic name %s.\n", entry);
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
                return EXIT_FAILURE;
            }

            apr_cpystrn(key, entry, strlen(entry) + 1);
            apr_hash_set(hash_stats, key, APR_HASH_KEY_STRING, key);
            stats_names[i] = key;
            i++;
        }
    }

    // Close directory
    if (closedir(dir) != 0) {
        fprintf(stdout, "--- Unable to close statistics folder for interface");
        fprintf(stdout, " %s:\n", self->name);
        fprintf(stdout, "    %s.\n", stats_path);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = stats_names;

    // Update cache
    self->stats_keys_cache = hash_stats;
    return EXIT_SUCCESS;
}

int viface_read_stat_file(struct viface* self, char* stat, uint64_t* result)
{
    // If no cache exists, create it.
    if (apr_hash_count(self->stats_keys_cache) == 0) {
        char** stats_names;
        if (viface_list_stats(self, &stats_names) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }

    // Check if stat is valid
    if (apr_hash_get(self->stats_keys_cache, stat,
                     APR_HASH_KEY_STRING) == NULL) {
        fprintf(stdout, "--- Unknown statistic %s", stat);
        fprintf(stdout, " for interface %s.\n", self->name);
        return EXIT_FAILURE;
    }

    // Open file
    int size_bytes_stats_path = strlen("/sys/class/net/") +
                                strlen(self->name) +
                                strlen("/statistics/") + strlen(stat);

    char stats_path[size_bytes_stats_path + 1];
    memset(&stats_path, 0, size_bytes_stats_path);

    snprintf(stats_path, sizeof(stats_path), "%s%s%s%s",
             "/sys/class/net/", self->name, "/statistics/", stat);

    int fd = -1;
    int nread = -1;
    char buffer[sizeof(uint64_t) + 1];

    // Opens MTU file
    fd = open(stats_path, O_RDONLY | O_NONBLOCK);

    // Check file open
    if (fd < 0) {
        fprintf(stdout, "--- Unable to open statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", self->name);
        return EXIT_FAILURE;
    }

    // Reads stats value
    nread = read(fd, &buffer, sizeof(buffer));
    buffer[nread] = '\0';

    // Handles errors
    if (nread == -1) {
        fprintf(stdout, "--- Unable to read statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    if (close(fd) < 0) {
        fprintf(stdout, "--- Unable to close statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Create entry if this stat wasn't cached
    if (apr_hash_get(self->stats_cache, stat,
                     APR_HASH_KEY_STRING) == NULL) {
        char* key = apr_pcalloc(self->viface_pool, strlen(stat) + 1);

        uint64_t* value = apr_pcalloc(self->viface_pool, sizeof(uint64_t));

        if ((key == NULL) ||
            (value == NULL)) {
            fprintf(stdout, "--- Memory could not be allocated reading");
            fprintf(stdout, " statistics values for %s.\n", self->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
        apr_cpystrn(key, stat, strlen(stat) + 1);
        *value = 0;

        apr_hash_set(self->stats_cache, key, APR_HASH_KEY_STRING, value);
    }

    *result = strtoul(buffer, NULL, 10);
    return EXIT_SUCCESS;
}

int viface_read_stat(struct viface* self, char* stat, uint64_t* result)
{
    uint64_t value;
    if (viface_read_stat_file(self, stat, &value) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    uint64_t* cache =
        (uint64_t*)apr_hash_get(self->stats_cache,
                                stat, APR_HASH_KEY_STRING);

    if (cache != NULL) {
        // Return value minus the cached value
        value -= *cache;
    }

    *result = value;

    return EXIT_SUCCESS;
}

int viface_clear_stat(struct viface* self, char* stat)
{
    uint64_t* value = apr_pcalloc(self->viface_pool, sizeof(uint64_t));

    if (value == NULL) {
        fprintf(stdout, "--- Memory could not be allocated clearing");
        fprintf(stdout, " statistics values for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    if (viface_read_stat_file(self, stat, &*value) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    char* key = apr_pcalloc(self->viface_pool, strlen(stat) + 1);
    if (key == NULL) {
        fprintf(stdout, "--- Memory could not be allocated clearing");
        fprintf(stdout, " statistics values for %s.\n", self->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    apr_cpystrn(key, stat, strlen(stat) + 1);

    // Set current value as cache
    apr_hash_set(self->stats_cache, key, APR_HASH_KEY_STRING, value);

    return EXIT_SUCCESS;
}

int viface_dispatch(struct viface* self, int num_ifaces, struct viface** ifaces,
                    dispatcher_cb callback, int millis)
{
    int fd = -1;
    int fds_read = -1;
    int nfds = -1;
    struct timeval tv;
    struct timeval* tvp = NULL;

    // Check non-is_empty( set
    if ((num_ifaces == 0) ||
        (ifaces == NULL)) {
        fprintf(stdout, "--- is_empty( virtual interfaces set.\n");
        return EXIT_FAILURE;
    }

    // Setup timeout
    if (millis >= 0) {
        tvp = &tv;
    }

    apr_hash_t* reverse_id = apr_hash_make(TEMPORAL_POOL);
    if (reverse_id == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " APR hash table.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Create and clear set of file descriptors
    fd_set rfds;

    int i = 0;

    // Create map of file descriptors and get maximum file descriptor for
    // select call
    for (i = 0; i < num_ifaces; i++) {
        // Store identity
        if (viface_get_rx(ifaces[i], &fd) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        char* key = apr_pcalloc(TEMPORAL_POOL, sizeof(int));

        struct viface* value = apr_pcalloc(TEMPORAL_POOL,
                                           sizeof(struct viface));

        if ((key == NULL) ||
            (value == NULL)) {
            fprintf(stdout, "--- Memory could not be allocated");
            fprintf(stdout, " for dispatch.\n");
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
        *key = fd;
        value = ifaces[i];
        apr_hash_set(reverse_id, key, APR_HASH_KEY_STRING, value);

        // Get maximum file descriptor
        if (fd > nfds) {
            nfds = fd;
        }
    }
    nfds++;

    // Perform select system call
    while (true) {
        // Re-create set
        FD_ZERO(&rfds);
        for (i = 0; i < num_ifaces; i++) {
            if (viface_get_rx(ifaces[i], &fd) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            FD_SET(fd, &rfds);
        }

        // Re-set timeout
        if (tvp != NULL) {
            tv.tv_sec = millis / 1000;
            tv.tv_usec = (millis % 1000) * 1000;
        }

        fds_read = select(nfds, &rfds, NULL, NULL, tvp);

        // Check if select error
        if (fds_read == -1) {
            // A signal was caught. Return.
            if (errno == EINTR) {
                return EXIT_SUCCESS;
            }

            // Something bad happened
            fprintf(stdout, "--- Unknown error in select() system call:");
            fprintf(stdout, " %d.\n", fds_read);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }

        // Check if timeout
        if (tvp != NULL && fds_read == 0) {
            return EXIT_SUCCESS;
        }

        apr_hash_index_t* index = NULL;
        void* pair_hash;
        struct viface* pair;
        int key = -1;

        for (index = apr_hash_first(self->viface_pool, reverse_id); index;
             index = apr_hash_next(index)) {
            apr_hash_this(index, NULL, NULL, &pair_hash);
            pair = (struct viface*)pair_hash;
            if (viface_get_rx(pair, &key) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            // Check if fd wasn't marked in select as available
            if (!FD_ISSET(key, &rfds)) {
                continue;
            }

            // File descriptor is ready, perform read and dispatch
            uint8_t* packet;
            if (viface_receive(pair, &packet) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            if (packet[0] == 0) {
                // Even if this is very unlikely, supposedly it can happen.
                // See receive() comments about this.
                continue;
            }

            char* name;
            uint id;

            if ((viface_get_name(pair, &name) == EXIT_FAILURE) ||
                (viface_get_id(pair, &id) == EXIT_FAILURE)) {
                return EXIT_FAILURE;
            }

            // Dispatch packet
            bool result_callback = false;
            if (callback(pair, packet, &result_callback) ==
                EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            if (!result_callback) {
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_SUCCESS;
}