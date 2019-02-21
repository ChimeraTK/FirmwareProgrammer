/*
 * File:   MtcaProgrammerBase.cpp
 * Author: pperek
 *
 * Created on 14 kwiecień 2015, 22:52
 */

#include <stdint.h>
#include <string>

#include "MtcaProgrammerBase.h"
#include <ChimeraTK/NumericAddress.h>

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessRaw &args)
    : mDevice(ChimeraTK::Device()),
      reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  mDevice.open(args.mDeviceName);

  std::string moduleName = ChimeraTK::numeric_address::BAR / args.mBar /
                           (args.mAddress) * (PROG_REG_AREA_SIZE);
  initRegisterAccessors(moduleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessMap &args)
    : mDevice(ChimeraTK::Device()),
      reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  // std::cout << "args.mDeviceName: " << args.mDeviceName << std::endl;
  // std::cout << "args.mMapFilePath: " << args.mMapFilePath << std::endl;

  std::string full_dev_map_name;
  if (args.mDeviceName.find("rebot") != std::string::npos)
    full_dev_map_name = args.mDeviceName + "," + args.mMapFilePath;
  else
    full_dev_map_name = args.mDeviceName + "=" + args.mMapFilePath;

  // std::cout << "full_dev_map_name: " << full_dev_map_name << std::endl;
  mDevice.open(full_dev_map_name); // e.g. "sdm://./pci:llrfutcs3=mymapfile.map"

  initRegisterAccessors(args.mModuleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessDmap &args)
    : mDevice(ChimeraTK::Device()),
      reg_area_write(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_area_read(ChimeraTK::OneDRegisterAccessor<int32_t>()),
      reg_spi_divider(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_write(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_bytes_to_read(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_control(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tck(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tms(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdi(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_tdo(ChimeraTK::ScalarRegisterAccessor<int32_t>()),
      reg_rev_switch(ChimeraTK::ScalarRegisterAccessor<int32_t>()) {
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath(args.mDmapFilePath);
  mDevice.open(args.mDeviceName);

  initRegisterAccessors(args.mModuleName);
}

MtcaProgrammerBase::~MtcaProgrammerBase() { mDevice.close(); }

void MtcaProgrammerBase::initRegisterAccessors(std::string registerPathName) {
  if (registerPathName == "**DEFAULT**") {
    if (mDevice.getRegisterCatalogue().hasRegister(PROG_DEFAULT_MODULE_NAME)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME;
    } else if (mDevice.getRegisterCatalogue().hasRegister(
                   PROG_DEFAULT_MODULE_NAME2)) {
      registerPathName = PROG_DEFAULT_MODULE_NAME2;
    } else {
      std::cout << "Neither " << PROG_DEFAULT_MODULE_NAME << " nor "
                << PROG_DEFAULT_MODULE_NAME2 << " register has "
                << "been found in the device. Please specify the correct name "
                   "of the AREA_BOOT register!"
                << std::endl;
      exit(1);
    }
    std::cout << "Auto-selected module name: " << registerPathName << std::endl;
  }

  reg_area_write.replace(mDevice.getOneDRegisterAccessor<int32_t>(
      registerPathName, AREA_WRITE_SIZE, AREA_WRITE,
      {ChimeraTK::AccessMode::raw}));
  reg_area_read.replace(mDevice.getOneDRegisterAccessor<int32_t>(
      registerPathName, AREA_READ_SIZE, AREA_READ,
      {ChimeraTK::AccessMode::raw}));
  reg_spi_divider.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_SPI_DIVIDER, {ChimeraTK::AccessMode::raw}));
  reg_bytes_to_write.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_BYTES_TO_WRITE, {ChimeraTK::AccessMode::raw}));
  reg_bytes_to_read.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_BYTES_TO_READ, {ChimeraTK::AccessMode::raw}));
  reg_control.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_CONTROL, {ChimeraTK::AccessMode::raw}));
  reg_tck.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_TCK, {ChimeraTK::AccessMode::raw}));
  reg_tms.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_TMS, {ChimeraTK::AccessMode::raw}));
  reg_tdi.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_TDI, {ChimeraTK::AccessMode::raw}));
  reg_tdo.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_TDO, {ChimeraTK::AccessMode::raw}));
  reg_rev_switch.replace(mDevice.getScalarRegisterAccessor<int32_t>(
      registerPathName, REG_REV_SWITCH, {ChimeraTK::AccessMode::raw}));
}
