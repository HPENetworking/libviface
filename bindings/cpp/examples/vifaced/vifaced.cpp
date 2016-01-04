#include <iostream>
#include <csignal>
#include <cstdlib>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;


volatile sig_atomic_t quit = 0;

/**
 * Signal handling.
 */
void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
            quit = 1;
            break;
    }
}

/**
 * Dispatcher printer callback.
 */
bool dispatcher(string const& name, uint id, vector<uint8_t>& packet) {
    static uint count = 0;

    cout << "+++ Received packet " << dec << count;
    cout << " from interface " << name;
    cout << " (" << id << ") of size " << packet.size();
    cout << " and CRC of 0x" << hex << viface::utils::crc32(packet);
    cout << endl;
    cout << viface::utils::hexdump(packet) << endl;
    count++;

    return quit == 0;
}

/**
 * This example shows how to create a small daemon that creates some
 * virtual interfaces using given prefix.
 */
int main(int argc, const char* argv[])
{
    // Check if arguments are provided
    if (argc < 3) {
        cerr << "-- Usage: vifaced [prefix] [num_ifaces]" << endl;
        return -1;
    }

    // Parse arguments
    string prefix = string(argv[1]) + "%d";
    int num_ifaces = atoi(argv[2]);

    // Check arguments values
    if (num_ifaces < 1) {
        cerr << "-- Invalid number of interfaces." << endl;
        return -1;
    }

    cout << "Starting viface daemon " VIFACE_VERSION " ..." << endl;

    set<viface::VIface*> ifaces;

    try {
        for (int i = 0; i < num_ifaces; i++) {
            viface::VIface* iface = new viface::VIface(prefix);
            ifaces.insert(iface);
            iface->up();
            cout << "Interface " << iface->getName() << " up!" << endl;
        }

        cout << "Starting dispath ..." << endl;
        signal(SIGINT, signal_handler);
        viface::dispatch(ifaces, dispatcher);
        cout << "Bye!" << endl;
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}