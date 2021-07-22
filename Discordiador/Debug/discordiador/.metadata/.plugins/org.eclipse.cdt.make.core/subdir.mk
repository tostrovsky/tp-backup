################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../discordiador/.metadata/.plugins/org.eclipse.cdt.make.core/specs.c 

OBJS += \
./discordiador/.metadata/.plugins/org.eclipse.cdt.make.core/specs.o 

C_DEPS += \
./discordiador/.metadata/.plugins/org.eclipse.cdt.make.core/specs.d 


# Each subdirectory must supply rules for building sources it contributes
discordiador/.metadata/.plugins/org.eclipse.cdt.make.core/%.o: ../discordiador/.metadata/.plugins/org.eclipse.cdt.make.core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


