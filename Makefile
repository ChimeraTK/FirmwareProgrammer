include /usr/share/mtca4u/MTCA4U.CONFIG

# Input file settings
CXX := g++
TARGET := llrf_prog

OBJS_PATH := ./obj
SRC_PATH := ./src
INCLUDE_PATH := ./include
DEPS_PATH := ./.deps
VPATH = $(SRC_PATH):$(OBJ_PATH):$(INCLUDE_PATH)

INCLUDES := $(INCLUDE_PATH:%=-I%) $(MtcaMappedDevice_INCLUDE_FLAGS)

LIB_DIRS :=
LIBS := curses

# Uncomment this to enable debug builds
DEBUG := yes

# compiler/linker flags
CXXFLAGS := -fPIC -Wall -Wextra -Wshadow -Weffc++ -ansi -pedantic -Wuninitialized -std=c++0x

LDFLAGS := $(LIBDIRS:%=-L%) $(LIBS:%=-l%) $(MtcaMappedDevice_LIB_FLAGS)
ifeq ($(DEBUG),yes)
	CXXFLAGS += -g -ggdb
else
	CXXFLAGS += -O3
endif

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

clean:
	-rm -rf $(DEPS)
	-rm -rf $(OBJS_PATH)
	-rm -rf *.d