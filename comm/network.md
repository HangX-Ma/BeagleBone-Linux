# Network Connection

BeagleBone-Black board can use USB cable to transfer network data, in which case, ethernet cable is no longer needed. If you use a Linux OS like Ubuntu as your host, you need to do some settings to enable internet on BBB.

## Shared Network Setting

If you use `ifconfig` on your BBB board, generally the debian network manager will show two interfaces such that `usb0` and `usb1`. You can use `ifconfig` to check it on BBB board.

- `usb0` IP: 192.168.7.2
- `usb1` IP: 192.168.6.2

If you use `ifconfig` on your host machine, two devices named similarly to `enxf4b898a93630` and `enxf4b898a93632`, whose `inets` correspond to what we see on BBB board, will appear.

### BBB Operation

On your BBB board, add default gateways.

```bash
route add default gw 192.168.7.1 usb0
route add default gw 192.168.6.1 usb1
```

Meanwhile, we need to add DNS name server to resolve domain name.

```bash
sudo vim /etc/resolv.conf
# add the following two name servers
nameserver 8.8.8.8
nameserver 8.8.4.4
```

### Host Operation

Here you have to share the internet between Wi-Fi and the ethernet. For that, you have to do two settings. One is IP table setting and the other is IP forwarding.

```bash
#### Enable IP forwarding on your host side ####

# enhance to privileged mode
su root 
# enable ip forwarding, not permanently
echo 1 > /proc/sys/net/ipv4/ip_forward

# iptables settings to share internet between wifi and ethernet
iptables --table nat --append POSTROUTING --out-interface <wifi-interface> -j MASQUERADE
iptables --append FORWARD --in-interface <ethernet interface to share with> -j ACCEPT
```

The first `iptables` command adds a rule to NAT table, enabling IP masquerade for your WAN. The second `iptables` command setup the FORWARD source.

What you need to do is to find the wifi interfaces and the ethernet interfaces that you want to share to. In my case, the wifi interface is `wlp3s0` and the ethernet interfaces are what I mentioned above `enxf4b898a93630` and `enxf4b898a93632` _(Find your own ones)_.

You can change the template [scripts/usbnet.sh](../boot/scripts/usbnet.sh) to fit your own demand. **Note:** This script needs to run whenever you restart your host machine, because its setting is temporary.

Done.
