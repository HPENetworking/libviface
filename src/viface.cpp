/**
 * Copyright (C) 2015 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "viface/private/viface.hpp"

namespace viface
{
/*= Helpers ==================================================================*/

static bool parse_mac(vector<uint8_t>& out, string const& in)
{
    unsigned int bytes[6];
    int scans = sscanf(
        in.c_str(),
        "%02x:%02x:%02x:%02x:%02x:%02x",
        &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]
        );

    if (scans != 6) {
        return false;
    }

    out.assign(&bytes[0], &bytes[6]);
    return true;
}


static void read_flags(int sockfd, string name, struct ifreq* ifr)
{
    ostringstream what;

    // Prepare communication structure
    memset(ifr, 0, sizeof(struct ifreq));

    // Set interface name
    (void) strncpy(ifr->ifr_name, name.c_str(), IFNAMSIZ - 1);

    // Read interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, ifr) != 0) {
        what << "--- Unable to read " << name << " flags." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }
}


static string alloc_viface(string name, bool tap, viface_queues_t* queues)
{
    int i = 0;
    int fd = -1;
    ostringstream what;

    /* Create structure for ioctl call
     *
     * Flags: IFF_TAP   - TAP device (layer 2, ethernet frame)
     *        IFF_TUN   - TUN device (layer 3, IP packet)
     *        IFF_NO_PI - Do not provide packet information
     *        IFF_MULTI_QUEUE - Create a queue of multiqueue device
     */
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_NO_PI | IFF_MULTI_QUEUE;
    if (tap) {
        ifr.ifr_flags |= IFF_TAP;
    } else {
        ifr.ifr_flags |= IFF_TUN;
    }

    (void) strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ - 1);

    // Allocate queues
    for (i = 0; i < 2; i++) {
        // Open TUN/TAP device
        fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            what << "--- Unable to open TUN/TAP device." << endl;
            what << "    Name: " << name << " Queue: " << i << endl;
            what << "    Error: " << strerror(errno);
            what << " (" << errno << ")." << endl;
            goto err;
        }

        // Register a network device with the kernel
        if (ioctl(fd, TUNSETIFF, (void *)&ifr) != 0) {
            what << "--- Unable to register a TUN/TAP device." << endl;
            what << "    Name: " << name << " Queue: " << i << endl;
            what << "    Error: " << strerror(errno);
            what << " (" << errno << ")." << endl;

            if (close(fd) < 0) {
                what << "--- Unable to close a TUN/TAP device." << endl;
                what << "    Name: " << name << " Queue: " << i << endl;
                what << "    Error: " << strerror(errno);
                what << " (" << errno << ")." << endl;
            }
            goto err;
        }

        ((int *)queues)[i] = fd;
    }

    return string(ifr.ifr_name);

err:
    // Rollback close file descriptors
    for (--i; i >= 0; i--) {
        if (close(((int *)queues)[i]) < 0) {
            what << "--- Unable to close a TUN/TAP device." << endl;
            what << "    Name: " << name << " Queue: " << i << endl;
            what << "    Error: " << strerror(errno);
            what << " (" << errno << ")." << endl;
        }
    }

    throw runtime_error(what.str());
}


/*= Virtual Interface Implementation =========================================*/

uint VIfaceImpl::idseq = 0;

VIfaceImpl::VIfaceImpl(string name, bool tap, int id)
{
    // Set id
    if (id < 0) {
        this->id = this->idseq;
    } else {
        this->id = id;
    }
    this->idseq++;

    // Check name length
    if (name.length() >= IFNAMSIZ) {
        throw invalid_argument("Virtual interface name too long.");
    }

    // Create queues and assign name
    viface_queues_t queues;
    memset(&queues, 0, sizeof(viface_queues_t));
    this->name = alloc_viface(name, tap, &queues);
    this->queues = queues;

    // Create socket channel to the NET kernel for later ioctl
    this->kernel_socket = -1;
    this->kernel_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->kernel_socket < 0) {
        ostringstream what;
        what << "--- Unable to create socket channel to the NET kernel.";
        what << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }
}

void VIfaceImpl::setMAC(string mac)
{
    ostringstream what;

    // Ignore non-set MAC address
    if (mac.length() == 0) {
        return;
    }

    vector<uint8_t> parsed(6);
    if (!parse_mac(parsed, mac)) {
        what << "--- Invalid MAC address (" << mac << ") for ";
        what << this->name << "." << endl;
        throw invalid_argument(what.str());
    }

    this->mac = mac;
    return;
}

string VIfaceImpl::getMAC() const
{
    ostringstream what;

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    if (ioctl(this->kernel_socket, SIOCGIFHWADDR, &ifr) != 0) {
        what << "--- Unable to get MAC addr for " << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    // Convert binary MAC address to string
    ostringstream addr;
    addr << hex << std::setfill('0');
    for (int i = 0; i < 6; i++) {
        addr << setw(2) << (unsigned int) (0xFF & ifr.ifr_hwaddr.sa_data[i]);
        if (i != 5) {
            addr << ":";
        }
    }
    return addr.str();
}

void VIfaceImpl::setIPv4(string ipv4)
{
    ostringstream what;

    // Ignore non-set IPv4 address
    if (ipv4.length() == 0) {
        return;
    }

    // Validate format
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;
    if (!inet_pton(AF_INET, ipv4.c_str(), &addr->sin_addr)) {
        what << "--- Invalid IPv4 address (" << ipv4 << ") for ";
        what << this->name << "." << endl;
        throw invalid_argument(what.str());
    }

    this->ipv4 = ipv4;
    return;
}

string VIfaceImpl::getIPv4() const
{
    ostringstream what;

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    if (ioctl(this->kernel_socket, SIOCGIFADDR, &ifr) != 0) {
        what << "--- Unable to get IPv4 for " << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    // Convert binary IP address to string
    char addr[INET_ADDRSTRLEN];
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in* ipaddr = (struct sockaddr_in*) &ifr.ifr_addr;
    if (inet_ntop(AF_INET, &(ipaddr->sin_addr), addr, sizeof(addr)) == NULL) {
        what << "--- Unable to convert IPv4 for " << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    return string(addr);
}

void VIfaceImpl::setMTU(uint mtu)
{
    ostringstream what;

    // RFC 791, p. 24: "Every internet module must be able to forward a
    // datagram of 68 octets without further fragmentation."
    if (mtu < 68) {
        what << "--- MTU " << mtu << " too small (< 68)." << endl;
        throw invalid_argument(what.str());
    }

    // Are we sure about this upper validation?
    // lo interface reports this number for it's MTU
    if (mtu > 65536) {
        what << "--- MTU " << mtu << " too large (> 65536)." << endl;
        throw invalid_argument(what.str());
    }

    this->mtu = mtu;

    return;
}

uint VIfaceImpl::getMTU() const
{
    ostringstream what;

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    if (ioctl(this->kernel_socket, SIOCGIFMTU, &ifr) != 0) {
        what << "--- Unable to get MTU for " << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    return ifr.ifr_mtu;
}

void VIfaceImpl::up()
{
    ostringstream what;

    if (this->isUp()) {
        what << "--- Virtual interface " << this->name;
        what << " is already up." << endl;
        what << "    up() Operation not permitted." << endl;
        throw runtime_error(what.str());
    }

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    // Set MAC address
    vector<uint8_t> mac_bin(6);
    if (!parse_mac(mac_bin, this->mac)) {
        what << "--- Invalid cached MAC address (" << this->mac << ") for ";
        what << this->name << "." << endl;
        what << "    Something really bad happened :/" << endl;
        throw runtime_error(what.str());
    }

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    for (int i = 0; i < 6; i++) {
        ifr.ifr_hwaddr.sa_data[i] = mac_bin[i];
    }
    if (ioctl(this->kernel_socket, SIOCSIFHWADDR, &ifr) != 0) {
        what << "--- Unable to set MAC Address (" << this->mac << ") for ";
        what << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    // Set IPv4
    struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;

    addr->sin_family = AF_INET;
    if (!inet_pton(AF_INET, this->ipv4.c_str(), &addr->sin_addr)) {
        what << "--- Invalid cached IPv4 address (" << this->ipv4 << ") for ";
        what << this->name << "." << endl;
        what << "    Something really bad happened :/" << endl;
        throw runtime_error(what.str());
    }

    if (ioctl(this->kernel_socket, SIOCSIFADDR, &ifr) != 0) {
        what << "--- Unable to set IPv4 (" << this->ipv4 << ") for ";
        what << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    // Set MTU
    ifr.ifr_mtu = this->mtu;
    if (ioctl(this->kernel_socket, SIOCSIFMTU, &ifr) != 0) {
        what << "--- Unable to set MTU (" << this->mtu << ") for ";
        what << this->name << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }
    this->pktbuff.resize(this->mtu);

    // Bring-up interface
    ifr.ifr_flags |= IFF_UP;
    if (ioctl(this->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        what << "--- Unable to bring-up interface " << this->name;
        what << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    return;
}

void VIfaceImpl::down() const
{
    ostringstream what;

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    // Bring-down interface
    ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(this->kernel_socket, SIOCSIFFLAGS, &ifr) != 0) {
        what << "--- Unable to bring-down interface " << this->name;
        what << "." << endl;
        what << "    Error: " << strerror(errno);
        what << " (" << errno << ")." << endl;
        throw runtime_error(what.str());
    }

    return;
}

bool VIfaceImpl::isUp() const
{
    ostringstream what;

    // Read interface flags
    struct ifreq ifr;
    read_flags(this->kernel_socket, this->name, &ifr);

    return (ifr.ifr_flags & IFF_UP) != 0;
}

vector<uint8_t> VIfaceImpl::receive(int timeout) const
{
    // FIXME: Implement!
    vector<uint8_t> packet;
    return packet;
}

void VIfaceImpl::send(vector<uint8_t>& packet) const
{
    // FIXME: Implement!
    return;
}




/*============================================================================
   =   PIMPL IDIOM BUREAUCRACY
   =
   =   Starting this point there is not much relevant things...
   =   Stop scrolling...
 *============================================================================*/

VIface::VIface(string name, bool tap, int id) :
    pimpl(new VIfaceImpl(name, tap, id))
{
    // Set default values
    this->setMAC();
    this->setIPv4();
    this->setMTU();
}
VIface::~VIface() = default;

string VIface::getName() const {
    return this->pimpl->getName();
}

uint VIface::getID() const {
    return this->pimpl->getID();
}

void VIface::setMAC(string mac)
{
    return this->pimpl->setMAC(mac);
}

string VIface::getMAC() const
{
    return this->pimpl->getMAC();
}

void VIface::setIPv4(string ipv4)
{
    return this->pimpl->setIPv4(ipv4);
}

string VIface::getIPv4() const
{
    return this->pimpl->getIPv4();
}

void VIface::setMTU(uint mtu)
{
    return this->pimpl->setMTU(mtu);
}

uint VIface::getMTU() const
{
    return this->pimpl->getMTU();
}

void VIface::up()
{
    return this->pimpl->up();
}

void VIface::down() const
{
    return this->pimpl->down();
}

bool VIface::isUp() const
{
    return this->pimpl->isUp();
}

vector<uint8_t> VIface::receive(int timeout) const
{
    return this->pimpl->receive(timeout);
}

void VIface::send(vector<uint8_t>& packet) const
{
    return this->pimpl->send(packet);
}
}