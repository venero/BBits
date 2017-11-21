obj-m += bbit.o

all: 
	make -C /home/venero/workspace/git/linux-nova-dev M=$(PWD) modules

clean:
	make -C /home/venero/workspace/git/linux-nova-dev M=$(PWD) clean