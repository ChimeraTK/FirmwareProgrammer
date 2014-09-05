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
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <dirent.h>

#include "II_interface.h"
#include "ii_constants.h"

#include "ipmi_cmd.h"
#include "spi_mem_tools.h"
#include "dsp_tools.h"
#include "global.h"

#define VERSION		"3.0"

#define PCIE_DOWN		0
#define PCIE_UP			1
#define PCIE_DOWN_UP	2

uint8_t show_example;

typedef enum{	ACT_NONE, 
				ACT_SPI_PROG,
				ACT_JTAG_PROG,
				ACT_SIS_JTAG_PROG,
				ACT_UVM_JTAG_PROG,
				ACT_UTC_MEM_PROG,
				ACT_UTC_MEM_REC,
				ACT_UTC_MEM_SWITCH,
				ACT_UTC_DSP_PCIE,
				ACT_UTC_DSP_REC,
				ACT_UTC_DSP_MEM,
				ACT_UTC_RTM_PROG,
				ACT_SISL_MEM_PROG,
				ACT_SIS_MEM_PROG,
				ACT_SIS_MEM_REC,
				ACT_SIS_MEM_SWITCH,
				ACT_DAMC2_MEM_PROG,
				ACT_DAMC2_MEM_REC,
				ACT_DAMC2_MEM_SWITCH
} action_t;

typedef struct{
	int8_t memory_number;
	uint8_t slot_number;
	uint8_t board;
	uint8_t hotplug;
	char* hostname;
	char* device;
	uint8_t protocol;
	uint8_t reload;
	action_t action;
	char* source_file;
} arguments_t;

extern int xsfv_player(int argc, char* argv[]);

const unsigned char xsvf_pattern[16] = {0x07, 0x00, 0x13, 0x00, 0x14, 0x00, 0x12, 0x00, 0x12, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02};
const unsigned char bit_pattern[14] = {0x00, 0x09, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x00, 0x00, 0x01, 0x61};

void init_arguments (arguments_t *arguments);
int check_XSVF_file(char* filename);
void PCIe_hot_plug(uint8_t slot_number, uint8_t option);

void PrintAllParameters (arguments_t *arguments){
	printf ("MCH name: %s\n", arguments->hostname);
	printf ("Device: %s\n", arguments->device);
	printf ("Memory Number: %d\n", arguments->memory_number);
	printf ("Slot Number: %d\n", arguments->slot_number);
	
	printf ("Board: %d\n", arguments->board);
	printf ("HotPlug: %d\n", arguments->hotplug);

	printf ("Protocol: %d\n", arguments->protocol);
	printf ("Reload: %d\n", arguments->reload);
	printf ("Action: %d\n", arguments->action);
	printf ("Source_file: %s\n", arguments->source_file);
}


/** explain the usage of the program */
void
usage (const char *progname, uint8_t show_example) 
{
  fprintf (stderr, "\nLLRF PROM programmer for uTC/uVM/SIS8300/SIS8300L/DAMC2 boards\n");
  fprintf (stderr, "%s -d [device] -i [interface] -f [firmware file] -H -r -m [MCH name] \n\n", progname);

  fprintf (stderr, "Example device: -d /dev/llrfutcs4 \n\n");  

  fprintf (stderr, "Supported interfaces:\n");  
  fprintf (stderr, "-i spi \n");   
  fprintf (stderr, "-i jtag\n\n");
  
  fprintf (stderr, "Optional parameter:\n");  
  fprintf (stderr, "-H: PCIe hot-plug, required write access to /sys/bus/proc/pci/.../enable files\n"); 
  fprintf (stderr, "-r: Reload FPGA, require access to MCH\n");   
  fprintf (stderr, "-m: MCH address, e.g. mskmchacc12 \n\n");
  
  fprintf (stderr, "\n Type %s -e  for examples \n\n", progname);
 

  if(show_example)
  {
	  fprintf (stderr, "\nExamples:\n");
	  fprintf (stderr, "a) uTC board - FPGA:\n");
	  fprintf (stderr, "\t%s -d /dev/llrfutcs4 -i spi -f /home/user/new_firmware.bit -H \n", progname);
	  fprintf (stderr, "\t%s -d /dev/llrfutcs4 -H -r\n\n", progname);

	  fprintf (stderr, "b) uVM board - FPGA\n");
	  fprintf (stderr, "\t%s -d /dev/llrfutcs4 -i jtag -f /home/user/new_firmware.bit \n\n", progname);
	  
	  fprintf (stderr, "c) SIS8300L board - FPGA:\n");
	  fprintf (stderr, "\t%s -d /dev/llrfadcs4 -i spi -f /home/user/new_firmware.bit -H \n", progname);
	  fprintf (stderr, "\t%s -d /dev/llrfadcs4 -H -r\n\n", progname);

	  fprintf (stderr, "d) SIS8300 board - FPGA:\n");
	  fprintf (stderr, "\t%s -d /dev/llrfadcs4 -i jtag -f /home/user/new_firmware.bit -H \n", progname);
	  fprintf (stderr, "\t%s -d /dev/llrfadcs4 -H -r\n\n", progname);
  }
  else
  {
	  fprintf (stderr, "Use option -e to see examples\n\n");
  }
  exit (1);
}

int check_bit_file(char* filename);

void program_SPI_prom  (arguments_t* arguments) 
{
	int ttyDesc;
	FILE* tmp_file = NULL;
	//Select memory for programming (0 or 1)
	if (arguments->memory_number != DO_NOT_SELECT_MEMORY)
	{
		if  	(arguments->memory_number == 0)
			set_PROM_number(arguments->hostname, arguments->slot_number, UTC_PROGRAMMER_MEM, uTC);
		else if	(arguments->memory_number == 1)
			set_PROM_number(arguments->hostname, arguments->slot_number, UTC_APPLICATION_MEM, uTC);
	} 
		
	//Programming if bit-file provided
	if (arguments->source_file != NULL)
	{
		//Check if device available before programming
		tmp_file = fopen(arguments->device, "r"); 
		if(tmp_file == NULL)
		{
			printf ("Opening device: %s\n", arguments->device);
			throw "Cannot open the device for programming. Maybe the driver is not loaded nor you do not have rights to open the device.";
		}
		else
		{
			fclose(tmp_file);
		}
		//Check if provided correct bit-file
		if( check_bit_file(arguments->source_file) == 0 )
			{
				fprintf (stderr, "Wrong file format\n");
				fprintf (stderr, "File: %s is not correct bit file\n\n", arguments->source_file);
				return;
			}
		else 
		{		
			// Select default memory (User Application) if not selected 
			if (arguments->memory_number == DO_NOT_SELECT_MEMORY)
			{
//				set_PROM_number(arguments->hostname, arguments->slot_number, UTC_PROGRAMMER_MEM, uTC);
			}
	
			#ifdef DEBUG
				printf("Device to program: %s\n", arguments->device);
			#endif
			//set up connection with PCIe device
			ttyDesc = SetUpDevice (arguments->device);
			writeRegister(ttyDesc,regAddress(spi_divider), 10);

			if(!checkId(ttyDesc))
			{
				fprintf(stderr, "Unknown, not present or busy SPI prom\n\n");
				exit(1);
			}
			
			//erase memory
			write_enable (ttyDesc);
			bulk_erase_mem (ttyDesc);
			
			//program memory
			write_enable (ttyDesc);
			program_mem (ttyDesc, arguments->source_file);
			
			//verify memory
			if( verify_mem (ttyDesc, arguments->source_file))
			{
				exit(1);
			}
		}
	}
	
	// Hot-plug is requested
	if(arguments->hotplug)
	{
			fprintf(stdout, "PCIe Hot-plug...\n");
		
		
	/* 	DIR* dir = opendir("mydir");
		if (dir)
		{
			Directory exists.
			closedir(dir);
		}
		else if (ENOENT == errno)
		{
			Directory does not exist.
		}
		else
		{
			opendir() failed for some other reason.
		} */
		
		PCIe_hot_plug(arguments->slot_number, PCIE_DOWN);
	}
	
	//FPGA reboot is requested
	if(arguments->reload)
	{
	  if(arguments->board = SIS8300)
		FRU_reset(arguments->hostname, arguments->slot_number, SIS8300);
	  else if (arguments->board = uTC)
		FRU_reset(arguments->hostname, arguments->slot_number, uTC);
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(15);
	}
	
	if(arguments->hotplug)
	{
	
		//rescan PCIe bus
		PCIe_hot_plug(arguments->slot_number, PCIE_UP);
	}

	//retrieve number of currently selected memory - to verify
	//get_PROM_number(hostname, slot_number, &tmp_mem_number);
	//fprintf(stdout, "Selected memory: %d\n", tmp_mem_number-1);	//-1 in order to change memory numbering
}

void program_JTAG_prom (arguments_t* arguments) //--char* device, char* filename, uint8_t hotplug, char* hostname, int slot_number)
{
	char* xsvf_argv[2];
	int ttyDesc;
	uint8_t tmp_reg_value, old_reg_value;
	
	//check XSVF file
	if( check_XSVF_file(arguments->source_file) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct XSVF file\n\n", arguments->source_file);
		return;
	}

	if (arguments->board == uVM)
	{

	//connect MMC JTAG lines to main JTAG chain 
		read_CPLD_register(arguments->hostname, arguments->slot_number+1, JTAG_SPI_CONF_REG, &old_reg_value);		//JTAG configuration register

#ifdef DEBUG
		printf("read JTAG_SPI_CONF_REG: 0x%x\n", old_reg_value);
#endif

		tmp_reg_value = old_reg_value & (~(1 << JTAG_MMC));
#ifdef DEBUG
		printf("write JTAG_SPI_CONF_REG: 0x%x\n", tmp_reg_value);
#endif
		write_CPLD_register(arguments->hostname, arguments->slot_number+1, JTAG_SPI_CONF_REG, tmp_reg_value);
	}
	ttyDesc = SetUpDevice (arguments->device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);
	writeRegister(ttyDesc, regAddress(control), 0x00000000);	// PCIe to RTM
	
	//use xsvf_player for uVM board programming
	xsvf_argv[0] = arguments->device;
	xsvf_argv[1] = arguments->source_file;
//	xsvf_argv[2] = NULL;
	
	xsfv_player(0, xsvf_argv);
	if (arguments->board == uVM)
	{
		//restore old value of JTAG configuration register
		write_CPLD_register(arguments->hostname, arguments->slot_number+1, JTAG_SPI_CONF_REG, old_reg_value);
#ifdef DEBUG
	printf("write old value to JTAG_SPI_CONF_REG: 0x%x\n", old_reg_value);
#endif
	}

	if(arguments->hotplug)
	{
		fprintf(stdout, "PCIe Hot-plug...\n");
//		PCIe_hot_plug(filename, PCIE_DOWN);
	}
	
	
	//send command causing FPGA reboot
	if(arguments->reload)
	{
	  if(arguments->board = SIS8300)
		FRU_reset(arguments->hostname, arguments->slot_number+1, SIS8300);
	  else if (arguments->board = uTC)
		printf ("Cannot reload uVM ver. <=1.2 \n");
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	}
	
	if(arguments->hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(char* pcie_path, PCIE_UP);
	}
}


#if 0
void SISL_SPI_program(char* device, char* filename, uint8_t hotplug)
{
	int ttyDesc;	
	//check bit file
	if( check_bit_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct bit file\n\n", filename);
	}
	
	//erase memory
	write_enable (ttyDesc);
	bulk_erase_mem (ttyDesc);
	
	//program memory
	write_enable (ttyDesc);
	program_mem (ttyDesc, filename);
	
	//verify memory
	if( verify_mem (ttyDesc, filename) )
	{
		exit(1);
	}

#if 0	
	if(hotplug)
	{
		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, uTC);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
		PCIe_hot_plug(slot_number, PCIE_UP);
	}
	
	//retrieve number of currently selected memory - to verify
	get_PROM_number(hostname, slot_number, &tmp_mem_number);
	fprintf(stdout, "Selected memory: %d\n", tmp_mem_number-1);	//-1 in order to change memory numbering
#endif
	
v	if(hotplug)
	{
	//	PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
//	FRU_reset(hostname, slot_number, SIS8300);
	
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
	//	PCIe_hot_plug(slot_number, PCIE_UP);
	}


}

#endif

void PCIe_hot_plug(uint8_t slot_number, uint8_t option)
{
	static char throw_output[128] = {""};
	char filename[128];
	FILE *file = NULL;
	
	//Check if xxxx_1 device exists  
	sprintf(filename, "/sys/bus/pci/slots/%d-1", slot_number);
	
	DIR* dir = opendir(filename);
	if (dir)
	{
		/* Directory exists. */
		closedir(dir);
		sprintf(filename, "/sys/bus/pci/slots/%d-1/power", slot_number);
	}
	else
	{
		sprintf(filename, "/sys/bus/pci/slots/%d/power", slot_number);
	}
#ifdef DEBUG
	printf("%s\n", filename);
#endif

	if(option == PCIE_DOWN || option == PCIE_DOWN_UP)
	{
		file = fopen(filename, "w");
		if(file != NULL)
		{
			if(fputc('0', file) == EOF)
			{
				fprintf(stderr, "Cannot write '0' to file %s\n", filename);
				//throw output;
			}
			fclose(file);
		}
		else
		{
			fprintf(stderr, "Cannot open file %s\n", filename);
			//throw output;
		}
		
		sleep(1);
	}
	
	if(option == PCIE_UP || option == PCIE_DOWN_UP)
	{
		file = fopen(filename, "w");
		if(file != NULL)
		{
			if(fputc('1', file) == EOF)
			{
				sprintf(throw_output, "Cannot write '1' to file %s\n", filename);
				throw throw_output;
			}
			fclose(file);
		}
		else
		{
			sprintf(throw_output, "Cannot open file %s\n", filename);
			throw throw_output;
		}
	}
}

/** Refresh PCIe bus - power down the PCIe link for given slot, wait a while
 *  and power up the link. 
 *  The function checks whether the fakephp module is loaded. 
 *  If some errors occur the function throws exceptions */
/*void PCIe_hot_plug(uint8_t slot_number)
{
	static char throw_output[128] = {""};
	char filename[128];
	FILE *file = NULL;
	
	sprintf(filename, "/sys/bus/pci/slots/%d/power", slot_number);

#ifdef DEBUG
	printf("%s\n", filename);
#endif

	file = fopen(filename, "w");
	if(file != NULL)
	{
		if(fputc('0', file) == EOF)
		{
			fprintf(stderr, "Cannot write '0' to file %s\n", filename);
			//throw output;
		}
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Cannot open file %s\n", filename);
		//throw output;
	}
	
	sleep(1);
	
	file = fopen(filename, "w");
	if(file != NULL)
	{
		if(fputc('1', file) == EOF)
		{
			sprintf(throw_output, "Cannot write '1' to file %s\n", filename);
			throw throw_output;
		}
		fclose(file);
	}
	else
	{
		sprintf(throw_output, "Cannot open file %s\n", filename);
		throw throw_output;
	}
}*/

int check_XSVF_file(char* filename)
{
	FILE *input_file;
	unsigned char buffer[16];
	int i;
	int ret = 1;

	input_file = fopen(filename, "r");
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
				ret = 0;
				break;
			}
		}
		
		fclose(input_file);
	}
	
	return ret;
}

int check_bit_file(char* filename)
{
	FILE *input_file;
	unsigned char buffer[16];
	int i;
	int ret = 1;

	input_file = fopen(filename, "r");
	if(input_file == NULL)
	{
		throw "Cannot open bit file. Maybe the file does not exist\n";
	}
	else
	{
		fread(buffer, 1, 14, input_file);

		for(i = 0; i < 14; i++)
		{
			if(buffer[i] != bit_pattern[i])
			{
				ret = 0;
				break;
			}
		}
		
		fclose(input_file);
	}
	
	return ret;
}

void uTC_mem_program(char* hostname, uint8_t slot_number, uint8_t memory_number, char* filename, int hotplug)
{
	char device[32] = {""};
	int ttyDesc;
	uint8_t tmp_mem_number;
	
	//check bit file
	if( check_bit_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct bit file\n\n", filename);
		return;
	}
	
	memory_number++;		//change the memory number range from 0-2 to 1-3
	//check if memory number is correct
	if(memory_number < 1 || memory_number > 3)
	{
		throw "Wrong memory number\n\n";
	}
	
	//select memory for programming
	set_PROM_number(hostname, slot_number, memory_number, uTC);
	
	//prepare device name appropriate to the board type
	sprintf(device, "/dev/llrfutcs%d", slot_number);
#ifdef DEBUG
	printf("%s\n", device);
#endif
	//set up connection with PCIe device
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);

	if(!checkId(ttyDesc))
	{
		fprintf(stderr, "Unknown, not present or busy SPI prom\n\n");
		exit(1);
	}
	
	//erase memory
	write_enable (ttyDesc);
	bulk_erase_mem (ttyDesc);
	
	//program memory
	write_enable (ttyDesc);
	program_mem (ttyDesc, filename);
	
	//verify memory
	if( verify_mem (ttyDesc, filename) )
	{
		exit(1);
	}
	
	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, uTC);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
	//	PCIe_hot_plug(slot_number, PCIE_UP);
	}
	
	//retrieve number of currently selected memory - to verify
	get_PROM_number(hostname, slot_number, &tmp_mem_number);
	fprintf(stdout, "Selected memory: %d\n", tmp_mem_number-1);	//-1 in order to change memory numbering
}

void uTC_mem_recover(char* hostname, uint8_t slot_number, int hotplug)
{
	uint8_t memory_number;
	
	//select memory no 1
	set_PROM_number(hostname, slot_number, UTC_PROGRAMMER_MEM, uTC);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, uTC);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
	
	//retrieve number of currently selected memory - to verify
	get_PROM_number(hostname, slot_number, &memory_number);
	fprintf(stdout, "Selected memory: %d\n", memory_number-1);	//-1 in order to change memory numbering
}

void uTC_mem_select(char* hostname, uint8_t slot_number, uint8_t memory_number, int hotplug)
{
	uint8_t tmp_mem_number;
	
	memory_number++;		//change the memory number range from 0-2 to 1-3
	//check if memory number is correct
	if(memory_number < 1 || memory_number > 3)
	{
		throw "Wrong memory number\n";
	}
	
	//select memory
	set_PROM_number(hostname, slot_number, memory_number, uTC);
	
	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, uTC);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
	
	//retrieve number of currently selected memory - to verify
	get_PROM_number(hostname, slot_number, &tmp_mem_number);
	fprintf(stdout, "Selected memory: %d\n", tmp_mem_number-1);	//-1 in order to change memory numbering
}

void uTC_DSP_PCIE_program(char* hostname, uint8_t slot_number, char* filename)
{
	char device[32];
	int ttyDesc;
	
	//prepare device name appropriate to the board 
	sprintf(device, "/dev/llrfutcs%d", slot_number);

#ifdef DEBUG
	printf("%s\n", device);
#endif

	//set up connection with PCIe device
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);
	
	dsp_init_boot(ttyDesc, PCIE_BOOT);
	
	//reset DSP
	dsp_reset(hostname, slot_number);
	
	//DSP booting
	dsp_boot_file(ttyDesc, filename);
}

void uTC_DSP_recover(char* hostname, uint8_t slot_number)
{
	char device[32];
	int ttyDesc;
	
	//prepare device name appropriate to the board 
	sprintf(device, "/dev/llrfutcs%d", slot_number);

#ifdef DEBUG
	printf("%s\n", device);
#endif

	//set up connection with PCIe device
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);
	
	//switch to DSP boot memory	
	set_PROM_number(hostname, slot_number, UTC_DEFAULT_DSP_MEM, uTC);
	
	dsp_init_boot(ttyDesc, SPI_BOOT);
	
	//reset DSP
	//dm dsp_reset(hostname, slot_number);
	
	dsp_spi_reader_reset(ttyDesc);
}

void uTC_DSP_MEM_program(char* hostname, uint8_t slot_number, char* filename)
{
	char device[32] = {""};
	int ttyDesc;
	uint8_t tmp_mem_number;
	
	//get number of currently selected memory
	get_PROM_number(hostname, slot_number, &tmp_mem_number);
	
	//select memory for programming
	set_PROM_number(hostname, slot_number, UTC_DEFAULT_DSP_MEM, uTC);
	
	//prepare device name appropriate to the board type
	sprintf(device, "/dev/llrfutcs%d", slot_number);
#ifdef DEBUG
	printf("%s\n", device);
#endif
	
	//set up connection with PCIe device
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);

	if(!checkId(ttyDesc))
	{
		fprintf(stderr, "Unknown, not present or busy SPI prom\n\n");
		exit(1);
	}
	
	//erase memory
	write_enable (ttyDesc);
	bulk_erase_mem (ttyDesc);
	
	//program memory
	write_enable (ttyDesc);
	program_dsp_mem (ttyDesc, filename);
	
	//verify memory
	if( verify_dsp_mem (ttyDesc, filename) )
	{
		exit(1);
	}
	
	//restore memory number for FPGA booting
	set_PROM_number(hostname, slot_number, tmp_mem_number, uTC);
}

void uTC_RTM_MEM_program(char* hostname, uint8_t slot_number, char* filename)
{
	char* xsvf_argv[2];
	char device[32];
	int ttyDesc;
	uint8_t tmp_reg_value, old_reg_value;
	
	//check XSVF file
	if( check_XSVF_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct XSVF file\n\n", filename);
		return;
	}
	
	//connect MMC JTAG lines to main JTAG chain 
	read_CPLD_register(hostname, slot_number, JTAG_SPI_CONF_REG, &old_reg_value);		//JTAG configuration register

#ifdef DEBUG
	printf("read JTAG_SPI_CONF_REG: 0x%x\n", old_reg_value);
#endif

	tmp_reg_value = old_reg_value & (~(1 << JTAG_MMC));
#ifdef DEBUG
	printf("write JTAG_SPI_CONF_REG: 0x%x\n", tmp_reg_value);
#endif
	write_CPLD_register(hostname, slot_number, JTAG_SPI_CONF_REG, tmp_reg_value);
	
	sprintf(device, "/dev/llrfutcs%d", slot_number);
#ifdef DEBUG
	printf("%s\n", device);
#endif
	
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);
	writeRegister(ttyDesc, regAddress(control), 0x00000000);	// PCIe to RTM
	
	//use xsvf_player for uVM board programming
	xsvf_argv[0] = device;
	xsvf_argv[1] = filename;
	xsfv_player(0, xsvf_argv);
	
	//restore old value of JTAG configuration register
	write_CPLD_register(hostname, slot_number, JTAG_SPI_CONF_REG, old_reg_value);
#ifdef DEBUG
	printf("write old value to JTAG_SPI_CONF_REG: 0x%x\n", tmp_reg_value);
#endif
}


void SISL_mem_program(char* hostname, uint8_t slot_number, uint8_t memory_number, char* filename, int hotplug)
{
	char device[32] = {""};
	int ttyDesc;
	uint8_t tmp_mem_number;
	
	//check bit file
	if( check_bit_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct bit file\n\n", filename);
		return;
	}
	
	memory_number++;		//change the memory number range from 0-2 to 1-3
	//check if memory number is correct
	if(memory_number < 1 || memory_number > 3)
	{
		throw "Wrong memory number\n\n";
	}
	
	//select memory for programming
//dm	set_PROM_number(hostname, slot_number, memory_number, uTC);
	
	//prepare device name appropriate to the board type
	sprintf(device, "/dev/llrfadcs%d", slot_number);
#ifdef DEBUG
	printf("%s\n", device);
#endif
	//set up connection with PCIe device
	ttyDesc = SetUpDevice (device);
	writeRegister(ttyDesc,regAddress(spi_divider),10);

	printf ("Divider: %x  ", spi_divider);

	if(!checkId(ttyDesc))
	{
		fprintf(stderr, "Unknown, not present or busy SPI prom\n\n");
		exit(1);
	}
	
	//erase memory
	write_enable (ttyDesc);
	bulk_erase_mem (ttyDesc);
	
	//program memory
	write_enable (ttyDesc);
	program_mem (ttyDesc, filename);
	
	//verify memory
	if( verify_mem (ttyDesc, filename) )
	{
		exit(1);
	}

#if 0	
	if(hotplug)
	{
		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, uTC);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
		PCIe_hot_plug(slot_number, PCIE_UP);
	}
	
	//retrieve number of currently selected memory - to verify
	get_PROM_number(hostname, slot_number, &tmp_mem_number);
	fprintf(stdout, "Selected memory: %d\n", tmp_mem_number-1);	//-1 in order to change memory numbering
#endif
	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, SIS8300);
	
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}


}



void SIS_mem_program(char* hostname, uint8_t slot_number, uint8_t memory_number, char* filename, int hotplug)
{	
	char* sis_argv[2];
	char device[32];	
	
	//check XSVF file
	if( check_XSVF_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct XSVF file\n\n", filename);
		return;
	}
	
	//check if revision number is correct
	if(memory_number > 1)
	{
		throw "Wrong memory number\n\n";
	}
	
	sprintf(device, "/dev/llrfadcs%d", slot_number);
	printf("%s\n", device);
	
	//select revision for programming
	set_PROM_number(hostname, slot_number, memory_number, SIS8300);
	
	//use xsfc_player for programming of SIS8300 board
	sis_argv[0] = device;
	sis_argv[1] = filename;
	if(xsfv_player(0, sis_argv) != 0) 
		return;

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, SIS8300);
	
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

void SIS_mem_recover(char* hostname, uint8_t slot_number, int hotplug)
{
	//select revision no 0
	set_PROM_number(hostname, slot_number, SIS_DEFAULT_PROGRAMMER_REV, SIS8300);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}

	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, SIS8300);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

void SIS_mem_select(char* hostname, uint8_t slot_number, uint8_t memory_number, int hotplug)
{
	//check if revision number is correct
	if(memory_number != 0 && memory_number != 1)
	{
		throw "Wrong memory number\n\n";
	}
		
	//select memory
	set_PROM_number(hostname, slot_number, memory_number, SIS8300);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}

	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, SIS8300);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

void DAMC2_mem_program(char* hostname, uint8_t slot_number, uint8_t memory_number, char* filename, int hotplug)
{
	char* damc2_argv[2];
	char device[32];
	
	//check XSVF file
	if( check_XSVF_file(filename) == 0 )
	{
		fprintf (stderr, "Wrong file format\n");
		fprintf (stderr, "File: %s is not correct XSVF file\n\n", filename);
		return;
	}
	
	//check if revision number is correct
	if(memory_number < 2 || memory_number > 3)
	{
		throw "Wrong memory number\n\n";
	}
	
	//configure JTAG chain
	configure_JTAG_chain(hostname, slot_number, DAMC2);
	
	sleep(1);
	
	sprintf(device, "/dev/llrfadcs%d", slot_number);
	printf("%s\n", device);
	
	//use xsvf_player for DAMC2 board programming
	damc2_argv[0] = device;
	damc2_argv[1] = filename;
	if( xsfv_player(0, damc2_argv) != 0)
		return;
	
	//select revision
	set_PROM_number(hostname, slot_number, memory_number, DAMC2);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, DAMC2);
	
	//Rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

void DAMC2_mem_recover(char* hostname, uint8_t slot_number, int hotplug)
{
	//select revision no 3
	set_PROM_number(hostname, slot_number, DAMC2_DEFAULT_PROGRAMMER_REV, DAMC2);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}
	
	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, DAMC2);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

void DAMC2_mem_select(char* hostname, uint8_t slot_number, uint8_t memory_number, int hotplug)
{
	//check if revision number is correct
	if(memory_number < 2 || memory_number > 3)
	{
		throw "Wrong memory number\n\n";
	}
	
	//select memory
	set_PROM_number(hostname, slot_number, memory_number, DAMC2);

	if(hotplug)
	{
//		PCIe_hot_plug(slot_number, PCIE_DOWN);
	}

	//send command causing FPGA reboot
	FRU_reset(hostname, slot_number, DAMC2);
	
	//rebooting FPGA
	fprintf(stdout, "Rebooting FPGA...\n");
	sleep(2);
	
	if(hotplug)
	{
		//rescan PCIe bus
//		PCIe_hot_plug(slot_number, PCIE_UP);
	}
}

arguments_t decode_arguments(int argc, char *argv[], arguments_t* arguments)
{
	
	uint8_t memory_number = 0;
	uint8_t slot_number = 0;
//	char* hostname = NULL;
	char* source_file = NULL;

	int program = 0;
	int recover = 0;
	int select = 0;
	int hotplug = 0;
	int opt;
	action_t action;
	uint8_t board_type = 0;
	uint8_t device_type = 0;
	int extract_slot_number = 1;
	
	FILE* tmp_file;
				
#ifdef DEBUG
	printf ("Debug active, parsing arguments... \n");
	fflush (stdout);
#endif


//get and parse input options			
	while ((opt = getopt (argc, argv, "hed:?i:?f:?Hrp:?m:?s:?")) != -1)
	{
		switch(opt)
		{
			case 'd':
				if(arguments->device)
				{
					throw "Cannot open more than one device at the same time\n";
				}	
				arguments->device = optarg;
#ifdef DEBUG
				printf ("Device to open: %s \n", arguments->device);	
#endif
				break;
			case 'i':
				if(arguments->protocol)
				{
					throw "Cannot use more than one protocol at the same time\n";
				}	
#ifdef DEBUG
				printf ("Protocol used to program memory: %s \n", optarg);
#endif
				if ((strncmp (optarg, "spi", 3)) == 0)
					arguments->action = ACT_SPI_PROG;
				else if ((strncmp (optarg, "jtag", 4)) == 0)
					arguments->action = ACT_JTAG_PROG;
				else 
				{
					throw ("Unknown protocol, use spi or jtag \n");
				}
				break;
			case 'f':
				if(arguments->source_file)
				{
					throw "Cannot open more than one file at the same time\n";
				}	
				arguments->source_file = optarg;				
				if(arguments->source_file != NULL)
				{
					tmp_file = fopen(arguments->source_file, "r");
					if(tmp_file == NULL)
					{
						throw "Cannot open source file.\n";
					}
					else
					{
						fclose(tmp_file);
					}
				}
#ifdef DEBUG			
				printf ("File to open: %s \n", optarg);	
#endif
				break;
			case 'p':
				if(arguments->memory_number != DO_NOT_SELECT_MEMORY)
				{
					throw "Cannot read more than one PROM number at the same time\n";
				}

				printf ("Switch to PROM number: %s \n", optarg);	

				arguments->memory_number = strtol (optarg, NULL, 10);	
				if(arguments->memory_number<=0)
				{
					throw "Provide correct PROM number\n";
				}
				if ((arguments->memory_number <= 0) || (arguments->memory_number  > 3))
				{
					throw "Provide correct PROM number\n";
				}
				if (arguments->action == NULL)
				{
					arguments->action = ACT_SPI_PROG;
				}
#ifdef DEBUG
				printf ("PROM number: %d \n", arguments->memory_number);	
#endif
				break;
			case 's':
				if(arguments->slot_number)
				{
					throw "Cannot read more than one slot number at the same time\n";
				}
#ifdef DEBUG
				printf ("Board in slot: %s \n", optarg);	
#endif
				arguments->slot_number = strtol (optarg, NULL, 10);	
				if ((arguments->slot_number <= 0) || (arguments->slot_number  > 12))
				{
					throw "Provide correct slot number\n";
				}
#ifdef DEBUG
				printf ("Slot number: %d \n", arguments->memory_number);	
#endif
				extract_slot_number = 0; 
				break;

			case 'H':
				arguments->hotplug = 1;
#ifdef DEBUG
				printf ("Use hot-plug \n");	
#endif
				if (arguments->action == NULL)
				{
					arguments->action = ACT_SPI_PROG;
				}
				break;
			case 'r':
				arguments->reload = 1;
#ifdef DEBUG
				printf ("Reload FPGA \n");	
#endif	
				break;
			case 'm':
#ifdef DEBUG
				printf ("MCH name: %s \n", optarg);
#endif
				arguments->hostname = optarg;
				break;
			case 'e':
				show_example = 1;
				usage (argv[0], show_example);
				exit (0);
				break;
			case 'h':
				show_example = 1;
				usage (argv[0], show_example);
				exit (0);
				break;	
			default:		/* '?' */
				usage (argv[0], show_example);
				exit (0);
		}
		fflush(stdout);
	}

	if(arguments->device != NULL)
	{
		/* tmp_file = fopen(arguments->device, "r"); 
		if(tmp_file == NULL)
		{
			printf ("Opening device: %s\n", arguments->device);
			throw "Cannot open the device for programming. Maybe the driver is not loaded nor you do not have rights to open the device.";
		}
		else
		{
			fclose(tmp_file);
		}*/
	}
	else 
	{
		throw "Cannot make any actions without provided device";
	}
	
	// Input parameters debug info
	PrintAllParameters (arguments);
	
	if (arguments->device != NULL)
	{
		if      (source_file = strstr (arguments->device, "adcs")) 
		{
			if (arguments->action == ACT_JTAG_PROG)
			{
				arguments->action = ACT_SIS_JTAG_PROG;
				arguments->board = SIS8300;
#if 1
				printf ("Programming SIS8300, device %s \n", source_file);
#endif
			}
			if (arguments->action == ACT_SPI_PROG)
			{
				arguments->board = SIS8300L;
#if 1
				printf ("Programming SIS8300L, device %s \n", source_file);
#endif
			}
		}
		else if (source_file = strstr (arguments->device, "utcs")) 
		{
			if (arguments->action == ACT_JTAG_PROG)
			{
				arguments->action = ACT_UVM_JTAG_PROG;
				arguments->board = uVM;	
#if 1
				printf ("Programming uVM, device %s \n", source_file);
#endif
			}
			if (arguments->action == ACT_SPI_PROG)
			{
				arguments->board = uTC;
#if 1
				printf ("Programming uTC, device %s \n", source_file);
#endif	
			}
		}
		else 
			throw "Unknown device, only DAMC-TCK7 (llrfutcs) and DRTM-VM2 are supported";
	}
	
// Extract slot number 		
	if (extract_slot_number == 1)
	{
		arguments->slot_number = strtol (source_file+4, NULL, 10);
		if ((arguments->slot_number <= 0) || (arguments->slot_number > 12))
		{
			throw ("Only slots between 1 and 12 can be opened \n");
		}
#ifdef DEBUG 
		printf ("Slot number: %d \n", arguments->slot_number);
#endif
		fflush (stdout);	
	}

	

#if 0	
	//if hostname is not specified
	if(hostname == NULL)
	{
		fprintf(stderr, "Please specify IP address of MCH\n\n");
		usage (argv[0], show_example);
	}
	
	//if no action is specified
	if(!recover && !program && !select)
    {
		fprintf(stderr, "Please specify an action to perform\n\n");
		usage (argv[0], show_example);    
    }
	
	//if two actions are specified at the same time
	if((recover && program) || (recover && select) || (program && select))
    {
		fprintf(stderr, "Do not use more than one function at the same time\n\n");
		usage (argv[0], show_example);
    }
	
	switch(board_type)
	{
		case uTC:
			switch(device_type)
			{
				case DEV_FPGA:
					//check if arguments number is correct
					if(program)
					{
						if (optind != argc - 3)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number, memory number and bit file\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_MEM_PROG;
						slot_number = atoi(argv[optind]);
						memory_number = atoi(argv[optind + 1]);
						source_file = argv[optind + 2];
					}
					else if(recover)
					{
						if (optind != argc - 1)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_MEM_REC;
						slot_number = atoi(argv[optind]);
					}
					else if(select)
					{
						if(optind != argc - 2)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number and memory number\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_MEM_SWITCH;
						slot_number = atoi(argv[optind]);
						memory_number = atoi(argv[optind + 1]);
					}
					break;
				case DEV_DSP:
					//check if arguments number is correct
					if(program)		//direct programming via PCIe
					{
						if (optind != argc - 2)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number and ldr file\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_DSP_PCIE;
						slot_number = atoi(argv[optind]);
						source_file = argv[optind + 1];
					}
					else if(recover)		//rebooting DSP from SPI memory
					{
						if (optind != argc - 1)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_DSP_REC;
						slot_number = atoi(argv[optind]);
					}
					else if(select)		//programming memory for DSP
					{
						if(optind != argc - 2)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number and ldr file\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_DSP_MEM;
						slot_number = atoi(argv[optind]);
						source_file = argv[optind + 1];
					}
					break;
				case DEV_RTM:
					//check if arguments number is correct
					if(program)		//direct programming via PCIe
					{
						if (optind != argc - 2)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number and xsvf file\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_UTC_RTM_PROG;
						slot_number = atoi(argv[optind]);
						source_file = argv[optind + 1];
					}
					else
					{
						fprintf(stderr, "Only programming option is supported by RTM\n\n");
						usage (argv[0], show_example);
					}  
					break;
				default:
					fprintf(stderr, "Please specify type of device - FPGA, DSP or RTM\n\n");
					usage (argv[0], show_example);    
					break;
			}
			break;




		case SIS8300L:
			if(program)
			{
				if (optind != argc - 3)
						{
							fprintf(stderr, "Wrong number of arguments\n");
							fprintf(stderr, "Please specify slot number, memory number and bit file\n\n");
							usage (argv[0], show_example);
						}
						action = ACT_SISL_MEM_PROG;
						slot_number = atoi(argv[optind]);
						memory_number = atoi(argv[optind + 1]);
						source_file = argv[optind + 2];
			}
			else if(recover)
			{
				if (optind != argc - 1)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_SIS_MEM_REC;
				slot_number = atoi(argv[optind]);
			}
			else if(select)
			{
				if(optind != argc - 2)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number and memory number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_SIS_MEM_SWITCH;
				slot_number = atoi(argv[optind]);
				memory_number = atoi(argv[optind + 1]);
			}
			break;



		case SIS8300:
			//check if arguments number is correct
			if(program)
			{
				if (optind != argc - 3)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number, memory number and xsvf file\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_SIS_MEM_PROG;
				slot_number = atoi(argv[optind]);
				memory_number = atoi(argv[optind + 1]);
				source_file = argv[optind + 2];
			}
			else if(recover)
			{
				if (optind != argc - 1)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_SIS_MEM_REC;
				slot_number = atoi(argv[optind]);
			}
			else if(select)
			{
				if(optind != argc - 2)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number and memory number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_SIS_MEM_SWITCH;
				slot_number = atoi(argv[optind]);
				memory_number = atoi(argv[optind + 1]);
			}
			break;
		case DAMC2:
			//check if arguments number is correct
			if(program)
			{
				if (optind != argc - 3)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number, memory number and xsvf file\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_DAMC2_MEM_PROG;
				slot_number = atoi(argv[optind]);
				memory_number = atoi(argv[optind + 1]);
				source_file = argv[optind + 2];
			}
			else if(recover)
			{
				if (optind != argc - 1)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_DAMC2_MEM_REC;
				slot_number = atoi(argv[optind]);
			}
			else if(select)
			{
				if(optind != argc - 2)
				{
					fprintf(stderr, "Wrong number of arguments\n");
					fprintf(stderr, "Please specify slot number and memory number\n\n");
					usage (argv[0], show_example);
				}
				action = ACT_DAMC2_MEM_SWITCH;
				slot_number = atoi(argv[optind]);
				memory_number = atoi(argv[optind + 1]);
			}
			break;
		default:
			fprintf(stderr, "Please specify type of board - uTC, SIS8300 or DAMC2\n\n");
			usage (argv[0], show_example);
			break;
	}
	
	arguments.hostname = hostname;
	arguments.hotplug = hotplug;
	arguments.action = action;
	arguments.slot_number = slot_number;
	arguments.memory_number = memory_number;
	arguments.source_file = source_file;
	
	//check if slot number is correct
	if(slot_number < 1 || slot_number > 12)
	{
		throw "Wrong slot number\n";
	}
	

#endif
	//check if source file exists
	if(arguments->source_file != NULL)
	{
		tmp_file = fopen(arguments->source_file, "r");
		if(tmp_file == NULL)
		{
			throw "Cannot open source file. Maybe the file does not exist\n";
		}
		else
		{
			fclose(tmp_file);
		}
	}
	
//	return arguments;
}

void init_arguments (arguments_t *arguments){

	arguments->memory_number = -1;
	arguments->slot_number = 0;
	arguments->hotplug = 0;
	arguments->hostname = NULL;
	arguments->device = NULL;
	arguments->protocol=0;
	arguments->reload=0;
	arguments->action=ACT_NONE;
	arguments->source_file=NULL;
}

int
main (int argc, char *argv[])
{
	show_example = 0;
	arguments_t arguments;
	char hostname[128] ={0};
	init_arguments (&arguments);
	char* mchSubName = NULL;


	if (argc == 1) 	usage (argv[0], show_example);
	
	try
    {
		printf("\nSimple llrf_prog ver. %s\n", VERSION);
		gethostname(hostname, 127);
		printf ("Hostname: %s \n", hostname);
	
		decode_arguments(argc, argv, &arguments);

		if (arguments.hostname == NULL){
	//	arguments.hostname = hostname;
		mchSubName = strstr (hostname, "cpu");
		strncpy (mchSubName, "mch", 3);
		arguments.hostname = hostname;
		printf("MCH name recovered from hostname: %s, %s \n", hostname, arguments.hostname);
}
		printf("MCH name: %s \n", arguments.hostname);
	
		//print arguments
		printf("Programing PROM of %s device\n", arguments.device);
		if(arguments.source_file) 
		{
			printf("Source file: %s\n", arguments.source_file);
		}
		printf("PCIe hotplug: %s\n\n", arguments.hotplug ? "ON" : "OFF"); 
	

	
#if 0
		int tmp, tmp2 =0;
		char * results=NULL, *out=NULL;
		//tmp = sscanf (arguments.device, "%d", &tmp2);
		if (results = strstr (arguments.device, "adcs")) printf ("Device %s \n", results);
		else if (results = strstr (arguments.device, "utcs"))  printf ("Device %s \n", results);
 
		tmp2=atoi (results);
		tmp=strtol (results+4, &out, 10);
		printf ("Slot: %d \n", tmp2);
		printf ("Slot2: %d \n", tmp);
		fflush (stdout);	
#endif


		switch(arguments.action)
		{
			case ACT_SPI_PROG:
				program_SPI_prom (&arguments);
				break;	
			case ACT_SIS_JTAG_PROG:
				program_JTAG_prom (&arguments); //.device, arguments.source_file, arguments.hotplug, arguments.hostname, arguments.slot_number);
				break;	
			case ACT_UVM_JTAG_PROG:
				program_JTAG_prom (&arguments); //.device, arguments.source_file, arguments.hotplug, arguments.hostname, arguments.slot_number);
				break;	


#if 0
			case ACT_UTC_MEM_PROG:
				uTC_mem_program(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.source_file, arguments.hotplug);
				break;
			case ACT_UTC_MEM_REC:
				uTC_mem_recover(arguments.hostname, arguments.slot_number, arguments.hotplug);
				break;
			case ACT_UTC_MEM_SWITCH:		
				uTC_mem_select(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.hotplug);
				break;
			case ACT_UTC_DSP_PCIE:
				uTC_DSP_PCIE_program(arguments.hostname, arguments.slot_number, arguments.source_file);
				break;
			case ACT_UTC_DSP_REC:
				uTC_DSP_recover(arguments.hostname, arguments.slot_number);
				break;
			case ACT_UTC_DSP_MEM:
				uTC_DSP_MEM_program(arguments.hostname, arguments.slot_number, arguments.source_file);
				break;
			case ACT_UTC_RTM_PROG:
				uTC_RTM_MEM_program(arguments.hostname, arguments.slot_number, arguments.source_file);
				break;

			case ACT_SISL_MEM_PROG:
				SISL_mem_program(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.source_file, arguments.hotplug);
				break;
				
				
			case ACT_SIS_MEM_PROG:
				SIS_mem_program(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.source_file, arguments.hotplug);
				break;
			case ACT_SIS_MEM_REC:
				SIS_mem_recover(arguments.hostname, arguments.slot_number, arguments.hotplug);
				break;
			case ACT_SIS_MEM_SWITCH:
				SIS_mem_select(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.hotplug);
				break;
			case ACT_DAMC2_MEM_PROG:
				DAMC2_mem_program(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.source_file, arguments.hotplug);
				break;
			case ACT_DAMC2_MEM_REC:
				DAMC2_mem_recover(arguments.hostname, arguments.slot_number, arguments.hotplug);
				break;
			case ACT_DAMC2_MEM_SWITCH:
				DAMC2_mem_select(arguments.hostname, arguments.slot_number, arguments.memory_number, arguments.hotplug);
				break;
#endif
			default:
				break;
		}
	}
    catch (const char *exc)		//handle the exceptions
	{
		fprintf (stderr, "\n \nError: %s\n", exc);
		return 1;
	}
	
	return 0;
}
