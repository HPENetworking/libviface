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
uint VIfaceImpl::idseq = 0;

VIfaceImpl::VIfaceImpl(string name, bool tap, int id)
{
    // Set name
    this->name = name;

    // Set tap
    this->tap = tap;

    // Set id
    if (id < 0) {
        this->id = this->idseq;
    } else {
        this->id = id;
    }

    this->idseq++;
}

void VIfaceImpl::setMac(string mac)
{
    // FIXME: Implement!
    return;
}

string VIfaceImpl::getMac() const
{
    // FIXME: Implement!
    return "";
}

void VIfaceImpl::setIPv4(string ipv4)
{
    // FIXME: Implement!
    return;
}

string VIfaceImpl::getIPv4() const
{
    // FIXME: Implement!
    return "";
}

void VIfaceImpl::setIPv6(string ipv6)
{
    // FIXME: Implement!
    return;
}

string VIfaceImpl::getIPv6() const
{
    // FIXME: Implement!
    return "";
}

void VIfaceImpl::setMTU(uint mtu)
{
    // FIXME: Implement!
    return;
}

uint VIfaceImpl::getMTU() const
{
    // FIXME: Implement!
    return 0;
}

void VIfaceImpl::up() const
{
    // FIXME: Implement!
    return;
}

void VIfaceImpl::down() const
{
    // FIXME: Implement!
    return;
}

bool VIfaceImpl::isUp() const
{
    // FIXME: Implement!
    return false;
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
{}
VIface::~VIface() = default;

string VIface::getName() const {
    return this->pimpl->getName();
}

uint VIface::getID() const {
    return this->pimpl->getID();
}

void VIface::setMac(string mac)
{
    return this->pimpl->setMac(mac);
}

string VIface::getMac() const
{
    return this->pimpl->getMac();
}

void VIface::setIPv4(string ipv4)
{
    return this->pimpl->setIPv4(ipv4);
}

string VIface::getIPv4() const
{
    return this->pimpl->getIPv4();
}

void VIface::setIPv6(string ipv6)
{
    return this->pimpl->setIPv6(ipv6);
}

string VIface::getIPv6() const
{
    return this->pimpl->getIPv6();
}

void VIface::setMTU(uint mtu)
{
    return this->pimpl->setMTU(mtu);
}

uint VIface::getMTU() const
{
    return this->pimpl->getMTU();
}

void VIface::up() const
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