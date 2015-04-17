/* 
 * File:   spi_prom_commands.h
 * Author: pperek
 *
 * Created on 15 kwiecień 2015, 21:12
 */

#ifndef SPI_PROM_COMMANDS_H
#define	SPI_PROM_COMMANDS_H

#include <map>

typedef enum {
    PROM_ADDR_24B,
    PROM_ADDR_32B,
} addressing_mode_t;

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
#endif	/* SPI_PROM_COMMANDS_H */

