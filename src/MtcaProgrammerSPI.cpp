/* 
 * File:   MtcaProgrammerSPI.cpp
 * Author: pperek
 * 
 * Created on 14 kwiecień 2015, 23:16
 */

#include <stdlib.h>
#include <stdio.h>
#include <mtca4u/Device.h>
#include <mtca4u/RegisterPath.h>
#include <mtca4u/AccessMode.h>

#include "MtcaProgrammerSPI.h"

#include "MtcaProgrammerBase.h"
#include "progress_bar.h"
#include "registers.h"

/*************************************************************************************/
/* Identifiers of known PROM memories mounted on boards supported by the programmer  */
/* If you want to add new supported memory, modify this map                          */
/*************************************************************************************/
const std::map<uint64_t, memory_info_t> MtcaProgrammerSPI::known_proms = {
    {0x0103182001, {PROM_ADDR_24B, QUAD_MODE_DIS}   },        //old uTC versions
    {0x014d190201, {PROM_ADDR_32B, QUAD_MODE_EN}    },        //TCK7
    {0x00001740EF, {PROM_ADDR_24B, QUAD_MODE_DIS}   },        //SIS8300L
    {0x0010172020, {PROM_ADDR_24B, QUAD_MODE_EN}    }         //PiezoBox (M25P64)
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
MtcaProgrammerSPI::MtcaProgrammerSPI(const ProgAccessRaw& args) 
    : MtcaProgrammerBase(args)
{
}

MtcaProgrammerSPI::MtcaProgrammerSPI(const ProgAccessMap& args) 
    : MtcaProgrammerBase(args)
{
}

MtcaProgrammerSPI::MtcaProgrammerSPI(const ProgAccessDmap& args) 
    : MtcaProgrammerBase(args)
{
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
    bool bit_file = true;
    size_t bytes_read;

    input_file = fopen(firmwareFile.c_str(), "r");
    if(input_file == NULL)
    {
            throw std::invalid_argument("Cannot open bit file. Maybe the file does not exist");
    }
    else
    {
            // Check if it is a 'bit' file
            bytes_read = fread(buffer, 1, 14, input_file);
            if(bytes_read != 14)
            {
                fclose(input_file);
                throw std::runtime_error("Cannot read verification pattern from bitstream file");
            }
            
            for(i = 0; i < 14; i++)
            {
                    if(buffer[i] != bit_pattern[i])
                    {
                        bit_file = false; //Incorrect bit file
                        break;   
                    }
            }
    }
    
    if(!bit_file)
    {
        // Check if it is a 'bin' file
        long int offset = findDataOffset(input_file);
        if(offset != 0)
            ret = false;
    }
    
    fclose(input_file);
    
    return ret;
}

void MtcaProgrammerSPI::erase()
{
    reg_spi_divider = 10;
    reg_spi_divider.write();
    
    uint64_t memID = getMemoryId();
    if(!checkMemoryId(memID))
        throw std::runtime_error("Unknown, not present or busy SPI prom");
    
    memoryWriteEnable();
    memoryBulkErase();
}

void MtcaProgrammerSPI::program(std::string firmwareFile)
{
    reg_spi_divider = 10;
    reg_spi_divider.write();
    
    uint64_t memID = getMemoryId();    
    if(!checkMemoryId(memID))
        throw std::runtime_error("Unknown, not present or busy SPI prom");
    
    quad_mode_t quad_mode = known_proms.at(memID).quad_mode;
    
    memoryWriteEnable();
    if(quad_mode == QUAD_MODE_EN)
        enableQuadMode();
    programMemory(firmwareFile);
}

bool MtcaProgrammerSPI::verify(std::string firmwareFile)
{
    reg_spi_divider = 10;
    reg_spi_divider.write();
    
    uint64_t memID = getMemoryId();
    if(!checkMemoryId(memID))
        throw std::runtime_error("Unknown, not present or busy SPI prom");
    
    unsigned char buffer[1024];
    uint32_t data;
    unsigned int bread;
    unsigned int addr = 0;
    unsigned int file_size;
    long int offset = 0;
    FILE *f = fopen (firmwareFile.c_str(), "rb");

    printf ("\nVerification\n");

    if (!f)
        throw std::invalid_argument("Cannot open firmware file");

    offset = findDataOffset(f);
    if(offset == -1)
        throw std::runtime_error("Cannot find start sequence in the file");
    
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, offset, SEEK_SET);
    
    uint64_t mem_id = getMemoryId();
    addressing_mode_t addr_mode = known_proms.at(mem_id).addressing_mode;
    
    do
    {
        bread = fread (buffer, 1, 1024, f);
        if (bread == 0)
        {
            printf ("\nverified %d bytes\n\n", addr);
            if (ferror (f))
            {
                fclose(f);
                throw std::runtime_error("Error reading firmware file");
            }
            fclose (f);
            return true;
        }

        reg_area_write[0] = getCommand("FAST_READ", addr_mode);
        uint32_t reg_offset = writeAddress(addr, addr_mode);
        reg_area_write.write();
        reg_bytes_to_write = reg_offset;
        reg_bytes_to_write.write();
        
        reg_bytes_to_read = bread - 1;
        reg_bytes_to_read.write();

        reg_control = (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START);
        reg_control.write();
        waitForSpi();
		
        reg_area_read.read();
        for (unsigned int i = 0; i < bread; i++)
        {
            data = reg_area_read[i];
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

void MtcaProgrammerSPI::rebootFPGA()
{
    printf("FPGA rebooting...\n");
    reg_rev_switch = FPGA_REBOOT_WORD;
    reg_rev_switch.write();
}

///////////////////////////////////////////////////////////////////////////////
//  PRIVATE                                                                   //
///////////////////////////////////////////////////////////////////////////////
uint64_t MtcaProgrammerSPI::getMemoryId()
{
    uint64_t mem_id = 0;
    
    for(int i = 0; i < 2; i++)   // first read returns garbage
    {
        reg_area_write[0] = 0x9F;
        reg_area_write.write();
        reg_bytes_to_write = 0;
        reg_bytes_to_write.write();
        reg_bytes_to_read = 4;
        reg_bytes_to_read.write();
        
        reg_control = (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START);
        reg_control.write();
        
        waitForSpi();
    }
    
    // read 5 bytes
    reg_area_read.read();
    for(int i = 0; i < 5; i++)
    {
        uint32_t id_val;
        id_val = reg_area_read[i];
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
    uint32_t data;
    unsigned int i=0;
    do
    {
        reg_control.read();
        data = reg_control;
        //printf("waitForSpi() - data = 0x%X\n", data);
        usleep(1);
        if(i++ == 10000) 
            throw std::runtime_error("Timeout waiting for SPI response\n"
                                     "It can be a problem with communication interface (PCIe/Eth) or programmer module is not available in FPGA");
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
        reg_area_write[i] = address & 0xFF;
        address = address >> 8;
    }
    
    return addr_len + 1;
}

void MtcaProgrammerSPI::memoryWriteEnable()
{
    reg_area_write[0] = 0x06;
    reg_area_write.write();
    reg_bytes_to_write = 0;
    reg_bytes_to_write.write();
    
    reg_control = (PCIE_V5 | SPI_PROG  | SPI_START);
    reg_control.write();
    
    waitForSpi();
}

void MtcaProgrammerSPI::memoryBulkErase()
{
    const int max_time = 65;
    int32_t data;
    double progress = 0;

    printf ("\nBulk erase\n");
    reg_area_write[0] = 0xC7;
    reg_area_write.write();
    reg_control = (PCIE_V5 | SPI_PROG | SPI_START);
    reg_control.write();
    waitForSpi();

    do
    {
        data = readStatus();
        if(progress <= max_time)
        {
            ProgressBar(max_time, progress++);
            sleep(1);
        }
        ///printf("data = 0x%X\n", data);
    }
    while (data & 1);
    ProgressBar(max_time, max_time);      //progress bar = 100%
    printf("\n");
}

/** read status register from the SPI flash */
uint32_t MtcaProgrammerSPI::readStatus()
{
    uint32_t data;
    reg_area_write[0] = 0x05;
    reg_area_write.write();
    reg_bytes_to_read = 0x0;
    reg_bytes_to_read.write();
    
    reg_control = (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START);
    reg_control.write();
    waitForSpi();

    reg_area_read.read();
    data = reg_area_read[0];

    return data;
}

void MtcaProgrammerSPI::enableQuadMode()
{
    uint32_t data;

    reg_area_write[0] = 0x35;
    reg_area_write.write();
    reg_bytes_to_read = 0x0;
    reg_bytes_to_read.write();
    
    reg_control = (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START);
    reg_control.write();
    waitForSpi();
    
    reg_area_read.read();
    data = reg_area_read[0];
    data |= 0x02;
    
    reg_area_write[0] = 0x01;
    reg_area_write[1] = 0x00;
    reg_area_write[2] = data;
    reg_area_write.write();
    reg_bytes_to_write = 0x2;
    reg_bytes_to_write.write();
    
    reg_control = (PCIE_V5 | SPI_PROG | SPI_START);
    reg_control.write();
    
    waitForSpi();
}

/** Searching for the end of bitstream header. Counting the offset for program_mem and verify_mem functions
    Return int offset - number of bytes of header to ommit. Returns 0 if no header found.
    Returns -1 if no binary start sequence (0xAA995566) found. */
long int MtcaProgrammerSPI::findDataOffset(FILE *file)
{
    unsigned char buffer[512];
    int offset = -1;
    const unsigned char c1 = 0xaa;
    const unsigned char c2 = 0x99;
    const unsigned char c3 = 0x55;
    const unsigned char c4 = 0x66;
    const unsigned char c0 = 0xff;
    size_t bytes_read;

    if(!file)
        return -1;

    fseek(file, 0, SEEK_SET);
    bytes_read = fread(buffer, 1, 512, file);
    if(bytes_read != 512)
    {
            fclose(file);
            throw std::runtime_error("Cannot read header data from bitstream file");
    }
    fseek(file, 0, SEEK_SET);

    /* ok if header shorter than 512 bytes  */
    for(int i = 0; i < 512; i++)
    {
        if((buffer[i] == c1) && (buffer[i+1] == c2) && (buffer[i+2] == c3) && (buffer[i+3] == c4))
        {
            for(int j = 1; j < i + 1; j++)
            {
                if(buffer[i-j] != c0)
                {
                    i = i - (j - (j % 16));
                    break;
                }
                if(j == i) 
                    i = i - (j - (j % 16));
            }

            offset = i;
            break;
        }	
    }
    
    return offset;
}

void MtcaProgrammerSPI::programMemory(std::string firmwareFile)
{
    unsigned char buffer[256];
    unsigned int bread;
    unsigned int addr = 0;
    unsigned int file_size;
    long int offset = 0;
    FILE *f = fopen (firmwareFile.c_str(), "rb");

    printf ("\nProgramming\n");

    if (!f)
        throw std::invalid_argument("Cannot open firmware file");

    offset = findDataOffset(f);
    if(offset == -1)
        throw std::runtime_error("Cannot find start sequence in the file");
    
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, offset, SEEK_SET);

    uint64_t mem_id = getMemoryId();
    addressing_mode_t addr_mode = known_proms.at(mem_id).addressing_mode;
    
    do
    {
        bread = fread (buffer, 1, 256, f);
        if (bread == 0)
        {
            printf ("\nprogrammed %d bytes\n", addr);
            if (ferror (f))
                throw std::runtime_error("Error reading firmware file");
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

void MtcaProgrammerSPI::programMemoryPage(unsigned int address, unsigned int size, unsigned char *buffer, addressing_mode_t addr_mode)
{
    unsigned int data;

//    printf("Programming flash at address %x\n",address);
    reg_area_write[0] = getCommand("PAGE_PROGRAM", addr_mode);
    uint32_t reg_offset = writeAddress(address, addr_mode);
    for (unsigned int i = 0; i < size; i++)
    {
        reg_area_write[(reg_offset + i)] = buffer[i];
        //printf("Data: 0x%x %d\n", buffer[i], i );
    }
    reg_area_write.write();   
    reg_bytes_to_write = size + reg_offset - 1;
    reg_bytes_to_write.write();

    reg_control = (PCIE_V5 | SPI_PROG | SPI_START);
    reg_control.write();
    waitForSpi();
    do
    {
        data = readStatus();
    }
    while (data & 1);
}
