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
#include "micro.h"

const uint8_t MtcaProgrammerJTAG::xsvf_pattern[16] = {0x07, 0x00, 0x13, 0x00, 0x14, 0x00, 0x12, 0x00, 0x12, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02};

FILE *xsvf_player_fp;

MtcaProgrammerJTAG::MtcaProgrammerJTAG(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) 
    : MtcaProgrammerBase(dev, base_address, bar), player(NULL) 
{
}

MtcaProgrammerJTAG::~MtcaProgrammerJTAG() 
{
    delete player;
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

void MtcaProgrammerJTAG::initialize()
{
    mDevPtr->writeReg(regAddress(spi_divider), 10, mProgBar);
    mDevPtr->writeReg(regAddress(control), 0x00000000, mProgBar);	// PCIe to RTM
}

void MtcaProgrammerJTAG::erase()
{
    
}

void MtcaProgrammerJTAG::program(std::string firmwareFile)
{
    printf("XSVF player for MicroTCA boards\n");
    extern const char* ERROR_CODES_MICRO_H[];

    xsvf_player_fp = fopen(firmwareFile.c_str(),"r");
    if (!xsvf_player_fp)
    {
        perror("Error opening xsfv file");
        exit(1);
    }
		
    player = new XSVFPlayer(this, xsvf_player_fp);
    player->initialize();
    fseek(xsvf_player_fp, 0, SEEK_SET);
    player->run();
    
#ifdef DEBUG
	printf("\nCommands number in file: %d", commandCounter);
#endif

//    printf("\nProgramming finished with message: %s\n", ERROR_CODES_MICRO_H[sxvfInfo.iErrorCode]);
    fclose(xsvf_player_fp);
}

bool MtcaProgrammerJTAG::verify(std::string firmwareFile)
{
    return true;
}

/* readByte:  Implement to source the next byte from your XSVF file location */
/* read in a byte of data from the prom */
void MtcaProgrammerJTAG::readByte (unsigned char *data)
{
    *data = fgetc(xsvf_player_fp);
}

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void MtcaProgrammerJTAG::setPort (short p, short val)
{    
    //extern SXsvfInfo sxvfInfo;
    if (p == TMS)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(tms), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(tms), 0x0, mProgBar);
    }
    else if (p == TDI)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(tdi), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(tdi), 0x0, mProgBar);
    }
    else if (p == TCK)
    {
        if (val == 1)
            mDevPtr->writeReg(regAddress(tck), 0x1, mProgBar);
        else
            mDevPtr->writeReg(regAddress(tck), 0x0, mProgBar);
    }
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char MtcaProgrammerJTAG::readTDOBit()
{
    int data = 0;
    mDevPtr->readReg(regAddress(tdo), &data, mProgBar);
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