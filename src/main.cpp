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

#include "version.h"

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
    std::string firmware_file_path;
    std::string device_name;
    bool device_name_raw;
    uint32_t address;
    uint8_t bar;
    std::string dmap_file_path;
    std::string map_file_path;
    std::string map_area_name;
    bool action_programming;
    bool action_verification;
    bool action_reboot;
    
    arguments_t() : 
        interface(ProgrammingInterface(ProgrammingInterface::INTERFACE_NONE)),
        firmware_file_path(),
        device_name(),
        device_name_raw(false),
        address(PROG_DEFAULT_ADDRESS),
        bar(PROG_DEFAULT_BAR),
        dmap_file_path(),
        map_file_path(),
        map_area_name(PROG_DEFAULT_MODULE_NAME),
        action_programming(false),
        action_verification(false),
        action_reboot(false)
    {
    }
    
    string toString()
    {
        ostringstream os;
        os << "Device: " << device_name << endl;
        os << "Interface: " << interface.toString() << endl;
        os << "Firmware file: " << firmware_file_path << endl;
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

arguments_t parse_arguments(int argc, char *argv[])
{
    int opt;
    arguments_t arguments;
    const std::string raw_prefix("sdm");
    
    //get and parse input options			
    while ((opt = getopt (argc, argv, "hpvri:f:d:a:D:M:R:")) != -1)
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
                if(!arguments.firmware_file_path.empty())
                {
                    throw "Cannot open more than one firmware file at the same time.\n";
                }
                arguments.firmware_file_path = strdup(optarg);				
                break;
            
            case 'd':
                if(!arguments.device_name.empty())
                {
                    throw "Cannot open more than one device at the same time.\n";
                }	
                arguments.device_name = strdup(optarg);
                //if device name starts with 'sdm' it is a raw name
                if(arguments.device_name.compare(0, raw_prefix.size(), raw_prefix) == 0)
                    arguments.device_name_raw = true;
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
                
            case 'D':
                if(!arguments.dmap_file_path.empty())
                {
                    throw "Cannot use more than one DMAP file at the same time.\n";
                }	
                arguments.dmap_file_path = strdup(optarg);
                break;
            
            case 'M':
                if(!arguments.map_file_path.empty())
                {
                    throw "Cannot use more than one MAP file at the same time.\n";
                }	
                arguments.map_file_path = strdup(optarg);
                break;
                
            case 'R':
                if(!arguments.map_area_name.empty())
                {
                    throw "Cannot use more than one register area at the same time.\n";
                }	
                arguments.map_area_name = strdup(optarg);
                break;    
                
            case 'p':
                arguments.action_programming = true;
                break;
            
            case 'v':
                arguments.action_verification = true;
                break;
            
            case 'r':
                arguments.action_reboot = true;
                break;
            
            case 'h':
            default:		/* '?' */
                usage (argv[0]);
                exit (0);
        }
    }
    
    return arguments;
}

void verify_arguments(char* app_name, arguments_t arguments)
{
    if(!arguments.action_programming &&
       !arguments.action_reboot &&
       !arguments.action_verification)
    {
        printf("Please specify action(s) to be executed.\n");
        usage(app_name);
    }
    
    if(arguments.device_name.empty())
    {
        printf("Cannot make any actions without device name.\n"
                "Please specify device for programming.\n");
        usage(app_name);
    }
    
    if(arguments.interface.getType() == ProgrammingInterface::INTERFACE_NONE) 
    {
        printf("Programming interface (SPI/JTAG) is missing.\n"
                "Please specify the interface appropriate for the device.\n");
        usage(app_name);
    } 

    if(arguments.firmware_file_path.empty())
    {
        printf("Firmware file is missing.\n"
                "Please specify the location of file with firmware.\n");
        usage(app_name);
    }
    else
    {
        FILE* tmp_file = fopen(arguments.firmware_file_path.c_str(), "r");
        if(tmp_file == NULL)
            throw "Cannot open source file. Please check if file exists and you have rights to access it.\n";
        else
            fclose(tmp_file);
    }
}

int main (int argc, char *argv[])
{
    boost::shared_ptr<MtcaProgrammerBase> programmer;
        
    arguments_t arguments;
    
    try
    {
        cout << "\nmtca4u_fw_programmer ver. " << VERSION << endl << endl;
        arguments = parse_arguments(argc, argv);
        verify_arguments(argv[0], arguments);
                               	
        //print arguments
/*        cout << "Programming PROM of " << arguments.device_name << " device\n";
        if(!arguments.firmware_file_path.empty()) 
        {
            printf("Source file: %s\n", arguments.firmware_file_path.c_str());
        }
*/        

        if(arguments.device_name_raw)
        {
            if(arguments.map_file_path.empty())     // Access raw
            {
                cout << "Input mode - Direct" << endl;
                cout << "Firmware file: " << arguments.firmware_file_path << endl;
                cout << "Device name: " << arguments.device_name << endl;
                cout << "Bar: " << (uint32_t)arguments.bar << endl;
                cout << "Address: " << arguments.address << endl;
                
                switch(arguments.interface.getType())
                {
                    case ProgrammingInterface::INTERFACE_SPI:
                        programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerSPI(ProgAccessRaw(arguments.device_name, arguments.bar, arguments.address)));
                        break;	
                    case ProgrammingInterface::INTERFACE_JTAG:
                        programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerJTAG(ProgAccessRaw(arguments.device_name, arguments.bar, arguments.address)));
                        break;	
                    default:
                        throw "Unknown interface\n\n";
                }
            }
            else
            {
                cout << "Input mode - MAP" << endl;
                cout << "Firmware file: " << arguments.firmware_file_path << endl;
                cout << "Device name: " << arguments.device_name << endl;
                cout << "MAP file: " << arguments.map_file_path << endl;
                cout << "Module name in MAP file: " << arguments.map_area_name << endl;
                
                switch(arguments.interface.getType())
                {
                    case ProgrammingInterface::INTERFACE_SPI:
                        programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerSPI(ProgAccessMap(arguments.device_name, arguments.map_file_path, arguments.map_area_name)));
                        break;	
                    case ProgrammingInterface::INTERFACE_JTAG:
                        programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerJTAG(ProgAccessMap(arguments.device_name, arguments.map_file_path, arguments.map_area_name)));
                        break;	
                    default:
                        throw "Unknown interface\n\n";
                }
            }
        }
        else
        {
            if(arguments.dmap_file_path.empty())
            {
                printf( "DMAP file is missing.\n"
                        "Please specify the location of DMAP file.\n");
                usage(argv[0]);
            }
            
            cout << "Input mode - DMAP" << endl;
            cout << "Firmware file: " << arguments.firmware_file_path << endl;
            cout << "DMAP file: " << arguments.dmap_file_path << endl;
            cout << "Device name: " << arguments.device_name << endl;
            cout << "Module name in MAP file: " << arguments.map_area_name << endl;
            
            switch(arguments.interface.getType())
            {
                case ProgrammingInterface::INTERFACE_SPI:
                    programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerSPI(ProgAccessDmap(arguments.device_name, arguments.dmap_file_path, arguments.map_area_name)));
                    break;	
                case ProgrammingInterface::INTERFACE_JTAG:
                    programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerJTAG(ProgAccessDmap(arguments.device_name, arguments.dmap_file_path, arguments.map_area_name)));
                    break;	
                default:
                    throw "Unknown interface\n\n";
            }
        }
        cout << endl << endl;
        
        
#if 1       
        if(!programmer->checkFirmwareFile(arguments.firmware_file_path))
        {
            fprintf(stderr, "Incorrect firmware file\n");
        }
        if(arguments.action_programming)
        {
            programmer->erase();
            programmer->program(arguments.firmware_file_path);
        }
        if(arguments.action_verification)
        {
            programmer->verify(arguments.firmware_file_path);
        }
        if(arguments.action_reboot)
        {
            programmer->rebootFPGA();
        }
#endif
    }
    catch (const char *exc)		//handle the exceptions
    {
        fprintf (stderr, "\n \nError: %s\n", exc);
        return 1;
    }
    catch (mtca4u::Exception e)
    {
        fprintf (stderr, "\n \nMTCA4U Error: %s\n", e.what());
        return 1;
    }
	
    return 0;
}
