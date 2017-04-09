

Installation on a Pi Zero W

Using Raspbain Lite

Setup wifi as root
    cat << EOF > /etc/wpa_supplicant/wpa_supplicant.conf 
    country=GB
    ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
    update_config=1
    network={
         ssid="yourSSID"
         psk="*********"
    }
    EOF
 Enable SSH
    raspi-config
    # then enable ssh server

reboot to get the wifi up.


# install signalK requirements all as root
    apt-get update
    apt-get install -y curl git build-essential libnss-mdns avahi-utils libavahi-compat-libdnssd-dev
    cd /opt/
    wget https://nodejs.org/dist/v6.10.2/node-v6.10.2-linux-armv6l.tar.xz
    tar xvJf node-v6.10.2-linux-armv6l.tar.xz 
    rm node-v6.10.2-linux-armv6l.tar.xz
    cd /usr/local/bin/
    ln -s /opt/node-v6.10.2-linux-armv6l/bin/* .

# Install Grafana and InfluxDB as root
    cd 
    wget http://ftp.us.debian.org/debian/pool/main/i/influxdb/influxdb_1.1.1+dfsg1-4_armhf.deb
    wget http://ftp.us.debian.org/debian/pool/main/i/influxdb/influxdb-client_1.1.1+dfsg1-4_armhf.deb
    dpkg -i influxdb_1.1.1+dfsg1-4_armhf.deb
    dpkg -i influxdb-client_1.1.1+dfsg1-4_armhf.deb
    apt-get install -f


    wget https://bintray.com/fg2it/deb-rpi-1b/download_file?file_path=main%2Fg%2Fgrafana_4.2.0_armhf.deb
    mv download_file\?file_path\=main%2Fg%2Fgrafana_4.2.0_armhf.deb grafana_4.2.0_armhf.deb
    dpkg -i grafana_4.2.0_armhf.deb 
    systemctl daemon-reload
    systemctl enable grafana-server
    systemctl start grafana-server
Influxdb is now installed with the cli influx. Use influx to connect to the local dabase.
Grafana now on 



Turn
    apt-get install collectd

Edit collectd to disable rrdtool, battery and irq plugins, removing the config sections.
Enable network and add the following
<Plugin network>
      Server "127.0.0.1" "8096"
</Plugin>

# Edit /etc/influxdb/influxdb.conf to enable collectd
[[collectd]]
  enabled = true
  bind-address = ":8096"
  database = "collectd"
  typesdb = "/usr/share/collectd/types.db"

Check that /usr/share/collectd/types.db exists  (if not its at https://github.com/collectd/collectd/blob/master/src/types.db)

    service influxdb restart
    service collectd restart


OS level info should now be in the collectd db, create a Grafana datasource pointing to that DB.
Stats into influxDB are 


apt-get update
   64  wget http://ftp.us.debian.org/debian/pool/main/i/influxdb/influxdb_1.1.1+dfsg1-4_armhf.deb
   65  wget http://ftp.us.debian.org/debian/pool/main/g/grafana/grafana-data_2.6.0%2bdfsg-3_all.deb
   66  wget
   67  wget http://ftp.us.debian.org/debian/pool/main/g/grafana/grafana_2.6.0%2bdfsg-3_armhf.deb
   68  ls -ltra
   69  dpkg -i influxdb_1.0.2+dfsg1-1_armhf.deb
   70  dpkg -i influxdb_1.1.1+dfsg1-4_armhf.deb 
   71  dpkg -i grafana-data_2.6.0+dfsg-3_all.deb
   72  apt-get install -f
   73  dpkg -i grafana_2.6.0+dfsg-3_armhf.deb
   74  apt-get install -f
