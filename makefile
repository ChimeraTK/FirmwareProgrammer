OUT = llrf_prog
CCP = g++
C=gcc
CFLAGS = -ggdb -O0 -W -I. -Wall -g -D__STANDALONE_CFG__ -DSELFTEST1 -D__DEV_LLRF__ -DBAR_NR=0 -DOFFSET=65536
IPMITOOL_PATH = ./ipmitool
IPMITOOL_INCLUDE = -I$(IPMITOOL_PATH)/include
IPMITOOL_LIB = $(IPMITOOL_PATH)/lib
IPMITOOL_PLUGINS_DIR = $(IPMITOOL_PATH)/src/plugins
IPMITOOL_LAN_PLUGIN = $(IPMITOOL_PLUGINS_DIR)/lan

all: $(OUT)

llrf_prog: II_pcie.o pcie_II_bridge.o xsfv_player.o lenval.o ports.o micro.o ipmitool_wrapper.o ipmi_cmd.o spi_mem_tools.o dsp_tools.o progress_bar.o
	$(CCP) $(CFLAGS) $^ -o $@ -L./lib/ -ldev -L$(IPMITOOL_LIB)/.libs -lipmitool -L$(IPMITOOL_PLUGINS_DIR)/.libs -lintf -lcurses -lcrypto
	
II_pcie.o: main.cpp II_interface.h ipmitool_wrapper.h
	$(CCP) $(CFLAGS) -DDEVICE=\"/dev/pcie_bar_2\" -c $< -o II_pcie.o

pcie_II_bridge.o: pcie_II_bridge.cpp II_interface.h
	$(CCP) $(CFLAGS) -c $<

xsfv_player.o: xsfv_player.c micro.h II_interface.h 
	$(CCP) $(CFLAGS) -c $<

progress_bar.o: progress_bar.c progress_bar.h 
	$(CCP) $(CFLAGS) -c $<

lenval.o: lenval.c lenval.h ports.h
	$(CCP) $(CFLAGS) -c $<

ports.o: ports.c ports.h II_interface.h ii_constants.h
	$(CCP) $(CFLAGS) -c $<

micro.o: micro.c ports.h lenval.h
	$(CCP) $(CFLAGS) -c $<

spi_mem_tools.o: spi_mem_tools.cpp spi_mem_tools.h II_interface.h ii_constants.h
	$(CCP) $(CFLAGS) -c $<

dsp_tools.o: dsp_tools.cpp dsp_tools.h II_interface.h ii_constants.h
	$(CCP) $(CFLAGS) -c $<
	
ipmi_cmd.o: ipmi_cmd.cpp ipmi_cmd.h ipmitool_wrapper.h
	$(CCP) $(CFLAGS) -c $<
	
ipmitool_wrapper.o: ipmitool_wrapper.c ipmitool_wrapper.h
	@if test ! -f $(IPMITOOL_PATH)/config.h; then \
		cd $(IPMITOOL_PATH); ./configure --prefix="$(CURDIR)/ipmitool"; \
	else :; fi
	cd $(IPMITOOL_PATH); $(MAKE) $(MAKEFLAGS)
	$(C) -O0 -Wall -g -c $< $(IPMITOOL_INCLUDE)

clean:
	-rm llrf_prog II_pcie.o pcie_II_bridge.o xsfv_player.o lenval.o ports.o micro.o ipmitool_wrapper.o ipmi_cmd.o spi_mem_tools.o dsp_tools.o progress_bar.o

distclean:
	-rm llrf_prog II_pcie.o pcie_II_bridge.o xsfv_player.o lenval.o ports.o micro.o ipmitool_wrapper.o ipmi_cmd.o spi_mem_tools.o dsp_tools.o progress_bar.o
	cd $(IPMITOOL_PATH); $(MAKE) distclean
