#ifndef __II_CONSTANTS_H__
#define __II_CONSTANTS_H__

#define II_MAX_ADDRESSES 1
#define II_DATA_SIZE 32
#define spi_divider 1
#define selector 2
#define write_data 3
#define read_data 4
#define ready_data 5
#define bytes_write 6
#define bytes_read 7
#define control 8
#define tck 9
#define tms 10
#define tdi 11
#define tdo 12
#define dsp_data 13
#define dsp_control 14
#define dsp_status 15
#define dsp_debug_data 16
enum ItemType {VII_PAGE,VII_VECT,VII_BITS,VII_WORD,VII_AREA};

struct reginfo
{
unsigned item_type;
const char* reg_name;
unsigned reg_id;
unsigned reg_bits;
unsigned items;
unsigned reg_words;
unsigned bit_position;
unsigned parent_id;
unsigned long addresses[II_MAX_ADDRESSES];
};

const struct reginfo ii_c[] ={
{VII_WORD,"spi_divider",spi_divider,32,1,1,0,0,{0x2000}},
{VII_WORD,"selector",selector,32,1,1,0,0,{0x2004}},
{VII_AREA,"write_data",write_data,8,1,1024,0,0,{0x0}},
{VII_AREA,"read_data",read_data,8,1,1024,0,0,{0x1000}},
{VII_WORD,"ready_data",ready_data,32,1,1,0,0,{0x2008}},
{VII_WORD,"bytes_write",bytes_write,32,1,1,0,0,{0x200c}},
{VII_WORD,"bytes_read",bytes_read,32,1,1,0,0,{0x2010}},
{VII_WORD,"control",control,32,1,1,0,0,{0x2014}},
{VII_WORD,"tck",tck,1,1,1,0,0,{0x2018}},
{VII_WORD,"tms",tms,1,1,1,0,0,{0x201c}},
{VII_WORD,"tdi",tdi,1,1,1,0,0,{0x2020}},
{VII_WORD,"tdo",tdo,1,1,1,0,0,{0x2024}},
{VII_WORD,"dsp_data",dsp_data,32,1,1,0,0,{0x2028}},
{VII_WORD,"dsp_control",dsp_control,32,1,1,0,0,{0x202c}},
{VII_WORD,"dsp_status",dsp_status,32,1,1,0,0,{0x2030}},
{VII_WORD,"dsp_debug_data",dsp_debug_data,32,1,1,0,0,{0x2034}},
};

#endif // __II_CONSTANTS_H__
