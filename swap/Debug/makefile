################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: swap

dependents:
	-cd /home/utnso/rrigonijami/tp-2015-2c-riganijomi/bibliotecaRiganijomi/Debug && $(MAKE) all

# Tool invocations
swap: $(OBJS) $(USER_OBJS) /home/utnso/rrigonijami/tp-2015-2c-riganijomi/bibliotecaRiganijomi/Debug/libbibliotecaRiganijomi.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/rrigonijami/tp-2015-2c-riganijomi/bibliotecaRiganijomi/Debug" -o "swap" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/rrigonijami/tp-2015-2c-riganijomi/bibliotecaRiganijomi/Debug/
# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) swap
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/rrigonijami/tp-2015-2c-riganijomi/bibliotecaRiganijomi/Debug/libbibliotecaRiganijomi.so:

-include ../makefile.targets
