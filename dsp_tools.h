
#ifndef _DSP_TOOLS_H_
#define _DSP_TOOLS_H_

#include <netdb.h>

#define PCIE_BOOT	0
#define SPI_BOOT	1
	
void dsp_init_boot(int ttyDesc, uint8_t boot_source);
void dsp_reset(char* hostname, uint8_t slot_number);
void dsp_boot_file(int ttyDesc, const char* filename);
void dsp_write_data(int ttyDesc, uint32_t data);
void dsp_spi_reader_reset(int ttyDesc);

#endif /*_DSP_TOOLS_H_*/
