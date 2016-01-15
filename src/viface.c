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

int  parse_mac(struct viface **self, char *mac, uint8_t **result)
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
    uint8_t *parsed = (uint8_t*)apr_pcalloc((*self)->viface_pool, 6);

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

int hexdump(struct viface **self, uint8_t *bytes, char **result)
{
    const int BYTES_PER_LINE = 72;

    char buffer[10];
    int number_lines = (bytes[0] / 16) + 1;

    *result =
        (char*)apr_pcalloc((*self)->viface_pool, BYTES_PER_LINE * number_lines);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'hexdump' method.\n");
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

int crc32(uint8_t *bytes, uint32_t *result)
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

static int read_flags(int sockfd, char *name, struct ifreq *ifr)
{
    // Prepare communication structure
    memset(ifr, 0, sizeof(struct ifreq));

    // Set interface name
    (void) strncpy(ifr->ifr_name, name, IFNAMSIZ - 1);

    // Read interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, ifr) != 0) {
        fprintf(stdout, "--- Unable to read %s flags.\n", name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


static int read_mtu(char *name, size_t size_bytes, uint *result)
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

static int alloc_viface(struct viface **self, char* name, bool tap,
                        struct viface_queues *queues, char **result)
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

    (void) strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

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
        if (ioctl(fd, TUNSETIFF, (void *)&ifr) != 0) {
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

        ((int *)queues)[i] = fd;
    }

    *result =
        (char*)apr_pcalloc((*self)->viface_pool, strlen(ifr.ifr_name) + 1);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " viface name in alloc_viface method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    strcpy(*result, ifr.ifr_name);
    return EXIT_SUCCESS;

err:
    // Rollback close file descriptors
    for (--i; i >= 0; i--) {
        if (close(((int *)queues)[i]) < 0) {
            fprintf(stdout, "--- Unable to close a TUN/TAP device.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
    }
    return EXIT_FAILURE;
}

static int hook_viface(char *name, struct viface_queues *queues)
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

        (void) strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

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

        ((int *)queues)[i] = fd;
    }
    return EXIT_SUCCESS;

err:
    // Rollback close file descriptors
    for (--i; i >= 0; i--) {
        if (close(((int *)queues)[i]) < 0) {
            fprintf(stdout, "--- Unable to close a Rx/Tx socket.\n");
            fprintf(stdout, "    Name: %s Queue: %d.\n", name, i);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
    }
    return EXIT_FAILURE;
}

static int isEmpty(char *buffer, bool *result)
{
    *result = true;

    if ((buffer != NULL) && (buffer[0] != '\0')) {
        *result = false;
    }
    return EXIT_SUCCESS;
}

/*= Virtual Interface Implementation =========================================*/

int viface_create(apr_pool_t **parent_pool, struct viface **result)
{
    apr_pool_t *viface_pool = NULL;
    apr_pool_create(&viface_pool, *parent_pool);

    struct viface *self =
        (struct viface*)apr_pcalloc(viface_pool, sizeof(struct viface));

    if (self == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " 'viface' struct.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    self->viface_pool = viface_pool;

    self->ipv6s = apr_hash_make(self->viface_pool);
    self->stats_keys_cache = apr_hash_make(self->viface_pool);
    self->stats_cache = apr_hash_make(self->viface_pool);

    memset(&self->queues, 0, sizeof(struct viface_queues));
    *result = self;


    return EXIT_SUCCESS;
}

int viface_destroy(struct viface **self)
{
    close((*self)->queues.rx);
    close((*self)->queues.tx);
    close((*self)->kernel_socket);
    close((*self)->kernel_socket_ipv6);

    if (*self != NULL) {
        apr_pool_destroy((*self)->viface_pool);
        //free(self);
        *self = NULL;
    }
    return EXIT_SUCCESS;
}

int vifaceImpl(struct viface **self, char *name, bool tap, int id)
{
    const int SIZE_BYTES_PATH = 15;

    // Check name length
    if (strlen(name) >= IFNAMSIZ) {
        fprintf(stdout, "--- Virtual interface name too long.\n");
        return EXIT_FAILURE;
    }

    // Create queues
    struct viface_queues queues;
    memset(&queues, 0, sizeof(struct viface_queues));

    int size_bytes_viface_path = SIZE_BYTES_PATH + strlen(name);

    char viface_path[size_bytes_viface_path + 1];
    memset(&viface_path, 0, size_bytes_viface_path);

    snprintf(viface_path, sizeof(viface_path), "%s%s",
             "/sys/class/net/", name);

    /* Checks if the path name can be accessed. If so,
     * it means that the network interface is already defined.
     */
    if (access(viface_path, F_OK) == 0) {
        if (hook_viface(name, &queues) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        strcpy((*self)->name, name);

        // Read MTU value and resize buffer
        if (read_mtu(name, sizeof((*self)->mtu),
                     &(*self)->mtu) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        (*self)->pktbuff =
            (uint8_t*)apr_pcalloc((*self)->viface_pool, sizeof((*self)->mtu));

        if ((*self)->pktbuff == NULL) {
            fprintf(stdout, "--- Memory could not be allocated for");
            fprintf(stdout, " pktbuff in %s interface.\n", name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    } else {
        char *viface_name;
        if (alloc_viface(&*self, name, tap, &queues, &viface_name) ==
            EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        strcpy((*self)->name, viface_name);

        // Other defaults
        (*self)->mtu = 1500;
    }

    (*self)->queues = queues;

    // Create socket channels to the NET kernel for later ioctl
    (*self)->kernel_socket = -1;
    (*self)->kernel_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ((*self)->kernel_socket < 0) {
        fprintf(stdout, "--- Unable to create IPv4 socket channel to the");
        fprintf(stdout, " NET kernel.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    (*self)->kernel_socket_ipv6 = -1;
    (*self)->kernel_socket_ipv6 = socket(AF_INET6, SOCK_STREAM, 0);
    if ((*self)->kernel_socket_ipv6 < 0) {
        fprintf(stdout, "--- Unable to create IPv6 socket channel to the");
        fprintf(stdout, " NET kernel.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Set id
    if (id < 0) {
        (*self)->id = (*self)->idseq;
    } else {
        (*self)->id = id;
    }

    (*self)->idseq++;
    return EXIT_SUCCESS;
}

int getName(struct viface **self, char **result)
{
    *result = (*self)->name;
    return EXIT_SUCCESS;
}

int getID(struct viface **self, uint *result)
{
    *result = (*self)->idseq;
    return EXIT_SUCCESS;
}

int getTX(struct viface **self, int *result)
{
    *result = (*self)->queues.tx;
    return EXIT_SUCCESS;
}

int getRX(struct viface **self, int *result)
{
    *result = (*self)->queues.rx;
    return EXIT_SUCCESS;
}


int setMAC(struct viface **self, char *mac)
{
    uint8_t *mac_bin;
    if (parse_mac(&*self, mac, &mac_bin) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    strcpy((*self)->mac, mac);
    return EXIT_SUCCESS;
}

int getMAC(struct viface **self, char **result)
{
    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl((*self)->kernel_socket, SIOCGIFHWADDR, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get MAC addr for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = (char*)apr_pcalloc((*self)->viface_pool, 18);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'getMAC' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    memset(*result, '\0', sizeof(result));

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

int setIPv4(struct viface **self, char *ipv4)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, ipv4, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 address (%s) for", ipv4);
        fprintf(stdout, " %s.\n", (*self)->name);
        return EXIT_FAILURE;
    }

    strcpy((*self)->ipv4, ipv4);
    return EXIT_SUCCESS;
}

int ioctlGetIPv4(struct viface **self, unsigned long request, char **result)
{
    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl((*self)->kernel_socket, request, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get IPv4 for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Convert binary IP address to string
    char addr[INET_ADDRSTRLEN];
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in* ipaddr = (struct sockaddr_in*) &ifr.ifr_addr;
    if (inet_ntop(AF_INET, &(ipaddr->sin_addr), addr, sizeof(addr)) == NULL) {
        fprintf(stdout, "--- Unable to convert IPv4 for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *result = (char*)apr_pcalloc((*self)->viface_pool, INET_ADDRSTRLEN);
    if (result == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for");
        fprintf(stdout, " result buffer at 'ioctlGetIPv4' method.\n");
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    strcpy(*result, addr);
    return EXIT_SUCCESS;
}

int getIPv4(struct viface **self, char **result)
{
    return ioctlGetIPv4(&*self, SIOCGIFADDR, &*result);
}

int setIPv4Netmask(struct viface **self, char *netmask)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, netmask, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 netmask (%s) for", netmask);
        fprintf(stdout, " %s.\n", (*self)->name);
        return EXIT_FAILURE;
    }

    strcpy((*self)->netmask, netmask);
    return EXIT_SUCCESS;
}

int getIPv4Netmask(struct viface **self, char **result)
{
    return ioctlGetIPv4(&*self, SIOCGIFNETMASK, &*result);
}

int setIPv4Broadcast(struct viface **self, char *broadcast)
{
    // Validate format
    struct in_addr addr;

    if (!inet_pton(AF_INET, broadcast, &addr)) {
        fprintf(stdout, "--- Invalid IPv4 address (%s) for", broadcast);
        fprintf(stdout, " %s.\n", (*self)->name);
        return EXIT_FAILURE;
    }

    strcpy((*self)->broadcast, broadcast);
    return EXIT_SUCCESS;
}

int getIPv4Broadcast(struct viface **self, char **result)
{
    return ioctlGetIPv4(&*self, SIOCGIFBRDADDR, &*result);
}

int setMTU(struct viface **self, uint mtu)
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

    (*self)->mtu = mtu;
    return EXIT_SUCCESS;
}

int getMTU(struct viface **self, uint *mtu)
{
    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (ioctl((*self)->kernel_socket, SIOCGIFMTU, &ifr) != 0) {
        fprintf(stdout, "--- Unable to get MTU for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    *mtu = ifr.ifr_mtu;
    return EXIT_SUCCESS;
}

int setIPv6(struct viface **self, apr_hash_t *ipv6s)
{
    // Validate format
    struct in6_addr addr6;

    apr_hash_index_t *index = NULL;
    void *ipv6_hash;
    char *ipv6;

    for (index = apr_hash_first((*self)->viface_pool, ipv6s); index;
         index = apr_hash_next(index)) {
        apr_hash_this(index, NULL, NULL, &ipv6_hash);
        ipv6 = (char*)ipv6_hash;

        if (!inet_pton(AF_INET6, ipv6, &addr6)) {
            fprintf(stdout, "--- Invalid IPv6 address");
            fprintf(stdout, " (%s) for %s.\n", ipv6, (*self)->name);
            return EXIT_FAILURE;
        }
    }

    (*self)->ipv6s = ipv6s;
    return EXIT_SUCCESS;
}

int getIPv6(struct viface **self, apr_hash_t **result)
{
    // Creates hash table
    *result = apr_hash_make((*self)->viface_pool);

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

        if (strcmp(node->ifa_name, (*self)->name) != 0) {
            continue;
        }

        // Convert IPv6 address to string representation
        addr = (struct sockaddr_in6*) node->ifa_addr;
        if (inet_ntop(AF_INET6, &(addr->sin6_addr), buff,
                      sizeof(buff)) == NULL) {
            fprintf(stdout, "--- Unable to convert IPv6 for");
            fprintf(stdout, " %s.\n", (*self)->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }

        char *key = (char*)apr_pcalloc((*self)->viface_pool, sizeof(buff) + 1);
        if (key == NULL) {
            fprintf(stdout, "--- Memory could not be allocated for");
            fprintf(stdout, " getting IPv6 addresses.\n");
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
        apr_cpystrn(key, buff, sizeof(buff));

        apr_hash_set(*result, key, APR_HASH_KEY_STRING, key);
    }

    freeifaddrs(head);
    return EXIT_SUCCESS;
}

int isUp(struct viface **self, bool *result)
{
    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    *result = (ifr.ifr_flags & IFF_UP) != 0;
    return EXIT_SUCCESS;
}

int up(struct viface **self)
{
    bool is_up;
    if (isUp(&*self, &is_up) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (is_up) {
        fprintf(stdout, "--- Virtual interface %s", (*self)->name);
        fprintf(stdout, " is already up.\n");
        fprintf(stdout, "    up() Operation not permitted.\n");
        return EXIT_FAILURE;
    }

    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Set MAC address
    bool is_mac_isEmpty;
    isEmpty((*self)->mac, &is_mac_isEmpty);
    if (!is_mac_isEmpty) {
        uint8_t *mac_bin;
        if (parse_mac(&*self, (*self)->mac, &mac_bin) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        int i = 0;
        ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
        for (i = 0; i < 6; i++) {
            ifr.ifr_hwaddr.sa_data[i] = mac_bin[i];
        }

        if (ioctl((*self)->kernel_socket, SIOCSIFHWADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set MAC Address (%s)", (*self)->mac);
            fprintf(stdout, " for %s.\n", (*self)->name);
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
    bool is_ipv4_isEmpty;
    isEmpty((*self)->ipv4, &is_ipv4_isEmpty);
    if (!is_ipv4_isEmpty) {
        if (!inet_pton(AF_INET, (*self)->ipv4, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 address");
            fprintf(stdout, " (%s) for %s.\n", (*self)->ipv4, (*self)->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl((*self)->kernel_socket, SIOCSIFADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4");
            fprintf(stdout, " (%s) for %s.\n", (*self)->ipv4, (*self)->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Netmask
    bool is_netmask_isEmpty;
    isEmpty((*self)->netmask, &is_netmask_isEmpty);
    if (!is_netmask_isEmpty) {
        if (!inet_pton(AF_INET, (*self)->netmask, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 netmask");
            fprintf(stdout, " (%s) for %s.\n", (*self)->netmask, (*self)->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl((*self)->kernel_socket, SIOCSIFNETMASK, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4 netmask");
            fprintf(stdout, " (%s) for %s.\n", (*self)->netmask, (*self)->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Broadcast
    bool is_broadcast_isEmpty;
    isEmpty((*self)->broadcast, &is_broadcast_isEmpty);
    if (!is_broadcast_isEmpty) {
        if (!inet_pton(AF_INET, (*self)->broadcast, &addr->sin_addr)) {
            fprintf(stdout, "--- Invalid cached IPv4 broadcast");
            fprintf(stdout, " (%s) for", (*self)->broadcast);
            fprintf(stdout, " %s.\n", (*self)->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }

        if (ioctl((*self)->kernel_socket, SIOCSIFBRDADDR, &ifr) != 0) {
            fprintf(stdout, "--- Unable to set IPv4 broadcast");
            fprintf(stdout, " (%s) for", (*self)->broadcast);
            fprintf(stdout, " %s.\n", (*self)->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }
    }

    // Set IPv6 related
    // FIXME: Refactor self, it's ugly :/
    if (apr_hash_count((*self)->ipv6s) != 0) {
        struct in6_ifreq ifr6;
        memset(&ifr6, 0, sizeof(struct in6_ifreq));

        // Get interface index
        if (ioctl((*self)->kernel_socket, SIOGIFINDEX, &ifr) < 0) {
            fprintf(stdout, "--- Unable to get interface index for");
            fprintf(stdout, " (%s).\n", (*self)->name);
            fprintf(stdout, "    Something really bad happened :/\n");
            return EXIT_FAILURE;
        }
        ifr6.ifr6_ifindex = ifr.ifr_ifindex;
        ifr6.ifr6_prefixlen = 64;

        apr_hash_index_t *index = NULL;
        void *ipv6_hash;
        char *ipv6;

        for (index = apr_hash_first((*self)->viface_pool, (*self)->ipv6s);
             index; index = apr_hash_next(index)) {
            apr_hash_this(index, NULL, NULL, &ipv6_hash);
            ipv6 = (char*)ipv6_hash;
            // Parse IPv6 address into IPv6 address structure
            if (!inet_pton(AF_INET6, ipv6, &ifr6.ifr6_addr)) {
                fprintf(stdout, "--- Invalid cached IPv6 address");
                fprintf(stdout, " (%s) for %s.\n", ipv6, (*self)->name);
                fprintf(stdout, "    Something really bad happened :/\n");
                return EXIT_FAILURE;
            }

            // Set IPv6 address
            if (ioctl((*self)->kernel_socket_ipv6, SIOCSIFADDR, &ifr6) < 0) {
                fprintf(stdout, "--- Unable to set IPv6 address");
                fprintf(stdout, " (%s) for %s.\n", ipv6, (*self)->name);
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
                return EXIT_FAILURE;
            }
        }
    }

    // Set MTU
    ifr.ifr_mtu = (*self)->mtu;
    if (ioctl((*self)->kernel_socket, SIOCSIFMTU, &ifr) != 0) {
        fprintf(stdout, "--- Unable to set MTU");
        fprintf(stdout, " (%d) for %s.\n", (*self)->mtu, (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    (*self)->pktbuff =
        (uint8_t*)realloc((*self)->pktbuff, sizeof((*self)->mtu));
    if ((*self)->pktbuff == NULL) {
        fprintf(stdout, "--- Memory could not be reallocated for pktbuff");
        fprintf(stdout, " for %s network interface.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Bring-up interface
    ifr.ifr_flags |= IFF_UP;
    if (ioctl((*self)->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        fprintf(stdout, "--- Unable to bring-up interface");
        fprintf(stdout, " %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int down(struct viface **self)
{
    // Read interface flags
    struct ifreq ifr;
    if (read_flags((*self)->kernel_socket, (*self)->name,
                   &ifr) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Bring-down interface
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl((*self)->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        fprintf(stdout, "--- Unable to bring-down interface");
        fprintf(stdout, " %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int receive(struct viface **self, uint8_t **result)
{
    // Read packet into our buffer
    int nread = read((*self)->queues.rx, &((*self)->pktbuff[0]), (*self)->mtu);

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
            char *packet =
                (char*)apr_pcalloc((*self)->viface_pool, sizeof(char) * (3));

            if (packet == NULL) {
                fprintf(stdout, "--- Memory could not be allocated for the");
                fprintf(stdout, " received packet for %s", (*self)->name);
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
        fprintf(stdout, "--- IO error while reading from %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Copy packet from buffer and return
    char *packet =
        (char*)apr_pcalloc((*self)->viface_pool, sizeof(char) * (nread + 2));

    if (packet == NULL) {
        fprintf(stdout, "--- Memory could not be allocated for the received");
        fprintf(stdout, " packet for %s network interface.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    memcpy(&packet[1], (*self)->pktbuff, nread);
    packet[0] = nread;
    packet[nread + 1] = '\0';
    *result = (uint8_t*)packet;
    return EXIT_SUCCESS;
}

int send_packet(struct viface **self, uint8_t *packet)
{
    int size = packet[0];

    // RFC 791, p. 24: "Every internet module must be able to forward a
    // datagram of 68 octets without further fragmentation."
    if (size < 68) {
        fprintf(stdout, "--- Packet too small (%d)", size);
        fprintf(stdout, " too small (< 68).\n");
        return EXIT_FAILURE;
    }

    if (size > (*self)->mtu) {
        fprintf(stdout, "--- Packet too large (%d)", size);
        fprintf(stdout, " for current MTU (> %d).\n", (*self)->mtu);
        return EXIT_FAILURE;
    }

    // Write packet to TX queue
    int written = write((*self)->queues.tx, &packet[1], size);

    if (written != size) {
        fprintf(stdout, "--- IO error while writting to %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int listStats(struct viface **self, apr_hash_t **result)
{
    // Creates hash table
    *result = apr_hash_make((*self)->viface_pool);

    DIR* dir;
    struct dirent* ent;

    int size_bytes_stats_path = strlen("/sys/class/net/") +
                                strlen((*self)->name) + strlen("/statistics/");

    char stats_path[size_bytes_stats_path + 1];
    memset(&stats_path, 0, size_bytes_stats_path);

    snprintf(stats_path, sizeof(stats_path), "%s%s%s",
             "/sys/class/net/", (*self)->name, "/statistics/");

    // Open directory
    if ((dir = opendir(stats_path)) == NULL) {
        fprintf(stdout, "--- Unable to open statistics folder for interface");
        fprintf(stdout, " %s:\n", (*self)->name);
        fprintf(stdout, "    %s.\n", stats_path);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // List files
    while ((ent = readdir(dir)) != NULL) {
        //char entry = ent->d_name;
        char *entry = ent->d_name;

        // Ignore current, parent and hidden files
        if (entry[0] != '.') {
            char *key =
                (char*)apr_pcalloc((*self)->viface_pool, strlen(entry) + 1);
            if (key == NULL) {
                fprintf(stdout, "--- Memory could not be allocated");
                fprintf(stdout, " getting statistics names.\n");
                fprintf(stdout, "    Error: %s", strerror(errno));
                fprintf(stdout, " (%d).\n", errno);
            }
            apr_cpystrn(key, entry, strlen(entry) + 1);

            apr_hash_set(*result, key, APR_HASH_KEY_STRING, key);
        }
    }

    // Close directory
    if (closedir(dir) != 0) {
        fprintf(stdout, "--- Unable to close statistics folder for interface");
        fprintf(stdout, " %s:\n", (*self)->name);
        fprintf(stdout, "    %s.\n", stats_path);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Update cache
    (*self)->stats_keys_cache = *result;
    return EXIT_SUCCESS;
}


int readStatFile(struct viface **self, char *stat, uint64_t *result)
{
    // If no cache exists, create it.
    if (apr_hash_count((*self)->stats_keys_cache) == 0) {
        apr_hash_t *hash;
        if (listStats(&*self, &hash) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }

    // Check if stat is valid
    if (apr_hash_get((*self)->stats_keys_cache, stat,
                     APR_HASH_KEY_STRING) == NULL) {
        fprintf(stdout, "--- Unknown statistic %s", stat);
        fprintf(stdout, " for interface %s.\n", (*self)->name);
        return EXIT_FAILURE;
    }

    // Open file
    int size_bytes_stats_path = strlen("/sys/class/net/") +
                                strlen((*self)->name) +
                                strlen("/statistics/") + strlen(stat);

    char stats_path[size_bytes_stats_path + 1];
    memset(&stats_path, 0, size_bytes_stats_path);

    snprintf(stats_path, sizeof(stats_path), "%s%s%s%s",
             "/sys/class/net/", (*self)->name, "/statistics/", stat);

    int fd = -1;
    int nread = -1;
    char buffer[sizeof(uint64_t) + 1];

    // Opens MTU file
    fd = open(stats_path, O_RDONLY | O_NONBLOCK);

    // Check file open
    if (fd < 0) {
        fprintf(stdout, "--- Unable to open statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", (*self)->name);
        return EXIT_FAILURE;
    }

    // Reads stats value
    nread = read(fd, &buffer, sizeof(buffer));
    buffer[nread] = '\0';

    // Handles errors
    if (nread == -1) {
        fprintf(stdout, "--- Unable to read statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    if (close(fd) < 0) {
        fprintf(stdout, "--- Unable to close statistics file %s", stats_path);
        fprintf(stdout, " for interface %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
        return EXIT_FAILURE;
    }

    // Create entry if this stat wasn't cached
    if (apr_hash_get((*self)->stats_cache, stat,
                     APR_HASH_KEY_STRING) == NULL) {
        char *key = (char*)apr_pcalloc((*self)->viface_pool, strlen(stat) + 1);

        uint64_t *value =
            (uint64_t*)apr_pcalloc((*self)->viface_pool, sizeof(uint64_t));

        if ((key == NULL) ||
            (value == NULL)) {
            fprintf(stdout, "--- Memory could not be allocated reading");
            fprintf(stdout, " statistics values for %s.\n", (*self)->name);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
        apr_cpystrn(key, stat, strlen(stat) + 1);
        *value = 0;

        apr_hash_set((*self)->stats_cache, key, APR_HASH_KEY_STRING, value);
    }

    *result = strtoul(buffer, NULL, 10);
    return EXIT_SUCCESS;
}

int readStat(struct viface **self, char *stat, uint64_t *result)
{
    uint64_t value;
    if (readStatFile(&*self, stat, &value) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    uint64_t *cache =
        (uint64_t*)apr_hash_get((*self)->stats_cache,
                                stat, APR_HASH_KEY_STRING);

    if (cache != NULL) {
        // Return value minus the cached value
        value -= *cache;
    }

    *result = value;

    return EXIT_SUCCESS;
}

int clearStat(struct viface **self, char *stat)
{
    uint64_t *value =
        (uint64_t*)apr_pcalloc((*self)->viface_pool, sizeof(uint64_t));

    if (value == NULL) {
        fprintf(stdout, "--- Memory could not be allocated clearing");
        fprintf(stdout, " statistics values for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
    }

    if (readStatFile(&*self, stat, &*value) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    char *key = (char*)apr_pcalloc((*self)->viface_pool, strlen(stat) + 1);
    if (key == NULL) {
        fprintf(stdout, "--- Memory could not be allocated clearing");
        fprintf(stdout, " statistics values for %s.\n", (*self)->name);
        fprintf(stdout, "    Error: %s", strerror(errno));
        fprintf(stdout, " (%d).\n", errno);
    }
    apr_cpystrn(key, stat, strlen(stat) + 1);

    // Set current value as cache
    apr_hash_set((*self)->stats_cache, key, APR_HASH_KEY_STRING, value);
    return EXIT_SUCCESS;
}

int dispatch(struct viface **self, my_ring_t *ifaces,
             dispatcher_cb callback, int millis)
{
    int fd = -1;
    int fdsread = -1;
    int nfds = -1;
    struct timeval tv;
    struct timeval* tvp = NULL;

    // Check non-isEmpty set
    if (APR_RING_EMPTY(ifaces, viface, link) == 1) {
        fprintf(stdout, "--- isEmpty virtual interfaces set.\n");
        return EXIT_FAILURE;
    }

    // Setup timeout
    if (millis >= 0) {
        tvp = &tv;
    }

    apr_hash_t *reverse_id = apr_hash_make((*self)->viface_pool);

    // Create and clear set of file descriptors
    fd_set rfds;

    struct viface *iface;

    // Create map of file descriptors and get maximum file descriptor for
    // select call
    for (iface = APR_RING_FIRST(ifaces);
         iface != APR_RING_SENTINEL(ifaces, viface, link);
         iface = APR_RING_NEXT(iface, link)) {
        // Store identity
        if (getRX(&iface, &fd) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        char *key = (char*)apr_pcalloc(iface->viface_pool, sizeof(int));

        struct viface *value =
            (struct viface*)apr_pcalloc(iface->viface_pool,
                                        sizeof(struct viface));

        if ((key == NULL) ||
            (value == NULL)) {
            fprintf(stdout, "--- Memory could not be allocated");
            fprintf(stdout, " for dispatch.\n");
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
        }
        *key = fd;
        value = iface;
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
        for (iface = APR_RING_FIRST(ifaces);
             iface != APR_RING_SENTINEL(ifaces, viface, link);
             iface = APR_RING_NEXT(iface, link)) {
            if (getRX(&iface, &fd) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            FD_SET(fd, &rfds);
        }

        // Re-set timeout
        if (tvp != NULL) {
            tv.tv_sec = millis / 1000;
            tv.tv_usec = (millis % 1000) * 1000;
        }

        fdsread = select(nfds, &rfds, NULL, NULL, tvp);

        // Check if select error
        if (fdsread == -1) {
            // A signal was caught. Return.
            if (errno == EINTR) {
                return EXIT_SUCCESS;
            }

            // Something bad happened
            fprintf(stdout, "--- Unknown error in select() system call:");
            fprintf(stdout, " %d.\n", fdsread);
            fprintf(stdout, "    Error: %s", strerror(errno));
            fprintf(stdout, " (%d).\n", errno);
            return EXIT_FAILURE;
        }

        // Check if timeout
        if (tvp != NULL && fdsread == 0) {
            return EXIT_SUCCESS;
        }

        apr_hash_index_t *index = NULL;
        void *pair_hash;
        struct viface *pair;
        int key = -1;

        for (index = apr_hash_first((*self)->viface_pool, reverse_id); index;
             index = apr_hash_next(index)) {
            apr_hash_this(index, NULL, NULL, &pair_hash);
            pair = (struct viface*)pair_hash;
            if (getRX(&pair, &key) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            // Check if fd wasn't marked in select as available
            if (!FD_ISSET(key, &rfds)) {
                continue;
            }

            // File descriptor is ready, perform read and dispatch
            uint8_t *packet;
            if (receive(&pair, &packet) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }

            if (packet[0] == 0) {
                // Even if this is very unlikely, supposedly it can happen.
                // See receive() comments about this.
                continue;
            }

            char *name;
            uint id;

            if ((getName(&pair, &name) == EXIT_FAILURE) ||
                (getID(&pair, &id) == EXIT_FAILURE)) {
                return EXIT_FAILURE;
            }

            // Dispatch packet
            bool result_callback = false;
            if (callback(&pair, name, id, &packet, &result_callback) ==
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