#ifndef __LIBDEV_H__
#define __LIBDEV_H__


#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define STATUS_OK       0
#define STATUS_CLOSED   1
#define OPEN_FAILURE    2
#define READ_FAILURE    3
#define WRITE_FAILURE   4
#define NOT_SUPPORTED   10

#define SET_LAST_ERROR(s, m)                                                    \
						do {				\
							last_error_str = m;     \
							last_error = s;		\
                                                        return last_error;      \
						} while (0);

#define SET_STATUS(s, m)                                                        \
						do{                             \
                                                        last_error = s;         \
                                                        last_error_str = m;     \
                                                        status = s;		\
                                                        return status;          \
						} while (0);

class devAccess
{
public:
    devAccess() {};
    ~devAccess() {};
             
    uint16_t openDev(const std::string &dev_name, int perm = O_RDWR);
    uint16_t closeDev();
    
    uint16_t readReg(uint32_t reg_offset, int32_t* data);
    uint16_t writeReg(uint32_t reg_offset, int32_t data);
    
    uint16_t readArea(uint32_t reg_offset, int32_t* data, size_t size);
    uint16_t writeArea(uint32_t reg_offset, int32_t* data, size_t size);
    
    uint16_t readDMA(uint32_t reg_offset, int32_t* data, size_t size);
    uint16_t writeDMA(uint32_t reg_offset, int32_t* data, size_t size);
    
    std::string getLastErrorString();
    uint16_t    getLastError();
};

#endif /*__LIBDEV_H__*/