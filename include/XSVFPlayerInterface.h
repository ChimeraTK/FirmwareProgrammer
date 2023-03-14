// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

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
