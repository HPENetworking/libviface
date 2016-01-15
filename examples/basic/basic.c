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
static char *ip = "192.168.25.46";
static char* mac = "ec:f1:f8:d5:47:6b";
static char* broadcast = "192.168.25.255";
static char* netmask = "255.255.255.0";
static uint mtu = 1000;
static int id = 1;

// Creates a network interface
int createsInterface(struct viface **self, apr_pool_t **parent_pool)
{
    // Creates interface
    if ((viface_create(&*parent_pool, &*self) == EXIT_FAILURE) ||
        (vifaceImpl(&*self, name, true, id) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Sets network configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int setInterfaceConfiguration(struct viface **self)
{
    // Configures interface
    if ((setIPv4(&*self, ip) == EXIT_FAILURE) ||
        (setMAC(&*self, mac) == EXIT_FAILURE) ||
        (setIPv4Broadcast(&*self, broadcast) == EXIT_FAILURE) ||
        (setIPv4Netmask(&*self, netmask) == EXIT_FAILURE) ||
        (setMTU(&*self, mtu) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* Checks the interface configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int checkInterfaceConfiguration(struct viface **self)
{
    char *vifaceName;
    int idValue;
    bool is_up;
    char *ipValue;
    char *macValue;
    char *broadcastValue;
    char *netmaskValue;
    uint mtuValue;

    // Gets interface configuration data
    if ((getName(&*self, &vifaceName) == EXIT_FAILURE) ||
        (getID(&*self, &idValue) == EXIT_FAILURE) ||
        (isUp(&*self, &is_up) == EXIT_FAILURE) ||
        (getIPv4(&*self, &ipValue) == EXIT_FAILURE) ||
        (getMAC(&*self, &macValue) == EXIT_FAILURE) ||
        (getIPv4Broadcast(&*self, &broadcastValue) == EXIT_FAILURE) ||
        (getIPv4Netmask(&*self, &netmaskValue) == EXIT_FAILURE) ||
        (getMTU(&*self, &mtuValue) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    printf("********** Viface Configuration Data **********\n\n");

    // Checks interface name
    printf("--- Viface name is: %s.\n", vifaceName);
    printf("    Expected:       %s.\n\n", name);

    // Checks interface ID
    printf("--- Viface ID is: %d.\n", idValue);
    printf("    Expected:     %d.\n\n", id);

    // Checks if interface is Up
    printf("--- Interface isUp: %s\n", is_up ? "true" : "false");
    printf("    Expected:       true.\n\n");

    // Checks interface IP
    printf("--- IP value is: %s\n", ipValue);
    printf("    Expected:    %s.\n\n", ip);

    // Checks mac value
    printf("--- MAC value is: %s\n", macValue);
    printf("    Expected:     %s.\n\n", mac);

    // Checks broadcast value
    printf("--- Broadcast value is: %s\n", broadcastValue);
    printf("    Expected:           %s.\n\n", broadcast);

    // Checks netmask value
    printf("--- Netmask value is: %s\n", netmaskValue);
    printf("    Expected:         %s.\n\n", netmask);

    // Checks mtu value
    printf("--- MTU value is: %d\n", mtuValue);
    printf("    Expected:     %u.\n\n", mtu);

    return EXIT_SUCCESS;
}

// Checks if the network interface is down
int checkInterfaceIsDown(struct viface **self)
{
    // Checks if interface is down
    bool is_down;

    if (isUp(&*self, &is_down) == EXIT_FAILURE) {
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
    struct viface *self;

    printf("\n--- Starting basic example...\n\n");

    if (argc > 1) {
        strcpy(name, argv[1]);
    }

    apr_initialize();

    // Creates parent pool
    apr_pool_t *parent_pool;
    apr_pool_create(&parent_pool, NULL);

    /* These IF statements do the following:
     * 1) Creates interface
     * 2) Sets interface configuration data
     * 3) Brings-up interface
     * 4) Checks interface configuration data
     * 5) Brings-down interface
     * 6) Checks if interface is down
     */
    if ((createsInterface(&self, &parent_pool) == EXIT_FAILURE) ||
        (setInterfaceConfiguration(&self) == EXIT_FAILURE) ||
        (up(&self) == EXIT_FAILURE) ||
        (checkInterfaceConfiguration(&self) == EXIT_FAILURE) ||
        (down(&self) == EXIT_FAILURE) ||
        (checkInterfaceIsDown(&self) == EXIT_FAILURE) ||
        (viface_destroy(&self) == EXIT_FAILURE)) {
        return EXIT_FAILURE;
    }

    apr_pool_destroy(parent_pool);
    apr_terminate();

    return 0;
}