/* 
 * File:   MtcaProgrammerBase.h
 * Author: pperek
 *
 * Created on 14 kwiecień 2015, 22:52
 */

#ifndef MTCAPROGRAMMERBASE_H
#define	MTCAPROGRAMMERBASE_H

#include <mtca4u/Device.h>

#include "registers.h"

struct ProgAccessRaw {
    ProgAccessRaw(std::string deviceName, uint8_t bar = PROG_DEFAULT_BAR, uint32_t address = PROG_DEFAULT_ADDRESS) 
                : mDeviceName(deviceName), mBar(bar), mAddress(address)
    {
    };
    
    std::string mDeviceName;
    uint8_t mBar;
    uint32_t mAddress;
};

struct ProgAccessMap {
    ProgAccessMap(std::string deviceName, std::string mapFilePath, std::string moduleName = PROG_DEFAULT_MODULE_NAME) 
                : mDeviceName(deviceName), mMapFilePath(mapFilePath), mModuleName(moduleName)
    {
    };
                
    std::string mDeviceName;
    std::string mMapFilePath;
    std::string mModuleName;
};

struct ProgAccessDmap {
    ProgAccessDmap(std::string deviceName, std::string dmapFilePath, std::string moduleName = PROG_DEFAULT_MODULE_NAME) 
                : mDeviceName(deviceName), mDmapFilePath(dmapFilePath), mModuleName(moduleName)
    {
    };
                
    std::string mDeviceName;
    std::string mDmapFilePath;
    std::string mModuleName;
};

class MtcaProgrammerBase {
public:
    MtcaProgrammerBase(const ProgAccessRaw & args);
    MtcaProgrammerBase(const ProgAccessMap & args);
    MtcaProgrammerBase(const ProgAccessDmap & args);
    
    virtual ~MtcaProgrammerBase();
    
    virtual bool checkFirmwareFile(std::string firmwareFile) = 0;
    virtual void erase() = 0;
    virtual void program(std::string firmwareFile) = 0;
    virtual bool verify(std::string firmwareFile) = 0;
    virtual void rebootFPGA() = 0;
    
protected:
    mtca4u::Device mDevice;
    
    mtca4u::OneDRegisterAccessor<uint32_t> reg_area_write;
    mtca4u::OneDRegisterAccessor<uint32_t> reg_area_read;    
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_spi_divider;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_bytes_to_write;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_bytes_to_read;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_control;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_tck;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_tms;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_tdi;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_tdo;
    mtca4u::ScalarRegisterAccessor<uint32_t> reg_rev_switch;
    
private:
    void initRegisterAccessors(const std::string &registerPathName);
};

#endif	/* MTCAPROGRAMMERBASE_H */

