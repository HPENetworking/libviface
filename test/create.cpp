#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "viface/viface.hpp"

TEST_CASE("Create")
{
    std::string name = "viface%d";
    std::string ipv4 = "192.168.20.21";

    viface::VIface iface(name);

    // Create interface
    REQUIRE(iface.getName() == "viface0");

    // Configure interface
    REQUIRE_NOTHROW(iface.setIPv4(ipv4));

    // Bring-up interface
    REQUIRE_NOTHROW(iface.up());
    REQUIRE(iface.isUp() == true);

    // Check parameters
    REQUIRE(iface.getMTU() == 1500);
    REQUIRE(iface.getIPv4() == ipv4);
}
