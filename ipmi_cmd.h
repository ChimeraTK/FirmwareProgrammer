#ifndef _IPMI_CMD_H_
#define _IPMI_CMD_H_

#include "ipmitool_wrapper.h"

#define JTAG_SPI_CONF_REG	0x03

#define JTAG_MMC			7
#define JTAG_RTM			5

void set_PROM_number(char* hostname, uint8_t slot_number, uint8_t memory_number, uint8_t board_type);
void get_PROM_number(char* hostname, uint8_t slot_number, uint8_t* memory_number);
void write_CPLD_register(char* hostname, uint8_t slot_number, uint8_t reg_addr, uint8_t value);
void read_CPLD_register(char* hostname, uint8_t slot_number, uint8_t reg_addr, uint8_t *value);
void configure_JTAG_chain(char* hostname, uint8_t slot_number, uint8_t board_type);
void FRU_reset(char* hostname, uint8_t slot_number, uint8_t board_type);
void dsp_reset(char* hostname, uint8_t slot_number);

#endif /*_IPMI_CMD_H_*/
