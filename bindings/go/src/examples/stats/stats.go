package main

import (
    "fmt"
    "wrapper"
)

// Viface configuration data
var name = "viface1"
var id = 1

// Example packet. In Scapy:
// packet = Ether()/IP()/TCP()/Raw('I\'m a packet!'*3)
// len(packet) == 93
var packet = []uint8 {
    0x5D, //This is the packet size (0x5D = 93)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x4F, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x7C, 0xA6, 0x7F, 0x00, 0x00, 0x01, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x14,
    0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02,
    0x20, 0x00, 0x04, 0x91, 0x00, 0x00, 0x49, 0x27, 0x6D, 0x20, 0x61, 0x20,
    0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20, 0x61,
    0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21, 0x49, 0x27, 0x6D, 0x20,
    0x61, 0x20, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x21}

var count = 0

// printStatistics Prints the network interface statistics values
func printStatistics(self wrapper.Viface, statsNames []string) int {
    for i := 0; i < len(statsNames); i++ {
        var resultKey uint64
        if (wrapper.VifaceReadStat(self, statsNames[i], &resultKey) ==
            wrapper.ExitFailure) {
            return wrapper.ExitFailure
        }
        fmt.Printf("     %v : %v\n", statsNames[i], resultKey)
    }

    fmt.Printf("\n")
    return wrapper.ExitSuccess
}

/* Shows the use of the interface statistics to read
 * the number of packets and bytes sent.
 */
func checkStatistics(self wrapper.Viface) int {
    var numberPackets = 100
    var crc32 uint32
    var hexDump string
    var statsNames []string

    fmt.Printf("********** Viface Statistics **********\n\n");

    // Prints statistics before sending packets
    fmt.Printf("--- Statistics before sending packets:\n");

    if ((wrapper.VifaceListStats(self, &statsNames) == wrapper.ExitFailure) ||
        (printStatistics(self, statsNames) == wrapper.ExitFailure) ||
        (wrapper.VifaceCrc32(packet, &crc32) == wrapper.ExitFailure) ||
        (wrapper.VifaceHexDump(self, packet, &hexDump) ==
            wrapper.ExitFailure)) {
        return wrapper.ExitFailure
    }

    fmt.Printf("--- About to send the following packet.\n")
    fmt.Printf("    Size: %v\n", packet[0])
    fmt.Printf("    CRC: 0x%x\n", crc32)
    fmt.Printf("%v\n\n", hexDump)

    fmt.Printf("--- Sending the packet %v times...", numberPackets)

    // Sends the packet for 'numberPackets' times
    for i := 0; i < numberPackets; i++ {
        if (i % 10 == 0) {
            fmt.Printf("\n")
        }
        fmt.Printf(" #%.2d ...", i + 1)
        if (wrapper.VifaceSend(self, packet) == wrapper.ExitFailure) {
            return wrapper.ExitFailure
        }
    }
    fmt.Printf("\n\n")

    // Prints statistics after sending packets
    fmt.Printf("--- Statistics after sending packets:\n");
    if (printStatistics(self, statsNames) == wrapper.ExitFailure) {
        return wrapper.ExitFailure;
    }

    // Clears statistics
    fmt.Printf("--- Clearing statistics:\n");

    for i := 0; i < 23; i++ {
       var resultKey uint64

        if ((wrapper.VifaceClearStat(self, statsNames[i]) ==
            wrapper.ExitFailure) ||
            (wrapper.VifaceReadStat(self, statsNames[i], &resultKey) ==
                wrapper.ExitFailure)) {
            return wrapper.ExitFailure
        }
        fmt.Printf("     %v : %v\n", statsNames[i], resultKey)
    }
    fmt.Printf("\n")
    return wrapper.ExitSuccess
}

// main This example shows the use of the interface statistics interface
// to read the number of packets and bytes sent.
func main() {

    fmt.Printf("\n--- Starting stats example...\n\n")

    // Creates viface struct
    var self = wrapper.CreateVifaceStruct()

    /* These IF statements do the following:
     * 1) Creates interface
     * 2) Brings-up interface
     * 3) Checks interface statistics
     * 4) Brings-down interface
     */
    if ((wrapper.VifaceCreateGlobalPool() == wrapper.ExitFailure) ||
        (wrapper.VifaceCreateViface(name, true, 0, &self) ==
            wrapper.ExitFailure) ||
        (wrapper.VifaceUp(self) == wrapper.ExitFailure) ||
        (checkStatistics(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDown(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDestroyViface(self) == wrapper.ExitFailure) ||
        (wrapper.VifaceDestroyGlobalPool() == wrapper.ExitFailure)) {
        fmt.Printf("--- An error occurred executing stats GO example\n")
    }
}
