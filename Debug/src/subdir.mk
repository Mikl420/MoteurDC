################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/GestionComm.cpp \
../src/MoteurDc.cpp \
../src/Pid.cpp 

CPP_DEPS += \
./src/GestionComm.d \
./src/MoteurDc.d \
./src/Pid.d 

OBJS += \
./src/GestionComm.o \
./src/MoteurDc.o \
./src/Pid.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-unknown-linux-gnueabihf-g++ -I"/home/isib/MoteurDc-workspace/MoteurDc/Include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/GestionComm.d ./src/GestionComm.o ./src/MoteurDc.d ./src/MoteurDc.o ./src/Pid.d ./src/Pid.o

.PHONY: clean-src

