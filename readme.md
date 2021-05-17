# free5GC-UPF

## Get Started
### Prerequisites
Libraries used in UPF
```bash
sudo apt-get -y update
sudo apt-get -y install git gcc g++ cmake go libmnl-dev autoconf libtool libyaml-dev
go get github.com/sirupsen/logrus
```

Linux kernel module 5G GTP-U (Linux kernel version = 5.0.0-23-generic or version >= 5.4)
```bash
git clone https://github.com/free5gc/gtp5g.git
cd gtp5g
git checkout v0.3.2
make
sudo make install
```

### Build
```bash
mkdir build
cd build
cmake ..
make -j`nproc`
```

### Test
```bash
cd build/bin
./testutlt
```

### Edit configuration file
After building from sources, edit `./build/config/upfcfg.yaml`

### Setup environment
#### [Option 1] Set up when reboot
* (Must) IPv4 forwarding
```bash
# Uncomment lines under /etc/sysctl.conf as below
net.ipv4.ip_forward=1
```
* (Recommend) Forwarding chain in iptables can forward packet
```bash
sudo apt install iptables-persistent netfilter-persistent
sudo iptables -A FORWARD -j ACCEPT
sudo netfilter-persistent save
sudo systemctl enable netfilter-persistent
sudo systemctl restart netfilter-persistent
```
* (Recommend) Close ubuntu firewall
```bash
sudo systemctl stop ufw
sudo systemctl disable ufw
```
* (Optional) Using NAT for UE to access data network
```bash
sudo apt install iptables-persistent netfilter-persistent
sudo iptables -t nat -A POSTROUTING -o <DN_Interface_Name> -j MASQUERADE
sudo netfilter-persistent save
sudo systemctl enable netfilter-persistent
sudo systemctl restart netfilter-persistent
```

#### [Option 2] Set up in runtime
```bash
# (Must) IPv4 forwarding
sudo sysctl -a | grep forward        # check sys rule
sudo sysctl -w net.ipv4.ip_forward=1

# (Recommend) Forwarding chain in iptables can forward packet
sudo iptables -A FORWARD -j ACCEPT

# (Recommend) Close ubuntu firewall
sudo systemctl stop ufw
sudo systemctl disable ufw

# (Optional) Using NAT for UE to access data network
sudo iptables -t nat -A POSTROUTING -o <DN_Interface_Name> -j MASQUERADE
```

### Run
```bash
cd build
sudo -E ./bin/free5gc-upfd
```
To show usage: `./bin/free5gc-upfd -h`


## Clean the Environment (if needed)
### Remove POSIX message queues
```bash
ls /dev/mqueue/
rm /dev/mqueue/*
```

### Remove gtp devices
```bash
sudo ip l del upfgtp
```
