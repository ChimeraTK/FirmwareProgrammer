#include "ipmi_cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** Open connection with MCH, send command which selects specific memory/revision on the board in specific slot  
 * and close connection. The function throws exceptions when some errors occur. */
void set_PROM_number(char* hostname, uint8_t slot_number, uint8_t memory_number, uint8_t board_type)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_set_PROM_number_cmd( board_type, memory_number );
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot switch PROM memory, maybe no hardware in slot %d\n", slot_number);
		throw output;
	}
}

/** Open connection with MCH, send command that retrieves the number of selected memory from the board in specific slot 
 * and close connection. The function throws exceptions when some errors occur. (supported only by uTC board!!!) */
void get_PROM_number(char* hostname, uint8_t slot_number, uint8_t* memory_number)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_get_PROM_number_cmd(memory_number);
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot check memory number in slot %d\n", slot_number);
		throw output;
	}
}

/** Open connection with MCH, send command which writes the CPLD register  
 * and close connection. The function throws exceptions when some errors occur. */
void write_CPLD_register(char* hostname, uint8_t slot_number, uint8_t reg_addr, uint8_t value)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_write_reg_cmd( reg_addr, value );
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot write CPLD register, maybe no hardware in slot %d\n", slot_number);
		throw output;
	}
}

/** Open connection with MCH, send command which reads the CPLD register  
 * and close connection. The function throws exceptions when some errors occur. */
void read_CPLD_register(char* hostname, uint8_t slot_number, uint8_t reg_addr, uint8_t *value)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_read_reg_cmd( reg_addr, value );
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot read CPLD register, maybe no hardware in slot %d\n", slot_number);
		throw output;
	}
}

/** Open connection with MCH, send command that configures JTAG chain for memory programming 
 * and close connection. The function throws exceptions when some errors occur. (supported only by DAMC2 board!!!) */
void configure_JTAG_chain(char* hostname, uint8_t slot_number, uint8_t board_type)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_configure_JTAG_chain(board_type);
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot configure JTAG chain on the board in slot %d\n", slot_number);
		throw output;
	}
}

/** Open connection with MCH, send command that causes FPGA reboot and close connection.
 * The function throws exceptions when some errors occur. */
void FRU_reset(char* hostname, uint8_t slot_number, uint8_t board_type)
{
	char ret;
	static char output[128];
	
	if(ipmi_lib_init(hostname, slot_number) == -1)
	{
		throw "Cannot initialize IPMI library";
	}
	ret = ipmi_fru_reset(board_type);
	ipmi_lib_close();
	
	if( ret == -1 )
	{
		sprintf(output, "Cannot reboot payload in slot %d\n", slot_number);
		throw output;
	}
}
