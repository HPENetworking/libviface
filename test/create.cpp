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

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "viface/viface.h"

// Interface configuration data
static char name[IFNAMSIZ];
static char ip[] = "192.168.25.46";
static char mac[] = "ec:f1:f8:d5:47:6b";
static char broadcast[] = "192.168.25.255";
static char netmask[] = "255.255.255.0";
static uint mtu = 1000;
static int id = 1;

// Creates a network interface
int createsInterface(struct viface **self, apr_pool_t **parent_pool)
{
    // Creates interface
    REQUIRE(viface_create(&*parent_pool, &*self) == EXIT_SUCCESS);
    REQUIRE(vifaceImpl(&*self, name, true, 1) == EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

/* Sets network configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int setInterfaceConfiguration(struct viface **self)
{
    // Configures interface
    REQUIRE(setIPv4(&*self, ip) == EXIT_SUCCESS);
    REQUIRE(setMAC(&*self, mac) == EXIT_SUCCESS);
    REQUIRE(setIPv4Broadcast(&*self, broadcast) == EXIT_SUCCESS);
    REQUIRE(setIPv4Netmask(&*self, netmask) == EXIT_SUCCESS);
    REQUIRE(setMTU(&*self, mtu) == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

/* Checks the interface configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int checkInterfaceConfiguration(struct viface **self, bool newViface)
{
    char *vifaceName;
    uint idValue;
    bool is_up;
    char *ipValue;
    char *macValue;
    char *broadcastValue;
    char *netmaskValue;
    uint mtuValue;

    // Gets interface configuration data
    REQUIRE(getName(&*self, &vifaceName) == EXIT_SUCCESS);
    REQUIRE(getID(&*self, &idValue) == EXIT_SUCCESS);
    REQUIRE(isUp(&*self, &is_up) == EXIT_SUCCESS);
    REQUIRE(getIPv4(&*self, &ipValue) == EXIT_SUCCESS);
    REQUIRE(getMAC(&*self, &macValue) == EXIT_SUCCESS);
    REQUIRE(getIPv4Broadcast(&*self, &broadcastValue) == EXIT_SUCCESS);
    REQUIRE(getIPv4Netmask(&*self, &netmaskValue) == EXIT_SUCCESS);
    REQUIRE(getMTU(&*self, &mtuValue) == EXIT_SUCCESS);

    if (newViface) {
        // Checks interface name
        REQUIRE(strcmp(vifaceName, name) == 0);

        // Checks interface ID
        REQUIRE(idValue == id);

        // Checks if interface is Up
        REQUIRE(is_up == true);

        // Checks interface IP
        REQUIRE(strcmp(ipValue, ip) == 0);

        // Checks mac value
        REQUIRE(strcmp(macValue, mac) == 0);

        // Checks broadcast value
        REQUIRE(strcmp(broadcastValue, broadcast) == 0);

        // Checks netmask value
        REQUIRE(strcmp(netmaskValue, netmask) == 0);

        // Checks mtu value
        REQUIRE(mtuValue == mtu);
    }
    return EXIT_SUCCESS;
}

TEST_CASE("Creates and tests new network interface")
{
    bool is_up;
    struct viface *self;

    apr_initialize();

    // Sets new viface name
    strcpy(name, "viface0");

    // Creates parent pool
    apr_pool_t *parent_pool;
    apr_pool_create(&parent_pool, NULL);

    // Creates interface
    REQUIRE(createsInterface(&self, &parent_pool) == EXIT_SUCCESS);

    // Sets interface configuration data
    REQUIRE(setInterfaceConfiguration(&self) == EXIT_SUCCESS);

    // Brings-up interface
    REQUIRE(up(&self) == 0);

    // Checks interface configuration data
    REQUIRE(checkInterfaceConfiguration(&self, true) == EXIT_SUCCESS);

    // Brings-down interface
    REQUIRE(down(&self) == 0);

    // Checks if interface is still Up
    REQUIRE(isUp(&self, &is_up) == EXIT_SUCCESS);
    REQUIRE(is_up == false);
}

TEST_CASE("Hooks and tests existing network interface")
{
    bool is_up;
    struct viface *self;

    apr_initialize();

    // Sets existing viface name
    strcpy(name, "eth0");

    // Creates parent pool
    apr_pool_t *parent_pool;
    apr_pool_create(&parent_pool, NULL);

    // Creates interface
    REQUIRE(createsInterface(&self, &parent_pool) == EXIT_SUCCESS);

    // Checks interface configuration data
    REQUIRE(checkInterfaceConfiguration(&self, false) == EXIT_SUCCESS);
}