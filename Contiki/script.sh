#!/bin/bash

usage() {
	echo "Usage $0 <opts>"
	echo "      -c to have complex scenario (default is base)"
	echo "      -A attack (DOS, RANK, WORMHOLE, FRAGMENT)"
	echo "      -M mitigation (DOS, RANK, WORMHOLE, FRAGMENT)"
	echo "      -S number of scenario (for RANK with opt c: [0,3])"
	exit -1;
}

clean() {
	cp ~/attacchi/Backup/rpl-icmp6.c ~/contiki-ng/os/net/routing/rpl-lite/
	cp ~/attacchi/Backup/client.c ~/contiki-ng/examples/rpl-udp/
	cp ~/attacchi/Backup/server.c ~/contiki-ng/examples/rpl-udp/
}

if [ $# == 0 ]; then
	echo "Use option -h to see usage"
	exit -1
fi

complex=0
scenario=0
attack=NULL
mitigation=NULL

while getopts ':chAS:M:' flag; do
	case "${flag}" in 
		'h') usage;;
		'c') complex=1;;
		'A') attack="${OPTARG^^}";;
		'M') mitigation="${OPTARG^^}"
			 attack=$mitigation;;
		'S') scenario="${OPTARG}";;
		*) echo "Invalid option"
		   usage;;
	esac
done

if [[ $scenario -lt 0 || $complex == 0 || $attack != "RANK" && $scenario -gt 2 ]]; then 
	echo "Invalid option"
	echo "The scenario must be:"
	echo "	- for rank attack: between [0,2] and the -c option must be present, where:"
	echo "		1) scenario 0 is equal to "
	echo "		2) scenario 1 is equal to "
	echo "		3) scenario 2 is equal to "
	exit -1;	
fi

<<comm
clean

if [ $complex == 1 ]; then
	cd ~/attacchi/Complex/
else
	cd ~/attacchi/
fi

if [ $attack == "DOS" ]; then
	cp DOS/attacco/client.c ~/contiki-ng/examples/rpl-udp
	cp DOS/Simulazione.csc ~/contiki-ng/examples/rpl-udp

elif [ $attack == "RANK" ]; then
	cp Rank/client.c ~/contiki-ng/examples/rpl-udp
	cp Rank/rpl-neighbor.c ~/contiki-ng/os/net/routing/rpl-lite

	cd Rank/scenario_$scenario/
	cp attacco/rpl-icmp6.c ~/contiki-ng/os/net/routing/rpl-lite
	cp Simulazione.csc ~/contiki-ng/examples/rpl-udp
elif [ $attack == "FRAGMENT" ]; then
	echo "[REDACTED]"
	# FILL
elif [ $attack == "WARMHOLE" ]; then
	echo "[REDACTED]"
	# FILL
fi

if [ $mitigation == "DOS" ]; then
	cp DOS/mitigazione/server.c ~/contiki-ng/examples/rpl-udp
elif [ $mitigation == "RANK" ]; then
	cp mitigazione/rpl-icmp6.c ~/contiki-ng/os/net/routing/rpl-lite
elif [ $mitigation == "FRAGMENT" ]; then
	echo "[REDACTED]"
elif [ $mitigation == "WARMHOLE" ]; then
	echo "[REDACTED]"
fi
comm

#docker run --privileged --sysctl net.ipv6.conf.all.disable_ipv6=0 --mount type=bind,source=$CNG_PATH,destination=/home/user/contiki-ng -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev/bus/usb:/dev/bus/usb -ti contiker/contiki-ng cooja
