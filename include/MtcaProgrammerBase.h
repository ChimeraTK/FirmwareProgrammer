/* 
 * File:   MtcaProgrammerBase.h
 * Author: pperek
 *
 * Created on 14 kwiecień 2015, 22:52
 */

#ifndef MTCAPROGRAMMERBASE_H
#define	MTCAPROGRAMMERBASE_H

#include <mtca4u/MtcaMappedDevice/devBase.h>
#include <mtca4u/MtcaMappedDevice/devPCIE.h>

#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<mtca4u::devBase> mtcaDevPtr;

class MtcaProgrammerBase {
public:
    MtcaProgrammerBase(mtcaDevPtr dev, uint32_t base_address, uint8_t bar);
    virtual ~MtcaProgrammerBase();
    
    virtual bool checkFirmwareFile(std::string firmwareFile) = 0;
    virtual void erase() = 0;
    virtual void program(std::string firmwareFile) = 0;
    virtual bool verify(std::string firmwareFile) = 0;
    
protected:
    mtcaDevPtr mDevPtr;
    uint32_t mProgBaseAddress;
    uint8_t mProgBar;
    int regAddress (int reg);
};

#endif	/* MTCAPROGRAMMERBASE_H */

