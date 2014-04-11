# Make file

net367: host.o utilities.o link.o man.o main.o net.o switchlink.o switch.o pkqueue.o fwtable.o top.o
	gcc -o net367 host.o utilities.o link.o man.o main.o net.o switchlink.o switch.o pkqueue.o fwtable.o top.o

top.o: top.c
	gcc -c top.c

fwtable.o: fwtable.c
	gcc -c fwtable.c

main.o: main.c
	gcc -c main.c

host.o: host.c 
	gcc -c host.c  

man.o:  man.c
	gcc -c man.c

net.o:  net.c
	gcc -c net.c

utilities.o: utilities.c
	gcc -c utilities.c

link.o:  link.c
	gcc -c link.c

switch.o: switch.c
	gcc -c switch.c 

pkqueue.o: pkqueue.c
	gcc -c pkqueue.c

switchlink.o: switchlink.c
	gcc -c switchlink.c

clean:
	rm -rf *o net367 pkqueue switch switchlink fwtable top
