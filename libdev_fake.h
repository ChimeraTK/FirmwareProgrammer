#ifndef LIBDEV_FAKE_H
#define	LIBDEV_FAKE_H

#include "libdev_b.h"
#include <stdint.h>
#include <stdlib.h>


class devAccess_Fake
{
private:
    int32_t             memory[1024*1024];
    uint16_t            status;
    uint16_t            last_error;
    std::string         last_error_str;
    char                err_buff[255];
public:
    devAccess_Fake();
             
    uint16_t openDev(const std::string &dev_name, int perm = O_RDWR);
    uint16_t closeDev();
    
    uint16_t readReg(uint32_t reg_offset, int32_t* data, uint8_t bar);
    uint16_t writeReg(uint32_t reg_offset, int32_t data, uint8_t bar);
    
    uint16_t readArea(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar);
    uint16_t writeArea(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar);
    
    uint16_t readDMA(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar);
    uint16_t writeDMA(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar);
    
    std::string getLastErrorString()    {return last_error_str;}
    uint16_t    getLastError()          {return last_error;}
};

#endif	/* LIBDEV_STRUCT_H */

