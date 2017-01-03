/* 
 * File:   MtcaProgrammerJTAG.cpp
 * Author: pperek
 * 
 * Created on 15 kwiecień 2015, 12:00
 */

#include "MtcaProgrammerJTAG.h"

#include <stdio.h>
#include <stdlib.h>

#include "progress_bar.h"
#include "registers.h"

const uint8_t MtcaProgrammerJTAG::xsvf_pattern[16] = {0x07, 0x00, 0x13, 0x00, 0x14, 0x00, 0x12, 0x00, 0x12, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02};

MtcaProgrammerJTAG::MtcaProgrammerJTAG(const ProgAccessRaw& args) 
    : MtcaProgrammerBase(args)
{
}

MtcaProgrammerJTAG::MtcaProgrammerJTAG(const ProgAccessMap& args) 
    : MtcaProgrammerBase(args)
{
}

MtcaProgrammerJTAG::MtcaProgrammerJTAG(const ProgAccessDmap& args) 
    : MtcaProgrammerBase(args)
{
}

MtcaProgrammerJTAG::~MtcaProgrammerJTAG() 
{
}

bool MtcaProgrammerJTAG::checkFirmwareFile(std::string firmwareFile)
{
    FILE *input_file;
    unsigned char buffer[16];
    int i;
    bool ret = true;
    size_t bytes_read;

    input_file = fopen(firmwareFile.c_str(), "r");
    if(input_file == NULL)
    {
            throw std::invalid_argument("Cannot open XSVF file. Maybe the file does not exist");
    }
    else
    {
            bytes_read = fread(buffer, 1, 16, input_file);
            if(bytes_read != 16)
            {
                fclose(input_file);
                throw std::runtime_error("Cannot read verification pattern from XSVF file");
            }
            
            for(i = 0; i < 16; i++)
            {
                    if(buffer[i] != xsvf_pattern[i])
                    {
                            ret = false;    //Incorrect XSVF file
                            break;
                    }
            }

            fclose(input_file);
    }

    return ret;
}

void MtcaProgrammerJTAG::erase()
{
    
}

void MtcaProgrammerJTAG::program(std::string firmwareFile)
{
//    printf("XSVF player for MicroTCA boards\n");

    reg_spi_divider = 10;
    reg_spi_divider.write();
    reg_control = 0x00000000;
    reg_control.write();
    
    XSVFPlayer player(*this);
    player.run(firmwareFile);
    printf("\nProgramming finished\n");
}

bool MtcaProgrammerJTAG::verify(std::string )
{
    return true;
}

void MtcaProgrammerJTAG::rebootFPGA()
{
    printf("FPGA rebooting...\n");
    reg_rev_switch = FPGA_REBOOT_WORD;
    reg_rev_switch.write();
}

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void MtcaProgrammerJTAG::setPort (jtag_port_t p, short val)
{    
    //extern SXsvfInfo sxvfInfo;
    if (p == TMS)
    {
        if (val == 1)   reg_tms = 0x1;
        else            reg_tms = 0x0;
        reg_tms.write();
    }
    else if (p == TDI)
    {
        if (val == 1)   reg_tdi = 0x1;
        else            reg_tdi = 0x0;
        reg_tdi.write();
    }
    else if (p == TCK)
    {
        if (val == 1)   reg_tck = 0x1;
        else            reg_tck = 0x0;
        reg_tck.write();
    }
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char MtcaProgrammerJTAG::readTDOBit()
{
    uint32_t data = 0;
    reg_tdo.read();
    data = reg_tdo;
    return ((unsigned char) (data & 0x1));
}

/* toggle tck LH.  No need to modify this code.  It is output via setPort. */
void MtcaProgrammerJTAG::pulseClock ()
{
    setPort (TCK, 0);		/* set the TCK port to low  */
    setPort (TCK, 1);		/* set the TCK port to high */
}

/* waitTime:  Implement as follows: */
/* REQUIRED:  This function must consume/wait at least the specified number  */
/*            of microsec, interpreting microsec as a number of microseconds.*/
/* REQUIRED FOR SPARTAN/VIRTEX FPGAs and indirect flash programming:         */
/*            This function must pulse TCK for at least microsec times,      */
/*            interpreting microsec as an integer value.                     */
/* RECOMMENDED IMPLEMENTATION:  Pulse TCK at least microsec times AND        */
/*                              continue pulsing TCK until the microsec wait */
/*                              requirement is also satisfied.               */
void MtcaProgrammerJTAG::waitTime (long microsec)
{
    if (microsec > 100000000)
    {
        microsec = 30 * 1000 * 1000;
        printf ("Pausing for %ld seconds\n", microsec / 1000000);
        microsec /= 100;

        int i=0;
        for (i=1; i<=100; i++)
        {
            ProgressBar(100, i);
            usleep (microsec);		
        }
        printf ("\nProgramming...\n");
        microsec = 1;
    }
    setPort (TCK, 0);
    //dm
    usleep (microsec);
}