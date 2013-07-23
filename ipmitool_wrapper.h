#ifndef __IPMI_H_
#define __IPMI_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//board types
#define uTC		1
#define SIS8300		2
#define DAMC2		3
#define SIS8300L	4
#define uVM		5

//device types
#define DEV_FPGA	1
#define DEV_DSP		2
#define	DEV_RTM		3

int ipmi_lib_init(char* hostname, uint8_t slot_number);
int ipmi_set_PROM_number_cmd(uint8_t board_type, uint8_t memory_number);
int ipmi_get_PROM_number_cmd(uint8_t* memory_number);
int ipmi_write_reg_cmd(uint8_t reg_addr, uint8_t value);
int ipmi_read_reg_cmd(uint8_t reg_addr, uint8_t *value);
int ipmi_configure_JTAG_chain(uint8_t board_type);
int ipmi_fru_reset(uint8_t board_type);
int ipmi_get_device_ID_cmd();
void ipmi_lib_close();

#ifdef __cplusplus
}
#endif

#endif /*__IPMI_H_*/
