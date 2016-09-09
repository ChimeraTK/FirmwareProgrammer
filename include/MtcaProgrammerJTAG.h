/* 
 * File:   MtcaProgrammerJTAG.h
 * Author: pperek
 *
 * Created on 15 kwiecień 2015, 12:00
 */

#ifndef MTCAPROGRAMMERJTAG_H
#define	MTCAPROGRAMMERJTAG_H

#include "MtcaProgrammerBase.h"
#include "XSVFPlayer.h"
#include "XSVFPlayerInterface.h"

class MtcaProgrammerJTAG : public MtcaProgrammerBase, XSVFPlayerInterface {
public:
    MtcaProgrammerJTAG(const ProgAccessRaw & args);
    MtcaProgrammerJTAG(const ProgAccessMap & args);
    MtcaProgrammerJTAG(const ProgAccessDmap & args);
    virtual ~MtcaProgrammerJTAG();
    
    bool checkFirmwareFile(std::string firmwareFile);
    void erase();
    void program(std::string firmwareFile);
    bool verify(std::string firmwareFile);
    void rebootFPGA();
    
    // XSVFPlayerInterface
    void setPort(jtag_port_t p, short val);
    unsigned char readTDOBit();
    void pulseClock();
    void waitTime (long microsec);
    
private:
    static const uint8_t xsvf_pattern[16];
};

#endif	/* MTCAPROGRAMMERJTAG_H */

