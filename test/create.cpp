#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "viface/viface.hpp"

TEST_CASE("Create")
{
    std::string name = "viface%d";
    std::string mac = "66:23:2d:28:c6:84";
    std::string ipv4 = "192.168.20.21";
    std::string netmask = "255.255.255.0";
    std::string broadcast = "192.168.20.255";
    std::set<std::string> ipv6s = {
        "fe80::6423:2dff:fe28:c684",
        "fe80::6423:2dff:fe28:c666",
        "fe80::6423:2dff:fe28:c622"
    };

    viface::VIface iface(name);

    // Create interface
    REQUIRE(iface.getName() == "viface0");

    // Configure interface
    REQUIRE_NOTHROW(iface.setMAC(mac));
    REQUIRE_NOTHROW(iface.setIPv4(ipv4));
    REQUIRE_NOTHROW(iface.setIPv4Netmask(netmask));
    REQUIRE_NOTHROW(iface.setIPv4Broadcast(broadcast));
    REQUIRE_NOTHROW(iface.setIPv6(ipv6s));

    // Bring-up interface
    REQUIRE_NOTHROW(iface.up());
    REQUIRE(iface.isUp() == true);

    // Check parameters
    REQUIRE(iface.getMTU() == 1500);
    REQUIRE(iface.getMAC() == mac);
    REQUIRE(iface.getIPv4() == ipv4);
    REQUIRE(iface.getIPv4Netmask() == netmask);
    REQUIRE(iface.getIPv4Broadcast() == broadcast);

    // List IPv6 addresses
    std::cout << "IPv6 Addresses found:" << std::endl;
    std::set<std::string> ipv6_addrs = iface.getIPv6();
    for (auto & ipv6_addr : ipv6_addrs) {
        std::cout << "    " << ipv6_addr << std::endl;
    }

    // List statistics keys
    std::set<std::string> stats = iface.listStats();
    std::cout << "Statistics found:" << std::endl;
    for (auto & key : stats) {
        std::cout << "    " << key << " : " << iface.readStat(key) << std::endl;
    }
}
