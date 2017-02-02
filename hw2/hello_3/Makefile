obj-m += hello_3.o

all:
	make -C /lib/modules/3.16.0-4-amd64/build M=$(PWD) modules

clean:
	make -C /lib/modules/3.16.0-4-amd64/build M=$(PWD) clean
