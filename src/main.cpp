#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <dirent.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "registers.h"
#include "MtcaProgrammerBase.h"
#include "MtcaProgrammerSPI.h"
#include "MtcaProgrammerJTAG.h"

#include <mtca4u/MtcaMappedDevice/devBase.h>
#include <mtca4u/MtcaMappedDevice/devPCIE.h>

#define VERSION                 "3.1"
//#define DEBUG

using namespace std;

class ProgrammingInterface {    
public:
    enum InterfaceType {
        INTERFACE_NONE,
        INTERFACE_SPI,
        INTERFACE_JTAG
    };
    
    ProgrammingInterface() : mType(INTERFACE_NONE){};
    ProgrammingInterface(InterfaceType type) : mType(type){};
    std::string toString()
    {
        switch(mType)
        {
            case INTERFACE_NONE:
                return "NONE";
            case INTERFACE_SPI:
                return "SPI";
            case INTERFACE_JTAG:
                return "JTAG";
            default:
                return "Unknown";
        }
    }
    
    InterfaceType getType() {return mType;}
private:
    InterfaceType mType;
};

struct arguments_t{
    ProgrammingInterface interface;
    char* source_file;
    char* device;
    uint32_t address;
    uint8_t bar;
    
    arguments_t() : 
        interface(ProgrammingInterface(ProgrammingInterface::INTERFACE_NONE)),
        source_file(NULL),
        device(NULL),
        address(PROG_DEFAULT_ADDRESS),
        bar(PROG_DEFAULT_BAR)
    {
    }
    
    string toString()
    {
        ostringstream os;
        os << "Device: " << device << endl;
        os << "Interface: " << interface.toString() << endl;
        os << "File: " << source_file << endl;
        os << "Programmer:" << endl;
        os << "\taddress: " << address << endl;
        os << "\tbar:" << bar << endl;
        
        return os.str();
    }
};

/** explain the usage of the program */
void
usage (const char *progname) 
{
    fprintf (stderr, "\nLLRF PROM programmer for uTC/uVM/SIS8300/SIS8300L/DAMC2 boards\n");
    fprintf (stderr, "%s -d [device] -i [interface] -f [firmware file] -a [address]b[bar]\n\n", progname);

    fprintf (stderr, "Example: %s -d /dev/llrfutcs4 -i spi -f firmware.bin -a 16384b0\n\n", progname);  

    fprintf (stderr, "Supported interfaces:\n");  
    fprintf (stderr, "-i spi \n");   
    fprintf (stderr, "-i jtag\n\n");

    exit (1);
}

arguments_t decode_and_verify_arguments(int argc, char *argv[])
{
    int opt;
    FILE* tmp_file;
				
    arguments_t arguments;
    
#ifdef DEBUG
    printf ("Debug active, parsing arguments... \n");
    fflush (stdout);
#endif

//get and parse input options			
    while ((opt = getopt (argc, argv, "hi:f:d:a:")) != -1)
    {
        switch(opt)
        {
            case 'i':
                if(arguments.interface.getType() != ProgrammingInterface::INTERFACE_NONE)
                {
                    throw "Cannot use more than one interface at the same time.\n";
                }	
                if ((strncmp (optarg, "spi", 3)) == 0)
                    arguments.interface = ProgrammingInterface(ProgrammingInterface::INTERFACE_SPI);
                else if ((strncmp (optarg, "jtag", 4)) == 0)
                    arguments.interface = ProgrammingInterface(ProgrammingInterface::INTERFACE_JTAG);
                else 
                    throw ("Unknown protocol, use spi or jtag \n");
                
                break;
                    
            case 'f':
                if(arguments.source_file)
                {
                    throw "Cannot open more than one file at the same time.\n";
                }
                arguments.source_file = strdup(optarg);				
                break;
            
            case 'd':
                if(arguments.device)
                {
                    throw "Cannot open more than one device at the same time.\n";
                }	
                arguments.device = strdup(optarg);
                break;
            
            case 'a':
                {
                    std::string address_input(optarg);
                    //std::cout << "Address input: " << address_input << std::endl;
                    uint32_t b_counter = 0;
                    std::size_t b_position = address_input.find_first_of("b");
                    while(b_position != std::string::npos)
                    {
                        b_counter++;
                        b_position = address_input.find_first_of("b", b_position + 1);
                    }
                    if(b_counter != 1)
                        throw "Wrong format of programmer address.\n";
                    
                    b_position = address_input.find_first_of("b");

                    try{
                        std::string address_ch = address_input.substr(0, b_position);
                        std::string bar_ch = address_input.substr(b_position + 1, address_input.length() - b_position);

                        size_t idx;
                        uint32_t address = std::stoi(address_ch, &idx);
                        if(idx != address_ch.length())
                            throw "Wrong format of programmer address.\n";
                        
                        uint8_t bar = std::stoi(bar_ch, &idx);
                        if(idx != bar_ch.length())
                            throw "Wrong format of programmer address.\n";
                        
                        arguments.address = address;
                        arguments.bar = bar;
                    }
                    catch(...)
                    {
                        throw "Wrong format of programmer address.\n";
                    }
                    
                    break;
                }
                
            case 'h':
            default:		/* '?' */
                usage (argv[0]);
                exit (0);
        }
    }

    //Arguments verification
    if(arguments.device == NULL)
    {
        printf("Cannot make any actions without provided device.\n"
                "Please specify device for programming.\n");
        usage(argv[0]);
    }
    else
    {
        tmp_file = fopen(arguments.device, "r"); 
        if(tmp_file == NULL)
            throw "Cannot open the device for programming. Maybe the driver is not loaded nor you do not have rights to open the device.";
        else
            fclose(tmp_file);
    }

    if(arguments.interface.getType() == ProgrammingInterface::INTERFACE_NONE) 
    {
        printf("Programming interface (SPI/JTAG) is missing.\n"
                "Please specify the interface appropriate for the device.\n");
        usage(argv[0]);
    } 

    if(arguments.source_file == NULL )
    {
        printf("Source file is missing.\n"
                "Please specify the location of file with firmware.\n");
        usage(argv[0]);
    }
    else
    {
        tmp_file = fopen(arguments.source_file, "r");
        if(tmp_file == NULL)
            throw "Cannot open source file. Please check if file exists and you have rights to access it.\n";
        else
            fclose(tmp_file);
    }
        
#ifdef DEBUG    
    // Input parameters debug info
    cout << arguments.toString();
#endif
    
    return arguments;
}

int main (int argc, char *argv[])
{
    boost::shared_ptr<MtcaProgrammerBase> programmer;
        
    arguments_t arguments;
    
    try
    {
        cout << "\nmtca4u_fw_programmer ver. " << VERSION << endl;
        arguments = decode_and_verify_arguments(argc, argv);
                               	
        //print arguments
        printf("Programing PROM of %s device\n", arguments.device);
        if(arguments.source_file) 
        {
            printf("Source file: %s\n", arguments.source_file);
        }
        
        mtcaDevPtr dev = mtcaDevPtr(new mtca4u::devPCIE());
        dev->openDev(arguments.device);
                	
        switch(arguments.interface.getType())
        {
            case ProgrammingInterface::INTERFACE_SPI:
                programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerSPI(dev, arguments.address, arguments.bar));
                break;	
            case ProgrammingInterface::INTERFACE_JTAG:
                programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerJTAG(dev, arguments.address, arguments.bar));
                break;	
            default:
                throw "Unknown interface\n\n";
        }
        
        if(!programmer->checkFirmwareFile(arguments.source_file))
        {
            fprintf(stderr, "Incorrect firmware file\n");
        }
        programmer->erase();
        programmer->program(arguments.source_file);
        programmer->verify(arguments.source_file);
    }
    catch (const char *exc)		//handle the exceptions
    {
        fprintf (stderr, "\n \nError: %s\n", exc);
        return 1;
    }
	
    return 0;
}
