################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c \
C:/Users/UW-Stream/Desktop/Sailboat\ Julia/Projects/src/sail_gps.c 

OBJS += \
./src/main.o \
./src/sail_gps.o 

C_DEPS += \
./src/main.d \
./src/sail_gps.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -I"C:/Users/UW-Stream/Desktop/Sailboat Julia/Projects/GPS_test/../src/" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sail_gps.o: C:/Users/UW-Stream/Desktop/Sailboat\ Julia/Projects/src/sail_gps.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -I"C:/Users/UW-Stream/Desktop/Sailboat Julia/Projects/GPS_test/../src/" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/sail_gps.d" -MT"src/sail_gps.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


