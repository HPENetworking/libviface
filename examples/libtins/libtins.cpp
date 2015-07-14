#include <iostream>
#include <tins/tins.h>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;

class TinsVIface: public viface::VIface
{
    public:

        // Default constructor... Ohhh C++ ...
        using viface::VIface::VIface;
        using viface::VIface::send;

        // Implement a send and receive function using libtins PDUs
        void send(Tins::PDU* packet) const {
            if (packet == NULL) {
                throw invalid_argument("packet is NULL");
            }
            vector<uint8_t> serialized = packet->serialize();
            this->send(serialized);
        }
};


/**
 * This example shows the basics to setup a virtual network interface.
 * This will configure and bring-up a network interface called viface0
 * (or use the name passed as first argument).
 */
int main(int argc, const char* argv[])
{
    cout << "Starting libtins example ..." << endl;

    try {
        // Setup interfaces
        TinsVIface iface("viface%d");
        iface.setIPv4("192.168.20.20");
        iface.up();
        cout << "Interface " << iface.getName() << " up!" << endl;

        // Create packet to send
        Tins::EthernetII pkt = \
            Tins::EthernetII("77:22:33:11:ad:ad", iface.getMAC()) /
            Tins::IP("192.168.20.1", iface.getIPv4()) /
            Tins::TCP(13, 15) /
            Tins::RawPDU("I'm a payload!");

        // Send packets
        cout << "Sending packets...";
        for (int i = 0; i < 100; i++) {
            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << setw(2) << i + 1 << " ...";
            iface.send(&pkt);
        }
        cout << endl;

        // Receive packets
        // FIXME: Implement!
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}