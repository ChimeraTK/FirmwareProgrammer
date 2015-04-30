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

typedef enum {
    PROM_ADDR_24B,
    PROM_ADDR_32B,
} addressing_mode_t;

class MtcaProgrammerSPI : public MtcaProgrammerBase{
public:
    MtcaProgrammerSPI(mtcaDevPtr dev, uint32_t base_address, uint8_t bar);
    virtual ~MtcaProgrammerSPI();
    
    bool checkFirmwareFile(std::string firmwareFile);
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
    void enableQuadMode();
    uint32_t writeAddress(uint32_t address, addressing_mode_t addr_mode);
    void memoryWriteEnable();
    void memoryBulkErase();
    void programMemory(std::string firmwareFile);
    void programMemoryPage(unsigned int address, unsigned int size, char *buffer, addressing_mode_t addr_mode);
};

#endif	/* MTCAPROGRAMMERSPI_H */

