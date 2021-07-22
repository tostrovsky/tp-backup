################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MI\ RAM\ HQ/miRamHQ.c \
../MI\ RAM\ HQ/utils.c 

OBJS += \
./MI\ RAM\ HQ/miRamHQ.o \
./MI\ RAM\ HQ/utils.o 

C_DEPS += \
./MI\ RAM\ HQ/miRamHQ.d \
./MI\ RAM\ HQ/utils.d 


# Each subdirectory must supply rules for building sources it contributes
MI\ RAM\ HQ/miRamHQ.o: ../MI\ RAM\ HQ/miRamHQ.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"MI RAM HQ/miRamHQ.d" -MT"MI\ RAM\ HQ/miRamHQ.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

MI\ RAM\ HQ/utils.o: ../MI\ RAM\ HQ/utils.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"MI RAM HQ/utils.d" -MT"MI\ RAM\ HQ/utils.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


