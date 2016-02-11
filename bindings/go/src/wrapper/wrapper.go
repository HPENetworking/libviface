package wrapper

// #cgo CFLAGS: -g -Wall -I/usr/local/apr/include/apr-1 -I/usr/lib/apr-util-1 -I.
// #cgo LDFLAGS: -L/usr/local/apr/lib -lapr-1 -laprutil-1 -L../../../../build/src -lviface
// #include "../../../../include/viface/viface.h"
import "C"
import (
    "unsafe"
    "strconv"
)

// ExitFailure status value indicating error
const ExitFailure int = 1;

// ExitSuccess status value indicating success
const ExitSuccess int = 0;

// Viface configuration data
var name = "viface0"
var ipv4 = "192.168.25.46"
var mac = "ec:f1:f8:d5:47:6b"
var broadcast = "192.168.25.255"
var netmask = "255.255.255.0"
var id = 1
var mtu = 1000

// Viface Viface C struct
type Viface *C.struct_viface

// CreateVifaceStruct Create a new C viface struct
func CreateVifaceStruct() *C.struct_viface {
    return new(C.struct_viface)
}

// VifaceHexDump Build a hexdump representation of a binary blob.
func VifaceHexDump(self *C.struct_viface, bytes []uint8,
                   result *string) int {
    var hexDump *C.char
    packet := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
    if(C.viface_hex_dump(self, packet, &hexDump) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(hexDump)
    return ExitSuccess
}

// VifaceCrc32 Calculate the 32 bit CRC of the given binary blob.
func VifaceCrc32(bytes []uint8, result *uint32) int {
    var crc32 C.uint32_t
    packet := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
    if(C.viface_crc_32(packet, &crc32) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = uint32(crc32)
    return ExitSuccess
}

// VifaceCreateGlobalPool Create an APR parent pool for memory allocation.
func VifaceCreateGlobalPool() int {
    return int(C.viface_create_global_pool())
}

// VifaceCreateViface Create a viface struct and an
// APR subpool from a parent pool.
func VifaceCreateViface(name string, tap bool, id int,
                        result **C.struct_viface) int {
    var nameValue = C.CString(name)
    var status =
        int(C.viface_create_viface(nameValue, C.bool(tap), C.int(id), result))
    C.free(unsafe.Pointer(nameValue))
    return status
}

// VifaceDestroyGlobalPool Destroy the static global APR parent pool.
func VifaceDestroyGlobalPool() int {
    return int(C.viface_destroy_global_pool())
}

// VifaceDestroyTemporalPool  Destroy the static temporal APR pool.
func VifaceDestroyTemporalPool() int {
    return int(C.viface_destroy_temporal_pool())
}

// VifaceDestroyViface Destroy a viface struct and free
// all allocated memory from the APR subpool
func VifaceDestroyViface(self *C.struct_viface) int {
    return int(C.viface_destroy_viface(&self))
}

// VifaceGetName Getter method for virtual interface associated name.
func VifaceGetName(self *C.struct_viface, result *string ) int {
    var name *C.char
    if(C.viface_get_name(self, &name) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(name)
    return ExitSuccess
}

// VifaceGetID Getter method for virtual interface associated numeric id.
func VifaceGetID(self *C.struct_viface, result *uint) int {

    var id C.uint
    if(C.viface_get_id(self, &id) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = uint(id)
    return ExitSuccess
}

// VifaceGetTx Getter method for virtual interface associated
// transmission file descriptor.
func VifaceGetTx(self *C.struct_viface, result *int) int {

    var tx C.int
    if(C.viface_get_tx(self, &tx) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = int(tx)
    return ExitSuccess
}

// VifaceGetRx Getter method for virtual interface associated
// reception file descriptor.
func VifaceGetRx(self *C.struct_viface, result *int) int {

    var rx C.int
    if(C.viface_get_rx(self, &rx) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = int(rx)
    return ExitSuccess
}

// VifaceSetMac Set the MAC address of the virtual interface.
//
// The format of the MAC address is verified, but is just until up()
// is called that the library will try to attempt to write it.
// If you don't provide a MAC address (the default) one will be
// automatically assigned when bringing up the interface.
func VifaceSetMac(self *C.struct_viface, mac string) int {
    var macValue = C.CString(mac)
    var status = int(C.viface_set_mac(self, macValue))
    C.free(unsafe.Pointer(macValue))
    return status
}

// VifaceGetMac Getter method for virtual interface associated MAC Address.
func VifaceGetMac(self *C.struct_viface, result *string) int {
    var mac *C.char
    if(C.viface_get_mac(self, &mac) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(mac)
    return ExitSuccess
}

// VifaceSetIpv4 Set the IPv4 address of the virtual interface.
//
// The format of the IPv4 address is verified, but is just until up()
// is called that the library will try to attempt to write it.
func VifaceSetIpv4(self *C.struct_viface, ipv4 string) int {
    var ipv4Value = C.CString(ipv4)
    var status = int(C.viface_set_ipv4(self, ipv4Value))
    C.free(unsafe.Pointer(ipv4Value))
    return status
}

// VifaceGetIpv4 Getter method for virtual interface associated IPv4 Address.
func VifaceGetIpv4(self *C.struct_viface, result *string) int {
    var ipv4 *C.char
    if(C.viface_get_ipv4(self, &ipv4) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(ipv4)
    return ExitSuccess
}

// VifaceSetIpv4Netmask Set the IPv4 netmask of the virtual interface.
//
// The format of the IPv4 netmask is verified, but is just until up()
// is called that the library will try to attempt to write it.
func VifaceSetIpv4Netmask(self *C.struct_viface, netmask string) int {
    var netmaskValue = C.CString(netmask)
    var status = int(C.viface_set_ipv4_netmask(self, netmaskValue))
    C.free(unsafe.Pointer(netmaskValue))
    return status
}

// VifaceGetIpv4Netmask Getter method for virtual interface
// associated IPv4 netmask.
func VifaceGetIpv4Netmask(self *C.struct_viface, result *string) int {
    var netmask *C.char
    if(C.viface_get_ipv4_netmask(self, &netmask) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(netmask)
    return ExitSuccess
}

// VifaceSetIpv4Broadcast Set the IPv4 broadcast address
// of the virtual interface.
//
// The format of the IPv4 broadcast address is verified, but is just
// until up() is called that the library will try to attempt to write it.
func VifaceSetIpv4Broadcast(self *C.struct_viface, broadcast string) int {
    var broadcastValue = C.CString(broadcast)
    var status = int(C.viface_set_ipv4_broadcast(self, broadcastValue))
    C.free(unsafe.Pointer(broadcastValue))
    return status
}

// VifaceGetIpv4Broadcast Getter method for virtual interface associated
// IPv4 broadcast address.
func VifaceGetIpv4Broadcast(self *C.struct_viface, result *string) int {
    var broadcast *C.char
    if(C.viface_get_ipv4_broadcast(self, &broadcast) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = C.GoString(broadcast)
    return ExitSuccess
}

func setIPv6(self *C.struct_viface, numIPv6s int, ipv6s []*C.char) int {
    C.viface_set_ipv6(self, C.int(numIPv6s), &ipv6s[0])
    return ExitSuccess
}

// VifaceSetIpv6 Set the IPv6 addresses of the virtual interface.
//
// The format of the IPv6 addresses are verified, but is just until
// up() is called that the library will try to attempt to write them.
func VifaceSetIpv6(self *C.struct_viface, numIPv6s int, ipv6s []string) int {

    ipv6sValues :=
        (*[1 << 30]*C.char)(unsafe.Pointer(&ipv6s[0]))[:numIPv6s:numIPv6s]

    for i := 0; i < numIPv6s; i++ {
        var ipv6 = C.CString(ipv6s[i])
        ipv6sValues[i] = ipv6
    }

    if(C.viface_set_ipv6(self, C.int(numIPv6s), &ipv6sValues[0]) ==
        C.int(ExitFailure)){
        return ExitFailure
    }

    return ExitSuccess
}

// VifaceGetIpv6 Getter method for virtual interface associated IPv6 Addresses
// (note the plural).
func VifaceGetIpv6(self *C.struct_viface, result *[]string) int {
    var ipv6sValues **C.char

    if(C.viface_get_ipv6(self, &ipv6sValues) == C.int(ExitFailure)){
        return ExitFailure
    }

    ipv6sListC := (*[1<<30]*C.char)(unsafe.Pointer(ipv6sValues))
    numberIpv6s, err := strconv.Atoi(C.GoString(ipv6sListC[0]))

    if(err != nil){
        return ExitFailure
    }

    ipv6sListGO := make([]string, numberIpv6s)

    for i := 0; i < numberIpv6s; i++ {
        ipv6sListGO[i] = C.GoString(ipv6sListC[i + 1])
    }

    *result = ipv6sListGO
    return ExitSuccess
}

// VifaceSetMtu Set the MTU of the virtual interface.
//
// The range of the MTU is verified, but is just until up() is called
// that the library will try to attempt to write it.
func VifaceSetMtu(self *C.struct_viface, mtu uint) int {
    return int(C.viface_set_mtu(self, C.uint(mtu)))
}

// VifaceGetMtu Getter method for virtual interface associated
// maximum transmission unit (MTU).
//
// MTU is the size of the largest packet or frame that can be sent
// using this interface.
func VifaceGetMtu(self *C.struct_viface, result *uint) int {

    var mtu C.uint
    if(C.viface_get_mtu(self, &mtu) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = uint(mtu)
    return ExitSuccess
}

// VifaceUp Bring up the virtual interface.
//
// This call will configure and bring up the interface.
func VifaceUp(self *C.struct_viface) int {
    return int(C.viface_up(self))
}

// VifaceDown Bring down the virtual interface.
func VifaceDown(self *C.struct_viface) int {
    return int(C.viface_down(self))
}

// VifaceIsUp Indicate if the virtual interface is up.
func VifaceIsUp(self *C.struct_viface, result *bool) int {

    var isUp C.bool = false
    if(C.viface_is_up(self, &isUp) == C.int(ExitFailure)){
        return ExitFailure
    }
    *result = bool(isUp)
    return ExitSuccess
}

// VifaceReceive Receive a packet from the virtual interface.
//
// Note: Receive a packet from a virtual interface means that another
// userspace program (or the kernel) sent a packet to the network
// interface with the name of the instance of this class. If not packet
// was available, and empty vector is returned.
func VifaceReceive(self *C.struct_viface, result *[]uint8) int {

    var packet *C.uint8_t
    if(C.viface_receive(self, &packet) == C.int(ExitFailure)){
        return ExitFailure
    }

    packetC := (*[1<<30]C.uint8_t)(unsafe.Pointer(packet))
    packetBytesSize := int(packetC[0])
    packetGO := make([]uint8, packetBytesSize + 1)

    for i := 0; i <= packetBytesSize; i++ {
        packetGO[i] = uint8(packetC[i])
    }

    *result = packetGO
    return ExitSuccess
}

// VifaceSend Send a packet to this virtual interface.
//
// Note: Sending a packet to this virtual interface means that it
// will reach any userspace program (or the kernel) listening for
// packets in the interface with the name of the instance of this
// class.
func VifaceSend(self *C.struct_viface, packet []uint8) int {
    packetC := (*C.uint8_t)(unsafe.Pointer(&packet[0]))
    if(C.viface_send(self, packetC) == C.int(ExitFailure)){
        return ExitFailure
    }
    return ExitSuccess
}

// VifaceListStats List available statistics for this interface.
func VifaceListStats(self *C.struct_viface, result *[]string) int {
    var stats **C.char
    if(C.viface_list_stats(self, &stats) == C.int(ExitFailure)){
        return ExitFailure
    }

    statsListC := (*[1<<30]*C.char)(unsafe.Pointer(stats))
    numerStats, err := strconv.Atoi(C.GoString(statsListC[0]))

    if(err != nil){
        return ExitFailure
    }

    statsListGO := make([]string, numerStats)

    for i := 0; i < numerStats; i++ {
        statsListGO[i] = C.GoString(statsListC[i+1])
    }

    *result = statsListGO
    return ExitSuccess
}

// VifaceReadStat Read given statistic for this interface.
func VifaceReadStat(self *C.struct_viface, stat string, result *uint64) int {
    var statValue C.uint64_t
    var statName = C.CString(stat)
    var status = int(C.viface_read_stat(self, statName, &statValue))
    C.free(unsafe.Pointer(statName))
    *result = uint64(statValue)
    return status
}

// VifaceClearStat Clear given statistic for this interface.
//
// Please note that this feature is implemented in library as there is
// currently no way to clear them (except by destroying the interface).
// If you clear the statistics using this method, subsequent calls to
// readStat() results will differ from, for example, those reported
// by tools like ifconfig.
func VifaceClearStat(self *C.struct_viface, stat string) int {
    var statName = C.CString(stat)
    var status = int(C.viface_clear_stat(self, statName))
    C.free(unsafe.Pointer(statName))
    return status
}
