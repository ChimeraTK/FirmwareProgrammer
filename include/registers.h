#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#define PROG_DEFAULT_ADDRESS        65536
#define PROG_DEFAULT_BAR            0
#define PROG_DEFAULT_MODULE_NAME    "BOARD0.AREA_BOOT"

#define PROG_REG_AREA_SIZE          65536

/* Addresses of programmer registers                                
 * All addressses are relative to base offset that can be specified 
 * as an argument of the application
 * All below values are in words (4 bytes)
 */

#define AREA_WRITE              0x000000        //size 0-1023
#define AREA_READ               0x000400        //size 1024-2047
#define REG_SPI_DIVIDER         0x000800
#define REG_BYTES_TO_WRITE      0x000803
#define REG_BYTES_TO_READ       0x000804
#define REG_CONTROL             0x000805
#define REG_TCK                 0x000806
#define REG_TMS                 0x000807
#define REG_TDI                 0x000808
#define REG_TDO                 0x000809
#define REG_REV_SWITCH          0x00080F
#define REG_REV_SEL             0x000810

#define AREA_WRITE_SIZE         0x400
#define AREA_READ_SIZE          0x400

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
