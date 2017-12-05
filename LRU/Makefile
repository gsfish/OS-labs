objects = main.o simlru.o

main: $(objects)
	gcc -o test $(objects)

main.o: main.c simlru.h

simlru.o: simlru.c simlru.h

.PHONY: clean
clean:
	-rm test $(objects)