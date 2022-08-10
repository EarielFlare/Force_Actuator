################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS/src/system_stm32f1xx.c 

OBJS += \
./CMSIS/src/system_stm32f1xx.o 

C_DEPS += \
./CMSIS/src/system_stm32f1xx.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/src/%.o: ../CMSIS/src/%.c CMSIS/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I"C:/Users/ALEX/STM32CubeIDE/workspace_1.7.0/STM32F103C8T6_CMSIS_04_encoder/CMSIS/inc" -I"C:/Users/ALEX/STM32CubeIDE/workspace_1.7.0/STM32F103C8T6_CMSIS_04_encoder/Core/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

