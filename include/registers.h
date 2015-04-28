#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#define PROG_DEFAULT_ADDRESS     65536
#define PROG_DEFAULT_BAR         0

/* Addresses of programmer registers                                
 * All addressses are relative to base offset that can be specified 
 * as an argument of the application
 */
#define AREA_WRITE              0x000000
#define AREA_READ               0x001000
#define REG_SPI_DIVIDER         0x002000
#define REG_BYTES_TO_WRITE      0x00200C
#define REG_BYTES_TO_READ       0x002010
#define REG_CONTROL             0x002014
#define REG_TCK                 0x002018
#define REG_TMS                 0x00201C
#define REG_TDI                 0x002020
#define REG_TDO                 0x002024
#define REG_REV_SWITCH          0x00203C
#define REG_REV_SEL             0x002040

/*********************************************************************/
/* BITS                                                              */
/*********************************************************************/
// Control register (REG_CONTROL)
#define SPI_START		0x00000001	//start spi operation
#define SPI_R_NW		0x00000002	//SPI read/write (0 - write, 1 - read)
#define DSP_SPI_BOOT		0x00000008	//DSP_boot source (1 - boot from SPI)
#define SPI_PROG		0x00000010	//0 - DSP, 1 - SPI
#define PCIE_V5			0x00000020	//PCIE mux (0 - RTM, 1 - V5)
#define SPIREADER_RESET		0x08000000
#define DSP_RESET		0x80000000


#endif /*_REGISTERS_H_*/
