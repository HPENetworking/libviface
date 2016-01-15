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

// Example packet. In Scapy:
// packet = Ether()/IP()/TCP()/Raw('I\'m a packet!'*3)
// len(packet) == 93
static uint8_t packet[94] = {
    0x5D, //This is the packet size (0x5D = 93)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x4F, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x7C, 0xA6, 0x7F, 0x00, 0x00, 0x01, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x14,
    0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02,
    0x20, 0x00, 0x04, 0x91, 0x00, 0x00, 0x49, 0x27, 0x6D, 0x20, 0x61, 0x20,
    0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20, 0x61,
    0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20,
    0x61, 0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21
};

int count = 0;

// Creates a network interface
int createsInterface(struct viface **self, apr_pool_t **pool,
                     char *viface_name, int id)
{
    // Creates interface
    if ((viface_create(&*pool, &*self) == EXIT_FAILURE) ||
        (vifaceImpl(&*self, viface_name, true, id) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Prints received packet information
int printPacket(struct viface **self, uint8_t **packet)
{
    uint32_t crc_32 = 0;
    char *hex_dump;

    if ((crc32(*packet, &crc_32) == EXIT_FAILURE) ||
        (hexdump(&*self, *packet, &hex_dump) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    printf("+++ Received packet %d from interface", count);
    printf(" %s (%d) of size", (*self)->name, (*self)->id);
    printf(" %d and CRC of 0x%x\n%s\n", *packet[0], crc_32, hex_dump);
    return EXIT_SUCCESS;
}

// Dispatch callback type to handle packet reception.
int handler(struct viface **self, char *name, uint id, uint8_t **packet,
            bool *result)
{
    if (printPacket(&*self, &*packet) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    count++;
    *result = true;
    return EXIT_SUCCESS;
}

/**
 * This example shows how to setup a dispatcher for a set of virtual interfaces.
 * This uses a method for the callback in order to show this kind of
 * usage, but any function using the same signature as dispatcher_cb type
 * can be used.
 *
 * To help with the example you can send a few packets to the created virtual
 * interfaces using scapy, wireshark, libtins or any other.
 */
main(int argc, const char* argv[])
{
    struct viface *iface0;
    struct viface *iface1;
    dispatcher_cb callback;
    my_ring_t *ring;

    char *name_iface0 = "viface0";
    char *name_iface1 = "viface1";

    printf("\n--- Starting dispatch example...\n\n");

    apr_initialize();

    // Creates parent pool
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);

    if ((createsInterface(&iface0, &pool, name_iface0, 0) == EXIT_FAILURE) ||
        (up(&iface0) == EXIT_FAILURE) ||
        (createsInterface(&iface1, &pool, name_iface1, 1) == EXIT_FAILURE) ||
        (up(&iface1) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    printf("--- Interface %s is up!.\n", name_iface0);
    printf("--- Interface %s is up!.\n", name_iface1);

    printf("--- Press Any Key to Receive Packet.\n");
    getchar();

    // Intializes the ring container
    ring = apr_palloc(pool, sizeof(my_ring_t));
    APR_RING_INIT(ring, viface, link);

    // Inserts vifaces to the ring
    APR_RING_INSERT_TAIL(ring, iface0, viface, link);
    APR_RING_INSERT_TAIL(ring, iface1, viface, link);

    printf("--- Calling dispatch ...\n");
    callback = handler;
    if (dispatch(&iface0, ring, callback, -1) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    apr_pool_destroy(pool);
    apr_terminate();

    return 0;
}