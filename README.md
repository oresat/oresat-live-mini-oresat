# Introduction
Mini OreSat brings the long distance wifi image streaming technology of [OreSat Live](https://github.com/oresat/oresat-dxwifi-software) to low-cost and easily-purchased hardware.  The Mini OreSat software turns a Raspberry Pi Zero W + Pi Camera into an ideal platform for hobbyist and classroom experiments with high-altitude ballooning.

When combined with the [OreSat Handheld Ground Station](https://github.com/oresat/oresat-live-handheld-ground-station) images can be transferred over much greater distances than traditional wifi.

# Hardware Setup
1. Connect the Pi Camera to the Raspberry Pi Zero W camera connector
2. Connect a USB power bank to the Raspberry Pi Zero W power input USB micro connection

In our testing the runtime for a 2500mAh battery is around 8 hours.  We recommend using the smallest battery that will meet your runtime requirements to reduce weight.

# Software Setup
1. [Download the SD card image oresat_mini_14_jun_2019.img](https://drive.google.com/a/pdx.edu/uc?id=1wvfyT9Je4e_oCBOsYYcLTs5nHJPRHxZ7&export=download)
2. Write the SD card image to a 2GB or larger card.  The [Raspberry Pi documentation](https://www.raspberrypi.org/documentation/installation/installing-images/) explains the procedure.

# Launch Procedure
1. Install the SD card in Raspberry Pi Zero W
2. Connect the USB power bank
3. Verify the ground station is able to receive images
4. Release the balloon

# Software Development
If you would like to build the system from source follow the [Build Instructions](BUILD.md)
