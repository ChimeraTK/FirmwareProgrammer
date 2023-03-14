// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MtcaProgrammerBase.h"

#include <map>

typedef enum { PROM_ADDR_24B, PROM_ADDR_32B } addressing_mode_t;

typedef enum { QUAD_MODE_EN, QUAD_MODE_DIS } quad_mode_t;

typedef struct {
  addressing_mode_t addressing_mode;
  quad_mode_t quad_mode;
} memory_info_t;

class MtcaProgrammerSPI : public MtcaProgrammerBase {
 public:
  MtcaProgrammerSPI(const ProgAccessRaw& args);
  MtcaProgrammerSPI(const ProgAccessMap& args);
  MtcaProgrammerSPI(const ProgAccessDmap& args);
  virtual ~MtcaProgrammerSPI();

  bool checkFirmwareFile(std::string firmwareFile);
  void erase();
  void program(std::string firmwareFile);
  bool verify(std::string firmwareFile);
  bool dump(std::string firmwareFile, uint32_t imageSize);
  void rebootFPGA();

 private:
  static const uint8_t bit_pattern[14];
  static const std::map<uint64_t, memory_info_t> known_proms;
  uint64_t getMemoryId();
  int checkMemoryId(uint64_t memory_id);
  void waitForSpi();
  uint32_t readStatus();
  void enableQuadMode();
  uint32_t writeAddress(uint32_t address, addressing_mode_t addr_mode);
  void memoryWriteEnable();
  void memoryBulkErase();
  void programMemory(std::string firmwareFile);
  void programMemoryPage(unsigned int address, unsigned int size, unsigned char* buffer, addressing_mode_t addr_mode);
  long int findDataOffset(FILE* file);
};
