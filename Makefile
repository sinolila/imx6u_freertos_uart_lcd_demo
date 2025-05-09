CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET		  	?= uart

CC 				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP 		:= $(CROSS_COMPILE)objdump


# 这里的路径需要根据自己的路径设置
#LIBPATH			:= -lgcc -L /usr/lib/gcc-cross/arm-linux-gnueabihf/7.5.0 \
                            /usr/arm-linux-gnueabihf/lib/libc.a
LIBPATH				:= -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4 \
                            /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/lib/libc.a



INCDIRS 		:= project \
                   imx6ul \
				   bsp/clk \
				   bsp/led \
				   bsp/delay \
				   bsp/beep \
				   bsp/gpio \
				   bsp/key \
				   bsp/exit \
				   bsp/int \
				   bsp/epittimer \
				   bsp/keyfilter \
				   bsp/uart \
				   bsp/lcd \
				   bsp/rtos/freertos \
				   bsp/rtos/freertos/include \
				   bsp/rtos/freertos/portable/GCC/ARM_CA9 \
				   bsp/rtos/freertos/portable/low_power_tickless \
				   bsp/rtos/freertos/portable/MemMang \
				   bsp/stdio/include


		

				   			   
SRCDIRS			:= project \
                   imx6ul \
				   bsp/clk \
				   bsp/led \
				   bsp/delay \
				   bsp/beep \
				   bsp/gpio \
				   bsp/key \
				   bsp/exit \
				   bsp/int \
				   bsp/epittimer \
				   bsp/keyfilter \
				   bsp/lcd \
				   bsp/uart \
				   bsp/rtos/freertos \
				   bsp/rtos/freertos/include \
				   bsp/rtos/freertos/portable/GCC/ARM_CA9 \
				   bsp/rtos/freertos/portable/low_power_tickless \
				   bsp/rtos/freertos/portable/MemMang\
				   bsp/stdio/lib 
				   
				   
INCLUDE			:= $(patsubst %, -I %, $(INCDIRS))

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR		:= $(notdir  $(SFILES))
CFILENDIR		:= $(notdir  $(CFILES))

SOBJS			:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS			:= $(SOBJS) $(COBJS)

VPATH			:= $(SRCDIRS)

.PHONY: clean
$(TARGET).bin : $(OBJS)
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^ $(LIBPATH) 
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -mfpu=neon-vfpv4 -nostdlib -fno-builtin -c -O2  $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -Wa,-mimplicit-it=thumb -mfpu=neon-vfpv4 -nostdlib -fno-builtin -c -O2  $(INCLUDE) -o $@ $<
	
clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(COBJS) $(SOBJS)

	
