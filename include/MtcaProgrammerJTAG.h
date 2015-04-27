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
#include "XSVFPlayer.h"
#include "XSVFPlayerInterface.h"

/* these constants are used to send the appropriate ports to setPort */
/* they should be enumerated types, but some of the microcontroller  */
/* compilers don't like enumerated types */
#define TCK (short) 0
#define TMS (short) 1
#define TDI (short) 2

class MtcaProgrammerJTAG : public MtcaProgrammerBase, XSVFPlayerInterface {
public:
    MtcaProgrammerJTAG(mtcaDevPtr dev, uint32_t base_address, uint8_t bar);
    virtual ~MtcaProgrammerJTAG();
    
    bool checkFirmwareFile(std::string firmwareFile);
    void initialize();
    void erase();
    void program(std::string firmwareFile);
    bool verify(std::string firmwareFile);
    
    // XSVFPlayerInterface
    void readByte(unsigned char *data);
    void setPort(short p, short val);
    unsigned char readTDOBit();
    void pulseClock();
    void waitTime (long microsec);
    
private:
    static const uint8_t xsvf_pattern[16];
    XSVFPlayer *player;
};

#endif	/* MTCAPROGRAMMERJTAG_H */

