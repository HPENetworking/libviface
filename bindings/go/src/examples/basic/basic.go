package main

import (
    "fmt"
    "wrapper"
)

// Viface configuration data
var name = "viface0"
var ipv4 = "192.168.25.46"
var ipv6s = []string{"fe80::6829:26ff:fe53:4670",
                     "fe20::5478:93f5:fe93:4680"}
var mac = "ec:f1:f8:d5:47:6b"
var broadcast = "192.168.25.255"
var netmask = "255.255.255.0"
var id = 1
var mtu uint = 1500 //Default value if not set

// setInterfaceConfiguration Sets network configuration data
// (name, id, ip, mac, netmask, broadcast, mtu).
func setInterfaceConfiguration(self wrapper.Viface) int {
    // Configures interface
    if ((wrapper.VifaceSetIpv4(self, ipv4) == wrapper.ExitFailure) ||
        (wrapper.VifaceSetMac(self, mac) == wrapper.ExitFailure) ||
        (wrapper.VifaceSetIpv4Broadcast(self,broadcast) ==
            wrapper.ExitFailure) ||
        (wrapper.VifaceSetIpv4Netmask(self, netmask) == wrapper.ExitFailure) ||
        (wrapper.VifaceSetIpv6(self, len(ipv6s), ipv6s) ==
            wrapper.ExitFailure)) {
        return wrapper.ExitFailure
    }

    return wrapper.ExitSuccess;
}

// checkInterfaceConfiguration Checks the interface configuration data
// (name, id, ip, mac, netmask, broadcast, mtu).
func checkInterfaceConfiguration(self wrapper.Viface) int {
    var idValue uint
    var vifaceUp bool
    var mtuValue uint
    var vifaceName string
    var ipValue string
    var macValue string
    var broadcastValue string
    var netmaskValue string
    var ipv6sValues []string

    // Gets interface configuration data
    if ((wrapper.VifaceGetName(self, &vifaceName) == wrapper.ExitFailure) ||
        (wrapper.VifaceGetID(self, &idValue) == wrapper.ExitFailure) ||
        (wrapper.VifaceIsUp(self, &vifaceUp) == wrapper.ExitFailure) ||
        (wrapper.VifaceGetIpv4(self, &ipValue) == wrapper.ExitFailure) ||
        (wrapper.VifaceGetMac(self, &macValue) == wrapper.ExitFailure) ||
        (wrapper.VifaceGetIpv4Broadcast(self, &broadcastValue) ==
            wrapper.ExitFailure) ||
        (wrapper.VifaceGetIpv4Netmask(self, &netmaskValue) ==
            wrapper.ExitFailure) ||
        (wrapper.VifaceGetMtu(self, &mtuValue) == wrapper.ExitFailure) ||
        (wrapper.VifaceGetIpv6(self, &ipv6sValues) == wrapper.ExitFailure)) {
        return wrapper.ExitFailure
    }

    fmt.Printf("********** Viface Configuration Data **********\n\n");

    // Checks interface name
    fmt.Printf("--- Viface name is: %v.\n", vifaceName);
    fmt.Printf("    Expected:       %v.\n\n", name);

    // Checks interface ID
    fmt.Printf("--- Viface ID is: %v.\n", idValue);
    fmt.Printf("    Expected:     %v.\n\n", id);

    // Checks if interface is Up
    fmt.Printf("--- Interface isUp: %v\n", vifaceUp);
    fmt.Printf("    Expected:       true.\n\n");

    // Checks interface IP
    fmt.Printf("--- IP value is: %v\n", ipValue);
    fmt.Printf("    Expected:    %v.\n\n", ipv4);

    // Checks mac value
    fmt.Printf("--- MAC value is: %v\n", macValue);
    fmt.Printf("    Expected:     %v.\n\n", mac);

    // Checks broadcast value
    fmt.Printf("--- Broadcast value is: %v\n", broadcastValue);
    fmt.Printf("    Expected:           %v.\n\n", broadcast);

    // Checks netmask value
    fmt.Printf("--- Netmask value is: %v\n", netmaskValue);
    fmt.Printf("    Expected:         %v.\n\n", netmask);

    // Checks mtu value
    fmt.Printf("--- MTU value is: %v\n", mtuValue);
    fmt.Printf("    Expected:     %v.\n\n", mtu);

    // Checks Ipv6s values
    fmt.Printf("--- Ipv6 values are:\n");

    for i:= 0; i < len(ipv6sValues); i++ {
        fmt.Printf("        %v\n", ipv6sValues[i]);
    }
    fmt.Printf("\n");

    return wrapper.ExitSuccess
}

// checkInterfaceIsDown Checks if the network interface is down
func checkInterfaceIsDown(self wrapper.Viface) int {
    // Checks if interface is down
    var isDown bool

    if (wrapper.VifaceIsUp(self, &isDown) == wrapper.ExitFailure) {
        return wrapper.ExitFailure;
    }

    fmt.Printf("--- Interface is still Up: %v\n", isDown);
    fmt.Printf("    Expected:              false\n\n");
    return wrapper.ExitSuccess;
}


// main This example shows the basics to setup a new virtual network
// interface called viface0
// (or use the name passed as first argument).
// It will show how to:
//  1) Set interface configuration data
//     (name, id, ip, ipv6, mac, netmask, broadcast, mtu).
//  2) Bring-up the network interface.
//  3) Check the interface configuration data
//     (name, id, ip, ipv6, mac, netmask, broadcast, mtu).
func main() {

    fmt.Printf("\n--- Starting basic GO example...\n\n")

    // Creates viface struct
    var self = wrapper.CreateVifaceStruct()

    /* These IF statements do the following:
     * 1) Creates global parent APR pool
     * 2) Creates interface
     * 3) Sets interface configuration data
     * 4) Brings-up interface
     * 5) Checks interface configuration data
     * 6) Brings-down interface
     * 7) Checks if interface is down
     * 8) Destroys viface struct
     * 9) Destroys global APR pool
     */
    if ((wrapper.VifaceCreateGlobalPool() == wrapper.ExitFailure) ||
        (wrapper.VifaceCreateViface(name, true, id, &self) ==
            wrapper.ExitFailure) ||
        (setInterfaceConfiguration(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceUp(self) == wrapper.ExitFailure) ||
        (checkInterfaceConfiguration(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDown(self) == wrapper.ExitFailure) ||
        (checkInterfaceIsDown(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDestroyViface(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDestroyGlobalPool() == wrapper.ExitFailure)) {
        fmt.Printf("--- An error occurred executing basic GO example\n")
    }


}
