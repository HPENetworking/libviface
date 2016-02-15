package main

import (
    "fmt"
    "vifacego"
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
func setInterfaceConfiguration(self vifacego.Viface) int {
    // Configures interface
    if ((vifacego.VifaceSetIpv4(self, ipv4) == vifacego.ExitFailure) ||
        (vifacego.VifaceSetMac(self, mac) == vifacego.ExitFailure) ||
        (vifacego.VifaceSetIpv4Broadcast(self,broadcast) ==
            vifacego.ExitFailure) ||
        (vifacego.VifaceSetIpv4Netmask(self, netmask) ==
            vifacego.ExitFailure) ||
        (vifacego.VifaceSetIpv6(self, len(ipv6s), ipv6s) ==
            vifacego.ExitFailure)) {
        return vifacego.ExitFailure
    }

    return vifacego.ExitSuccess;
}

// checkInterfaceConfiguration Checks the interface configuration data
// (name, id, ip, mac, netmask, broadcast, mtu).
func checkInterfaceConfiguration(self vifacego.Viface) int {
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
    if ((vifacego.VifaceGetName(self, &vifaceName) == vifacego.ExitFailure) ||
        (vifacego.VifaceGetID(self, &idValue) == vifacego.ExitFailure) ||
        (vifacego.VifaceIsUp(self, &vifaceUp) == vifacego.ExitFailure) ||
        (vifacego.VifaceGetIpv4(self, &ipValue) == vifacego.ExitFailure) ||
        (vifacego.VifaceGetMac(self, &macValue) == vifacego.ExitFailure) ||
        (vifacego.VifaceGetIpv4Broadcast(self, &broadcastValue) ==
            vifacego.ExitFailure) ||
        (vifacego.VifaceGetIpv4Netmask(self, &netmaskValue) ==
            vifacego.ExitFailure) ||
        (vifacego.VifaceGetMtu(self, &mtuValue) == vifacego.ExitFailure) ||
        (vifacego.VifaceGetIpv6(self, &ipv6sValues) == vifacego.ExitFailure)) {
        return vifacego.ExitFailure
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

    return vifacego.ExitSuccess
}

// checkInterfaceIsDown Checks if the network interface is down
func checkInterfaceIsDown(self vifacego.Viface) int {
    // Checks if interface is down
    var isDown bool

    if (vifacego.VifaceIsUp(self, &isDown) == vifacego.ExitFailure) {
        return vifacego.ExitFailure;
    }

    fmt.Printf("--- Interface is still Up: %v\n", isDown);
    fmt.Printf("    Expected:              false\n\n");
    return vifacego.ExitSuccess;
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
    var self = vifacego.CreateVifaceStruct()

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
    if ((vifacego.VifaceCreateGlobalPool() == vifacego.ExitFailure) ||
        (vifacego.VifaceCreateViface(name, true, id, &self) ==
            vifacego.ExitFailure) ||
        (setInterfaceConfiguration(self) == vifacego.ExitFailure) ||
        (vifacego.VifaceUp(self) == vifacego.ExitFailure) ||
        (checkInterfaceConfiguration(self) == vifacego.ExitFailure) ||
        (vifacego.VifaceDown(self) == vifacego.ExitFailure) ||
        (checkInterfaceIsDown(self) == vifacego.ExitFailure) ||
        (vifacego.VifaceDestroyViface(self) == vifacego.ExitFailure) ||
        (vifacego.VifaceDestroyGlobalPool() == vifacego.ExitFailure)) {
        fmt.Printf("--- An error occurred executing basic GO example\n")
    }


}
