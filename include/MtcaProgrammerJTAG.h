// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MtcaProgrammerBase.h"
#include "XSVFPlayer.h"
#include "XSVFPlayerInterface.h"

class MtcaProgrammerJTAG : public MtcaProgrammerBase, XSVFPlayerInterface {
 public:
  MtcaProgrammerJTAG(const ProgAccessRaw& args);
  MtcaProgrammerJTAG(const ProgAccessMap& args);
  MtcaProgrammerJTAG(const ProgAccessDmap& args);
  virtual ~MtcaProgrammerJTAG();

  bool checkFirmwareFile(std::string firmwareFile);
  void erase();
  void program(std::string firmwareFile);
  bool verify(std::string firmwareFile);
  bool dump(std::string firmwareFile, uint32_t imageSize);
  void rebootFPGA();

  // XSVFPlayerInterface
  void setPort(jtag_port_t p, short val);
  unsigned char readTDOBit();
  void pulseClock();
  void waitTime(long microsec);

 private:
  static const uint8_t xsvf_pattern[16];
};
