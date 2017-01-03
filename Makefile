PWD	:= $(shell pwd)
EXTRA_CFLAGS	:= -I$(PWD)/src/include -I.

obj-m	+= libfann.o jiffies.o nn.o
libfann-objs	:= math.o twister.o libfann_kmod.o src/fann.o src/fann_cascade.o src/fann_train_data.o src/fann_train.o src/fann_activation.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) $(EXTRA_CFLAGS) modules
	$(CC) test.c -o test -lm

rmmod:
	sudo rmmod nn

insmod:
	sudo insmod nn.ko
	cat xor_init.data > /proc/nn/init

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	$(RM) Module.markers modules.order
	rm test
