################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../I-Mongo-Store/iMongoStore.c \
../I-Mongo-Store/utils.c 

OBJS += \
./I-Mongo-Store/iMongoStore.o \
./I-Mongo-Store/utils.o 

C_DEPS += \
./I-Mongo-Store/iMongoStore.d \
./I-Mongo-Store/utils.d 


# Each subdirectory must supply rules for building sources it contributes
I-Mongo-Store/%.o: ../I-Mongo-Store/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


