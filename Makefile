myapp: STATS.o
	gcc -o myapp STATS.o

STATS.o: STATS.c arrayStruct.h semun.h
	gcc -c STATS.c
