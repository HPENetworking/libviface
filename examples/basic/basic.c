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

// Interface configuration data
static char name[IFNAMSIZ] = "viface0";
static char ip[INET_ADDRSTRLEN] = "192.168.25.46";
static char mac[18] = "ec:f1:f8:d5:47:6b";
static char broadcast[INET_ADDRSTRLEN] = "192.168.25.255";
static char netmask[INET_ADDRSTRLEN] = "255.255.255.0";
static uint mtu = 1000;
static int id = 1;

/* Sets network configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int set_interface_configuration(struct viface* self)
{
    // Configures interface
    if ((viface_set_ipv4(self, ip) == EXIT_FAILURE) ||
        (viface_set_mac(self, mac) == EXIT_FAILURE) ||
        (viface_set_ipv4_broadcast(self, broadcast) == EXIT_FAILURE) ||
        (viface_set_ipv4_netmask(self, netmask) == EXIT_FAILURE) ||
        (viface_set_mtu(self, mtu) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* Checks the interface configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int check_interface_configuration(struct viface* self)
{
    int id_value;
    bool is_viface_up;
    uint mtu_value;
    char* viface_name;
    char* ip_value;
    char* mac_value;
    char* broadcast_value;
    char* netmask_value;

    // Gets interface configuration data
    if ((viface_get_name(self, &viface_name) == EXIT_FAILURE) ||
        (viface_get_id(self, &id_value) == EXIT_FAILURE) ||
        (viface_is_up(self, &is_viface_up) == EXIT_FAILURE) ||
        (viface_get_ipv4(self, &ip_value) == EXIT_FAILURE) ||
        (viface_get_mac(self, &mac_value) == EXIT_FAILURE) ||
        (viface_get_ipv4_broadcast(self, &broadcast_value) == EXIT_FAILURE) ||
        (viface_get_ipv4_netmask(self, &netmask_value) == EXIT_FAILURE) ||
        (viface_get_mtu(self, &mtu_value) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    printf("********** Viface Configuration Data **********\n\n");

    // Checks interface name
    printf("--- Viface name is: %s.\n", viface_name);
    printf("    Expected:       %s.\n\n", name);

    // Checks interface ID
    printf("--- Viface ID is: %d.\n", id_value);
    printf("    Expected:     %d.\n\n", id);

    // Checks if interface is Up
    printf("--- Interface isUp: %s\n", is_viface_up ? "true" : "false");
    printf("    Expected:       true.\n\n");

    // Checks interface IP
    printf("--- IP value is: %s\n", ip_value);
    printf("    Expected:    %s.\n\n", ip);

    // Checks mac value
    printf("--- MAC value is: %s\n", mac_value);
    printf("    Expected:     %s.\n\n", mac);

    // Checks broadcast value
    printf("--- Broadcast value is: %s\n", broadcast_value);
    printf("    Expected:           %s.\n\n", broadcast);

    // Checks netmask value
    printf("--- Netmask value is: %s\n", netmask_value);
    printf("    Expected:         %s.\n\n", netmask);

    // Checks mtu value
    printf("--- MTU value is: %d\n", mtu_value);
    printf("    Expected:     %u.\n\n", mtu);

    return EXIT_SUCCESS;
}

// Checks if the network interface is down
int check_interface_is_down(struct viface* self)
{
    // Checks if interface is down
    bool is_down;

    if (viface_is_up(self, &is_down) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    printf("--- Interface isDown: %s\n", is_down ? "false" : "true");
    printf("    Expected:         true\n\n");
    return EXIT_SUCCESS;
}

/**
 * This example shows the basics to setup a new virtual network
 * interface called viface0
 * (or use the name passed as first argument).
 * It will show how to:
 *  1) Set interface configuration data
 *     (name, id, ip, ipv6, mac, netmask, broadcast, mtu).
 *  2) Bring-up the network interface.
 *  3) Check the interface configuration data
 *     (name, id, ip, ipv6, mac, netmask, broadcast, mtu).
 */
main(int argc, const char* argv[])
{
    struct viface* self;

    printf("\n--- Starting basic example...\n\n");

    if (argc > 1) {
        strcpy(name, argv[1]);
    }

    /* These IF statements do the following:
     * 1) Creates global parent APR pool
     * 2) Creates interface
     * 3) Sets interface configuration data
     * 4) Brings-up interface
     * 5) Checks interface configuration data
     * 6) Brings-down interface
     * 7) Checks if interface is down
     * 8) Destroys viface struct
     * 9) Destroys global APR pool
     */
    if ((viface_create_global_pool() == EXIT_FAILURE) ||
        (viface_create_viface(name, true, id, &self) == EXIT_FAILURE) ||
        (set_interface_configuration(self) == EXIT_FAILURE) ||
        (viface_up(self) == EXIT_FAILURE) ||
        (check_interface_configuration(self) == EXIT_FAILURE) ||
        (viface_down(self) == EXIT_FAILURE) ||
        (check_interface_is_down(self) == EXIT_FAILURE) ||
        (viface_destroy_viface(&self) == EXIT_FAILURE) ||
        (viface_destroy_global_pool() == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    return 0;
}