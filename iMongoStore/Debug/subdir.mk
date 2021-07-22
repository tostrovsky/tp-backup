################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bitacoras.c \
../filesystem.c \
../iMongoStore.c \
../recursos.c \
../token.c \
../utils.c 

OBJS += \
./bitacoras.o \
./filesystem.o \
./iMongoStore.o \
./recursos.o \
./token.o \
./utils.o 

C_DEPS += \
./bitacoras.d \
./filesystem.d \
./iMongoStore.d \
./recursos.d \
./token.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


