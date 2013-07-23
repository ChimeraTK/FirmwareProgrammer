#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <arpa/inet.h>

#include "II_interface.h"

#include "libdev.h"

//#define OFFSET 65536

devAccess_Struct dev;

/* Set up the PCI Express device. Throw exception on failure. */
int
SetUpDevice (const char *device)
{
if (dev.openDev(device) != STATUS_OK){
    throw (dev.getLastErrorString().c_str());
}
return 1;
}


/* Write 32-bit data to the device in the global variable dev  at the specified address.
   Throw exception on failure */
void
writeRegister (int fd, unsigned int address, unsigned int data)
{

   if(dev.writeReg(address+OFFSET, data, BAR_NR)!=STATUS_OK)
     throw("Write error");  
}


/* Read 32-bit data from the device in the global variable dev from the specified address.
   Throw exception on failure */
void
readRegister (int fd, unsigned int address, unsigned int *data)
{
   int32_t data2;
   if(dev.readReg(address+OFFSET, &data2, BAR_NR)!=STATUS_OK)
     throw("Read error");
//     printf ("Data: %x, \n", data2);
   *data = data2;
}
