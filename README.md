# Embedded-Linux-BBB

Embedded Linux study on BeagleBone Black Rev C

## 1. Connect BBB with serial port

You can use `picocom` or `minicom` on Ubuntu to interact between host and BBB board, using a serial port convertor.

### picocom

I prefer to use `picocom`!

```bash
sudo chmod a+rw /dev/ttyUSB0
picocom -b 115200 /dev/ttyUSB0
```

### minicom

Using `minicom` needs to do some settings.

```bash
sudo minicom -s
```

1. Select [Serial port setup]

    Press a letter, which correspond to one property of serial port, and change to what you need. I change `Serial Device` to `/dev/ttyUSB0`, `Bps/Par/Bits` to `115200 8N1`, `Hardware Flow Control` and `Software Flow Control`to `No`.

2. Select [Save setup as dfl]

    Because we don't want to do this setting again.

3. Select [Exit]

Afterwards, you can run minicom with command `sudo minicom`.

## 2. Shared Network Setting

BeagleBone-Black board can use USB cable to transfer network data, in which case, ethernet cable is no longer needed. If you use a Linux OS like Ubuntu as your host, you need to do some settings to enable internet on BBB.

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

What you need to do is to find the wifi interfaces and the ethernet interfaces that you want to share to. In my case, the wifi interface is `wlp3s0` and the ethernet interfaces are what I mentioned above `enxf4b898a93630` and `enxf4b898a93632` _(Find your own ones)_.

```bash
sudo iptables --table nat --append POSTROUTING --out-interface wlp3s0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enxf4b898a93630 -j ACCEPT
```

Done.
