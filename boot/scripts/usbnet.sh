#!/bin/bash

## chmod + x usbnet.sh
## ./usbnet.sh

echo 1 > /proc/sys/net/ipv4/ip_forward
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enxf4b898a93630 -j ACCEPT