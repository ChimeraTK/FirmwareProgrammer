/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/* Revisions:                                          */
/* 12/01/2008:  Same code as before (original v5.01).  */
/*              Updated comments to clarify instructions.*/
/*              Add print in setPort for xapp058_example.exe.*/
/*******************************************************/
#include "ports.h"
#include <stdio.h>
#include <unistd.h>
#include "micro.h"
#include "II_interface.h"
#include "ii_constants.h"
#include "libdev.h"

#include "progress_bar.h"

//unsigned int TMS_p = 0;
//unsigned int TMS_n = 0;
//unsigned int TDI_p = 0;
//unsigned int TDI_n = 0;
//unsigned int TCK_p = 0;
//unsigned int TCK_n = 0;

extern char dummy_xsvf_player;

int regAddress (int reg);


#ifdef __DEV_DBG__
extern devAccess_Multifile dev;
#endif
#ifdef __DEV_LLRF__
extern devAccess_Struct dev;
#endif

/* setPort:  Implement to set the named JTAG signal (p) to the new value (v).*/
/* if in debugging mode, then just set the variables */
void
setPort (short p, short val)
{    
	if(!dummy_xsvf_player)
	{

		//extern SXsvfInfo sxvfInfo;
		if (p == TMS)
		{
			if (val == 1)
				dev.writeReg(regAddress(tms)+OFFSET, 0x1, BAR_NR);  
				//writeRegister (sxvfInfo.devDescriptor, tms, 0x1);
			else
				dev.writeReg(regAddress(tms)+OFFSET, 0x0, BAR_NR);  
				//writeRegister (sxvfInfo.devDescriptor, tms, 0x0);
			}
		else if (p == TDI)
		{
			if (val == 1)
				dev.writeReg(regAddress(tdi)+OFFSET, 0x1, BAR_NR);   
				//writeRegister (sxvfInfo.devDescriptor, tdi, 0x1);
			else
				dev.writeReg(regAddress(tdi)+OFFSET, 0x0, BAR_NR);    
				//writeRegister (sxvfInfo.devDescriptor, tdi, 0x0);
		}
		else if (p == TCK)
		{
			if (val == 1)
				dev.writeReg(regAddress(tck)+OFFSET, 0x1, BAR_NR); 
				//writeRegister (sxvfInfo.devDescriptor, tck, 0x1);
			else
				dev.writeReg(regAddress(tck)+OFFSET, 0x0, BAR_NR);  
				//writeRegister (sxvfInfo.devDescriptor, tck, 0x0);
		}
	}
}

/* toggle tck LH.  No need to modify this code.  It is output via setPort. */
void
pulseClock ()
{
  	setPort (TCK, 0);		/* set the TCK port to low  */
	setPort (TCK, 1);		/* set the TCK port to high */
}

/* readByte:  Implement to source the next byte from your XSVF file location */
/* read in a byte of data from the prom */
void
readByte (unsigned char *data)
{
  extern FILE *fp;
  *data = fgetc (fp);
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
unsigned char
readTDOBit ()
{
	int data = 0;
	if(!dummy_xsvf_player)
	{
//   usleep(1);
	//readRegister (sxvfInfo.devDescriptor, tdo, &data);
	dev.readReg(regAddress(tdo)+OFFSET, &data, BAR_NR);
//        fflush(stdout);
	}
	return ((unsigned char) (data & 0x1));
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
void
waitTime (long microsec)
{
	if(!dummy_xsvf_player)
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
}
