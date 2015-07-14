#include <iostream>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

using namespace std;

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
            return true;
        }
};

/**
 * This example shows how to setup a dispatcher for a set of virtual interfaces.
 * This uses a class method for the callback in order to show this kind of
 * usage, but any function using the same signature as dispatcher_cb type
 * can be used.
 *
 * To help with the example you can send a few packets to the created virtual
 * interfaces using scapy, wireshark, libtins or any other.
 */
int main(int argc, const char* argv[])
{
    cout << "Starting dispatch example ..." << endl;

    try {
        viface::VIface iface1("viface%d");
        iface1.up();
        cout << "Interface " << iface1.getName() << " up!" << endl;

        viface::VIface iface2("viface%d");
        iface2.up();
        cout << "Interface " << iface2.getName() << " up!" << endl;

        // Call dispatch
        set<viface::VIface*> myifaces = {&iface1, &iface2};

        MyDispatcher printer;
        viface::dispatcher_cb mycb = bind(
            &MyDispatcher::handler,
            &printer,
            placeholders::_1,
            placeholders::_2,
            placeholders::_3
            );

        cout << "Calling dispath ..." << endl;
        viface::dispatch(myifaces, mycb);
    } catch(exception const & ex) {
        cerr << ex.what() << endl;
        return -1;
    }

    return 0;
}