/* 
 * File:   MtcaProgrammerBase.cpp
 * Author: pperek
 * 
 * Created on 14 kwiecień 2015, 22:52
 */

#include "MtcaProgrammerBase.h"

MtcaProgrammerBase::MtcaProgrammerBase(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) {
    if(dev == NULL)
        throw "Device pointer is null";
    else if(!dev->isOpen())
        throw "Device is not opened";
        
    mDevPtr = dev;
    mProgBaseAddress = base_address;
    mProgBar = bar;
}

MtcaProgrammerBase::~MtcaProgrammerBase() {
}

int MtcaProgrammerBase::regAddress (int reg)
{
    return ii_c[reg - 1].addresses[0] + mProgBaseAddress;
}
