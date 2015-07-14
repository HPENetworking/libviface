#include <iostream>
#include <viface/viface.hpp>

using namespace std;

/**
 * This example shows the basics to setup a virtual network interface.
 * This will configure and bring-up a network interface called viface0
 * (or use the name passed as first argument).
 */
int main(int argc, const char* argv[])
{
    string name = "viface0";
    if (argc > 1) {
        name = string(argv[1]);
    }

    string ip = "192.168.25.46";
    if (argc > 2) {
        ip = string(argv[2]);
    }

    string mac = "ec:f1:f8:d5:47:6b";
    if (argc > 3) {
        mac = string(argv[3]);
    }

    try {
        // Create interface
        viface::VIface iface(name);

        // Configure interface
        iface.setIPv4(ip);
        iface.setMAC(mac);

        // Bring up interface
        iface.up();
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}