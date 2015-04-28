/* 
 * File:   MtcaProgrammerBase.cpp
 * Author: pperek
 * 
 * Created on 14 kwiecień 2015, 22:52
 */

#include "MtcaProgrammerBase.h"

MtcaProgrammerBase::MtcaProgrammerBase(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) :
    mDevPtr(dev),
    mProgBaseAddress(base_address),
    mProgBar(bar)
{
    if(dev == NULL)
        throw "Device pointer is null";
    else if(!dev->isOpen())
        throw "Device is not opened";
}

MtcaProgrammerBase::~MtcaProgrammerBase() 
{
}

int MtcaProgrammerBase::regAddress (int reg)
{
    return mProgBaseAddress + reg;
}
