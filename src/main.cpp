// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, https://msk.desy.de
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MtcaProgrammerBase.h"
#include "MtcaProgrammerJTAG.h"
#include "MtcaProgrammerSPI.h"
#include "progress_bar.h"
#include "registers.h"
#include "version.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <string>

// #define DEBUG

using namespace std;
namespace po = boost::program_options;

class ProgrammingInterface {
 public:
  enum InterfaceType { INTERFACE_NONE, INTERFACE_SPI, INTERFACE_JTAG };

  ProgrammingInterface() : mType(INTERFACE_NONE) {};
  ProgrammingInterface(InterfaceType type) : mType(type) {};
  std::string toString() {
    switch(mType) {
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

  InterfaceType getType() { return mType; }

 private:
  InterfaceType mType;
};

struct arguments_t {
  ProgrammingInterface interface{ProgrammingInterface::INTERFACE_NONE};
  std::string firmware_file_path;
  std::string device_name;
  bool device_name_raw{false};
  uint32_t address{PROG_DEFAULT_ADDRESS};
  uint8_t bar{PROG_DEFAULT_BAR};
  bool addressValid{false};
  uint32_t flash_size{0};
  std::string dmap_file_path;
  std::string map_area_name{"**DEFAULT**"};
  bool action_programming{false};
  bool action_verification{false};
  bool action_dump{false};
  bool action_reload{false};
  bool quiet_mode{false};

  string toString() {
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
void usage(const char* progname) {
  std::cout << "Usage:" << std::endl;
  std::cout << "1) DMAP: " << progname
            << " [-p] [-v] [-m] [-r] -d <device_alias> [-i <interface>] -f <firmware file> -D <dmap_file> [-R "
               "<boot_area_name>]"
            << std::endl;
  std::cout << "2) MAP: " << progname
            << " [-p] [-v] [-m] [-r] -d <device_descriptor> [-i <interface>] -f <firmware file> [-R <boot_area_name>]"
            << std::endl;
  std::cout << "3) Direct: " << progname
            << " [-p] [-v] [-m] [-r] -d <device_descriptor> [-i <interface>] -f <firmware file> -a <address>b<bar>"
            << std::endl;
}

arguments_t parse_arguments(int argc, char* argv[]) {
  arguments_t args;
  const std::string raw_prefix("(");

  // Declare a group of options that will be
  // allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()("help,h", "produce help message")("config,c", po::value<string>(),
      "set configuration file name containing key=value pairs (hints: use long forms like 'program' or 'device' and "
      "set on/off flags to true)");

  // Declare a group of options that will be
  // allowed both on command line and in
  // config file
  po::options_description config("Configuration");
  config.add_options()("program,p", po::bool_switch()->default_value(false), "erase and program memory")("verify,v",
      po::bool_switch()->default_value(false), "verify memory")("dump,m", po::bool_switch()->default_value(false),
      "dump memory")("reload,r", po::bool_switch()->default_value(false), "reload FPGA")("interface,i",
      po::value<string>(), "memory interface (spi/jtag)")("firmware_file,f", po::value<string>(), "FPGA firmware file")(
      "device,d", po::value<string>(), "device name")("address,a", po::value<string>(),
      ("address and bar of boot area in the format: [address]b[bar]. Use 'default' to refer to the default address (" +
          std::to_string(PROG_DEFAULT_ADDRESS) + "b" + std::to_string(PROG_DEFAULT_BAR) + ").")
          .c_str())("flash_size,s", po::value<string>(),
      "size of flash chip to dump, in bytes - example for a 256M (32MiB) chip: -s 33554432")(
      "dmap,D", po::value<string>(), "DMAP file path")("map,M", po::value<string>(), "MAP file path")("boot_area,R",
      po::value<string>(),
      "boot area name in MAP file")("quiet,q", po::bool_switch()->default_value(false), "disable progress bar");

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config);

  po::options_description config_file_options;
  config_file_options.add(config);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
  po::notify(vm);

  if(vm.count("help")) {
    usage(argv[0]);
    cout << cmdline_options << "\n";
    exit(1);
  }

  if(vm.count("config")) {
    std::string config_file_path = vm["config"].as<std::string>();
    cout << "Configuration file path: " << config_file_path << "\n";

    po::store(po::parse_config_file<char>(config_file_path.c_str(), config_file_options), vm);
    po::notify(vm);
  }

  args.action_programming = vm["program"].as<bool>();
  args.action_verification = vm["verify"].as<bool>();
  args.action_dump = vm["dump"].as<bool>();
  args.action_reload = vm["reload"].as<bool>();
  args.quiet_mode = vm["quiet"].as<bool>();

  if(vm.count("device")) {
    args.device_name = vm["device"].as<std::string>();
    if(args.device_name.compare(0, raw_prefix.size(), raw_prefix) == 0) args.device_name_raw = true;
  }

  if(vm.count("interface")) {
    std::string interface = vm["interface"].as<std::string>();
    if(interface.empty())
      args.interface = ProgrammingInterface(ProgrammingInterface::INTERFACE_NONE);
    else if(interface.compare("spi") == 0)
      args.interface = ProgrammingInterface(ProgrammingInterface::INTERFACE_SPI);
    else if(interface.compare("jtag") == 0)
      args.interface = ProgrammingInterface(ProgrammingInterface::INTERFACE_JTAG);
    else
      throw std::invalid_argument("Unknown protocol, use spi or jtag\n");
  }

  if(vm.count("firmware_file")) args.firmware_file_path = vm["firmware_file"].as<std::string>();

  if(vm.count("dmap")) args.dmap_file_path = vm["dmap"].as<std::string>();

  if(vm.count("boot_area")) args.map_area_name = vm["boot_area"].as<std::string>();

  if(vm.count("flash_size")) args.flash_size = std::stoi(vm["flash_size"].as<std::string>());

  if(vm.count("address")) {
    std::string address_input = vm["address"].as<std::string>();
    if(address_input == "default") {
      address_input = std::to_string(PROG_DEFAULT_ADDRESS) + "b" + std::to_string(PROG_DEFAULT_BAR);
    }
    std::cout << "Address input: " << address_input << std::endl;
    uint32_t b_counter = 0;
    std::size_t b_position = address_input.find_first_of("b");
    while(b_position != std::string::npos) {
      b_counter++;
      b_position = address_input.find_first_of("b", b_position + 1);
    }
    if(b_counter != 1) throw std::invalid_argument("Wrong format of programmer address.\n");

    b_position = address_input.find_first_of("b");

    try {
      std::string address_ch = address_input.substr(0, b_position);
      std::string bar_ch = address_input.substr(b_position + 1, address_input.length() - b_position);

      size_t idx;
      uint32_t address = std::stoi(address_ch, &idx);
      if(idx != address_ch.length()) throw std::invalid_argument("Wrong format of programmer address.\n");

      uint8_t bar = std::stoi(bar_ch, &idx);
      if(idx != bar_ch.length()) throw std::invalid_argument("Wrong format of programmer address.\n");

      args.address = address;
      args.bar = bar;
      args.addressValid = true;
    }
    catch(...) {
      throw std::invalid_argument("Wrong format of programmer address.\n");
    }
  }

  return args;
}

void verify_arguments(arguments_t arguments) {
  if(!arguments.action_programming && !arguments.action_reload && !arguments.action_verification &&
      !arguments.action_dump) {
    throw std::logic_error("Please specify action(s) to be executed");
  }

  if((arguments.action_programming || arguments.action_verification) && arguments.action_dump) {
    throw std::logic_error("Cannot program/verify a new firmware and also dump the current firmware");
  }

  if(arguments.action_dump && arguments.flash_size == 0) {
    throw std::logic_error("Cannot dump a flash image without knowing the size of the chip.\n"
                           "Please specify the number of bytes to dump.\n");
  }

  if(arguments.flash_size && (arguments.flash_size % 1024)) {
    throw std::logic_error("The flash size must be a multiple of 1024.\n");
  }

  if(arguments.device_name.empty()) {
    throw std::logic_error("Cannot make any actions without device name.\n"
                           "Please specify device for programming.\n");
  }

  if(arguments.interface.getType() == ProgrammingInterface::INTERFACE_NONE) {
    throw std::logic_error("Programming interface (SPI/JTAG) is missing.\n"
                           "Please specify the interface appropriate for the device.\n");
  }

  if((arguments.action_programming || arguments.action_verification) && arguments.firmware_file_path.empty()) {
    throw std::logic_error("Firmware file is missing.\n"
                           "Please specify the location of file with firmware.\n");
  }
  else if(arguments.action_programming || arguments.action_verification) {
    FILE* tmp_file = fopen(arguments.firmware_file_path.c_str(), "r");
    if(tmp_file == NULL)
      throw std::invalid_argument("Cannot open firmware file. Please check if file exists and you have "
                                  "rights to access it.\n");
    else
      fclose(tmp_file);
  }
}

int main(int argc, char* argv[]) {
  boost::shared_ptr<MtcaProgrammerBase> programmer;

  arguments_t arguments;

  try {
    cout << "\nmtca4u_fw_programmer ver. " << VERSION << endl << endl;
    arguments = parse_arguments(argc, argv);
    verify_arguments(arguments);

    // Quite mode disables the progress bar
    ProgressBar::setDoNotShow(arguments.quiet_mode);

    if(!arguments.dmap_file_path.empty()) // DMAP mode
    {
      // printf( "DMAP file is missing.\n"
      //        "Please specify the location of DMAP file.\n");

      cout << "Input mode - DMAP" << endl;
      cout << "Firmware file: " << arguments.firmware_file_path << endl;
      cout << "DMAP file: " << arguments.dmap_file_path << endl;
      cout << "Device name: " << arguments.device_name << endl;
      cout << "Module name in MAP file: " << arguments.map_area_name << endl;

      switch(arguments.interface.getType()) {
        case ProgrammingInterface::INTERFACE_SPI:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerSPI(
              ProgAccessDmap(arguments.device_name, arguments.dmap_file_path, arguments.map_area_name)));
          break;
        case ProgrammingInterface::INTERFACE_JTAG:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(new MtcaProgrammerJTAG(
              ProgAccessDmap(arguments.device_name, arguments.dmap_file_path, arguments.map_area_name)));
          break;
        default:
          throw std::invalid_argument("Unknown interface\n\n");
      }
    }
    else if(arguments.device_name_raw && !arguments.addressValid) // MAP mode
    {
      cout << "Input mode - MAP" << endl;
      cout << "Firmware file: " << arguments.firmware_file_path << endl;
      cout << "Device name: " << arguments.device_name << endl;
      cout << "Module name in MAP file: " << arguments.map_area_name << endl;

      switch(arguments.interface.getType()) {
        case ProgrammingInterface::INTERFACE_SPI:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(
              new MtcaProgrammerSPI(ProgAccessMap(arguments.device_name, arguments.map_area_name)));
          break;
        case ProgrammingInterface::INTERFACE_JTAG:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(
              new MtcaProgrammerJTAG(ProgAccessMap(arguments.device_name, arguments.map_area_name)));
          break;
        default:
          throw std::invalid_argument("Unknown interface\n\n");
      }
    }
    else // RAW mode
    {
      if(!arguments.device_name_raw) {
        throw std::invalid_argument("Wrong device name.\nPlease specify device "
                                    "name in 'sdm' format\n\n");
      }

      cout << "Input mode - Direct" << endl;
      cout << "Firmware file: " << arguments.firmware_file_path << endl;
      cout << "Device name: " << arguments.device_name << endl;
      cout << "Bar: " << (uint32_t)arguments.bar << endl;
      cout << "Address: " << arguments.address << endl;

      switch(arguments.interface.getType()) {
        case ProgrammingInterface::INTERFACE_SPI:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(
              new MtcaProgrammerSPI(ProgAccessRaw(arguments.device_name, arguments.bar, arguments.address)));
          break;
        case ProgrammingInterface::INTERFACE_JTAG:
          programmer = boost::shared_ptr<MtcaProgrammerBase>(
              new MtcaProgrammerJTAG(ProgAccessRaw(arguments.device_name, arguments.bar, arguments.address)));
          break;
        default:
          throw std::invalid_argument("Unknown interface\n\n");
      }
    }
    cout << endl << endl;

    if(arguments.action_programming || arguments.action_verification) {
      if(!programmer->checkFirmwareFile(arguments.firmware_file_path)) {
        throw std::runtime_error("Incorrect firmware file\n");
      }
    }
    if(arguments.action_programming) {
      programmer->erase();
      programmer->program(arguments.firmware_file_path);
    }
    if(arguments.action_dump) {
      if(programmer->dump(arguments.firmware_file_path, arguments.flash_size) == false) return 1;
    }
    if(arguments.action_verification) {
      if(programmer->verify(arguments.firmware_file_path) == false) return 1;
    }
    if(arguments.action_reload) {
      programmer->rebootFPGA();
    }
  }
  catch(const std::exception& e) {
    std::cerr << "\nError: " << e.what() << std::endl;
    return 1;
  }
  catch(...) {
    std::cerr << "An unexpected error has occured" << std::endl;
    return 1;
  }

  return 0;
}
