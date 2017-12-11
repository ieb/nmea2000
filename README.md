This repository contains a number of utilities generally aimed at interfacing with a NMEA2000 CAN bus and collecting stats in InfluxDB for analysis in Grafana.

The repository is mainly to keep a record of work in progress, rather than a neat and tidy implementation, but if it helps others, great.

The code runs on an Arduino Due with a CAN bus trancever.
Some code may also run on an Arduino Mega to simulate Raymarine wind, speed and temperature sensors.
The heavy lifting on the Arduino as been done by https://github.com/ttlappalainen/NMEA2000.git to use the code here on a Due you may also need
https://github.com/collin80/due_can.git https://github.com/ttlappalainen/NMEA2000_due.git which the NMEA2000 implementation and CAN Bus drivers for the Due. 


* libs contains c++ code for the ArduinoDue
** batteryMonitor.h - monitoring batteries and current using Hall Effect sensors on ADC pins.
** boarMonitor.h - emitting boat speed, wind speed data as NMEA2000 PGNs
** enviroMonitor.h - monitoring pressure and temperature
** events.h - an event queue.
** monitors.h - constants
** motionSensor.h - rotation, acceleration sensor
** multiSensor.h - wind and water speed sensor with corrections for motion etc, also calculates 
** pogo1250.h - Data for a pogo1250 polar.
** polar.h - polar performance calculations.
** statistics.h - stats for linear and radial data.
** testmocks.h - mocks to allow tests to run.
** waterMonitor.h - water temp sensor.
* monitorAndBridge contains a sensor monitor and bridge from a NMEA2000 bus to ActisenseSerial, with the ability to read the raw signals from Airmar water speed and RayMarine WindSensorts, battery voltages and a 10DoF sensor board. There is a built in polar performance monitor that emits % polar performance and target boat speed.

The Marine instrument installation is a Raymarine SeatalkNG installation with a e7 MFD, althogh there is nothing here that is specific to Raymarine. At the electrical and protocol level SeatalkNG == NMEA2000 == CAN@250kb/s. The differences are physical, mainly to ease installation.

As well as emitting CAM messages onto the NMEA2000 bus the Arduino Due runs code to implement sufficient of the Actisense protocol to feed SignalK running on a Pi Zero W. Installation of that part is listed below. Most of this is achieved with confgiguration and no additional code.


# Pi Zero W installation - Rough notes.

Installation on a Pi Zero W for pushing stats from NMEA2000 -> Arduino Due -> SkignalK -> InfluxDB -> Grafana

Using Raspbain Lite written to a SDMicro card boot and configure the Pi Zero W, once booted. At the console (HDMI+USB Keyboard)

## Setup wifi as root
    
    cat << EOF > /etc/wpa_supplicant/wpa_supplicant.conf 
    country=GB
    ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
    update_config=1
    network={
         ssid="yourSSID"
         psk="*********"
    }
    EOF

## TODO: setup as a Access Point, assuming it can be done, if not make the MFD the access point and join that network.

## Enable SSH

    
    raspi-config
    # then enable ssh server

## reboot to get the wifi up.

Now switch to using ssh in from a Linux box.

## install signalK requirements all as root

    apt-get update
    apt-get install -y curl git build-essential libnss-mdns avahi-utils libavahi-compat-libdnssd-dev
    cd /opt/
//    wget https://nodejs.org/dist/v6.10.2/node-v6.10.2-linux-armv6l.tar.xz
    wget https://nodejs.org/dist/v6.11.1/node-v6.11.1-linux-armv6l.tar.gz 
    tar xvzf node-v6.11.1-linux-armv6l.tar.gz  
    rm node-v6.11.1-linux-armv6l.tar.gz 
    cd /usr/local/bin/
    ln -s /opt/node-v6.11.1-linux-armv6l/bin/* .


## Install InfluxDB as root
    cd 
    wget http://ftp.us.debian.org/debian/pool/main/i/influxdb/influxdb_1.1.1+dfsg1-4_armhf.deb
    wget http://ftp.us.debian.org/debian/pool/main/i/influxdb/influxdb-client_1.1.1+dfsg1-4_armhf.deb
    dpkg -i influxdb_1.1.1+dfsg1-4_armhf.deb
    dpkg -i influxdb-client_1.1.1+dfsg1-4_armhf.deb
    apt-get install -f

## Install Grafana as root

    wget https://bintray.com/fg2it/deb-rpi-1b/download_file?file_path=main%2Fg%2Fgrafana_4.2.0_armhf.deb
    mv download_file\?file_path\=main%2Fg%2Fgrafana_4.2.0_armhf.deb grafana_4.2.0_armhf.deb
    dpkg -i grafana_4.2.0_armhf.deb 
    systemctl daemon-reload
    systemctl enable grafana-server
    systemctl start grafana-server

## Install collectd to get stats on the OS into InfluxDB

This will allow you to see if the Pi Zero is overloaded. 

    apt-get install collectd


Edit collectd to disable rrdtool, battery and irq plugins, removing the config sections.
Enable network and add the following

    <Plugin network>
          Server "127.0.0.1" "8096"
    </Plugin>

Edit /etc/influxdb/influxdb.conf to enable collectd

    [[collectd]]
      enabled = true
      bind-address = ":8096"
      database = "collectd"
      typesdb = "/usr/share/collectd/types.db"

Check that /usr/share/collectd/types.db exists  (if not its at https://github.com/collectd/collectd/blob/master/src/types.db)

    service influxdb restart
    service collectd restart

Check all is Ok

    service influxdb status
    service collectd status
    service grafana-server status

## Complete SignalK installation

As a normal user who is a member of the staff group

    cd /usr/local/src
    git clone https://github.com/SignalK/signalk-server-node.git
    cd signalk-server-node
    npm install     # Wait for some time as everything gets installed.
    npm install mdns
    sudo bash rpi-setup.sh

use port 80 to run SignalK see https://github.com/SignalK/signalk-server-node/blob/master/raspberry_pi_installation.md for more detailed info


## Add reporting to InfluxDB

    cd /usr/local/src/signalk-server-node
    npm install https://github.com/sbender9/signalk-to-influxdb
    service signalk restart

check all ok 

    service signalk status 

Then go to http://localhost and configure the influxDB plugn

check all ok again

    service signalk status 


OS level info should now be in the collectd db, create a Grafana datasource pointing to that DB.
See OS Dashboard.json and windDashboard.json for exmaples.

The SignalK process will report to InfluxDB over the http API, which involves 1 POST oepration per report.
This is probably less overhead than reporting using the InfluxDB UDP route, but keep an eye on load average
and IO stats in Grafana (see OSDashboard.json for a Grafana Daskboard on the OS). Also see WindDashboard for 
a Dashboard on AWA etc.




## Connections

The Arduino is connected to the Pi using a USB Cable, the Arduino appears as a USB CDC Serial port at /dev/ttyACM0 which can be opened at 115200 baud. 

lsusb should show

    root@lunacore:/usr/local/src/signalk-server-node# lsusb
    Bus 001 Device 005: ID 2341:003d Arduino SA 
    Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    root@lunacore:/usr/local/src/signalk-server-node# 
    root@lunacore:/usr/local/src/signalk-server-node# usb-devices 

    T:  Bus=01 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#=  1 Spd=480 MxCh= 1
    D:  Ver= 2.00 Cls=09(hub  ) Sub=00 Prot=01 MxPS=64 #Cfgs=  1
    P:  Vendor=1d6b ProdID=0002 Rev=04.04
    S:  Manufacturer=Linux 4.4.50+ dwc_otg_hcd
    S:  Product=DWC OTG Controller
    S:  SerialNumber=20980000.usb
    C:  #Ifs= 1 Cfg#= 1 Atr=e0 MxPwr=0mA
    I:  If#= 0 Alt= 0 #EPs= 1 Cls=09(hub  ) Sub=00 Prot=00 Driver=hub

    T:  Bus=01 Lev=01 Prnt=01 Port=00 Cnt=01 Dev#=  5 Spd=12  MxCh= 0
    D:  Ver= 1.10 Cls=02(commc) Sub=00 Prot=00 MxPS= 8 #Cfgs=  1
    P:  Vendor=2341 ProdID=003d Rev=00.01
    S:  Manufacturer=Arduino (www.arduino.cc)
    S:  Product=Arduino Due Prog. Port
    S:  SerialNumber=9543334393335111B181
    C:  #Ifs= 2 Cfg#= 1 Atr=c0 MxPwr=100mA
    I:  If#= 0 Alt= 0 #EPs= 1 Cls=02(commc) Sub=02 Prot=01 Driver=cdc_acm
    I:  If#= 1 Alt= 0 #EPs= 2 Cls=0a(data ) Sub=00 Prot=00 Driver=cdc_acm
    root@lunacore:/usr/local/src/signalk-server-node# 




Signak node server should be configured with an actisense input listening to this port, and should have plugins configured to 
a) send data to InfluxDB b) NMEA0183. Currently there are only plugins for TCP connections which causes a high level of 
turnover of TCP ports, it might be better to use UDP, that that will need some coding.

It is possible to power the Pi from the ArduinoDue over USB and visa versa. The Arduino uses < 100mA, but the Pi might use a bit more when under full load. 



# Update - September 2017

Sending data to influxDB on a standard SignalK setup swamps InfluxDB making it unusable, probably as a result of the frequency of packets > 1s in some cases, and the way the standard signalk-to-influxDB plugin is written. New signalk-telemetry plugin dixes this sending data in one block, queued to be more effcient at a fixed rate.

It may be possible to run everything from the Pi using socketCan rather than using a Due. This needs MCP2515 trancevers, 12C for the 10Dof sensor. In addition 1Wire temperature sensors and a I2C RTC can be added. Potentially this reduces power consumption as 80mA isnt needed to power the Due any more. the Pi has no A2D so a SPI based chip would need to be used.

The Pi supports 2x CAN trancevers with SocketCAN.


Raspberry Pi 10Dof

The npm module https://www.npmjs.com/package/nodeimu works for several 10DOf sensors on i2C, setup only requires i2c is enabled.
The IMU Requires configuration see https://github.com/RTIMULib/RTIMULib2 which also requires Octave to be installed for full calibration.

dtparam=i2c_arm=on

Raspberry Pi RTC
Disconnected from ntp the Pi will not keep time, using a ds3231 over i2c gives acurate time once setup.

dtoverlay=i2c-rtc,ds3231
see http://raspmer.blogspot.co.uk/2015/07/how-to-use-ds3231-i2c-real-time-clock.html




Raspberry Pi 1 Wire

DS18B20 are 1 wire temperature sensors. To read on a Pi enable 1wire and connedt with a 4K7 pullup to 3.3v, where the 1 wire is the default pin 7 

Output is at cat  /sys/bus/w1/devices/<ID>/w1_slave.
Map the ID to the sensor by warming each sensor in turn.

https://www.npmjs.com/package/ds18b20 for NPM



Raspberry Pi CanBus

Investigating how to run over SocketCan

2x MCP2515 drivers with Trancevers from eBay, MUST be connected with 3.3V to 5V level shifters to avoid GPIO damage as the trancevers will run at 5V. 2 can be connected to get CAN0 and CAN1

Wiring 

    2 5V   --------------- Vdd
    1 3V3   ------> LS 3V3
    2 5V ---------> LS 5V
    9 GND --------> LS GND
    9 GND  --------------  GND
    19 MOSI ---->  LS ----> SI0 & SI1
    21 MOSO <----  LS <---- SO0 & SO1
    23 SCLK ------> LS ----> SCLK0 & SCLK1
    24 CE0 ------> LS ----> CS0
    26 CE1 ------> LS ----> CS1
    18 GPIO24 <--  LS <---- INT0
    22 xGPIO25 <--  LS <---- INT1

    dtoverlay mcp2515-can0  oscillator=8000000 interrupt=24
    dtoverlay mcp2515-can1  oscillator=8000000 interrupt=25

    root@raspb:~/can-utils# dmesg | egrep "spi|can"
    [  477.594591] mcp251x spi0.0 can0: MCP2515 successfully initialized.
    [  487.165293] mcp251x spi0.1 can1: MCP2515 successfully initialized.
    [  495.704786] can: controller area network core (rev 20120528 abi 9)
    root@raspb:~/can-utils# ifconfig can0
    can0: flags=128<NOARP>  mtu 16
            unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
            RX packets 0  bytes 0 (0.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    root@raspb:~/can-utils# ifconfig can1
    can1: flags=128<NOARP>  mtu 16
            unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
            RX packets 0  bytes 0 (0.0 B)
            RX errors 0  dropped 0  overruns 0  frame 0
            TX packets 0  bytes 0 (0.0 B)
            TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

    root@raspb:~/can-utils# 


Install can-utils from source.

    # git clone https://github.com/linux-can/can-utils.git
    # cd can-utils
    # ./autogen.sh
    # ./configure
    # make
    # sudo make install

If initialisation reports a wiring problem, then there almost cerainly is a wiring problem.

Configure interface


    root@raspb:~/can-utils# ip link set can0 up type can bitrate 250000
    root@raspb:~/can-utils#  ip link set can1 up type can bitrate 250000

Test

    root@raspb:~/can-utils# ./candump can1 &
    [1]+ ./candump can1 &
    root@raspb:~/can-utils# ./cansend can0 001#1122334455667788
      can1  001   [8]  11 22 33 44 55 66 77 88
    root@raspb:~/can-utils# ./cansend can0 001#1122334455667788
      can1  001   [8]  11 22 33 44 55 66 77 88
    root@raspb:~/can-utils# ./cansend can0 001#1122334455667788
      can1  001   [8]  11 22 33 44 55 66 77 88
    root@raspb:~/can-utils# 





To echo all CAN1 traffic to CAN0, setup CAN1 to listen only, and setup CAN0 to have loopback enabled to anything reading CAN0 gets the CAN0 messages as well.


    root@raspb:~/can-utils# ip link set can0 up type can bitrate 250000 loopback on
    root@raspb:~/can-utils#  ip link set can1 up type can bitrate 250000 listen-only on

Then to bridge

    candump can1 -B can0

For can0 -> SK

see https://github.com/chacal/signalk-socketcan-device



------------------------------------------------------

Moved back to the Due for now as its single threaded and doesnt stop expecially with no memory allocation. Processing power is a lot less, but it does a lot less also, not having an OS. 84MHz is plenty for this usage.

Todo:
Test polar calcs - partially done.
Test Can0 - done
Test IMU and calibrate - done.
Test Cos and Sin - done
Test Pulse counters  - done
Test SD card
Test 1 Wire
Test wind and speed calcs
Test linear voltage regulator vs noise from dc-dc converter 
   Linear voltage is no nose, but too hot
   LTSpice model indicates a 10uF cap will eliminate ripple from buck converter.






Input is candump can0 pipe, output is signalk-socketcan-device.

    "pipedProviders": [{
      "id": "can0",
      "pipeElements": [
        {
          "type": "providers/execute",
          "options": {
            "command": "candump can0 | candump2analyzer | analyzer -json -si -nv"
          }
        },
        {
          "type": "providers/liner"
        },
        {
          "type": "providers/from_json"
        },
        {
          "type": "signalk-socketcan-device",
          "options": {
            "n2kAddress": 110,
            "canDevice": "can0"
          }
        },
        {
          "type": "providers/n2k-signalk"
        }
      ]
    }]
