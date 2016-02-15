==================================================================
libvifacego : libviface GO wrapper for managing and creating network interfaces.
==================================================================

``libvifacego`` is a GOlang wrapper of the libviface C library that allows to create 
and configure virtual network interfaces in Linux based Operating Systems.

::

   import (
	 "fmt"
	 "vifacego" 
   )

   // Creates viface struct
   var self = wrapper.CreateVifaceStruct()

   // Viface configuration data
   var name = "viface0"
   var ipv4 = "192.168.25.46"
   var id = 1

   if ((wrapper.VifaceCreateGlobalPool() == wrapper.ExitFailure) ||
       (wrapper.VifaceCreateViface(name, true, id, &self) == wrapper.ExitFailure) ||
       (wrapper.VifaceSetIpv4(self, ipv4) == wrapper.ExitFailure)) {
        fmt.Printf("--- An error occurred executing basic GO example\n")
    }


Then you can send ``VifaceSend()`` or receive ``VifaceReceive()`` to
handle virtual interfaces incoming and outgoing packets. Also, interface
statistics (rx/tx packets, bytes, etc) are available to read using
``VifaceReadStat()`` and related functions.

For a complete overview check the reference documentation and examples.


Features
========

- Object Oriented approach to create virtual interfaces.
- Multiple strategies for packet reception and emission.
- Interface configuration API (MAC, Ipv4, IPv6, MTU).
- Interface statistics reading and clearing.


Dependencies
============

::

   sudo apt-get install build-essential cmake golang


Build
=====

::
   Set GOPATH env variable to the location where the libviface repository is
   located at:

   export GOPATH="libviface_directory_path"/bindings/go
   export PATH=$PATH:$GOPATH/bin

   mkdir build
   cd build
   cmake ..
   make libvifacego
   make docgo


Improvements
============

- Improve and fix possible race conditions when up/down is issued (and thus
  packet buffer based on MTU is resized) and a dispatcher is active or any
  other IO is active.


License
=======

::

   Copyright (C) 2015 Hewlett Packard Enterprise Development LP

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
