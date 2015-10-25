################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../configuracion.c \
../interprete.c \
../monitorSocket.c \
../socketCliente.c \
../socketServidor.c 

OBJS += \
./configuracion.o \
./interprete.o \
./monitorSocket.o \
./socketCliente.o \
./socketServidor.o 

C_DEPS += \
./configuracion.d \
./interprete.d \
./monitorSocket.d \
./socketCliente.d \
./socketServidor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


