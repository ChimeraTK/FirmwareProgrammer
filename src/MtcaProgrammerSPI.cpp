/* 
 * File:   MtcaProgrammerSPI.cpp
 * Author: pperek
 * 
 * Created on 14 kwiecień 2015, 23:16
 */

#include <stdlib.h>
#include <stdio.h>

#include "MtcaProgrammerSPI.h"

#include "MtcaProgrammerBase.h"
#include "progress_bar.h"
#include "registers.h"

/*************************************************************************************/
/* Identifiers of known PROM memories mounted on boards supported by the programmer  */
/* If you want to add new supported memory, modify this map                          */
/*************************************************************************************/
const std::map<uint64_t, addressing_mode_t> MtcaProgrammerSPI::known_proms = {
    {0x0103182001, PROM_ADDR_24B},        //old uTC versions
    {0x014d190201, PROM_ADDR_32B},        //TCK7
    {0x00001740EF, PROM_ADDR_24B},        //SIS8300L
};
/*************************************************************************************/

const uint8_t MtcaProgrammerSPI::bit_pattern[14] = {0x00, 0x09, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x00, 0x00, 0x01, 0x61};

const std::map<std::string, std::pair<uint32_t, uint32_t> > commands= {
    //Name              24b     32b
    {"PAGE_PROGRAM",    {0x02,  0x12}},
    {"FAST_READ",       {0x0B,  0x0C}},
    {"WRITE_DISABLE",   {0x04,  0x04}},
};

inline uint32_t getCommand(std::string command_name, addressing_mode_t addr_mode)
{
    std::pair<uint32_t, uint32_t> value = commands.at(command_name);
    if(addr_mode == PROM_ADDR_24B)
        return value.first;
    else
        return value.second;
}

///////////////////////////////////////////////////////////////////////////////
MtcaProgrammerSPI::MtcaProgrammerSPI(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) 
    : MtcaProgrammerBase(dev, base_address, bar){
}

MtcaProgrammerSPI::~MtcaProgrammerSPI() {
}

///////////////////////////////////////////////////////////////////////////////
//  PUBLIC                                                                   //
///////////////////////////////////////////////////////////////////////////////
bool MtcaProgrammerSPI::checkFirmwareFile(std::string firmwareFile)
{
    FILE *input_file;
    unsigned char buffer[16];
    int i;
    bool ret = true;

    input_file = fopen(firmwareFile.c_str(), "r");
    if(input_file == NULL)
    {
            throw "Cannot open bit file. Maybe the file does not exist\n";
    }
    else
    {
            fread(buffer, 1, 14, input_file);

            for(i = 0; i < 14; i++)
            {
                    if(buffer[i] != bit_pattern[i])
                    {
                        ret = false; //Incorrect bit file
                        break;   
                    }
            }

            fclose(input_file);
    }
    
    return ret;
}

void MtcaProgrammerSPI::erase()
{
    mDevPtr->writeReg(regAddress(REG_SPI_DIVIDER), 10, mProgBar);
    
    uint64_t memID = getMemoryId();
    if(!checkMemoryId(memID))
        throw "Unknown, not present or busy SPI prom 1\n\n";
    
    memoryWriteEnable();
    memoryBulkErase();
}

void MtcaProgrammerSPI::program(std::string firmwareFile)
{
    mDevPtr->writeReg(regAddress(REG_SPI_DIVIDER), 10, mProgBar);
    
    uint64_t memID = getMemoryId();
    if(!checkMemoryId(memID))
        throw "Unknown, not present or busy SPI prom 1\n\n";
    
    memoryWriteEnable();
    programMemory(firmwareFile);
}

bool MtcaProgrammerSPI::verify(std::string firmwareFile)
{
    mDevPtr->writeReg(regAddress(REG_SPI_DIVIDER), 10, mProgBar);
    
    uint64_t memID = getMemoryId();
    if(!checkMemoryId(memID))
        throw "Unknown, not present or busy SPI prom 1\n\n";
    
    unsigned char buffer[1024];
    int32_t data;
    unsigned int bread;
    unsigned int addr = 0;
    unsigned int file_size;
    FILE *f = fopen (firmwareFile.c_str(), "rb");

    printf ("\nVerification\n");

    if (!f)
        throw "cannot open input file";

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint64_t mem_id = getMemoryId();
    addressing_mode_t addr_mode = known_proms.at(mem_id);
    
    do
    {
        bread = fread (buffer, 1, 1024, f);
        if (bread == 0)
        {
            printf ("\nverified %d bytes\n\n", addr);
            if (ferror (f))
            {
                fclose(f);
                throw ("Error reading input file");
            }
            fclose (f);
            return true;
        }

        mDevPtr->writeReg(regAddress (AREA_WRITE), getCommand("FAST_READ", addr_mode) /*0x12*/, mProgBar);
        uint32_t reg_offset = writeAddress(addr, addr_mode);
        mDevPtr->writeReg(regAddress(REG_BYTES_TO_WRITE), reg_offset, mProgBar); // 5 bytes + dummy

        //  wait for operation end
        mDevPtr->writeReg(regAddress(REG_BYTES_TO_READ), bread-1, mProgBar);
        mDevPtr->writeReg(regAddress(REG_CONTROL), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START), mProgBar);
        waitForSpi();
		
        for (unsigned int i = 0; i < 1024; i++)
        {
            mDevPtr->readReg(regAddress(AREA_READ) + 4 * i, &data, mProgBar);
            if(buffer[i] != (data & 0xff))
            {
                fprintf(stderr,"\nVerify error at address %u: read 0x%02x instead of 0x%02x\n",
                      addr+i, (data & 0xff), buffer[i]);
                return false;
            }
        }
        addr = addr + bread;
        ProgressBar(file_size, addr);
    }
    while (1);
}

///////////////////////////////////////////////////////////////////////////////
//  PRIVATE                                                                   //
///////////////////////////////////////////////////////////////////////////////
uint64_t MtcaProgrammerSPI::getMemoryId()
{
    uint64_t mem_id = 0;
    
    for(int i = 0; i < 2; i++)   // first read returns garbage
    {
        mDevPtr->writeReg(regAddress(AREA_WRITE), 0x9f, mProgBar);
        mDevPtr->writeReg(regAddress(REG_BYTES_TO_WRITE), 0x00, mProgBar);
        mDevPtr->writeReg(regAddress(REG_BYTES_TO_READ), 0x04, mProgBar);
        mDevPtr->writeReg(regAddress(REG_CONTROL), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START), mProgBar);
        waitForSpi();
    }
    
    // read 5 bytes
    for(int i = 0; i < 5; i++)
    {
        int32_t id_val;
        mDevPtr->readReg(regAddress(AREA_READ) + 4 * i, &id_val, mProgBar);
        mem_id |= ((uint64_t)id_val & 0xFF) << (i * 8);
    }
    
    printf("SPI prom ID: 0x%010lX\n\n", mem_id);
    
    return mem_id;
}

/** Check if the SPI flash is present. Return 1 on success, 0 on failure.
   Please note that it may return failure if the PROM is busy e.g. erasing the memory block. */
int MtcaProgrammerSPI::checkMemoryId(uint64_t memory_id)
{
    if(known_proms.find(memory_id) == known_proms.end())
        return 0;
    
    return 1;
}

/** wait until SPI data is transmitted to the device and the response is received from the device */
void MtcaProgrammerSPI::waitForSpi()
{
    int32_t data;
    unsigned int i=0;
    do
    {
        mDevPtr->readReg(regAddress(REG_CONTROL), &data, mProgBar);
        usleep(1);
        if(i++ == 10000) 
            throw("Timeout waiting for SPI completion\nTry rescanning PCIe bus\n");
    }
    while (data & 1);
}

uint32_t MtcaProgrammerSPI::writeAddress(uint32_t address, addressing_mode_t addr_mode)
{
    uint32_t addr_len = 0;
    
    if(addr_mode == PROM_ADDR_24B) addr_len = 3;
    else if(addr_mode == PROM_ADDR_32B) addr_len = 4;
    
    for(uint i = addr_len; i > 0; i--)
    {
        mDevPtr->writeReg(regAddress (AREA_WRITE) + 4 * i, address & 0xff, mProgBar);
        address = address >> 8;
    }
    
    return addr_len + 1;
}

void MtcaProgrammerSPI::memoryWriteEnable()
{
    mDevPtr->writeReg(regAddress(AREA_WRITE), 0x06, mProgBar);
    mDevPtr->writeReg(regAddress(REG_BYTES_TO_WRITE), 0x0, mProgBar);
    mDevPtr->writeReg(regAddress(REG_CONTROL), (PCIE_V5 | SPI_PROG  | SPI_START), mProgBar);
    waitForSpi();
}

void MtcaProgrammerSPI::memoryBulkErase()
{
    const int max_time = 65;
    int32_t data;
    double progress = 0;

    printf ("\nBulk erase\n");
    mDevPtr->writeReg(regAddress(AREA_WRITE), 0xc7, mProgBar);
    mDevPtr->writeReg(regAddress (REG_CONTROL),(PCIE_V5 | SPI_PROG | SPI_START), mProgBar); 
    waitForSpi();

    do
    {
        data = readStatus();
        if(progress <= max_time)
        {
            ProgressBar(max_time, progress++);
            sleep(1);
        }
    }
    while (data & 1);
    ProgressBar(max_time, max_time);      //progress bar = 100%
    printf("\n");
}

/** read status register from the SPI flash */
int32_t MtcaProgrammerSPI::readStatus()
{
    int32_t data;
    mDevPtr->writeReg(regAddress (REG_BYTES_TO_READ), 0x0, mProgBar);
    mDevPtr->writeReg(regAddress (AREA_WRITE), 0x05, mProgBar);
    mDevPtr->writeReg(regAddress (REG_CONTROL), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START), mProgBar);
    waitForSpi();
    mDevPtr->readReg(regAddress (AREA_READ), &data, mProgBar);

    return data;
}

void MtcaProgrammerSPI::programMemory(std::string firmwareFile)
{
    char buffer[256];
    unsigned int bread;
    unsigned int addr = 0;
    unsigned int file_size;
    FILE *f = fopen (firmwareFile.c_str(), "rb");

    printf ("\nProgramming\n");

    if (!f)
        throw "cannot open input file";

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint64_t mem_id = getMemoryId();
    addressing_mode_t addr_mode = known_proms.at(mem_id);
    
    do
    {
        bread = fread (buffer, 1, 256, f);
        if (bread == 0)
        {
            printf ("\nprogrammed %d bytes\n", addr);
            if (ferror (f))
                throw ("Error reading input file");
            fclose (f);
            return;
        }

        memoryWriteEnable();
        programMemoryPage(addr, bread, buffer, addr_mode);
        addr = addr + bread;
        ProgressBar(file_size, addr);
    }
    while (1);
}

void MtcaProgrammerSPI::programMemoryPage(unsigned int address, unsigned int size, char *buffer, addressing_mode_t addr_mode)
{
    unsigned int data;

//    printf("Programming flash at address %x\n",address);
    mDevPtr->writeReg(regAddress (AREA_WRITE), getCommand("PAGE_PROGRAM", addr_mode) /*0x12*/, mProgBar);
    uint32_t reg_offset = writeAddress(address, addr_mode);
    for (unsigned int i = 0; i < size; i++)
    {
        mDevPtr->writeReg(regAddress (AREA_WRITE) + (reg_offset + i) * 4, buffer[i], mProgBar);
        //printf("Data: 0x%x %d\n", buffer[i], i );
    }

    mDevPtr->writeReg(regAddress(REG_BYTES_TO_WRITE), size + reg_offset - 1, mProgBar);
    mDevPtr->writeReg(regAddress(REG_CONTROL), (PCIE_V5 | SPI_PROG | SPI_START), mProgBar);
    waitForSpi();
    do
    {
        data = readStatus();
    }
    while (data & 1);
}
