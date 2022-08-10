################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/src/clock_init.c \
../Core/src/delay.c \
../Core/src/main.c \
../Core/src/pwm_init.c \
../Core/src/rcc_set.c \
../Core/src/shapes.c \
../Core/src/systick.c \
../Core/src/tim1_init.c \
../Core/src/tim2_enc.c \
../Core/src/tim3_enc.c \
../Core/src/tim4_enc.c \
../Core/src/trash_func.c 

OBJS += \
./Core/src/clock_init.o \
./Core/src/delay.o \
./Core/src/main.o \
./Core/src/pwm_init.o \
./Core/src/rcc_set.o \
./Core/src/shapes.o \
./Core/src/systick.o \
./Core/src/tim1_init.o \
./Core/src/tim2_enc.o \
./Core/src/tim3_enc.o \
./Core/src/tim4_enc.o \
./Core/src/trash_func.o 

C_DEPS += \
./Core/src/clock_init.d \
./Core/src/delay.d \
./Core/src/main.d \
./Core/src/pwm_init.d \
./Core/src/rcc_set.d \
./Core/src/shapes.d \
./Core/src/systick.d \
./Core/src/tim1_init.d \
./Core/src/tim2_enc.d \
./Core/src/tim3_enc.d \
./Core/src/tim4_enc.d \
./Core/src/trash_func.d 


# Each subdirectory must supply rules for building sources it contributes
Core/src/%.o: ../Core/src/%.c Core/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I"C:/Users/ALEX/STM32CubeIDE/workspace_1.7.0/STM32F103C8T6_CMSIS_04_encoder/CMSIS/inc" -I"C:/Users/ALEX/STM32CubeIDE/workspace_1.7.0/STM32F103C8T6_CMSIS_04_encoder/Core/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

