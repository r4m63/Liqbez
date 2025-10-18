Wireguard VPN
```bash
#!/bin/sh
# metadata_begin
# recipe: Wireguard
# tags: centos9,alma8,alma9,rocky8,rocky9,oracle8,oracle9,debian11,debian12,ubuntu2004,ubuntu2204,ubuntu2404
# revision: 3
# description_ru: Wireguard server. Клиентский конфиг доступен в /etc/wireguard/client/clientXXX
# description_en: Wireguard server. Client config placed in /etc/wireguard/client/clientXXX
# metadata_end
RNAME=Wireguard

set -x

LOG_PIPE=/tmp/log.pipe.$$                                                                                                                                                                                                                    
mkfifo ${LOG_PIPE}
LOG_FILE=/root/${RNAME}.log
touch ${LOG_FILE}
chmod 600 ${LOG_FILE}

tee < ${LOG_PIPE} ${LOG_FILE} &

exec > ${LOG_PIPE}
exec 2> ${LOG_PIPE}

killjobs() {
    jops="$(jobs -p)"
    test -n "${jops}" && kill ${jops} || :
}
trap killjobs INT TERM EXIT

echo
echo "=== Recipe ${RNAME} started at $(date) ==="
echo

if [ -f /etc/redhat-release ]; then
    OSNAME=centos
else
    OSNAME=debian
fi

if [ "${OSNAME}" = "debian" ]; then
    export DEBIAN_FRONTEND="noninteractive"

    # Wait firstrun script
    while ps uxaww | grep  -v grep | grep -Eq 'apt-get|dpkg' ; do echo "waiting..." ; sleep 3 ; done
    
    OSREL=$(lsb_release -s -c)
    apt-get update --allow-releaseinfo-change || :
    apt-get update
    # Installing packages
    apt-mark hold qemu-guest-agent || :
    apt upgrade -y
    apt-get -y install wireguard
    apt-mark unhold qemu-guest-agent || :
else
    OSREL=$(rpm -qf --qf '%{version}' /etc/redhat-release | cut -d . -f 1)
    OS_NAME=$(grep '^NAME=' /etc/os-release | cut -d '"' -f 2)
    yum -y install yum-plugin-versionlock
    yum versionlock qemu-guest-agent
    yum -y update

    # Проверка для Oracle Linux 8
    if [ "$OSREL" = "8" ] && [ "$OS_NAME" = "Oracle Linux Server" ]; then
        yum -y install https://www.elrepo.org/elrepo-release-8.el8.elrepo.noarch.rpm
        yum --setopt=timeout=5 install -y kmod-wireguard wireguard-tools
    # Для всех релизов 8, кроме Oracle Linux 8
    elif [ "$OSREL" = "8" ] && [ "$OS_NAME" != "Oracle Linux Server" ]; then
        yum install -y epel-release elrepo-release
        yum --setopt=timeout=5 install -y kmod-wireguard wireguard-tools
    # Для всех остальных случаев
    else
        yum -y install epel-release || yum -y install oracle-epel-release-el9
        yum -y install wireguard-tools
    fi
    yum versionlock delete qemu-guest-agent
fi

DIR=/etc/wireguard
umask 077
if [ -f $DIR/publickey ]; then
    INSTALLED=1
fi

prepare_server() {
    echo "net.ipv4.ip_forward = 1" >> /etc/sysctl.conf
    sysctl -p /etc/sysctl.conf

    mkdir -p $DIR
    KEY=$(wg genkey)
    PUB=$(echo $KEY | wg pubkey)

    echo $KEY > $DIR/privatekey
    echo $PUB > $DIR/publickey

    cat << EOF > $DIR/wg0.conf
[Interface]
Address = 192.168.15.1/24
SaveConfig = true
ListenPort = 51194
PrivateKey = $KEY
EOF
    systemctl enable wg-quick@wg0.service
    systemctl start wg-quick@wg0.service
    if [ "${OSNAME}" = "debian" ]; then
        ifname=$(ip route get 1 | grep -Po '(?<=dev )[^ ]+')
        if [ -n "$(which nft)" ] && [ -z "$(which iptables)" ]; then
            cat << EOF > /etc/nftables.conf
#!/usr/sbin/nft -f

flush ruleset

table inet filter {
    chain input {
        type filter hook input priority 0; policy accept;
    }

    chain forward {
        type filter hook forward priority 0; policy accept;
    }

    chain output {
        type filter hook output priority 0; policy accept;
    }
}
table ip nat {
    chain postrouting {
        type nat hook postrouting priority srcnat; policy accept;
        ip saddr 192.168.15.0/24 oif "ens3" masquerade
    }
}
EOF
    cat << EOF > /etc/systemd/system/nft.service
[Unit]
Description=Run NFT rules at startup after all systemd services are loaded
After=default.target

[Service]
Type=simple
RemainAfterExit=yes
ExecStart=/usr/sbin/nft -f /etc/nftables.conf
TimeoutStartSec=0

[Install]
WantedBy=default.target
EOF
            systemctl daemon-reload
            systemctl enable nft.service
        else
            iptables -t nat -A POSTROUTING -s 192.168.15.0/24 -o ${ifname} -j MASQUERADE
            apt install -y iptables-persistent
        fi
else
    firewall-cmd --permanent --zone=public --add-port=51194/udp
    firewall-cmd --permanent --zone=public --add-masquerade
    firewall-cmd --reload
fi
}

prepare_first_client() {
    CLIENT_KEY=$(wg genkey)
    CLIENT_PUB=$(echo $CLIENT_KEY | wg pubkey)
    mkdir -p $DIR/client
    CLIENT_DIR=$(mktemp -d $DIR/client/clientXXX)

    echo $CLIENT_KEY > $CLIENT_DIR/privatekey
    echo $CLIENT_PUB > $CLIENT_DIR/publickey

    cat << EOF > $CLIENT_DIR/client.conf
[Interface]
PrivateKey = $CLIENT_KEY
Address = 192.168.15.2/24
DNS = 1.1.1.1, 8.8.8.8

[Peer]
PublicKey = $(cat $DIR/publickey)
AllowedIPs = 0.0.0.0/0
Endpoint = $(ip route get 1 | grep -Po '(?<=src )[^ ]+'):51194
EOF
    vm_export_file client.conf $CLIENT_DIR/client.conf
    START=/root/startup.sh
    cat << EOF > $START
#!/bin/sh
wg set wg0 peer '$CLIENT_PUB' allowed-ips 192.168.15.2
systemctl disable run-at-startup.service
EOF

    chmod +x $START
    cat << EOF > /etc/systemd/system/run-at-startup.service
[Unit]
Description=Run script at startup after all systemd services are loaded
After=default.target

[Service]
Type=simple
RemainAfterExit=yes
ExecStart=$START
TimeoutStartSec=0

[Install]
WantedBy=default.target
EOF

    systemctl daemon-reload
    systemctl enable run-at-startup.service

    shutdown -r
}
prepare_client(){
    CLIENT_KEY=$(wg genkey)
    CLIENT_PUB=$(echo $CLIENT_KEY | wg pubkey)
    CLIENT_DIR=$(mktemp -d $DIR/client/clientXXX)
    CLIENT_COUNT=$(ls $DIR/client | wc -l)
    NEW_CLIENT=$(expr $CLIENT_COUNT + 1)
    echo $CLIENT_KEY > $CLIENT_DIR/privatekey
    echo $CLIENT_PUB > $CLIENT_DIR/publickey

    cat << EOF > $CLIENT_DIR/client.conf
[Interface]
PrivateKey = $CLIENT_KEY
Address = 192.168.15.$NEW_CLIENT/24
DNS = 1.1.1.1, 8.8.8.8

[Peer]
PublicKey = $(cat $DIR/publickey)
AllowedIPs = 0.0.0.0/0
Endpoint = $(ip route get 1 | grep -Po '(?<=src )[^ ]+'):51194
EOF
    wg set wg0 peer "$CLIENT_PUB" allowed-ips "192.168.15.$NEW_CLIENT"
    vm_export_file client.conf $CLIENT_DIR/client.conf
}

if [ -z "$INSTALLED" ]; then
    prepare_server
    prepare_first_client
else
    prepare_client
fi
```



Wireguard VPN 2

```bash
#!/bin/sh
# metadata_begin
# recipe: Wireguard
# tags: centos9,alma8,alma9,rocky8,rocky9,oracle8,oracle9,debian11,debian12,ubuntu2004,ubuntu2204,ubuntu2404
# revision: 2
# description_ru: Wireguard server. Клиентский конфиг доступен в /etc/wireguard/client/clientXXX
# description_en: Wireguard server. Client config placed in /etc/wireguard/client/clientXXX
# metadata_end
RNAME=Wireguard

set -x

LOG_PIPE=/tmp/log.pipe.$$                                                                                                                                                                                                                    
mkfifo ${LOG_PIPE}
LOG_FILE=/root/${RNAME}.log
touch ${LOG_FILE}
chmod 600 ${LOG_FILE}

tee < ${LOG_PIPE} ${LOG_FILE} &

exec > ${LOG_PIPE}
exec 2> ${LOG_PIPE}

killjobs() {
    jops="$(jobs -p)"
    test -n "${jops}" && kill ${jops} || :
}
trap killjobs INT TERM EXIT

echo
echo "=== Recipe ${RNAME} started at $(date) ==="
echo

if [ -f /etc/redhat-release ]; then
    OSNAME=centos
else
    OSNAME=debian
fi

if [ "${OSNAME}" = "debian" ]; then
    export DEBIAN_FRONTEND="noninteractive"

    # Wait firstrun script
    while ps uxaww | grep  -v grep | grep -Eq 'apt-get|dpkg' ; do echo "waiting..." ; sleep 3 ; done
    
    OSREL=$(lsb_release -s -c)
    apt-get update --allow-releaseinfo-change || :
    apt-get update
    # Installing packages
    apt-mark hold qemu-guest-agent || :
    apt upgrade -y
    apt-get -y install wireguard
    apt-mark unhold qemu-guest-agent || :
else
    OSREL=$(rpm -qf --qf '%{version}' /etc/redhat-release | cut -d . -f 1)
    OS_NAME=$(grep '^NAME=' /etc/os-release | cut -d '"' -f 2)
    yum -y install yum-plugin-versionlock  #<=======
    yum versionlock qemu-guest-agent       #<=======
    yum -y update

    # Проверка для Oracle Linux 8
    if [ "$OSREL" = "8" ] && [ "$OS_NAME" = "Oracle Linux Server" ]; then
        yum -y install https://www.elrepo.org/elrepo-release-8.el8.elrepo.noarch.rpm
        yum --setopt=timeout=5 install -y kmod-wireguard wireguard-tools
    # Для всех релизов 8, кроме Oracle Linux 8
    elif [ "$OSREL" = "8" ] && [ "$OS_NAME" != "Oracle Linux Server" ]; then
        yum install -y epel-release elrepo-release
        yum --setopt=timeout=5 install -y kmod-wireguard wireguard-tools
    # Для всех остальных случаев
    else
        yum -y install epel-release || yum -y install oracle-epel-release-el9
        yum -y install wireguard-tools
    fi
    yum versionlock delete qemu-guest-agent #<=======
fi

DIR=/etc/wireguard
umask 077
if [ -f $DIR/publickey ]; then
    INSTALLED=1
fi

prepare_server() {
    echo "net.ipv4.ip_forward = 1" >> /etc/sysctl.conf
    sysctl -p /etc/sysctl.conf

    mkdir -p $DIR
    KEY=$(wg genkey)
    PUB=$(echo $KEY | wg pubkey)

    echo $KEY > $DIR/privatekey
    echo $PUB > $DIR/publickey

    cat << EOF > $DIR/wg0.conf
[Interface]
Address = 192.168.15.1/24
SaveConfig = true
ListenPort = 51194
PrivateKey = $KEY
EOF
    systemctl enable wg-quick@wg0.service
    systemctl start wg-quick@wg0.service
    if [ "${OSNAME}" = "debian" ]; then
        ifname=$(ip route get 1 | grep -Po '(?<=dev )[^ ]+')
        if [ -n "$(which nft)" ] && [ -z "$(which iptables)" ]; then
            cat << EOF > /etc/nftables.conf
#!/usr/sbin/nft -f

flush ruleset

table inet filter {
    chain input {
        type filter hook input priority 0; policy accept;
    }

    chain forward {
        type filter hook forward priority 0; policy accept;
    }

    chain output {
        type filter hook output priority 0; policy accept;
    }
}
table ip nat {
    chain postrouting {
        type nat hook postrouting priority srcnat; policy accept;
        ip saddr 192.168.15.0/24 oif "ens3" masquerade
    }
}
EOF
    cat << EOF > /etc/systemd/system/nft.service
[Unit]
Description=Run NFT rules at startup after all systemd services are loaded
After=default.target

[Service]
Type=simple
RemainAfterExit=yes
ExecStart=/usr/sbin/nft -f /etc/nftables.conf
TimeoutStartSec=0

[Install]
WantedBy=default.target
EOF
            systemctl daemon-reload
            systemctl enable nft.service
        else
            iptables -t nat -A POSTROUTING -s 192.168.15.0/24 -o ${ifname} -j MASQUERADE
            apt install -y iptables-persistent
        fi
else
    firewall-cmd --permanent --zone=public --add-port=51194/udp
    firewall-cmd --permanent --zone=public --add-masquerade
    firewall-cmd --reload
fi
}

prepare_first_client() {
    CLIENT_KEY=$(wg genkey)
    CLIENT_PUB=$(echo $CLIENT_KEY | wg pubkey)
    mkdir -p $DIR/client
    CLIENT_DIR=$(mktemp -d $DIR/client/clientXXX)

    echo $CLIENT_KEY > $CLIENT_DIR/privatekey
    echo $CLIENT_PUB > $CLIENT_DIR/publickey

    cat << EOF > $CLIENT_DIR/client.conf
[Interface]
PrivateKey = $CLIENT_KEY
Address = 192.168.15.2/24
DNS = 1.1.1.1, 8.8.8.8

[Peer]
PublicKey = $(cat $DIR/publickey)
AllowedIPs = 0.0.0.0/0
Endpoint = $(ip route get 1 | grep -Po '(?<=src )[^ ]+'):51194
EOF
    vm_export_file client.conf $CLIENT_DIR/client.conf
    START=/root/startup.sh
    cat << EOF > $START
#!/bin/sh
wg set wg0 peer '$CLIENT_PUB' allowed-ips 192.168.15.2
systemctl disable run-at-startup.service
EOF

    chmod +x $START
    cat << EOF > /etc/systemd/system/run-at-startup.service
[Unit]
Description=Run script at startup after all systemd services are loaded
After=default.target

[Service]
Type=simple
RemainAfterExit=yes
ExecStart=$START
TimeoutStartSec=0

[Install]
WantedBy=default.target
EOF

    systemctl daemon-reload
    systemctl enable run-at-startup.service

    shutdown -r
}
prepare_client(){
    CLIENT_KEY=$(wg genkey)
    CLIENT_PUB=$(echo $CLIENT_KEY | wg pubkey)
    CLIENT_DIR=$(mktemp -d $DIR/client/clientXXX)
    CLIENT_COUNT=$(ls $DIR/client | wc -l)
    NEW_CLIENT=$(expr $CLIENT_COUNT + 1)
    echo $CLIENT_KEY > $CLIENT_DIR/privatekey
    echo $CLIENT_PUB > $CLIENT_DIR/publickey

    cat << EOF > $CLIENT_DIR/client.conf
[Interface]
PrivateKey = $CLIENT_KEY
Address = 192.168.15.$NEW_CLIENT/24
DNS = 1.1.1.1, 8.8.8.8

[Peer]
PublicKey = $(cat $DIR/publickey)
AllowedIPs = 0.0.0.0/0
Endpoint = $(ip route get 1 | grep -Po '(?<=src )[^ ]+'):51194
EOF
    wg set wg0 peer "$CLIENT_PUB" allowed-ips "192.168.15.$NEW_CLIENT"
    vm_export_file client.conf $CLIENT_DIR/client.conf
}

if [ -z "$INSTALLED" ]; then
    prepare_server
    prepare_first_client
else
    prepare_client
fi
```


Teamspeak
```bash
#!/bin/sh
#
# metadata_begin
# recipe: Teamspeak
# tags: centos9,debian11,debian12,ubuntu2004,ubuntu2204,ubuntu2404,alma8,alma9,oracle8,oracle9,rocky8,rocky9
# revision: 21
# description_ru: Teamspeak 3 сервер. Логин, пароль и токен можно найти в файле /root/ts3_login_data
# description_en: Teamspeak 3 server. Login, password and token placed in file /root/ts3_login_data
# metadata_end
#
RNAME=Teamspeak

set -x

LOG_PIPE=/tmp/log.pipe.$$                                                                                                                                                                                                                    
mkfifo ${LOG_PIPE}
LOG_FILE=/root/${RNAME}.log
touch ${LOG_FILE}
chmod 600 ${LOG_FILE}

tee < ${LOG_PIPE} ${LOG_FILE} &

exec > ${LOG_PIPE}
exec 2> ${LOG_PIPE}

killjobs() {
	jops="$(jobs -p)"
	test -n "${jops}" && kill ${jops} || :
}
trap killjobs INT TERM EXIT

echo
echo "=== Recipe ${RNAME} started at $(date) ==="
echo

if [ -f /etc/redhat-release ]; then
	OSNAME=centos
else
	OSNAME=debian
fi

Service() {
	# $1 - name
	# $2 - command

	if [ -n "$(which systemctl 2>/dev/null)" ]; then
		systemctl ${2} ${1}.service
	else
		if [ "${2}" = "enable" ]; then
			if [ "${OSNAME}" = "debian" ]; then
				update-rc.d ${1} enable
			else
				chkconfig ${1} on
			fi
		else
			service ${1} ${2}
		fi
	fi
}

RootMyCnf() {
    # Saving mysql password
    touch /root/.my.cnf 
    chmod 600 /root/.my.cnf
    echo "[client]" > /root/.my.cnf
    echo "password=${1}" >> /root/.my.cnf

}

if [ "${OSNAME}" = "debian" ]; then
	export DEBIAN_FRONTEND="noninteractive"

	# Wait firstrun script
	while ps uxaww | grep  -v grep | grep -Eq 'apt-get|dpkg' ; do echo "waiting..." ; sleep 3 ; done
	apt-get update --allow-releaseinfo-change || :
	apt-get update
	test -f /usr/bin/which || apt-get -y install which
	which lsb_release 2>/dev/null || apt-get -y install lsb-release
	which logger 2>/dev/null || apt-get -y install bsdutils
	OSREL=$(lsb_release -s -c)
	
	pkglist="openssl sqlite3 wget apache2 bzip2 cron ca-certificates libapache2-mod-php"
	
    # Installing packages
    apt-get -y install ${pkglist}
else
	OSREL=$(rpm -qf --qf '%{version}' /etc/redhat-release | cut -d . -f 1)
	yum -y update
	
	if [ "$OSREL" = "8" ]; then
		yum -y install epel-release || yum -y install oracle-epel-release-el8
	else
		yum -y install epel-release || yum -y install oracle-epel-release-el9
	fi

	# Setting proxy
	# shellcheck disable=SC2154
	if [ ! "($HTTPPROXYv4)" = "()" ]; then
		# Стрипаем пробелы, если они есть
		PR="($HTTPPROXYv4)"
		PR=$(echo ${PR} | sed "s/''//g" | sed 's/""//g')
		if [ -n "${PR}" ]; then
			echo "proxy=${PR}" >> /etc/yum.conf
		fi
	fi

	if [ "$OSREL" = "9" ]; then 
		yum install -y https://rpms.remirepo.net/enterprise/remi-release-9.rpm
		yum module reset -y php
		yum module enable -y php:remi-7.4
	fi

	pkglist="openssl sqlite wget httpd php bzip2 which anacron tar"
	
    yum -y install ${pkglist} || yum -y install ${pkglist} || yum -y install ${pkglist}

	# Removing proxy
	sed -r -i "/proxy=/d" /etc/yum.conf
fi

arch=$(uname -m)
test "${arch}" = "x86_64" && arch=amd64


useradd teamspeak
mkdir -p /home/teamspeak
chown teamspeak:teamspeak /home/teamspeak
mkdir /home/teamspeak/ts3
chown teamspeak:teamspeak /home/teamspeak/ts3
cd /home/teamspeak || exit
TSVER=3.13.7
wget --no-check-certificate https://files.teamspeak-services.com/releases/server/${TSVER}/teamspeak3-server_linux_${arch}-${TSVER}.tar.bz2

tar --strip-components=1 -xpf teamspeak3-server_linux_${arch}-${TSVER}.tar.bz2 -C /home/teamspeak/ts3
chown -R teamspeak:teamspeak /home/teamspeak/ts3
cd ts3 || exit
touch .ts3server_license_accepted
su teamspeak -c "./ts3server_startscript.sh start" > /root/credits.txt 2>&1
sleep 5
login=serveradmin
password=$(cat /root/credits.txt | sed -r -n 's/.+password=\s*"(.+)"/\1/p')
token=$(cat /root/credits.txt | sed -r -n 's/.+token=\s*"*?(.+)"*?/\1/p')
#rm -f /root/credits.txt
touch /root/credits.txt
chmod 600 /root/credits.txt

_tmppass="($PASS)"
if [ -n "${_tmppass}" ] && [ "${_tmppass}" != "()" ]; then
	newpass=$(printf "%s" "${_tmppass}" | openssl dgst -binary -sha1 | awk '{printf $NF}' | base64)
    su teamspeak -c "./ts3server_startscript.sh stop"
    echo "UPDATE clients set client_login_password='${newpass}' where client_id=1;" | sqlite3 ts3server.sqlitedb
    su teamspeak -c "./ts3server_startscript.sh start"
	password="${_tmppass}"
fi

cat > /root/ts3_login_data << EOF
login=${login}
password=${password}
token=${token}
EOF
touch /root/ts3_login_data
chmod 600 /root/ts3_login_data

crontab -u teamspeak -l > /tmp/ts.crontab
echo "@reboot      cd /home/teamspeak/ts3 ; ./ts3server_startscript.sh start" >> /tmp/ts.crontab
crontab -u teamspeak /tmp/ts.crontab
rm -f /tmp/ts.crontab


if [ -d /var/www/html ]; then
    cd /var/www/html || exit
else
    cd /var/www || exit
fi
wget --no-check-certificate http://download.ispsystem.com/external/ts3cp.tar.gz -O ts3cp.tar.gz
tar --strip-components=1 -xpf ts3cp.tar.gz
rm -f ts3cp.tar.gz

mkdir -p templates_c icons
if [ "${OSNAME}" = "debian" ]; then
    chown -R www-data templates_c icons
else
    chown -R apache templates_c icons
fi

echo >> motd.txt
echo "You can find login data in file /root/ts3_login_data" > motd.txt

mv config.php config.orig.php
cat > config.php << EOF
<?php
/*
*Copyright (C) 2012-2013  Psychokiller
*
*This program is free software; you can redistribute it and/or modify it under the terms of 
*the GNU General Public License as published by the Free Software Foundation; either 
*version 3 of the License, or any later version.
*
*This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
*without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*See the GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses/>. 
*/
if(!defined("SECURECHECK")) {die(\$lang['error_file_alone']);} 
/*
REGARD!!
If you use the web interface, they must write the webserver ip in the query_ip_whitelist.txt.
After adding the ip, the server must be restarted!

Add more Server Ip's.
For Example
\$server[0]['alias']= "Lokaler Server1";
\$server[0]['ip']= "127.0.0.1";
\$server[0]['tport']= "10011";

\$server[1]['alias']= "Lokaler Server2";
\$server[1]['ip']= "127.0.0.2";
\$server[1]['tport']= "20022";
*/

\$server[0]['alias']= "Server #1";
\$server[0]['ip']= "127.0.0.1";
\$server[0]['tport']= 10011;
\$cfglang        =       "en";                   //Language German = de, English = en, Netherlandish=nl (by pd1evl), French = fr (by supra63200)
\$duration = "100";                              //Set the Limit for Clients show per Page on Client List
\$fastswitch=true;                               //If true you can switch the Server on the header
\$showicons="left";                              //Define the position where the icons on the Viewer will show left or right
\$style="bootstrap";                                     //Chose your design  set 'new' for the default design or the name of your own create design
\$msgsend_name="Control_panel";  //This Name will be show if you send a message to a Server
\$show_motd=true;                                // Set it to false to not show the message of the day window
\$show_version=true;                             // Set it to false to not show the Webinterface Version on the footer
?>
EOF

rm -f index.html
if [ "${OSNAME}" = "debian" ]; then
   Service apache2 enable
   Service apache2 restart
else
   Service httpd enable
   Service httpd restart
   if service firewalld status >/dev/null ; then
	   # http port
	   firewall-cmd --add-service=http --zone=public --permanent
	   # TS3 ports
	   firewall-cmd --add-port=30033/tcp --zone=public --permanent
	   firewall-cmd --add-port=10011/tcp --zone=public --permanent
	   firewall-cmd --add-port=9987/udp --zone=public --permanent
	   firewall-cmd --reload
   elif [ -f /sbin/iptables ]; then
	   # TS3 ports
	   iptables -A INPUT -p udp --dport 9987 -j ACCEPT
	   iptables -A INPUT -p tcp --dport 30033 -j ACCEPT
	   iptables -A INPUT -p tcp --dport 10011 -j ACCEPT
	   # httpd port
	   iptables -A INPUT -p tcp --dport 80 -j ACCEPT
	   service iptables save || :
   fi
fi
```


