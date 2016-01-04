#include <iostream>
#include <viface/viface.hpp>

using namespace std;

// Dispatcher callback, just print something
bool mycb(string const& name, uint id, vector<uint8_t>& packet) {
    cout << "+++ Received packet from interface " << name;
    cout << " (" << id << ") of size " << packet.size() << endl;
    return true;
}

/**
 * This example shows how to setup a dispatcher that will use a 5 seconds
 * timeout. In this the dispatch will restart 5 times then exit.
 */
int main(int argc, const char* argv[])
{
    cout << "Starting timeout example ..." << endl;

    try {
        // Setup interfaces
        viface::VIface iface1("viface%d");
        iface1.up();
        cout << "Interface " << iface1.getName() << " up!" << endl;
        viface::VIface iface2("viface%d");
        iface2.up();
        cout << "Interface " << iface2.getName() << " up!" << endl;
        set<viface::VIface*> myifaces = {&iface1, &iface2};

        // Dispatch loop
        for (int times = 1; times < 6; times++) {
            viface::dispatch(myifaces, mycb, 5000);
            cout << "dispatch() timeout #" << times << " ..." << endl;
        }
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}