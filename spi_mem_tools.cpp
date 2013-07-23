#include "spi_mem_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "II_interface.h"
#include "ii_constants.h"
#include "global.h"
#include "progress_bar.h"

/** read Integral Interface register with a specific identifier */
int
regAddress (int reg)
{
  return ii_c[reg - 1].addresses[0];
}

/** wait until SPI data is transmitted to the device and the response is received from the device */
void
wait_for_spi (int ttyDesc)
{
  unsigned int data;
  unsigned int i=0;
  do
    {
      readRegister (ttyDesc, regAddress (control), &data);
      usleep(1);
      if(i++ == 10000) throw("Timeout waiting for SPI completion\nTry rescanning PCIe bus\n");
    }
  while (data & 1);
}

/** write-protect the SPI flash */
void
write_disable (int ttyDesc)
{
  writeRegister (ttyDesc, regAddress (bytes_write), 0x0);
  writeRegister (ttyDesc, regAddress (write_data), 0x04);
  //writeRegister (ttyDesc, regAddress (control), 0x01 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG  | SPI_START));
  wait_for_spi (ttyDesc);
}

/** enable write possibility in the SPI flash */
void
write_enable (int ttyDesc)
{
  writeRegister (ttyDesc, regAddress (bytes_write), 0x0);
  writeRegister (ttyDesc, regAddress (write_data), 0x06);
  //writeRegister (ttyDesc, regAddress (control), 0x01 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG  | SPI_START));
  wait_for_spi (ttyDesc);
}

/** read status register from the SPI flash */
unsigned int
read_status (int ttyDesc)
{
  unsigned int data;
  writeRegister (ttyDesc, regAddress (bytes_read), 0x0);
  writeRegister (ttyDesc, regAddress (write_data), 0x05);
//  writeRegister (ttyDesc, regAddress (control), 0x03 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
  wait_for_spi (ttyDesc);

  readRegister (ttyDesc, regAddress (read_data), &data);

  return data;
}

/** bulk erase the entire SPI flash memory */
void
bulk_erase_mem (int ttyDesc)
{
  unsigned int data;
  double progress = 0;

  printf ("\nStarting bulk erase\n");

  writeRegister (ttyDesc, regAddress (write_data), 0xc7);
//  writeRegister (ttyDesc, regAddress (control), 0x01 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control),(PCIE_V5 | SPI_PROG | SPI_START)); 
  wait_for_spi (ttyDesc);

  do
    {
      data = read_status (ttyDesc);
	  if(progress < 66)
	  {
		ProgressBar(65, progress++);
		sleep(1);
	  }
    }
  while (data & 1);
  ProgressBar(65, 65);		//progress bar = 100%
  printf("\n");
}

/** program the page in the SPI flash memory from the contents of buffer with the specific size, 
 * starting from the specified address  */
void
page_program (int ttyDesc, unsigned int address, unsigned int size,
	      char *buffer)
{
  unsigned int data;

//    printf("Programming flash at address %x\n",address);

  writeRegister (ttyDesc, regAddress (write_data), 0x02);
  writeRegister (ttyDesc, regAddress (write_data) + 4,
		 (address >> 16) & 0xff);
  writeRegister (ttyDesc, regAddress (write_data) + 8, (address >> 8) & 0xff);
  writeRegister (ttyDesc, regAddress (write_data) + 12,
		 (address >> 0) & 0xff);
  for (unsigned int i = 0; i < size; i++)
  {
    writeRegister (ttyDesc, regAddress (write_data) + 16 + 4 * i, buffer[i]);
    //printf("Data: 0x%x %d\n", buffer[i], i );
    }

  writeRegister (ttyDesc, regAddress (bytes_write), size + 4 - 1);
 // writeRegister (ttyDesc, regAddress (control), 0x01 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control),(PCIE_V5 | SPI_PROG | SPI_START));
  wait_for_spi (ttyDesc);
  do
    {
      data = read_status (ttyDesc);
    }
  while (data & 1);

}

/** read the memory block from the specified address to the buffer of the specified size */
void
read_mem_block (int ttyDesc, unsigned int address, char *buffer,
		unsigned int size)
{
  unsigned int data;
  writeRegister (ttyDesc, regAddress (bytes_write), 0x4);	// 4 bytes + dummy
  writeRegister (ttyDesc, regAddress (bytes_read), size - 1);

  writeRegister (ttyDesc, regAddress (write_data), 0x0b);
  writeRegister (ttyDesc, regAddress (write_data) + 4,
		 (address >> 16) & 0xff);
  writeRegister (ttyDesc, regAddress (write_data) + 8, (address >> 8) & 0xff);
  writeRegister (ttyDesc, regAddress (write_data) + 12,
		 (address >> 0) & 0xff);

  //writeRegister (ttyDesc, regAddress (control), 0x03 + 0x08 + 0x20);
  writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
  wait_for_spi (ttyDesc);
  for (unsigned int i = 0; i < 1024; i++)
    {
      readRegister (ttyDesc, regAddress (read_data) + 4 * i, &data);
      buffer[i] = data;
    }
}

/** read 8 megabytes of the SPI flash to the file */ 
void
fast_read_mem (int ttyDesc, const char *filename)
{
  unsigned int data;
  unsigned int address;
  char buffer[1024];
  FILE *f = fopen (filename, "wb+");
  if (!f)
    throw "cannot open readback file";

  for (address = 0; address < 64 * 1024 * 1024 / 8; address += 1024)
    {
      writeRegister (ttyDesc, regAddress (bytes_write), 0x4);	// 4 bytes + dummy
      writeRegister (ttyDesc, regAddress (bytes_read), 1023);

      writeRegister (ttyDesc, regAddress (write_data), 0x0b);
      writeRegister (ttyDesc, regAddress (write_data) + 4,
		     (address >> 16) & 0xff);
      writeRegister (ttyDesc, regAddress (write_data) + 8,
		     (address >> 8) & 0xff);
      writeRegister (ttyDesc, regAddress (write_data) + 12,
		     (address >> 0) & 0xff);

//  wait for operation end
      //writeRegister (ttyDesc, regAddress (control), 0x03 + 0x08 + 0x20);
      writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
      wait_for_spi (ttyDesc);

//    printf("Memory dump:\n");
      for (unsigned int i = 0; i < 1024; i++)
	{
	  readRegister (ttyDesc, regAddress (read_data) + 4 * i, &data);
	  buffer[i] = data;
	}
      if (1024 != fwrite (buffer, 1, 1024, f))
	throw ("Error writing readback file");
    }

  fclose (f);

}

/** Verify if the flash memory contains the data from the specific file. 
   Return 0 on success, 1 on failure, throw an exception on other errors */
int
verify_mem (int ttyDesc, const char *filename)
{
	unsigned char buffer[1024];
	unsigned int data;
	unsigned int bread;
	unsigned int addr = 0;
	unsigned int file_size;
	FILE *f = fopen (filename, "rb");

	printf ("\nStarting verification\n");

	if (!f)
		throw "cannot open input file";

	fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
	do
    {
		bread = fread (buffer, 1, 1024, f);
		if (bread == 0)
		{
			printf ("\nverified %d bytes\n\n", addr);
			if (ferror (f))
				throw ("Error reading input file");
			fclose (f);
			return 0;
		}

		writeRegister (ttyDesc, regAddress (bytes_write), 0x4);	// 4 bytes + dummy
		writeRegister (ttyDesc, regAddress (bytes_read), bread-1);

		writeRegister (ttyDesc, regAddress (write_data), 0x0b);
		writeRegister (ttyDesc, regAddress (write_data) + 4,
		     (addr >> 16) & 0xff);
		writeRegister (ttyDesc, regAddress (write_data) + 8,
		     (addr >> 8) & 0xff);
		writeRegister (ttyDesc, regAddress (write_data) + 12,
		     (addr >> 0) & 0xff);

		//  wait for operation end
		//writeRegister (ttyDesc, regAddress (control), 0x03 + 0x08 + 0x20);
		writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
		wait_for_spi (ttyDesc);
	
		for (unsigned int i = 0; i < 1024; i++)
		{
			readRegister (ttyDesc, regAddress (read_data) + 4 * i, &data);
			if(buffer[i] != (data & 0xff))
			{
				fprintf(stderr,"\nVerify error at address %u: read 0x%02x instead of 0x%02x\n",
					  addr+i, (data & 0xff), buffer[i]);
				return 1;
			}
		}
		addr = addr + bread;
		ProgressBar(file_size, addr);
    }
	while (1);
}

/** Verify if the flash memory contains the data from the specific file. 
   Return 0 on success, 1 on failure, throw an exception on other errors */
int
verify_dsp_mem (int ttyDesc, const char *filename)
{
  unsigned char buffer[1024];
  unsigned int data;
  unsigned int bread;
  unsigned int addr = 0;
  unsigned int size = 0, word_cnt = 0;
  FILE *f = fopen (filename, "rb");

  printf ("\nStarting verification\n");

  if (!f)
    throw "cannot open input file";

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    word_cnt = size/4;
    while(word_cnt%4 != 0)
    {
        word_cnt++;
    }
#ifdef DEBUG
    printf("word_cnt: %d\n", word_cnt);
#endif

    buffer[0] = word_cnt & 0xFF;
    buffer[1] = (word_cnt >> 8) & 0xFF;
    buffer[2] = (word_cnt >> 16) & 0xFF;
    buffer[3] = (word_cnt >> 24) & 0xFF;

  do
    {
      if(!addr)
      {
        bread = fread (buffer+4, 1, 1020, f);
        bread += 4;
	}
	else
	{
	bread = fread (buffer, 1, 1024, f);
	}
      if (bread == 0)
	{
	  ProgressBar(size, size);
	  printf ("\nverified %d bytes\n\n", addr);
	  if (ferror (f))
	    throw ("Error reading input file");
	  fclose (f);
	  return 0;
	}

      writeRegister (ttyDesc, regAddress (bytes_write), 0x4);	// 4 bytes + dummy
      writeRegister (ttyDesc, regAddress (bytes_read), bread-1);

      writeRegister (ttyDesc, regAddress (write_data), 0x0b);
      writeRegister (ttyDesc, regAddress (write_data) + 4,
		     (addr >> 16) & 0xff);
      writeRegister (ttyDesc, regAddress (write_data) + 8,
		     (addr >> 8) & 0xff);
      writeRegister (ttyDesc, regAddress (write_data) + 12,
		     (addr >> 0) & 0xff);

//  wait for operation end
      //writeRegister (ttyDesc, regAddress (control), 0x03 + 0x08 + 0x20);
      writeRegister (ttyDesc, regAddress (control), (PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
      wait_for_spi (ttyDesc);
	
      for (unsigned int i = 0; i < 1024; i++)
	{
	  readRegister (ttyDesc, regAddress (read_data) + 4 * i, &data);
	  if(buffer[i] != (data & 0xff))
	    {
	      fprintf(stderr,"\nVerify error at address %u: read 0x%02x instead of 0x%02x\n",
	              addr+i, (data & 0xff), buffer[i]);
	              return 1;
	    }
	}
	  ProgressBar(size, addr);
      addr = addr + bread;
    }
  while (1);
}

/** Program the flash memory with the contents of the specific file */
void
program_mem (int ttyDesc, const char *filename)
{
  char buffer[256];
  unsigned int bread;
  unsigned int addr = 0;
  unsigned int file_size;
  FILE *f = fopen (filename, "rb");

  printf ("\nStarting programming\n");

  if (!f)
    throw "cannot open input file";

	fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

	do
	{
		bread = fread (buffer, 1, 256, f);
		if (bread == 0)
		{
			printf ("\nprogrammed %d bytes\n", addr);
			if (ferror (f))
				throw ("Error reading input file");
			fclose (f);
			return;
		}
		write_enable (ttyDesc);
		page_program (ttyDesc, addr, bread, buffer);
		addr = addr + bread;
		ProgressBar(file_size, addr);
    }
	while (1);
}

/** Program the flash memory for DSP booting with the contents of the specific file */
void
program_dsp_mem (int ttyDesc, const char *filename)
{
	char buffer[256];
	unsigned int bread;
	unsigned int addr = 0;
	unsigned int word_cnt = 540;
	unsigned int size;
	FILE *f = fopen (filename, "rb");

	printf ("\nStarting programming\n");

	if (!f)
		throw "cannot open input file";

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	word_cnt = size/4;
	while(word_cnt%4 != 0)
	{
	    word_cnt++;
	}
#ifdef DEBUG
	printf("word_cnt: %d\n", word_cnt);
#endif

	buffer[0] = word_cnt & 0xFF;
	buffer[1] = (word_cnt >> 8) & 0xFF;
	buffer[2] = (word_cnt >> 16) & 0xFF;
	buffer[3] = (word_cnt >> 24) & 0xFF;
	do
	{
		if(!addr)
		{
		    bread = fread (buffer+4, 1, 252, f);
		    bread = bread + 4;
		}
		else
		{
		    bread = fread (buffer, 1, 256, f);
		}
		
		if (bread == 0)
		{
			if (ferror (f))
				throw ("Error reading input file");
			fclose (f);
			if((addr % 16) != 0)
			{
				bread = 16 - (addr % 16); 
				memset(buffer, 0, 256);
				write_enable (ttyDesc);
				page_program (ttyDesc, addr, bread, buffer);
				ProgressBar(size, addr);
			}
			printf ("\nprogrammed %d bytes\n", addr);
			return;
		}
		write_enable (ttyDesc);
		page_program (ttyDesc, addr, bread, buffer);
		ProgressBar(size, addr);
		addr = addr + bread;
    }
    while (1);
}

/** Check if the SPI flash is present. Return 1 on success, 0 on failure.
   Please note that it may return failure if the PROM is busy e.g. erasing the memory block. */
int checkId(int ttyDesc)
{
    unsigned int data[5];
    unsigned int i;

for(i=0;i<2;i++)   // first read returns garbage
{
    writeRegister(ttyDesc,regAddress(bytes_write),0x00);
    writeRegister(ttyDesc,regAddress(bytes_read),0x04);
    
    writeRegister(ttyDesc,regAddress(write_data),0x9f);

    //writeRegister(ttyDesc,regAddress(control),0x03 + 0x08 + 0x20);
    writeRegister(ttyDesc,regAddress(control),(PCIE_V5 | SPI_PROG | SPI_R_NW | SPI_START));
    wait_for_spi(ttyDesc);
}
   printf("SPI prom ID: ");

// read 3 bytes
    for(i=0; i<5; i++)
    {
      readRegister(ttyDesc,regAddress(read_data) + 4*i,data + i);
      data[i] &= 0xff;
      printf("%02x ",data[i]);
    }
    
    printf("\n");
    if (((data[0] == 0x01) && (data[1] == 0x20) &&
        (data[2] == 0x18) && (data[3] == 0x03 || data[3] == 0x4D)  && (data[4] == 0x01)) ||
  	((data[0] == 0xef) && (data[1] == 0x40) && (data[2] == 0x17) && (data[3] == 0x00) && (data[4] == 0x00)))
      return 1;
    else
      return 0;
}
