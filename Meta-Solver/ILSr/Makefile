#I_CF =-O3 -funroll-loops -fforce-mem  -fomit-frame-pointer -Wall
I_CF =-O3 -funroll-loops  -Wall -std=c99
#I_CF = -O3 -funroll-loops -fforce-mem  -fomit-frame-pointer -mcpu=k6  -g -Wall 
#I_CF = -O0 -g -Wall 
CFLAGS =  $(I_CF) -DI_CFLAGS="\"$(I_CF)\""  #-a -pg
CC = gcc
LDLIBS = -lm #-a -pg 


.PHONY = clean all

all: ils_lop


ils_lop: ils_lop.o instance.o utilities.o timer.o lop.ck.o ck.o ac.o

clean:	
	rm -f *.o *~ ils_lop

#%.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $* 

# timer.o: timer.h timer.c
# 	gcc -O -c timer.c -o timer.o -ansi -pedantic -Wall

# instance.o: instance.h instance.c
# 	gcc -O -c instance.c -o instance.o -ansi -pedantic -Wall

# utilities.o: utilities.h utilities.c
# 	gcc -O -c utilities.c -o utilities.o -ansi -pedantic -Wall




