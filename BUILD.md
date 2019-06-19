# Hardware
Some packages will need to be installed, which will require Internet access.  We recommend developing on a Raspberry Pi version which has onboard Ethernet.
Alternately, you can use a USB Ethernet dongle on a Raspberry Pi Zero W.  A USB to serial cable such as [Adafruit product 954](https://www.adafruit.com/product/954) is also recommended.

# Theory of Operation
On boot up /etc/rc.local calls /root/oresatmini.sh

oresatmini.sh configures the wifi interface, then starts jpeg_stream_tx_stdout.py and wifibroadcast tx.

jpeg_stream_tx_stdout.py repeatedly takes a JPEG photo with the Pi Camera, adds a header with image size and integrity check information and then outputs a stream of bytes to stdout.  The byte stream is piped into wifibroadcast tx which adds forward error correction information, splits up the data to fit into wifi sized packets, and then sends the packets to the wifi chip for transmission.

# Install Raspbian on SD Card
Download [2018-11-13-raspbian-stretch-lite.img](https://downloads.raspberrypi.org/raspbian_lite/images/raspbian_lite-2018-11-15/2018-11-13-raspbian-stretch-lite.zip)

Install the Raspian Lite image onto a 2GB or larger SD card.  The Raspberry Pi documentation explains the proceedure:
https://www.raspberrypi.org/documentation/installation/installing-images/


# Copy Git content
This Git repository is structured like the file system on the SD card.  Copy the files in the repo to the same location on your SD card.


# Boot configuration
Be sure to copy /boot/config.txt from the Git repo to the boot partition on your SD card.  The following lines should appear at the end of the file: 
```
dtoverlay=pi3-disable-bt
gpu_mem=256
enable_uart=1
```


# Enable SSH
sudo raspi-config
choose interface options
enable ssh

# Enable serial console (optional)
Having access to a serial console can be handy for debug.  Connect a [USB serial cable](https://elinux.org/RPi_Serial_Connection) to the Raspberry Pi.

You will need to use terminal emulator software such as `screen` or `minicom` to access the serial console.

```
sudo screen /dev/ttyUSB0 115200
```


# Enable Pi Camera
```
sudo raspi-config
choose interface
enable camera
apt install python-picamera
```
# Test Pi Camera
```
raspistill -v -o test.jpg
```

# Compile wifibroadcast
```
apt-get install libpcap-dev
cd /root/wifibroadcast
make
```

# Enable Wi-Fi injection (using nexmon)
The RPI3 and Zero W use the Broadcom bcm43430a1 Wi-Fi chip.  The official Broadcom firmware does not support packet injection.  Using the nexmon project we are able to modify the firmware to fit our needs.

For more information on nexmon refere to https://github.com/seemoo-lab/bcm-rpi3 and https://github.com/seemoo-lab/nexmon

```
sudo apt install git
sudo apt-get install git gawk qpdf adb flex bison

sudo apt install raspberrypi-kernel-headers git libgmp3-dev gawk qpdf bison flex make
git clone https://github.com/seemoo-lab/nexmon.git
cd nexmon/buildtools/isl-0.10/
sudo su
./configure
make
make install
ln -s /usr/local/lib/libisl.so /usr/lib/arm-linux-gnueabihf/libisl.so.10
cd ..
source setup_env.sh
make
cd patches/bcm43430a1/7_45_41_46/nexmon/
make
make backup-firmware

outputs:	cp /lib/firmware/brcm/brcmfmac43430-sdio.bin brcmfmac43430-sdio.bin.orig
make install-firmware

cd ../../../../utilities/nexutil/
make
make install

apt-get remove wpasupplicant

# note RPi3B uses kernel 4.14.79-v7+ and RPi0W uses 4.14.79+
cp /home/pi/nexmon/patches/bcm43430a1/7_45_41_46/nexmon/brcmfmac_4.14.y-nexmon/brcmfmac.ko /lib/modules/4.14.79+/kernel/drivers/net/wireless/broadcom/brcm80211/brcmfmac/brcmfmac.ko

depmod -a
```


You may now reboot the platform and OreSat Live should function correctly.  If you would like to manually test the new nexmon patched firmware before rebooting try the following:
```
modprobe -r brcmfmac
modprobe brcmfmac

iw phy phy0 interface add mon0 type monitor
ifconfig mon0 up
nexutil -k3 # set channel 3
```


# Useful notes
https://www.bountysource.com/issues/53837760-nexutil-on-raspberry-pi-3

"We changed the behaviour of the brcmfmac driver on the rpi3 by adding a
separate monitor interface. All monitored frames end up in the mon0
interface and wlan0 can only be used for regular wifi traffic. There is
only one exception, if no additional interface mon0 exists, the monitored
frames would end up at wlan0."
