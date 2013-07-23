#include "dsp_tools.h"

#include <stdio.h>
#include <unistd.h>
#include "II_interface.h"
#include "ii_constants.h"
#include "spi_mem_tools.h"
#include "ipmi_cmd.h"
#include "global.h"
#include "progress_bar.h"

void dsp_init_boot(int ttyDesc, uint8_t boot_source)
{
	//printf("writeRegister (ttyDesc, regAddress (control), 0x80000000)\n");
	writeRegister (ttyDesc, regAddress (control), (DSP_RESET));	//(31 -> DSP_COMM_MODULE_RESET, 27 ->SPIREADER_RESET_N, 5 -> SPIREADER_ACTIVE_N) 
	sleep(1);
	
	if(boot_source == PCIE_BOOT)
	{
		//printf("writeRegister (ttyDesc, regAddress (control), 0x00000000);\n");
		writeRegister (ttyDesc, regAddress (control), (PCIE_V5));	
		//printf("writeRegister (ttyDesc, regAddress (dsp_control), 0x00000001);\n");
		writeRegister (ttyDesc, regAddress (dsp_control), 0x00000001);	//(1 -> SPI_BOOT, 0 -> BOOT_MODE) 
	}
	else if(boot_source == SPI_BOOT)
	{
		//printf("writeRegister (ttyDesc, regAddress (control), 0x00000000);\n");
		writeRegister (ttyDesc, regAddress (control), 0x00000000);
		sleep(1);
		
		//printf("writeRegister (ttyDesc, regAddress (dsp_control), 0x00000003);\n");
		writeRegister (ttyDesc, regAddress (dsp_control), 0x00000003);
		sleep(1);
		writeRegister (ttyDesc, regAddress (control), DSP_SPI_BOOT);
		sleep(1);
	}
}

/** Reset DSP processor on the uTC board 
 * */
void dsp_reset(char* hostname, uint8_t slot_number)
{
	write_CPLD_register(hostname, slot_number, 0x08, 0x01); 
	usleep(1000);
	write_CPLD_register(hostname, slot_number, 0x08, 0x03);
	usleep(1000);
}

void dsp_write_data(int ttyDesc, uint32_t data)
{
  static int counter=0;
  uint32_t d;
  do
  {
    readRegister (ttyDesc, regAddress (dsp_status), &d);
  } 
  while(!(d & 1));
  
  //printf("writing long: %x\n",data);
  writeRegister (ttyDesc, regAddress (dsp_data), data);
  //printf("written: %d\n",counter++);
}

void dsp_boot_file(int ttyDesc, const char* filename)
{
	uint32_t buffer;
	size_t len=0;
	size_t addr=0;
	unsigned int file_size;
	FILE *f = fopen (filename, "rb");

	printf ("Starting booting\n");

	if (!f)
		throw "cannot open input file";

	fseek(f, 0, SEEK_END);
    file_size = ftell(f) / 4;		//size in words
    fseek(f, 0, SEEK_SET);

	do
	{
		len = fread (&buffer, 1, 4, f);
		if (len == 0)
		{
			if (ferror (f))
			{
				fclose (f);
				throw ("Error reading input file");
			}
			fclose(f);
			
			while((addr % 4) != 0)
			{
				dsp_write_data(ttyDesc, 0);
				addr++;
			}
			ProgressBar(100, 100);
			printf ("\nwritten %d words\n", addr);
			writeRegister (ttyDesc, regAddress (dsp_control), 0x00000000);
			return;
		}
		dsp_write_data(ttyDesc, buffer);
		if(addr % 100 == 0)
		{
		    ProgressBar(file_size, addr);
		}
		addr++;
	} while(1);
	fclose(f);
}

void dsp_spi_reader_reset(int ttyDesc)
{
	writeRegister (ttyDesc, regAddress (control), SPIREADER_RESET);	//(27 -> SPIREADER_RESET_N)
}
