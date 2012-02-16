################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
<<<<<<< HEAD
=======
-include src/subdir.mk
>>>>>>> 2de90737c4ed4d8a0b9906ec7172930eee3d793b
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Praser

# Tool invocations
Praser: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
<<<<<<< HEAD
	g++ -L/usr/lib -L/home/charles/glui-2.36/src/lib -o "Praser" $(OBJS) $(USER_OBJS) $(LIBS)
=======
	g++ -L/home/simon/Downloads/glui-2.36/src/lib -L/usr/lib -L/usr/lib/i386-linux-gnu -o "Praser" $(OBJS) $(USER_OBJS) $(LIBS)
>>>>>>> 2de90737c4ed4d8a0b9906ec7172930eee3d793b
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) Praser
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
