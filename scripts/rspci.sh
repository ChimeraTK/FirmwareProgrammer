#!/bin/bash
# By Olek

usage()
{
cat << EOF

usage: $0 [options] slot_number

This script allows for add/remove/rescan operation for PCIe devices.

OPTIONS:
    -h      Show this message
    -u      Only bring device up
    -d      Only bring device down

EOF
}

do_down=yes
do_up=yes

set -- $(getopt hud "$@")
while [ $# -gt 0 ]
do
    case "$1" in
    (-u) do_down=no;;
    (-d) do_up=no;;
    (-h) usage; exit 0;;
    (--) shift; break;;
    (-*) echo "$0: error - unrecognized option $1" 1>&2; exit 1;;
    (*)  break;;
    esac
    shift
done

if [[ -z $1 ]]; then
    echo "Slot number missing"
    usage
    exit 2
fi

if [[ $do_down == yes ]]; then
    echo "Slot $1 power down"
    sudo sh -c "echo 0 > /sys/bus/pci/slots/$1/power" 2> /dev/null
    [ $? -ne 0 ] && echo -e "\e[00;31mOperation failed\e[00m"
fi

if [[ $do_up == yes ]]; then
    echo "Slot $1 power up"
    sudo sh -c "echo 1 > /sys/bus/pci/slots/$1/power" 2> /dev/null
    [ $? -ne 0 ] && echo -e "\e[00;31mOperation failed\e[00m"
fi

echo "Done"

