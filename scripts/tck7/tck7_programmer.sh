#!/bin/bash 

# Parameters
# #1 number of the slot with TCK7: from 3 to 12
# #2 revision from which FPGA will be booted, from 1 to 3
# #3 file with firmware: firmware.bit

[ "$UID" -eq 0 ] || exec sudo bash "$0" "$@"

echo ""
echo "FPGA configuration memory programmer for TCK7 board"
echo "by Piotr Perek, Dariusz Makowski and Grzegorz Jablonski"
echo ""

PATH_TO_PROGRAMMER=../../obj
#MCH=` hostname | sed 's/cpu/mch/'`
#MCH=mskmchacc1
MCH=10.1.3.222

function print_usage {
	echo "Usage:"
        echo "  $0 <Device e.g. /dev/llrfutcs3> <Number of the slot with TCK7: 1-12> <Memory bank with firmware: 0-1> <Firmware.bit>"         
        exit -1
}

DEVICE=$1
SLOT=$2
MEM_BANK=$3
FIRMWARE=$4

if [ "$#" -ne 4 ]
then
        echo "Wrong number of arguments"
	print_usage
fi
        if [ "$SLOT" -lt 1 ] || [ "$SLOT" -gt 12 ]
        then
                echo 'Wrong slot number'
		print_usage
        fi

        if [ "$MEM_BANK" -lt 0 ] || [ "$MEM_BANK" -gt 1 ]
        then
                echo "Wrong memory number"
		print_usage
        fi

echo "Switch to memory with firmware revision $MEM_BANK"

GEO_ADDR=$((0x70+2*$SLOT))

# Switch memory for programming
cmd_output=$(ipmitool -I lan -H $MCH -P "" -B 0 -T 0x82 -b 7 -t $GEO_ADDR raw 0x30 0x01 $MEM_BANK 2>&1)
if [ $? -ne 0 ]
then
    echo "Cannot switch memory"
    echo "Ipmitool" $cmd_output
    exit 1
fi

$PATH_TO_PROGRAMMER/mtca4u_fw_programmer -d $DEVICE -i spi -f $FIRMWARE
if [ $? -ne 0 ]
then
    exit 1
fi

../rspci.sh -d $SLOT
# reload FPGA
cmd_output=$(ipmitool -I lan -H $MCH -P "" -B 0 -T 0x82 -b 7 -t $GEO_ADDR raw 0x30 0x04 2>&1)
if [ $? -ne 0 ]
then
    echo "Cannot reload FPGA"
    echo "Ipmitool" $cmd_output
fi
sleep 10
../rspci.sh $SLOT



