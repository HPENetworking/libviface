#include <iomanip>
#include <iostream>
#include <tins/tins.h>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;

class TinsVIface : public viface::VIface
{
    public:

        // Default constructor
        using viface::VIface::VIface;

        // Overload the send method to receive libtins PDUs
        using viface::VIface::send;
        void send(Tins::PDU* packet) const {
            if (packet == NULL) {
                throw invalid_argument("packet is NULL");
            }
            vector<uint8_t> raw = packet->serialize();
            this->send(raw);
        }

        // Overload the receive method to return libtins PDUs
        // I know using receive_tins is ugly, but hey, C++ cannot overload
        // based on return type :/
        using viface::VIface::receive;
        Tins::PDU* receive_tins() {
            vector<uint8_t> raw = this->receive();
            if (raw.size() == 0) {
                return NULL;
            }
            return new Tins::RawPDU(&raw[0], raw.size());
        }
};


/**
 * This example several levels of integrating the libviface library with the
 * awesome libtins library.
 *
 * We try the following:
 *
 * - Create a TinsVIface wrapper class for the liviface VIface class that
 *   overloads the send() and receive() methods to use libtins PDU's for
 *   packets I/O.
 * - We use libtins sniffing capabilities to capture on the other end a series
 *   of packets we sent out to the interface.
 * - We use libtins generation capabilities to receive packets on our
 *   interface.
 * - In both sending and receiving we store the packets in a pcap file.
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

        // Create packet for the demo
        Tins::EthernetII pkt = \
            Tins::EthernetII("77:22:33:11:ad:ad", iface.getMAC()) /
            Tins::IP("192.168.20.1", iface.getIPv4()) /
            Tins::TCP(13, 15) /
            Tins::RawPDU("I'm a payload!");

        /**
         * Send and capture packets
         *
         * We are going to send packets from our network interface and capture
         * them (sniff and store) on the other side using libtins.
         */
        string pcap_s = "/tmp/libtins_sent.pcap";
        Tins::Sniffer sniffer(iface.getName());
        Tins::PacketWriter writer_s(pcap_s, Tins::PacketWriter::ETH2);

        // Send packets
        cout << "####################################################" << endl;
        cout << "* Sending packets...";
        for (int i = 0; i < 100; i++) {
            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << setw(2) << i + 1 << " ...";

            iface.send(&pkt);
        }
        cout << endl;

        // Store sniffed packets
        cout << "* Sniffing and storing packets...";
        for (int i = 0; i < 100; i++) {
            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << setw(2) << i + 1 << " ...";

            Tins::PDU* sniff = sniffer.next_packet();
            writer_s.write(sniff);
            delete sniff;
        }
        cout << endl;

        /**
         * Generate and receive packets
         *
         * We are going to generate packets to our network interface using
         * libtins and receive and store them on our side.
         */
        string pcap_r = "/tmp/libtins_received.pcap";
        Tins::PacketSender sender;
        Tins::NetworkInterface tins_iface(iface.getName());
        Tins::PacketWriter writer_r(pcap_r, Tins::PacketWriter::ETH2);

        // Generate packets
        cout << "####################################################" << endl;
        cout << "* Generate packets...";
        for (int i = 0; i < 100; i++) {
            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << setw(2) << i + 1 << " ...";

            sender.send(pkt, tins_iface);
        }
        cout << endl;

        // Receive packets
        // This is a naive packet reception routine, don't do this, use a
        // dispatcher, this is just to demostrate the integration.
        // BTW, it is common to have receive more than 100 packets here, as
        // different processes in the userspace or kernel space send a few
        // packets to query stuffs.
        cout << "* Receiving and storing packets...";
        int i = 0;
        while (true) {
            Tins::PDU* received = iface.receive_tins();
            if (received == NULL) {
                break;
            }

            if (i % 10 == 0) {
                cout << endl;
            }
            cout << " #" << setw(2) << i + 1 << " ...";

            writer_r.write(received);
            delete received;
            i++;
        }
        cout << endl;
        cout << "* No more packets available, exiting..." << endl << endl;
        cout << "* Packets sent from the interface: " << pcap_s << endl;
        cout << "* Packets received in the interface: " << pcap_r << endl;
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}