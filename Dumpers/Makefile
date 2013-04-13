PGMS=palm_datebook_dump

CC = g++
GDB = -ggdb

all: $(PGMS)

clean: 
	rm -f *.o

clobber: 
	rm -f $(PGMS) *.o

palm_datebook_dump: datebook.o palmarchive.o appt.o
	$(CC) $(GDB) -o $@ $^

datebook.o: datebook.cpp palmarchive.h appt.h

palmarchive.o: palmarchive.cpp palmarchive.h

appt.o:: appt.cpp appt.h
