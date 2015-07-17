#include <iostream>
#include <iomanip>
#include <algorithm>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;

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

/**
 * This example shows the use of the interface statistics interface to read
 * the number of packets and bytes sent.
 */
int main(int argc, const char* argv[])
{
    cout << "Starting stats example ..." << endl;

    try {
        // Create and bring-up interface
        viface::VIface iface;
        iface.up();

        // Get set of available statistics
        set<string> stats_set = iface.listStats();

        // Let create a list and order it just to output
        vector<string> stats(stats_set.size());
        stats.assign(stats_set.begin(), stats_set.end());
        sort(stats.begin(), stats.end());

        // Print statistics before send packets
        cout << endl;
        cout << "####################################################" << endl;
        cout << "* Statistics:" << endl;
        for (auto & key : stats) {
            cout << "    " << key << " : " << iface.readStat(key) << endl;
        }

        // Send packets
        cout << endl;
        cout << "####################################################" << endl;
        cout << "* About to send the following packet:" << endl;
        cout << "*     Size: " << pkt.size() << endl;
        cout << "*     CRC : 0x" << hex << viface::utils::crc32(pkt) << endl;
        cout << viface::utils::hexdump(pkt) << endl;
        cout << "* Sending packets...";
        for (int i = 0; i < 100; i++) {
            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << dec << setw(2) << i + 1 << " ...";

            iface.send(pkt);
        }
        cout << endl;

        // Print statistics after send packets
        cout << endl;
        cout << "####################################################" << endl;
        cout << "* Statistics:" << endl;
        for (auto & key : stats) {
            cout << "    " << key << " : " << iface.readStat(key) << endl;
        }

        // Clear and print statistics one more time
        cout << endl;
        cout << "####################################################" << endl;
        cout << "* Clearing statistics:" << endl;
        for (auto & key : stats) {
            iface.clearStat(key);
            cout << "    " << key << " : " << iface.readStat(key) << endl;
        }
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}