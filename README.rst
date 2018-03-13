==================================================================
libviface : C++ bindings for Linux tun/tap and netdevice interface
==================================================================

``libviface`` is a small C++11 library that allows to create (or hook to) and
configure network interfaces in Linux based Operating Systems.

::

   #include "viface/viface.hpp"

   // Create interface
   viface::VIface iface("viface%d");

   // Configure interface
   iface.setMAC("66:23:2d:28:c6:84");
   iface.setIPv4("192.168.20.21");

   // Bring-up interface
   iface.up();

Then you can ``send()``, ``receive()`` or setup a ``dispath()`` callback to
handle virtual interfaces incoming and outgoing packets. Also, interface
statistics (rx/tx packets, bytes, etc) are available to read using
``readStat()`` and related functions.

For a complete overview check the reference documentation and examples.


Features
========

- Object Oriented approach to create virtual interfaces.
- Can also hook to existing interfaces (real).
- Multiple strategies for packet reception and emission.
- Interface configuration API (MAC, Ipv4, IPv6, MTU).
- Interface statistics reading and clearing.
- Easily integrated with ``libtins``.


Dependencies
============

::

   sudo apt-get install build-essential cmake doxygen graphviz


Build
=====

::

   mkdir build
   cd build
   cmake ..
   make
   make doc


Improvements
============

- Improve and fix possible race conditions when up/down is issued (and thus
  packet buffer based on MTU is resized) and a dispatcher is active or any
  other IO is active.


License
=======

::

   Copyright (C) 2015-2018 Hewlett Packard Enterprise Development LP

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.
