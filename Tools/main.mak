#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************

#**********************************************************************
## @file
# Called by makefile, this includes all of the makefiles
#  from module.mak, generates target.txt, then launches the EDK2 build.exe
#**********************************************************************
include $(UTILITIES_MAK)
include $(TOKEN_MAK)

ifeq ($(SILENT), 1) 
APTIO_MAKE+=-s
.SILENT :
endif

# Set target debug type based on SDL token DEBUG_MODE
ifeq ($(DEBUG_MODE), 1)
 TARGET	= DEBUG
else
 TARGET	= RELEASE
endif	#ifeq ($(DEBUG_MODE), 1)

ifeq ($(RUN_VSVARS_BAT), 1)
EDKII_BUILD_COMMAND = $(TOOLS_DIR)\RunInVsEnv.bat $(EDII_BUILD)
else
EDKII_BUILD_COMMAND = $(EDII_BUILD)
endif	#ifeq (TOOL_CHAIN_TAG, MYTOOLS)

ABS_BUILD_DIR:=$(WORKSPACE)$(PATH_SLASH)$(BUILD_DIR)
ABS_OUTPUT_DIR:=$(WORKSPACE)$(PATH_SLASH)$(OUTPUT_DIRECTORY)



include $(CONFIGURATION_DIR)ToolChainInit.mak 

.PHONY : all run  EdkII  End ModuleEnd 

all:  EdkII


# Create Conf/target.txt, based on SDL tokens
Conf/target.txt : $(TOKEN_MAK) $(MAIN_MAK) Conf
	@$(ECHO) \
"ACTIVE_PLATFORM	   = $(subst \,/, $(ACTIVE_PLATFORM))\
$(EOL)\
$(EOL)TARGET				= $(TARGET)\
$(EOL)\
$(EOL)TOOL_CHAIN_CONF	   = $(TOOL_DEFINITION_FILE)\
$(EOL)\
$(EOL)TOOL_CHAIN_TAG		= $(TOOL_CHAIN_TAG)\
$(EOL)\
$(EOL)MAX_CONCURRENT_THREAD_NUMBER = $(NUMBER_OF_BUILD_PROCESS_THREADS)\
$(EOL)\
$(EOL)BUILD_RULE_CONF = $(BUILD_RULE_FILE)" > Conf/target.txt

Conf: 
	mkdir Conf

# Make sure the variables required to launch EDKII build process are set
ifeq ("$(wildcard $(ACTIVE_PLATFORM))","")
  $(error Project description(.dsc) file "$(ACTIVE_PLATFORM)" defined by the ACTIVE_PLATFORM SDL token is not found.\
  This can happen when PLATFORM_DSC or <Arch-Type>_PLATFORM_DSC SDL output directive does not exist\
  or refers to a different file)
endif
ifeq ("$(PLATFORM_GUID)","INVALID")
  $(error PLATFORM_GUID SDL token must be cloned in every project)
endif
ifeq ("$(SUPPORTED_ARCHITECTURES)","INVALID")
  $(error The value of the SUPPORTED_ARCHITECTURES SDL token is invalid.\
  The token must be cloned by the CPU architecture module)
endif

ifeq ($(SILENT), 1) 
# EDKII_FLAGS can be set with a command line argument.
# If a variable has been set with a command line argument, the ordinary assignments in the makefile are ignored.
# Such a variable can be updated using an override directive.
override EDKII_FLAGS += -s
endif

####################################################
# Single Module Build Support.
# There are two ways to define a module to build:
# 1. Using VeB GUI. VeB will set VEB_BUILD_MODULE environment variable.
# 2. Using EDKII_FLGAS (-m <module-name>)
####################################################
# Extract module name from EDKII_FLAGS (Don't try this at home :)
EDKII_ACTIVE_MODULE_NAME:=$(strip $(subst ~~~~~,,$(filter ~~~~~%,$(subst -m$(SPACE),~~~~~,$(EDKII_FLAGS)))))
ifneq ("$(EDKII_ACTIVE_MODULE_NAME)","")
ifneq ("$(VEB_BUILD_MODULE)","")
$(error Ambiguous single component build parameters. VEB_BUILD_MODULE=$(VEB_BUILD_MODULE). EDKII_FLAGS module = $(EDKII_ACTIVE_MODULE_NAME).)
endif
endif
ifneq ("$(VEB_BUILD_MODULE)","")
override EDKII_FLAGS += -m $(VEB_BUILD_MODULE)
EDKII_ACTIVE_MODULE_NAME:=$(VEB_BUILD_MODULE)
else
override EDKII_FLAGS += 
endif



ifeq ("$(EDKII_ACTIVE_MODULE_NAME)","")
all: ModuleEnd 
else
all: ModuleEnd End
endif

# Clear MAKEFLAGS during execution of the EdkII target. 
# We don't want to pass our options to EDKII build process.
EdkII: MAKEFLAGS=
# If using MYTOOLS, create a batch file that runs VSVARS32.BAT before build.exe
#  so that the build can inherit the newly set up env variables
EdkII: Conf/target.txt
#$(EDKII_BUILD_COMMAND) $(EDKII_FLAGS)
	$(EDKII_BUILD_COMMAND)


ModuleEnd: bin
	$(CP)  $(BUILD_DIR)\$(TARGET)_$(TOOL_CHAIN_TAG)\$(SUPPORTED_ARCHITECTURES)\.efi    "bin\$(PROJECT_NAME)$(CurrentDate)_$(SUPPORTED_ARCHITECTURES).efi"
	$(ECHO) TARGET: $(TARGET). TOOL CHAIN: $(TOOL_CHAIN_TAG). ARCHITECTURE: "$(SUPPORTED_ARCHITECTURES)".
	$(ECHO) Build Tools: $(BUILD_TOOLS_VERSION). Single component build mode. ROM image generation is skipped.
	
bin:
	mkdir bin
#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************