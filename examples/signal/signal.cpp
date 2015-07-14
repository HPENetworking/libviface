#include <iostream>
#include <csignal>
#include <unistd.h>
#include <viface/viface.hpp>

using namespace std;

// Atomic boolean to determine if we need to exit the application
volatile sig_atomic_t quit = 0;

// Atomic counter to count thread interruptions
volatile sig_atomic_t interrupted = 0;

// Dispatcher callback, just print something
bool mycb(string const& name, uint id, vector<uint8_t>& packet) {
    cout << "+++ Received packet from interface " << name;
    cout << " (" << id << ") of size " << packet.size() << endl;
    return true;
}

// Signal handler routine
void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
            quit = 1;
            break;
        case SIGALRM:
            interrupted++;
            alarm(4);
            break;
    }
}

/**
 * This example shows how to setup a dispatcher that will restart on signal
 * interruptions. The dispatch() call will return only when a signal has
 * interrupted the underlying select() call (or when the timeout is specified,
 * but for that check timeout example).
 *
 * In this example we setup an alarm that will trigger a signal repeatedly,
 * but only a keyboard interrupt signal will stop the application.
 */
int main(int argc, const char* argv[])
{
    cout << "Starting signal example ..." << endl;

    try {
        // Setup interfaces
        viface::VIface iface1("viface%d");
        iface1.up();
        cout << "Interface " << iface1.getName() << " up!" << endl;
        viface::VIface iface2("viface%d");
        iface2.up();
        cout << "Interface " << iface2.getName() << " up!" << endl;
        set<viface::VIface*> myifaces = {&iface1, &iface2};

        // Setup signals
        signal(SIGINT, signal_handler);
        signal(SIGALRM, signal_handler);
        alarm(4);

        // Dispatch loop
        cout << "Calling dispath ... Press Ctrl+C to exit." << endl;
        while (true) {
            viface::dispatch(myifaces, mycb);
            if (quit) {
                cout << "Quit requested, exiting..." << endl;
                break;
            }
            cout << "dispath() interruption #" << interrupted;
            cout << ", restarting..." << endl;
        }
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}