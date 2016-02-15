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
    REQUIRE(viface_set_ipv4(self, ip) == EXIT_SUCCESS);
    REQUIRE(viface_set_mac(self, mac) == EXIT_SUCCESS);
    REQUIRE(viface_set_ipv4_broadcast(self, broadcast) == EXIT_SUCCESS);
    REQUIRE(viface_set_ipv4_netmask(self, netmask) == EXIT_SUCCESS);
    REQUIRE(viface_set_mtu(self, mtu) == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

/* Checks the interface configuration data
 * (name, id, ip, mac, netmask, broadcast, mtu).
 */
int check_interface_configuration(struct viface* self, bool new_viface)
{
    uint id_value;
    bool is_viface_up;
    uint mtu_value;
    char* viface_name;
    char* ip_value;
    char* mac_value;
    char* broadcast_value;
    char* netmask_value;

    // Gets interface configuration data
    REQUIRE(viface_get_name(self, &viface_name) == EXIT_SUCCESS);
    REQUIRE(viface_get_id(self, &id_value) == EXIT_SUCCESS);
    REQUIRE(viface_is_up(self, &is_viface_up) == EXIT_SUCCESS);
    REQUIRE(viface_get_ipv4(self, &ip_value) == EXIT_SUCCESS);
    REQUIRE(viface_get_mac(self, &mac_value) == EXIT_SUCCESS);
    REQUIRE(viface_get_ipv4_broadcast(self, &broadcast_value) == EXIT_SUCCESS);
    REQUIRE(viface_get_ipv4_netmask(self, &netmask_value) == EXIT_SUCCESS);
    REQUIRE(viface_get_mtu(self, &mtu_value) == EXIT_SUCCESS);

    if (new_viface) {
        // Checks interface name
        REQUIRE(strcmp(viface_name, name) == 0);

        // Checks interface ID
        REQUIRE(id_value == id);

        // Checks if interface is Up
        REQUIRE(is_viface_up == true);

        // Checks interface IP
        REQUIRE(strcmp(ip_value, ip) == 0);

        // Checks mac value
        REQUIRE(strcmp(mac_value, mac) == 0);

        // Checks broadcast value
        REQUIRE(strcmp(broadcast_value, broadcast) == 0);

        // Checks netmask value
        REQUIRE(strcmp(netmask_value, netmask) == 0);

        // Checks mtu value
        REQUIRE(mtu_value == mtu);
    }
    return EXIT_SUCCESS;
}

TEST_CASE("Creates and tests new network interface")
{
    bool is_viface_up;
    struct viface* self;

    // Sets new viface name
    strcpy(name, "viface0");

    // Creates global APR pool
    REQUIRE(viface_create_global_pool() == EXIT_SUCCESS);

    // Creates interface
    REQUIRE(viface_create_viface(name, true, id, &self) == EXIT_SUCCESS);

    // Sets interface configuration data
    REQUIRE(set_interface_configuration(self) == EXIT_SUCCESS);

    // Brings-up interface
    REQUIRE(viface_up(self) == 0);

    // Checks interface configuration data
    REQUIRE(check_interface_configuration(self, true) == EXIT_SUCCESS);

    // Brings-down interface
    REQUIRE(viface_down(self) == 0);

    // Checks if interface is still Up
    REQUIRE(viface_is_up(self, &is_viface_up) == EXIT_SUCCESS);
    REQUIRE(is_viface_up == false);

    // Destroys viface struct
    REQUIRE(viface_destroy_viface(&self) == EXIT_SUCCESS);

    // Destroys static global APR pool
    REQUIRE(viface_destroy_global_pool() == EXIT_SUCCESS);
}

TEST_CASE("Hooks and tests existing network interface")
{
    bool is_viface_up;
    struct viface* self;

    // Sets existing viface name
    strcpy(name, "eth0");

    // Creates global APR pool
    REQUIRE(viface_create_global_pool() == EXIT_SUCCESS);

    // Creates interface
    REQUIRE(viface_create_viface(name, true, id, &self) == EXIT_SUCCESS);

    // Checks interface configuration data
    REQUIRE(check_interface_configuration(self, false) == EXIT_SUCCESS);

    // Destroys viface struct
    REQUIRE(viface_destroy_viface(&self) == EXIT_SUCCESS);

    // Destroys static global APR pool
    REQUIRE(viface_destroy_global_pool() == EXIT_SUCCESS);
}