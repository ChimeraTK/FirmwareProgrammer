#ifndef __LIBDEV_MULTIFILE_H__
#define __LIBDEV_MULTIFILE_H__

#include "libdev_b.h"
#include <string>
#include <stdint.h>
  

class devAccess_Multifile
{
private:
    std::string         dev_name;
    int                 dev_id;
    uint16_t            status;
    uint16_t            last_error;
    std::string         last_error_str;
    char                err_buff[255];
public:
    devAccess_Multifile();
    virtual ~devAccess_Multifile(){};
             
    uint16_t openDev(const std::string &dev_name, int perm = O_RDWR);
    uint16_t closeDev();
    
    uint16_t readReg(uint32_t reg_offset, int32_t* data, uint8_t bar = 0);
    uint16_t writeReg(uint32_t reg_offset, int32_t data, uint8_t bar = 0);
    
    uint16_t readArea(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar = 0);
    uint16_t writeArea(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar = 0);
    
    uint16_t readDMA(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar = 0);
    uint16_t writeDMA(uint32_t reg_offset, int32_t* data, size_t &size, uint8_t bar = 0);
    
    std::string getLastErrorString()    {return last_error_str;}
    uint16_t    getLastError()          {return last_error;}
};

#endif /*__LIBDEV_MULTIFILE_H__*/