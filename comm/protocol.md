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

### TFTP Server Configuration

- Install TFTP server and client tools

    ```bash
    sudo apt-get install xinetd tftpd tftp tftpd-hpa tftp-hpa
    ```

- Write the configuration file.

    ```bash
    sudo vi /etc/xinetd.d/tftp

    # copy the following content
    service tftp
    {
    protocol        = udp
    port            = 69
    socket_type     = dgram
    wait            = yes
    user            = <user name>
    server          = /usr/sbin/in.tftpd
    server_args     = /root/tftpboot
    disable         = no
    }
    ```

- Create tftp-server folder to store your files.

    ```bash
    sudo mkdir /root/tftpboot
    sudo chmod -R 755 /root/tftpboot
    sudo chown -R <user name> /root/tftpboot
    ```

- Create tftpd-hpa file.

    ```bash
    sudo cp /etc/default/tftpd-hpa /etc/default/tftpd-hpa-copy
    sudo vi /etc/default/tftpd-hpa
    
    # copy the following content
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/root/tftpboot"
    TFTP_ADDRESS="0.0.0.0:69"
    TFTP_OPTIONS="-l-c-s"
    ```

- Restart the service

    ```bash
    sudo service xinetd restart
    ```

### Transfer Files

After you store the files needed to be transferred into the `/root/tftpboot`, you need to check your IP configuration to ensure the host and the board are both located in the same network segment.

Using picocom or minicom as visualization tools, you can type the following command to pull files from the host on your board.

```bash
tftp -r <remote file name> -g <tftp server IP>
```
