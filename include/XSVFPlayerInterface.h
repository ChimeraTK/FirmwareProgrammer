/* 
 * File:   XSVFPlayerInterface.h
 * Author: pperek
 *
 * Created on 24 kwiecień 2015, 10:11
 */

#ifndef XSVFPLAYERINTERFACE_H
#define	XSVFPLAYERINTERFACE_H

class XSVFPlayerInterface {
public:
    virtual ~XSVFPlayerInterface(){};
    virtual void readByte(unsigned char *data) = 0;
    virtual void setPort(short p, short val) = 0;
    virtual unsigned char readTDOBit() = 0;
    virtual void pulseClock() = 0;
    virtual void waitTime(long microsec) = 0;
private:

};

#endif	/* XSVFPLAYERINTERFACE_H */

