# Protocols used on BeagleBone Black board

## Serial Port Protocol

Before you use serial protocols such as Xmodem, Ymodem, Zmodem and Kermit, log printing on the host seems to be big problem. Recommended ways are shown here.

### Connect BBB with serial port

You can use `picocom` or `minicom` on Ubuntu to interact between host and BBB board, using a serial port convertor.

#### picocom

I prefer to use `picocom`!

```bash
sudo chmod a+rw /dev/ttyUSB0
picocom -b 115200 /dev/ttyUSB0
```

#### minicom

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

### How serial protocol works?

If you want to use serial booting, please read [boot.md](../boot/boot.md) first. You will notice we can only boot the embedded system when S2 button pressed. In summary, you need to do these required setting.

- Unplug microSD card on the board.
- The board is preferably powered using power adapter. Otherwise, when S2 button pressed, USB0 booting will be activated first, leading to unnecessary waiting for booting method switching.
- Use USB to TTL serial port convertor to connect BBB's UART0 serial pins to PC.

loading...

## TFTP Protocol

If you want to transfer file between the host and the board using TFTP protocol, you need to move the files to your tftp server folder. Therefore, TFTP configuration needs to be done first.

**NOTE:** Remember insert your network cable before using TFTP. Otherwise cpsw will waiting for PHY auto negotiation to complete until timeout. I set the ipv4 manually, with `ip: 192.168.7.1`, `netmask: 255.255.255.0`, `gateway: 192.168.7.1`, dns: 8.8.8.8`.

### TFTP Server Configuration

- Install TFTP server and client tools

    ```bash
    # Don't install tftpd !!!
    sudo apt-get install xinetd tftpd-hpa tftp-hpa
    ```

- Write the configuration file.

    ```bash
    sudo vim /etc/xinetd.d/tftp

    # copy the following content
    service tftp
    {
    protocol        = udp
    port            = 69
    socket_type     = dgram
    wait            = yes
    user            = root 
    server          = /usr/sbin/in.tftpd
    server_args     = -s /var/lib/tftpboot
    disable         = no
    per_source      = 11
    cps             = 100 2
    flags           = IPv4
    }
    ```

- Create tftp-server folder to store your files.

    ```bash
    sudo mkdir /var/lib/tftpboot
    sudo chmod -R 755 /var/lib/tftpboot
    sudo chown -R <user name, example root> /var/lib/tftpboot
    ```

- Create tftpd-hpa file.

    ```bash
    sudo cp /etc/default/tftpd-hpa /etc/default/tftpd-hpa-copy
    sudo vim /etc/default/tftpd-hpa
    
    # copy the following content
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/var/lib/tftpboot"
    TFTP_ADDRESS=":69"
    TFTP_OPTIONS="-l -c -s"
    ```

- Restart the service

    ```bash
    sudo service xinetd restart
    sudo service tftp-hpa restart
    sudo service xinetd status
    sudo service tftp-hpa status
    ```

### BeagleBone Black U-Boot using TFTP

If you want to use TFTP protocol to transfer files during the U-Boot stage, you need to move the file into your `TFTP_DIRECTORY`. Server (host) and client (BBB) connection uses USB cable and are both configured in the same network segment. I prefer to set the host `192.168.7.1`, and the client `192.168.7.2` on Ubuntu.

Prepare `SPL/MLO`, `u-boot.img` (`uEnv.txt` also, used to automate the boot process) on the client, `initramfs`, `arm335x-boneblack.dtb`, `uImage` on the server.

#### Minimalist file system boot using TFTP

```bash
# host
sudo ifconfig enp2s0 192.168.7.1

# client (Make sure you have entered the U-Boot terminal)
# Fist configure the 'serverip' and 'ipaddr' environment variables
setenv serverip 192.168.7.1
setenv ipaddr 192.168.7.2
ping 192.168.7.1 # to ensure the host is alive
tftpboot 0x82000000 uImage
tftpboot 0x88000000 am335x-boneblack.dtb 
tftpboot 0x88080000 initramfs
setenv bootargs console=yytO0,115200n8 root=/dev/ram0 rw initrd=0x88080000
bootm 0x82000000 0x88080000 0x88000000
```

#### Boot file system using TFTP with uEnv.txt

##### Prepare NFS

```bash
# Install server and client components
sudo apt-get install nfs-kernel-server  # install nfs server
sudo apt-get install nfs-common         # install nfs client 

# Create shared folder
sudo mkdir -p /srv/nfs/bbb

# NFS will translate any root operations on the client to the nobody:nogroup credentials as a security measure. Therefore, we need to change the directory ownership to match those credentials.
sudo chmod 755 -R /srv/nfs/bbb
sudo chown nobody:nogroup /srv/nfs/bbb


sudo vim /etc/exports
# Add the following codes
/srv/nfs/bbb 192.168.7.2(rw,sync,no_root_squash,no_subtree_check)

# ------------------ mount NFS ------------------
# 'a': Maintain table of exported NFS file systems
# 'r': Reexport all directories.
# 'v': Be verbose.
sudo exportfs -arv
sudo service nfs-kernel-server restart
sudo service nfs-kernel-server status
```

- `rw`: This option gives the **client** computer both read and write access to the volume.
- `sync`: This option forces NFS to write changes to disk before replying. This results in a more stable and consistent environment since the reply reflects the actual state of the remote volume. However, it also reduces the speed of file operations.
- `no_subtree_check`: This option prevents subtree checking, which is a process where the **host** must check whether the file is actually still available in the exported tree for every request. This can cause many problems when a file is renamed while the **client** has it opened. In almost all cases, it is better to disable subtree checking.
- `no_root_squash`: By default, NFS translates requests from a **root** user remotely into a non-privileged user on the server. This was intended as security feature to prevent a **root** account on the **client** from using the file system of the **host** as **root**. `no_root_squash` disables this behavior for certain shares.

##### NFS Problems Record

**I tried my best, but NFS seems run into a rabbit hole.** I can't mount the file system through NFS !!! _2022.12_

- If you use Ubuntu version older than 16.04, NFS protocol support NFS version 2. Since Ubuntu 17.04, NFS protocol only support version 3, 4. What you need to do is modify the settings in `/etc/default/nfs-kernel-server`. Add `vers=4` to `uEnv.txt` after `nfsroot=` if kernel support NFSv4. In case of failure use a very helpful flag `nfsrootdebug` at the tail of the bootargs:

```bash
sudo vim /etc/default/nfs-kernel-server

# modify these commands
PRCNFSDCOUNT="-V 2 8"
PRCMOUNTDOPTS="-V 2 --manage-gids"
RPCNFSDOPTS="--nfs-version 2,3,4 --debug --syslog"

sudo /etc/init.d/nfs-kernel-server restart
```

- In addition, you need to configure the IP address of PC, otherwise the boot process will stick into `Loading*`.

```bash
sudo ipconfig enp2s0 192.168.7.1 
```

##### Operation Steps

- STEP 1: Copy the file system you download or generated from busybox into `/srv/nfs/bbb`, copy `am335x-boneblack.dtb`, `uImage` from `linux-kernel` into `/var/lib/tftpboot` and copy the `MLO`, `uEnv.txt(nfs version)`, `u-boot.img` into mircoSD card `/BOOT` partition.

    **NOTE:** Make sure you have configured the TFTP and the NFS. If we have put the files into `/var/lib/tftpboot` folder, we don't need to add the directory prefix before the files we want to transfer. This is a bit different from previous version.

- STEP 2: Insert the SD card and boot.

### TFTP Transfer Files

After you store the files needed to be transferred into the `/root/tftpboot`, you need to check your IP configuration to ensure the host and the board are both located in the same network segment.

Using picocom or minicom as visualization tools, you can type the following command to pull files from the host on your board.

```bash
tftp -r <remote file name> -g <tftp server IP>
```
