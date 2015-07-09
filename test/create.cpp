#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "viface/viface.hpp"

TEST_CASE("Create")
{
    std::string name = "viface%d";
    viface::VIface iface(name);

    REQUIRE(
         iface.getName()
         ==
         "viface0"
    );
}
