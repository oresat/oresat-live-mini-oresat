#!/bin/sh
echo Starting OreSatMini in 10 seconds
sleep 10
echo Adding monitor mode interface mon0
iw phy phy0 interface add mon0 type monitor
sleep 1
ifconfig mon0 up
sleep 1
echo Setting frequency to 2422MHz - channel 3 
nexutil -k3
sleep 3
echo Starting wifi video broadcast at 6 Mbps
/root/wifibroadcast/sharedmem_init_tx
sleep 1
/root/jpeg_stream_tx_stdout.py | /root/wifibroadcast/tx_rawsock -p 0 -b 4 -r 4 -f 1400 -t 1 -d 6 -y 0 mon0
