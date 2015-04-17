/* 
 * File:   MtcaProgrammerSPI.h
 * Author: pperek
 *
 * Created on 14 kwiecień 2015, 23:16
 */

#ifndef MTCAPROGRAMMERSPI_H
#define	MTCAPROGRAMMERSPI_H

#include "MtcaProgrammerBase.h"

#include <map>
#include "spi_prom_commands.h"

//bits in FPGA control register
#define SPI_START		0x00000001	//start spi operation
#define SPI_R_NW		0x00000002	//SPI read/write (0 - write, 1 - read)
#define DSP_SPI_BOOT		0x00000008	//DSP_boot source (1 - boot from SPI)
#define SPI_PROG		0x00000010	//0 - DSP, 1 - SPI
#define PCIE_V5			0x00000020	//PCIE mux (0 - RTM, 1 - V5)
#define SPIREADER_RESET		0x08000000
#define DSP_RESET		0x80000000

class MtcaProgrammerSPI : public MtcaProgrammerBase{
public:
    MtcaProgrammerSPI(mtcaDevPtr dev, uint32_t base_address, uint8_t bar);
    virtual ~MtcaProgrammerSPI();
    
    bool checkFirmwareFile(std::string firmwareFile);
    void initialize();
    void erase();
    void program(std::string firmwareFile);
    bool verify(std::string firmwareFile);
    
private:
    static const uint8_t bit_pattern[14];
    static const std::map<uint64_t, addressing_mode_t> known_proms;
    uint64_t getMemoryId();
    int checkMemoryId(uint64_t memory_id);
    void waitForSpi();
    int32_t readStatus();
    uint32_t writeAddress(uint32_t address, addressing_mode_t addr_mode);
    void memoryWriteEnable();
    void memoryBulkErase();
    void programMemory(std::string firmwareFile);
    void programMemoryPage(unsigned int address, unsigned int size, char *buffer, addressing_mode_t addr_mode);
};

#endif	/* MTCAPROGRAMMERSPI_H */

