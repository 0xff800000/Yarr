######
## Author: Timon Heim/Jan Kuechler
## Last edit: 09.12.2015

UNAME_S := $(shell uname -s)

MODE = DEBUG

AR = ar

# Compilera
CPP = g++
#CC = gcc
ifeq ($(UNAME_S),Darwin)
	CPP = clang++
#	CC = clang
endif

CPPFLAGS += -pthread -std=c++11 -Wall -Wunused-variable -IlibSpec/include \
			-IlibYarr/include -IlibUtil/include -IlibFei4/include -IlibFe65p2/include
CPPFLAGS_DEBUG += -g -O1 -DDEBUG
CPPFLAGS_RELEASE += -O2 -DNDEBUG

# Linker
LD = g++
LDFLAGS = -pthread -std=c++11
ifeq ($(UNAME_S),Darwin)
	LD = clang++
	LDFLAGS = -std=c++11
endif
LDFLAGS_DEBUG +=
LDFLAGS_RELEASE += 
LIBS +=

BIN = bin
SRC = tools
SPEC = libSpec
YARR = libYarr
FEI4 = libFei4
FE65P2 = libFe65p2
UTIL = libUtil
BUILD = build
LLIB = lib

GCC_MAJOR = $(shell $(CPP) -dumpversion | cut -d'.' -f1)
GCC_MINOR = $(shell $(CPP) -dumpversion | cut -d'.' -f2)

ifneq ($(UNAME_S), Darwin)
	ifeq ($(shell echo ${GCC_MAJOR}\<4| bc), 1)
		$(error $(CPP) version is too old, need atleast 4.7!)
	endif
	ifeq ($(shell echo ${GCC_MINOR}\<7 | bc), 1)
		$(error $(CPP) version is too old, need atleast 4.7!)
	endif
endif

# Set build mode specific variables
CPPFLAGS += $(CPPFLAGS_$(MODE))
LDFLAGS += $(LDFLAGS_$(MODE))

######
##  File lists

SRC_FILES = $(wildcard $(SRC)/*.cpp) 
SPEC_FILES = $(wildcard $(SPEC)/*.cpp)
YARR_FILES = $(wildcard $(YARR)/*.cpp)
FEI4_FILES = $(wildcard $(FEI4)/*.cpp)
FE65P2_FILES = $(wildcard $(FE65P2)/*.cpp)
UTIL_FILES = $(wildcard $(UTIL)/*.cpp)

SRC_DEPS = $(patsubst $(SRC)/%.cpp,$(BUILD)/%.d,$(SRC_FILES))
SPEC_DEPS = $(patsubst $(SPEC)/%.cpp,$(BUILD)/%.d,$(SPEC_FILES))
YARR_DEPS = $(patsubst $(YARR)/%.cpp,$(BUILD)/%.d,$(YARR_FILES))
FEI4_DEPS = $(patsubst $(FEI4)/%.cpp,$(BUILD)/%.d,$(FEI4_FILES))
FE65P2_DEPS = $(patsubst $(FE65P2)/%.cpp,$(BUILD)/%.d,$(FE65P2_FILES))
UTIL_DEPS = $(patsubst $(UTIL)/%.cpp,$(BUILD)/%.d,$(UTIL_FILES))

ALL_DEPS = $(SRC_DEPS) $(SPEC_DEPS) $(YARR_DEPS) $(FEI4_DEPS) $(FE65P2_DEPS) $(UTIL_DEPS)

SRC_OBJS = $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(SRC_FILES))
SPEC_OBJS = $(patsubst $(SPEC)/%.cpp,$(BUILD)/%.o,$(SPEC_FILES))
YARR_OBJS = $(patsubst $(YARR)/%.cpp,$(BUILD)/%.o,$(YARR_FILES))
FEI4_OBJS = $(patsubst $(FEI4)/%.cpp,$(BUILD)/%.o,$(FEI4_FILES))
FE65P2_OBJS = $(patsubst $(FE65P2)/%.cpp,$(BUILD)/%.o,$(FE65P2_FILES))
UTIL_OBJS = $(patsubst $(UTIL)/%.cpp,$(BUILD)/%.o,$(UTIL_FILES))

ALL_OBJS = $(SPEC_OBJS) $(YARR_OBJS) $(FEI4_OBJS) $(FE65P2_OBJS) $(UTIL_OBJS)

TARGETS = $(patsubst $(SRC)/%.cpp,$(BIN)/%,$(SRC_FILES))

######
##  Targets

all: yarrlib $(TARGETS)

yarrlib: $(LLIB)/libyarr.a

help:
	@echo # Use 'make' to create all programs
	@echo # Use 'make tools' to create the librce_tools.a
	@echo # Use 'make analysis' to create the librce_ana.a
	@echo # Special targets:
	@echo "#   list           - List all targets"
	@echo "#   mkinfo         - List makefile internals"
	@echo "#   help           - Show this help message"

# Dependency file creation
$(BUILD)/%.d: $(SRC)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$<) $< > $@

$(BUILD)/%.d: $(SPEC)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(SPEC)/%.cpp,$(BUILD)/%.o,$<) $< > $@

$(BUILD)/%.d: $(YARR)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(YARR)/%.cpp,$(BUILD)/%.o,$<) $< > $@

$(BUILD)/%.d: $(FEI4)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(FEI4)/%.cpp,$(BUILD)/%.o,$<) $< > $@

$(BUILD)/%.d: $(FE65P2)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(FE65P2)/%.cpp,$(BUILD)/%.o,$<) $< > $@

$(BUILD)/%.d: $(UTIL)/%.cpp
	@$(CPP) $(CPPFLAGS) -MM -MT $(patsubst $(UTIL)/%.cpp,$(BUILD)/%.o,$<) $< > $@

# Object creation
$(BUILD)/%.o:
	@echo [Compiling] $@
	@$(CPP) $(CPPFLAGS) -c $< -o $@
        
# Target
$(BIN)/%: $(BUILD)/%.o $(SPEC_OBJS) $(YARR_OBJS) $(FEI4_OBJS) $(FE65P2_OBJS) $(UTIL_OBJS)
	@echo [Linking] $@
	@$(LD) $(LDFLAGS) $< $(SPEC_OBJS) $(YARR_OBJS) $(FEI4_OBJS) $(FE65P2_OBJS) $(UTIL_OBJS) $(LIBS) -o $@

# Libs

$(LLIB)/libyarr.a: $(SPEC_OBJS) $(YARR_OBJS) $(FEI4_OBJS) $(FE65P2_OBJS) $(UTIL_OBJS)
	@echo [Creating] $@
	@$(AR) rcs $@ $(SPEC_OBJS) $(YARR_OBJS) $(FEI4_OBJS) $(FE65P2_OBJS) $(UTIL_OBJS)

-include $(ALL_DEPS)

clean:
	@$(RM) $(ALL_OBJS) $(ALL_DEPS) $(TARGETS) $(LLIB)/libspec.a $(LLIB)/libyarr.a $(LLIB)/libutil.a $(LLIB)/libfei4.a $(LLIB)/libfe65p2.a
	@echo "[Clean] … done!"

list:
	@echo Target List
	@echo ===========
	@echo $(TARGETS)

mkinfo:
	@echo Make Variables
	@echo ==============
	@echo MODE = $(MODE)
	@echo
	@echo CPP = $(CPP)
	@echo CPPFLAGS = $(CPPFLAGS)
	@echo LD = $(LD)
	@echo LDFLAGS = $(LDFLAGS)
	@echo
	@echo Program Files
	@echo =============
	@echo Source files:
	@echo "    $(SRC_FILES)"
	@echo Dependency files:
	@echo "    $(SRC_DEPS)"
	@echo Object files:
	@echo "    $(SRC_OBJS)"
	@echo 
	@echo SPEC Files
	@echo =============
	@echo Source files:
	@echo "    $(SPEC_FILES)"
	@echo Dependency files:
	@echo "    $(SPEC_DEPS)"
	@echo Object files:
	@echo "    $(SPEC_OBJS)"
	@echo 
	@echo YARR Files
	@echo =============
	@echo Source files:
	@echo "    $(YARR_FILES)"
	@echo Dependency files:
	@echo "    $(YARR_DEPS)"
	@echo Object files:
	@echo "    $(YARR_OBJS)"
	@echo 
	@echo Fei4 Files
	@echo =============
	@echo Source files:
	@echo "    $(FEI4_FILES)"
	@echo Dependency files:
	@echo "    $(FEI4_DEPS)"
	@echo Object files:
	@echo "    $(FEI4_OBJS)"
	@echo 
	@echo Fe65p2 Files
	@echo =============
	@echo Source files:
	@echo "    $(FE65P2_FILES)"
	@echo Dependency files:
	@echo "    $(FE65P2_DEPS)"
	@echo Object files:
	@echo "    $(FE65P2_OBJS)"
	@echo 
	@echo Util Files
	@echo =============
	@echo Source files:
	@echo "    $(UTIL_FILES)"
	@echo Dependency files:
	@echo "    $(UTIL_DEPS)"
	@echo Object files:
	@echo "    $(UTIL_OBJS)"
	@echo 
        
.PHONY: clean distclean mkinfo help
