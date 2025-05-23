// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "registers.h"

#include <ChimeraTK/Device.h>

struct ProgAccessRaw {
  ProgAccessRaw(std::string deviceName, uint8_t bar = PROG_DEFAULT_BAR, uint32_t address = PROG_DEFAULT_ADDRESS)
  : mDeviceName(deviceName), mBar(bar), mAddress(address) {};

  std::string mDeviceName;
  uint8_t mBar;
  uint32_t mAddress;
};

struct ProgAccessMap {
  ProgAccessMap(std::string deviceName, std::string moduleName = "**DEFAULT**")
  : mDeviceName(deviceName), mModuleName(moduleName) {};

  std::string mDeviceName;
  std::string mModuleName;
};

struct ProgAccessDmap {
  ProgAccessDmap(std::string deviceName, std::string dmapFilePath, std::string moduleName = "**DEFAULT**")
  : mDeviceName(deviceName), mDmapFilePath(dmapFilePath), mModuleName(moduleName) {};

  std::string mDeviceName;
  std::string mDmapFilePath;
  std::string mModuleName;
};

class MtcaProgrammerBase {
 public:
  MtcaProgrammerBase(const ProgAccessRaw& args);
  MtcaProgrammerBase(const ProgAccessMap& args);
  MtcaProgrammerBase(const ProgAccessDmap& args);

  virtual ~MtcaProgrammerBase();

  virtual bool checkFirmwareFile(std::string firmwareFile) = 0;
  virtual void erase() = 0;
  virtual void program(std::string firmwareFile) = 0;
  virtual bool verify(std::string firmwareFile) = 0;
  virtual bool dump(std::string firmwareFile, uint32_t imageSize) = 0;
  virtual void rebootFPGA() = 0;

 protected:
  ChimeraTK::Device mDevice;

  ChimeraTK::OneDRegisterAccessor<int32_t> reg_area_write;
  ChimeraTK::OneDRegisterAccessor<int32_t> reg_area_read;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_spi_divider;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_bytes_to_write;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_bytes_to_read;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_control;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_tck;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_tms;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_tdi;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_tdo;
  ChimeraTK::ScalarRegisterAccessor<int32_t> reg_rev_switch;

 private:
  void initRegisterAccessors(std::string registerPathName);
};
