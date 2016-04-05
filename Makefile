client : socklib.o main.o information.o
	gcc -g -o client socklib.o main.o information.o -lm -lpthread
socklib.o : socklib.c socklib.h
	gcc -g -c socklib.c
information.o : information.c information.h
	gcc -g -c information.c
main.o : main.c
	gcc -g -c main.c

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
