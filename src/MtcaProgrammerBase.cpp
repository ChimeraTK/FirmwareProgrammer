/* 
 * File:   MtcaProgrammerBase.cpp
 * Author: pperek
 * 
 * Created on 14 kwiecień 2015, 22:52
 */

#include <stdint.h>
#include <string>

#include "MtcaProgrammerBase.h"
#include <mtca4u/NumericAddress.h>

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessRaw & args) :
    mDevice(mtca4u::Device()),
    reg_area_write(mtca4u::OneDRegisterAccessor<uint32_t>()),
    reg_area_read(mtca4u::OneDRegisterAccessor<uint32_t>()),    
    reg_spi_divider(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_write(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_read(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_control(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tck(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tms(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdi(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdo(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_rev_switch(mtca4u::ScalarRegisterAccessor<uint32_t>())
{
    mDevice.open(args.mDeviceName);
    
    std::string moduleName = mtca4u::numeric_address::BAR/args.mBar/(args.mAddress)*(PROG_REG_AREA_SIZE);
    initRegisterAccessors(moduleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessMap & args) :
    mDevice(mtca4u::Device()),
    reg_area_write(mtca4u::OneDRegisterAccessor<uint32_t>()),
    reg_area_read(mtca4u::OneDRegisterAccessor<uint32_t>()),    
    reg_spi_divider(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_write(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_read(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_control(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tck(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tms(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdi(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdo(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_rev_switch(mtca4u::ScalarRegisterAccessor<uint32_t>())
{
    //std::cout << "args.mDeviceName: " << args.mDeviceName << std::endl;
    //std::cout << "args.mMapFilePath: " << args.mMapFilePath << std::endl;
    
    std::string full_dev_map_name;
    if(args.mDeviceName.find("rebot") != std::string::npos)
        full_dev_map_name = args.mDeviceName + "," + args.mMapFilePath;
    else
        full_dev_map_name = args.mDeviceName + "=" + args.mMapFilePath;

    //std::cout << "full_dev_map_name: " << full_dev_map_name << std::endl;
    mDevice.open(full_dev_map_name);     // e.g. "sdm://./pci:llrfutcs3=mymapfile.map"
    
    initRegisterAccessors(args.mModuleName);
}

MtcaProgrammerBase::MtcaProgrammerBase(const ProgAccessDmap & args) :
    mDevice(mtca4u::Device()),
    reg_area_write(mtca4u::OneDRegisterAccessor<uint32_t>()),
    reg_area_read(mtca4u::OneDRegisterAccessor<uint32_t>()),    
    reg_spi_divider(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_write(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_bytes_to_read(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_control(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tck(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tms(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdi(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_tdo(mtca4u::ScalarRegisterAccessor<uint32_t>()),
    reg_rev_switch(mtca4u::ScalarRegisterAccessor<uint32_t>())
{
    mtca4u::BackendFactory::getInstance().setDMapFilePath(args.mDmapFilePath);
    mDevice.open(args.mDeviceName);
    
    initRegisterAccessors(args.mModuleName);    
}

MtcaProgrammerBase::~MtcaProgrammerBase() 
{
    mDevice.close();
}

void MtcaProgrammerBase::initRegisterAccessors(const std::string& registerPathName)
{
    reg_area_write.replace(mDevice.getOneDRegisterAccessor<uint32_t>(registerPathName, AREA_WRITE_SIZE, AREA_WRITE));
    reg_area_read.replace(mDevice.getOneDRegisterAccessor<uint32_t>(registerPathName, AREA_READ_SIZE, AREA_READ));
    reg_spi_divider.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_SPI_DIVIDER));
    reg_bytes_to_write.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_BYTES_TO_WRITE));
    reg_bytes_to_read.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_BYTES_TO_READ));
    reg_control.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_CONTROL));
    reg_tck.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_TCK));
    reg_tms.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_TMS));
    reg_tdi.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_TDI));
    reg_tdo.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_TDO));
    reg_rev_switch.replace(mDevice.getScalarRegisterAccessor<uint32_t>(registerPathName, REG_REV_SWITCH));
}
