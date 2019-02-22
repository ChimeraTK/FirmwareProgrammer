/*
 * File:   XSVFPlayerInterface.h
 * Author: pperek
 *
 * Created on 24 kwiecień 2015, 10:11
 */

#ifndef XSVFPLAYERINTERFACE_H
#define XSVFPLAYERINTERFACE_H

/* these constants are used to send the appropriate ports to setPort */
enum jtag_port_t { TCK, TMS, TDI };

class XSVFPlayerInterface {
 public:
  virtual ~XSVFPlayerInterface(){};
  virtual void setPort(jtag_port_t p, short val) = 0;
  virtual unsigned char readTDOBit() = 0;
  virtual void pulseClock() = 0;
  virtual void waitTime(long microsec) = 0;

 private:
};

#endif /* XSVFPLAYERINTERFACE_H */
