/*
 * PCIE_Programmer.cpp
 *
 *  Created on: 26-05-2011
 *      Author: tsk
 */


#include <stdio.h>
#include <stdlib.h>

#include "micro.h"
#include "II_interface.h"
#include "libdev.h"
#include "global.h"
#include "progress_bar.h"

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <assert.h>

SXsvfInfo sxvfInfo;

#ifdef __DEV_DBG__
extern devAccess_Multifile dev;
#endif
#ifdef __DEV_LLRF__
extern devAccess_Struct dev;
#endif

FILE* fp;

extern char dummy_xsvf_player;

/* Play the contents of the xsvf file passed as the second non-option argument.
   The arguments are assumed to be already rearranged by getopt.
   Return 0 on success, 1 on failure */
int xsfv_player(int optind, char* argv[]) {

	int commandCounter = 0, totalCommandCounter=1;

	printf("XSVF player for MicroTCA boards\n");
	extern const char* ERROR_CODES_MICRO_H[];


//dummy xsvf_player------------------------
	dummy_xsvf_player = 1;

#if 1	
	fp = fopen(argv[optind+1],"r");
    if (!fp)
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
//-----------------------------------------
#endif
	
	dummy_xsvf_player = 0;
	commandCounter = 0;
	
	SetUpDevice (argv[optind]);

	fp = fopen(argv[optind+1],"r");
    if (!fp)
	{
		perror("Error opening xsfv file");
		exit(1);
	}

	xsvfInitialize(&sxvfInfo);
	do {
		commandCounter++;
		xsvfRun(&sxvfInfo);
			if(sxvfInfo.iErrorCode != XSVF_ERROR_NONE) break;
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

    printf("\nProgramming finished with message: %s\n", ERROR_CODES_MICRO_H[sxvfInfo.iErrorCode]);
	fclose(fp);
	return (sxvfInfo.iErrorCode == XSVF_ERROR_NONE) ? 0 : 1;
}
