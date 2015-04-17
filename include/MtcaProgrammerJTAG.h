/* 
 * File:   MtcaProgrammerJTAG.h
 * Author: pperek
 *
 * Created on 15 kwiecień 2015, 12:00
 */

#ifndef MTCAPROGRAMMERJTAG_H
#define	MTCAPROGRAMMERJTAG_H

#include "MtcaProgrammerBase.h"
#include "micro.h"

class MtcaProgrammerJTAG : public MtcaProgrammerBase {
public:
    MtcaProgrammerJTAG(mtcaDevPtr dev, uint32_t base_address, uint8_t bar);
    virtual ~MtcaProgrammerJTAG();
    
    bool checkFirmwareFile(std::string firmwareFile);
    void initialize();
    void erase();
    void program(std::string firmwareFile);
    bool verify(std::string firmwareFile);
    
private:
    static const uint8_t xsvf_pattern[16];
    SXsvfInfo sxvfInfo;
};

#endif	/* MTCAPROGRAMMERJTAG_H */

