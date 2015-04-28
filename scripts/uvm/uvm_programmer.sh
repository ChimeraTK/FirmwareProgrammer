#!/bin/bash 

# Parameters
# #1 number of the slot with TCK7: from 3 to 12
# #2 revision from which FPGA will be booted, from 1 to 3
# #3 file with firmware: firmware.bit

[ "$UID" -eq 0 ] || exec sudo bash "$0" "$@"

echo ""
echo "FPGA configuration memory programmer for uVM board"
echo "by Piotr Perek, Dariusz Makowski and Grzegorz Jablonski"
echo ""

PATH_TO_PROGRAMMER=../../obj
#MCH=` hostname | sed 's/cpu/mch/'`
#MCH=mskmchacc1
#MCH=131.169.132.160
MCH=10.1.3.222

function print_usage {
	echo "Usage:"
        echo "  $0 <Device e.g. /dev/llrfutcs3> <Number of the slot with TCK7: 1-12> <Firmware.xsvf>"         
        exit -1
}

DEVICE=$1
SLOT=$2
FIRMWARE=$3

if [ "$#" -ne 3 ]
then
        echo "Wrong number of arguments"
	print_usage
fi
        if [ "$SLOT" -lt 1 ] || [ "$SLOT" -gt 12 ]
        then
                echo 'Wrong slot number'
		print_usage
        fi

GEO_ADDR=$((0x70+2*$SLOT))

cmd_output=$(ipmitool -I lan -H $MCH -P "" -B 0 -T 0x82 -b 7 -t $GEO_ADDR raw 0x30 0x02 0x08 2>&1)
if [ $? -ne 0 ]
then
    echo "Cannot read current JTAG settings"
    echo "Ipmitool" $cmd_output
    exit 1
fi

old_jtag_sett=$cmd_output
new_jtag_sett=$(( $old_jtag_sett | 0x10))
#printf "0x%x\n" $old_jtag_sett
#printf "0x%x\n" $new_jtag_sett

# Switch JTAG to be controlled by the FPGA
cmd_output=$(ipmitool -I lan -H $MCH -P "" -B 0 -T 0x82 -b 7 -t $GEO_ADDR raw 0x30 0x03 0x08 $new_jtag_sett 2>&1)
if [ $? -ne 0 ]
then
    echo "Cannot configure JTAG for memory programming"
    echo "Ipmitool" $cmd_output
    exit 1
fi

$PATH_TO_PROGRAMMER/llrf_prog -d $DEVICE -i jtag -f $FIRMWARE

# Restore previous configuration of JTAG
cmd_output=$(ipmitool -I lan -H $MCH -P "" -B 0 -T 0x82 -b 7 -t $GEO_ADDR raw 0x30 0x03 0x08 $old_jtag_sett 2>&1)
if [ $? -ne 0 ]
then
    echo "Cannot restore previous JTAG settings"
    echo "Ipmitool" $cmd_output
    exit 1
fi
