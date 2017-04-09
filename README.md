This repository contains a number of utilities generally aimed at interfacing with a NMEA2000 CAN bus and collecting stats in InfluxDB for analysis in Grafana.

The repository is mainly to keep a record of work in progress, rather than a neat and tidy implementation, but if it helps others, great.

The code runs on an Arduino Due with a CAN bus trancever.
Some code may also run on an Arduino Mega to simulate Raymarine wind, speed and temperature sensors.
The heavy lifting on the Arduino as been done by https://github.com/ttlappalainen/NMEA2000.git to use the code here on a Due you may also need
https://github.com/collin80/due_can.git https://github.com/ttlappalainen/NMEA2000_due.git which the NMEA2000 implementation and CAN Bus drivers 
for the Due. 

This repository adds sailing performance functionality to a basic NMEA2000 -> SignalK bridge as well as some additional sensors. 

* statistics.h contains time based stats aggregation to allow a stream of random time measurements to be collected and agregated in uniform time periods, usefull for reducing raw update rates to prevent staturation of IO or CPU.
* polar.h contains generic polar performance calculations typically used to converted TWA/TWS to a target STW, or with current STW to a % performance so that these messages can be injeted back onto the CAN bus.
* pogo1250.h is the standard Pogo 1250 polar data for polar.h
* events.h is an event processing queue for Arduino
* enviroMonitor.h adds pressure and temperature CANbus messages from BPM180 sensor (Bosh, tipically < $5)
* batteryMonitor.h adds battery montoring using Hall Effect current sesors. 
* monitorAndBridge.ino is the monitor and Bridging Arduino code for the Arduino Due.

The Marine instrument installation is a Raymarine SeatalkNG installation with a e7 MFD, althogh there is nothing here that is specific to Raymarine. At the electrical and protocol level SeatalkNG == NMEA2000 == CAN@250kb/s. The differences are physical, mainly to ease installation.

As well as emitting CAM messages onto the NMEA2000 bus the Arduino Due runs code to implement sufficient of the Actisense protocol to feed SignalK running on a Pi Zero W. Installation of that part is listed below. Most of this is achieved with confgiguration and no additional code.


# Pi Zero W intallation - Rough notes.

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
    wget https://nodejs.org/dist/v6.10.2/node-v6.10.2-linux-armv6l.tar.xz
    tar xvJf node-v6.10.2-linux-armv6l.tar.xz 
    rm node-v6.10.2-linux-armv6l.tar.xz
    cd /usr/local/bin/
    ln -s /opt/node-v6.10.2-linux-armv6l/bin/* .


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
    service grafana status

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

