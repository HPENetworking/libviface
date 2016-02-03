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

// Prints the network interface statistics values
int print_statistics(struct viface* self, char** stats_names)
{
    int i = 0;

    for (i = 0; i < 23; i++) {
        uint64_t result_key;
        if (viface_read_stat(self, stats_names[i],
                             &result_key) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        printf("     %s : %zu\n", stats_names[i], result_key);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

/* Shows the use of the interface statistics to read
 * the number of packets and bytes sent.
 */
int check_statistics(struct viface* self)
{
    int i = 0;
    int number_packets = 100;
    uint32_t crc_32 = 0;
    char *hex_dump;
    char** stats_names;

    printf("********** Viface Statistics **********\n\n");

    // Prints statistics before sending packets
    printf("--- Statistics before sending packets:\n");

    if ((viface_list_stats(&*self, &stats_names) == EXIT_FAILURE) ||
        (print_statistics(&*self, stats_names) == EXIT_FAILURE) ||
        (viface_crc_32(packet, &crc_32) == EXIT_FAILURE) ||
        (viface_hex_dump(&*self, packet, &hex_dump) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    printf("--- About to send the following packet.\n");
    printf("    Size: %d\n", packet[0]);
    printf("    CRC: 0x%x\n", crc_32);
    printf("%s\n\n", hex_dump);

    printf("--- Sending the packet %d times...", number_packets);

    // Sends the packet for 'number_packets' times
    for (i = 0; i < number_packets; i++) {
        if (i % 10 == 0) {
            printf("\n");
        }
        printf(" #%.2d ...", i + 1);
        if (viface_send(self, packet) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }
    printf("\n\n");

    // Prints statistics after sending packets
    printf("--- Statistics after sending packets:\n");
    if (print_statistics(self, stats_names) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Clears statistics
    printf("--- Clearing statistics:\n");

    for (i = 0; i < 23; i++) {
        uint64_t result_key;

        if ((viface_clear_stat(self, stats_names[i]) == EXIT_FAILURE) ||
            (viface_read_stat(self, stats_names[i],
                              &result_key) == EXIT_FAILURE)) {
            return EXIT_FAILURE;
        }
        printf("     %s : %zu\n", stats_names[i], result_key);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

/**
 * This example shows the use of the interface statistics interface to read
 * the number of packets and bytes sent.
 */
main(int argc, const char* argv[])
{
    struct viface* self;
    char name[IFNAMSIZ] = "viface0";
    int id = 1;

    printf("\n--- Starting stats example...\n\n");

    if (argc > 1) {
        strcpy(name, argv[1]);
    }

    /* These IF statements do the following:
     * 1) Creates interface
     * 2) Brings-up interface
     * 3) Checks interface statistics
     * 4) Brings-down interface
     */
    if ((viface_create_global_pool() == EXIT_FAILURE) ||
        (viface_create_viface(name, true, 0, &self) == EXIT_FAILURE) ||
        (viface_up(self) == EXIT_FAILURE) ||
        (check_statistics(self) == EXIT_FAILURE) ||
        (viface_down(self) == EXIT_FAILURE) ||
        (viface_destroy_viface(&self) == EXIT_FAILURE) ||
        (viface_destroy_global_pool() == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    return 0;
}