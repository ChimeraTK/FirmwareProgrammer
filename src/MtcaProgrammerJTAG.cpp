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

MtcaProgrammerJTAG::MtcaProgrammerJTAG(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) 
    : MtcaProgrammerBase(dev, base_address, bar)
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

    input_file = fopen(firmwareFile.c_str(), "r");
    if(input_file == NULL)
    {
            throw "Cannot open XSVF file. Maybe the file does not exist\n";
    }
    else
    {
            fread(buffer, 1, 16, input_file);

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

    mDevPtr->writeReg(regAddress(REG_SPI_DIVIDER), 10, mProgBar);
    mDevPtr->writeReg(regAddress(REG_CONTROL), 0x00000000, mProgBar);	// PCIe to RTM
    		
    XSVFPlayer player(*this);
    player.run(firmwareFile);
    printf("\nProgramming finished\n");
}

bool MtcaProgrammerJTAG::verify(std::string firmwareFile)
{
    return true;
}

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void MtcaProgrammerJTAG::setPort (jtag_port_t p, short val)
{    
    //extern SXsvfInfo sxvfInfo;
    if (p == TMS)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(REG_TMS), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(REG_TMS), 0x0, mProgBar);
    }
    else if (p == TDI)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(REG_TDI), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(REG_TDI), 0x0, mProgBar);
    }
    else if (p == TCK)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(REG_TCK), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(REG_TCK), 0x0, mProgBar);
    }
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char MtcaProgrammerJTAG::readTDOBit()
{
    int data = 0;
    mDevPtr->readReg(regAddress(REG_TDO), &data, mProgBar);
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