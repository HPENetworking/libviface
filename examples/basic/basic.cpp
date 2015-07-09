#include <iostream>
#include "viface/viface.hpp"

using namespace std;

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
        viface::VIface iface(name);
    } catch(exception const & ex) {
        cerr << "Unable to create virtual interface " << name << endl;
        cerr << ":: " << ex.what() << endl;
        return -1;
    }

    return 0;
}