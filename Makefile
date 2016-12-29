PWD	:= $(shell pwd)
EXTRA_CFLAGS	:= -I$(PWD)/src/include -I.

obj-m	+= libfann.o
libfann-objs	:= math.o twister.o libfann_kmod.o src/fann.o src/fann_cascade.o src/fann_train_data.o src/fann_train.o src/fann_activation.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) $(EXTRA_CFLAGS) modules
	$(CC) test.c -o test

rmmod:
	sudo rmmod libfann

insmod:
	sudo insmod libfann.ko lfnum_layers=3 lfnum_input=2 lfnum_neurons_hidden=3 lfnum_output=1 lfnum_data=4;
	sudo chmod 666 /dev/NN

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	$(RM) Module.markers modules.order
	rm test
