PGMS=palm_datebook_dump

CC = g++
GDB = -ggdb
CFLAGS = $(GDB)

%.o : %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PGMS)

clean: 
	rm -f *.o

clobber: 
	rm -f $(PGMS) *.o

palm_datebook_dump: main.o datebook.o palmarchive.o appt.o
	$(CC) $(GDB) -o $@ $^

datebook.o: datebook.cpp palmarchive.h appt.h

palmarchive.o: palmarchive.cpp palmarchive.h

main.o: main.cpp palmarchive.h

appt.o:: appt.cpp appt.h
