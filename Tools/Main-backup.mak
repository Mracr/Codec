include $(UTILITIES_MAK)
include $(TOKEN_MAK)


all: Prepare  EDKII  End


####################################################
# Single Module Build Support.
# There are two ways to define a module to build:
# 1. Using VeB GUI. VeB will set VEB_BUILD_MODULE environment variable.
# 2. Using EDKII_FLGAS (-m <module-name>)
####################################################
# Extract module name from EDKII_FLAGS (Don't try this at home :)
ifneq ("$(VEB_BUILD_MODULE)","")
override EDKII_FLAGS += -m $(VEB_BUILD_MODULE)
EDKII_ACTIVE_MODULE_NAME:=$(VEB_BUILD_MODULE)
else
override EDKII_FLAGS +=
endif


Prepare:
	echo $(TOKEN_MAK) 1111
	
Config/target.txt:$(TOKEN_MAK) Conf
	@$(ECHO)\
	"ACTIVE_PLATFORM		=$(subset \,/, $(ACTIVE_PLATFORM))\
	$(EOL)\
	$(EOL)TARGET 			=$(TARGET)\
	$(EOL)\
	$(EOL)TOOL_CHAIN_CONF	=$(TOOL_DEFINITION_FILE)\
	$(EOL)\
	$(EOL)TOOL_CHAIN_TAG 	=$(TOOL_CHAIN_TAG)\
	$(EOL)\
	$(EOL)MAX_CONCURRENT_THREAD_NUMBER=$(NUMBER_OF_PROCESSORS)\
	$(EOL)\
	$(EOL)BUILD_RULE_CONF	=$(BUILD_RULE_FILE)" > Conf/target.txt

Conf:
	mkdir Conf
	
	

EDKII: Config/target.txt
#### Build
#### build -m  module.inf
	$(EDKII_BUILD_COMMAND) $(EDKII_FLAGS)

# Copy generated efi file form build directory to bin folder
End: bin
	
	
	
bin:
	mkdir bin
	
	