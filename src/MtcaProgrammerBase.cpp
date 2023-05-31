// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MtcaProgrammerBase.h"

#include <ChimeraTK/NumericAddress.h>

#include <stdint.h>
#include <string>

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessRaw& args)
: mDevice(ChimeraTK::Device()), reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  mDevice.open(args.mDeviceName);

  std::string moduleName = ChimeraTK::numeric_address::BAR() / args.mBar / (args.mAddress) * (PROG_REG_AREA_SIZE);
  initRegisterAccessors(moduleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessMap& args)
: mDevice(ChimeraTK::Device()), reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  mDevice.open(args.mDeviceName);

  initRegisterAccessors(args.mModuleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessDmap& args)
: mDevice(ChimeraTK::Device()), reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
  reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
  reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()), reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath(args.mDmapFilePath);
  mDevice.open(args.mDeviceName);

  initRegisterAccessors(args.mModuleName);
}

MtcaProgrammerBase::~MtcaProgrammerBase() {
  mDevice.close();
}

void MtcaProgrammerBase::initRegisterAccessors(std::string registerPathName) {
  if(registerPathName == "**DEFAULT**") {
    if(mDevice.getRegisterCatalogue().hasRegister(PROG_DEFAULT_MODULE_NAME)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME;
    }
    else if(mDevice.getRegisterCatalogue().hasRegister(PROG_DEFAULT_MODULE_NAME2)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME2;
    }
    else if(mDevice.getRegisterCatalogue().hasRegister(PROG_DEFAULT_MODULE_NAME3)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME3;
    }
    else if(mDevice.getRegisterCatalogue().hasRegister(PROG_DEFAULT_MODULE_NAME4)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME4;
    }
    else {
      std::cout << "Neither " << PROG_DEFAULT_MODULE_NAME << " nor " << PROG_DEFAULT_MODULE_NAME2 << " nor "
                << PROG_DEFAULT_MODULE_NAME3 << " nor " << PROG_DEFAULT_MODULE_NAME4 << " register has "
                << "been found in the device. Please specify the correct name "
                   "of the AREA_BOOT register!"
                << std::endl;
      exit(1);
    }
    std::cout << "Auto-selected module name: " << registerPathName << std::endl;
  }

  reg_area_write.replace(mDevice.getOneDRegisterAccessor<int32_t>(
      registerPathName, AREA_WRITE_SIZE, AREA_WRITE, {ChimeraTK::AccessMode::raw}));
  reg_area_read.replace(mDevice.getOneDRegisterAccessor<int32_t>(
      registerPathName, AREA_READ_SIZE, AREA_READ, {ChimeraTK::AccessMode::raw}));
  reg_spi_divider.replace(
      mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_SPI_DIVIDER, {ChimeraTK::AccessMode::raw}));
  reg_bytes_to_write.replace(
      mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_BYTES_TO_WRITE, {ChimeraTK::AccessMode::raw}));
  reg_bytes_to_read.replace(
      mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_BYTES_TO_READ, {ChimeraTK::AccessMode::raw}));
  reg_control.replace(
      mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_CONTROL, {ChimeraTK::AccessMode::raw}));
  reg_tck.replace(mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_TCK, {ChimeraTK::AccessMode::raw}));
  reg_tms.replace(mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_TMS, {ChimeraTK::AccessMode::raw}));
  reg_tdi.replace(mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_TDI, {ChimeraTK::AccessMode::raw}));
  reg_tdo.replace(mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_TDO, {ChimeraTK::AccessMode::raw}));
  reg_rev_switch.replace(
      mDevice.getScalarRegisterAccessor<int32_t>(registerPathName, REG_REV_SWITCH, {ChimeraTK::AccessMode::raw}));
}
