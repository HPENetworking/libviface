#include <iostream>
#include <thread>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;

// Atomic boolean to determine if we need to exit the application
volatile sig_atomic_t quit = 0;

// Example packet. In Scapy:
// pkt = Ether()/IP()/TCP()/Raw('I\'m a packet!'*3)
// len(pkt) == 93
static vector<uint8_t> pkt = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x00, 0x45, 0x00, 0x00, 0x4F, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
0x7C, 0xA6, 0x7F, 0x00, 0x00, 0x01, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x14,
0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02,
0x20, 0x00, 0x04, 0x91, 0x00, 0x00, 0x49, 0x27, 0x6D, 0x20, 0x61, 0x20,
0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20, 0x61,
0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20,
0x61, 0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21
};

// Signal handler routine
void signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
            quit = 1;
            break;
    }
}

// Packet reception dispatcher
class MyDispatcher
{
    private:

        int count = 0;

    public:

        bool handler(string const& name, uint id, vector<uint8_t>& packet) {
            cout << "+++ Received packet " << dec << count;
            cout << " from interface " << name;
            cout << " (" << id << ") of size " << packet.size();
            cout << " and CRC of 0x" << hex << viface::utils::crc32(packet);
            cout << endl;
            cout << viface::utils::hexdump(packet) << endl;
            this->count++;

            // Continue processing only if quit wasn't requested
            return quit == 0;
        }
};

// Receive worker function
void receive_wkr(viface::VIface* iface)
{
    set<viface::VIface*> ifaces = {iface};
    MyDispatcher printer;
    viface::dispatcher_cb mycb = bind(
        &MyDispatcher::handler,
        &printer,
        placeholders::_1,
        placeholders::_2,
        placeholders::_3
        );

    while (!quit) {
        // We configure a dispatcher with a timeout of 1 second
        viface::dispatch(ifaces, mycb, 1000);
    }
}

// Send worker function
void send_wkr(viface::VIface* iface)
{
    while (!quit) {
        iface->send(pkt);
        this_thread::sleep_for(chrono::seconds(1));
    }
}

/**
 * This example shows how to use threads to handle both IO tasks of a virtual
 * interface: send and receive packets.
 *
 * We setup a signal handler to catch the SIGINT signal (Ctrl+C) that will
 * request both threads to stop. Those threads will poll a flag to terminate.
 *
 * Sadly, signals doesn't propagate to child threads, and thus select() and
 * sleep() calls don't get interrupted, and thus polling is required :/
 */
int main(int argc, const char* argv[])
{
    cout << "Starting threads example ..." << endl;

    try {
        // Create and bring-up interface
        viface::VIface iface;
        iface.up();

        // Setup signal handling to stop execution
        signal(SIGINT, signal_handler);

        // Create reception thread
        thread thr_r(receive_wkr, &iface);

        // Create sending thread
        thread thr_s(send_wkr, &iface);

        // Wait for both of them to finish
        cout << "* Threads started... Ctrl+C to exit." << endl;
        thr_r.join();
        thr_s.join();
        cout << "* Welcome back to the main thread..." << endl;

        // Print final statistics and exit
        set<string> stats_set = iface.listStats();

        // Let create a list and order it just to output
        vector<string> stats(stats_set.size());
        stats.assign(stats_set.begin(), stats_set.end());
        sort(stats.begin(), stats.end());

        // Print statistics before quitting
        cout << "* Statistics:" << endl;
        for (auto & key : stats) {
            cout << "    " << key << " : ";
            cout << dec << iface.readStat(key) << endl;
        }

        cout << "* Bye!" << endl;
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}