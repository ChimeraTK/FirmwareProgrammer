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

extern char dummy_xsvf_player;
const uint8_t MtcaProgrammerJTAG::xsvf_pattern[16] = {0x07, 0x00, 0x13, 0x00, 0x14, 0x00, 0x12, 0x00, 0x12, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02};

FILE *xsvf_player_fp;
extern boost::shared_ptr<mtca4u::devBase> mtca_dev_for_ports;

MtcaProgrammerJTAG::MtcaProgrammerJTAG(mtcaDevPtr dev, uint32_t base_address, uint8_t bar) 
    : MtcaProgrammerBase(dev, base_address, bar) 
{
    mtca_dev_for_ports = mDevPtr;
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
    int commandCounter = 0, totalCommandCounter=1;

    printf("XSVF player for MicroTCA boards\n");
    extern const char* ERROR_CODES_MICRO_H[];


    //dummy xsvf_player------------------------
    dummy_xsvf_player = 1;

#if 1	
    xsvf_player_fp = fopen(firmwareFile.c_str(),"r");
    if (!xsvf_player_fp)
    {
        perror("Error opening xsfv file");
        exit(1);
    }
		
    xsvfInitialize(&sxvfInfo);
    do {
        commandCounter++;
        xsvfRun(&sxvfInfo);
        //if(sxvfInfo.iErrorCode != XSVF_ERROR_NONE) break;
    }
    while(sxvfInfo.ucComplete == 0);
    totalCommandCounter = commandCounter;
    fclose(xsvf_player_fp);
    //-----------------------------------------
#endif
	
    dummy_xsvf_player = 0;
    commandCounter = 0;

    xsvf_player_fp = fopen(firmwareFile.c_str(),"r");
    if (!xsvf_player_fp)
    {
        perror("Error opening xsfv file");
        exit(1);
    }

    xsvfInitialize(&sxvfInfo);
    do {
        commandCounter++;
        xsvfRun(&sxvfInfo);
        if(sxvfInfo.iErrorCode != XSVF_ERROR_NONE) 
            break;
        if(commandCounter%100 == 0)
        {
            ProgressBar(totalCommandCounter, commandCounter);
        }
    }
    while(sxvfInfo.ucComplete == 0);
    ProgressBar(totalCommandCounter, totalCommandCounter);

#ifdef DEBUG
	printf("\nCommands number in file: %d", commandCounter);
#endif

//    printf("\nProgramming finished with message: %s\n", ERROR_CODES_MICRO_H[sxvfInfo.iErrorCode]);
    fclose(xsvf_player_fp);
    if(sxvfInfo.iErrorCode != XSVF_ERROR_NONE)
        throw ERROR_CODES_MICRO_H[sxvfInfo.iErrorCode];
}

bool MtcaProgrammerJTAG::verify(std::string firmwareFile)
{
    return true;
}

