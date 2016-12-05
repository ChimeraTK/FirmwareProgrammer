#
# Debian packages are built on tagged versions of the project. The tagged
# version number is specified through the fw_programmer_version file. Including
# this file imports the FW_PROGRAMMER_VERSION variable into the makefile
#include fw_programmer_version

# Input file settings
CXX := g++
TARGET := mtca4u_fw_programmer

OBJS_PATH := ./obj
SRC_PATH := ./src
INCLUDE_PATH := ./include
DEPS_PATH := ./.deps
VPATH = $(SRC_PATH):$(OBJ_PATH):$(INCLUDE_PATH)

INCLUDES := $(INCLUDE_PATH:%=-I%)

LIB_DIRS :=
LIBS := curses boost_program_options

# Uncomment this to enable debug builds
DEBUG := yes

# compiler/linker flags
CXXFLAGS := -fPIC -Wall -Wextra -Wshadow -Weffc++ -ansi -pedantic -Wuninitialized -std=c++0x
CXXFLAGS += $(shell mtca4u-deviceaccess-config --cppflags)
ifeq ($(DEBUG),yes)
	CXXFLAGS += -g -ggdb
else
	CXXFLAGS += -O3
endif

# SVN revision taken from Jenkins environment variable
ifdef SVN_REVISION
CXXFLAGS += -DSVN_REV=$(SVN_REVISION)
else
CXXFLAGS += -DSVN_REV=XX
endif

LDFLAGS := $(LIBDIRS:%=-L%) $(LIBS:%=-l%) 
LDFLAGS += $(shell mtca4u-deviceaccess-config --ldflags)

# Examples
SOURCES := $(wildcard $(SRC_PATH)/*.cpp)
OBJS := $(SOURCES:$(SRC_PATH)/%.cpp=%.o)
OBJS := $(addprefix $(OBJS_PATH)/,$(OBJS))	
DEPS := $(SOURCES:$(SRC_PATH)/%.cpp=%.d)
#DEPS := $(addprefix $(DEPS_PATH)/,$(DEPS))
###############################################################################
# rules

.PHONY: clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OBJS_PATH)/$(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS_PATH)/%.o : %.cpp
	mkdir -p $(OBJS_PATH)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<
	
$(DEPS): %.d : %.cpp
	$(CXX) -MM $< $(CXXFLAGS) $(INCLUDES) > $@
	@cp $@ $@.tmp
	@sed -e s/\\.o/\\.d/ < $@.tmp >> $@
	@rm $@.tmp
	
-include $(DEPS)


# debian packaging related targets.
# The debian packaging borrows from the matlab tools project as template:
# https://svnsrv.desy.de/desy/mtca4u_applications/matlab_tools/trunk/
################################################################################
#
# This target does the following.
# - sets up debian_from_template diectory with files required by the packaging
#   tools
configure-package-files:
	./configure_package_files.sh ${FW_PROGRAMMER_VERSION}
	
# make debian_package generates the debian packages
debian_package: configure-package-files
	./make_debian_package.sh ${FW_PROGRAMMER_VERSION}
################################################################################

clean:
	-rm -rf $(DEPS)
	-rm -rf $(OBJS_PATH)
	-rm -rf *.d
	rm -rf debian_from_template debian_package