client : socklib.o main.o
	gcc -g -o client socklib.o main.o -lm
socklib.o : socklib.c socklib.h
	gcc -g -c socklib.c
main.o : main.c
	gcc -g -c main.c

.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
