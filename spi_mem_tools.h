#ifndef _SPI_TOOLS_
#define _SPI_TOOLS_

int regAddress (int reg);
void wait_for_spi (int ttyDesc);
void write_disable (int ttyDesc);
void write_enable (int ttyDesc);
unsigned int read_status (int ttyDesc);
void bulk_erase_mem (int ttyDesc);
void page_program (int ttyDesc, unsigned int address, unsigned int size, char *buffer);
void read_mem_block (int ttyDesc, unsigned int address, char *buffer, unsigned int size);
void fast_read_mem (int ttyDesc, const char *filename);
int verify_mem (int ttyDesc, const char *filename);
int verify_dsp_mem (int ttyDesc, const char *filename);
void program_mem (int ttyDesc, const char *filename);
void program_dsp_mem(int ttyDesc, const char *filename);
int checkId(int ttyDesc);

#endif /*_SPI_TOOLS_*/
