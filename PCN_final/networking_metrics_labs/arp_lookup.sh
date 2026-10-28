#!/bin/sh
# ARP lookup/metrics demo for Linux/Kali.
# ARP resolves an IPv4 address to a MAC address on the local LAN.

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <IPv4-address>"
    echo "Example: $0 192.168.1.1"
    exit 1
fi

IP="$1"

echo "=== ARP LOOKUP ==="
date '+Timestamp: %Y-%m-%d %H:%M:%S'
echo "Target IPv4: $IP"
echo

echo "1) Check neighbour/ARP cache:"
ip neigh show "$IP"
echo

echo "2) Ping target once to encourage neighbour resolution:"
ping -c 1 -W 1 "$IP" >/dev/null 2>&1
echo

echo "3) Neighbour cache after ping:"
ip neigh show "$IP"
echo

echo "4) ARP table (ip neighbour table):"
ip neigh show

echo
echo "Metrics available in this simple OS-level lookup:"
echo "- Resolution result/state (REACHABLE, STALE, FAILED, etc.)"
echo "- Target IP and learned MAC address, if present"
echo "- Timestamp of command execution"
echo "Note: ARP is local-LAN only; routers do not forward ARP broadcasts."
