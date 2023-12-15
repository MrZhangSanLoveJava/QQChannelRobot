


main:main.o qqserver.o
	gcc -o main main.o qqserver.o -l json-c -l curl -I ../include
qqserver.o:qqserver.c
	gcc -c qqserver.c -o qqserver.o -l json-c -l curl -I ../include
main.o:main.c
	gcc -c main.c -o main.o -I ../include
