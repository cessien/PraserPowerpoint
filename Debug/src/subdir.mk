################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/SceneDrawer.cpp \
../src/kinect.cpp \
../src/main.cpp \
../src/revelrecorder.cpp 

OBJS += \
./src/SceneDrawer.o \
./src/kinect.o \
./src/main.o \
./src/revelrecorder.o 

CPP_DEPS += \
./src/SceneDrawer.d \
./src/kinect.d \
./src/main.d \
./src/revelrecorder.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/charles/glui-2.36/src/include -I/usr/include/ni -I/usr/include/nite -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


