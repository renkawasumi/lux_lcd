SRCS	:= smile.c
obj-m	:= $(SRCS:.c=.o)
# ccflags-y  := -Wno-unused-variable
BUILD_DIR := /lib/modules/$(shell uname -r)/build

all:
	/bin/echo -e "\\033c"
	make -C $(BUILD_DIR) M=$(PWD) modules
	
clean:
	make -C $(BUILD_DIR) M=$(PWD) clean
	/bin/echo | sudo tee /var/log/kern.log
	/bin/echo | sudo tee /var/log/syslog
 
install:
	make -C $(BUILD_DIR) M=$(PWD) modules_install
	depmod -a
