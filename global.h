#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define DEBUG
#define DO_NOT_SELECT_MEMORY -1
#define UTC_PROGRAMMER_MEM		1
#define UTC_APPLICATION_MEM		0
#define UTC_DEFAULT_DSP_MEM			3
#define SIS_DEFAULT_PROGRAMMER_REV		0
#define DAMC2_DEFAULT_PROGRAMMER_REV		3

//bits in FPGA control register
#define SPI_START		0x00000001	//start spi operation
#define SPI_R_NW		0x00000002	//SPI read/write (0 - write, 1 - read)
#define DSP_SPI_BOOT		0x00000008	//DSP_boot source (1 - boot from SPI)
#define SPI_PROG		0x00000010	//0 - DSP, 1 - SPI
#define PCIE_V5			0x00000020	//PCIE mux (0 - RTM, 1 - V5)
#define SPIREADER_RESET		0x08000000
#define DSP_RESET		0x80000000

#endif /*_GLOBAL_H_*/
