/*
 * module:	datebook.c
 *
 * purpose:	to process a Palm Datebook Archive
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "palmarchive.h"
#include "appt.h"

bool verbose = true;


#define	FIELDS_PER_DATEBOOK_ENTRY	15

Appt *datebook_entry( PalmArchive * );

int main( int argc, char **argv ) {

	const char *name = argv[1];
	PalmArchive *arc = new PalmArchive( name );
	const char *err = arc->error();
	if (err) {
		fprintf( stderr, "Error (%s) initializing %s\n", 
			err, name );
		return( 1 );
	}

	if (verbose) {
		printf( "Palm Archve: %s\n", name );
		printf( "     TYPE=%s (0x%08lx)\n", arc->typeName(), arc->fileType() );
		printf( "     filename=%s\n", arc->fileName() );
		printf( "     header=%s\n", arc->headerString() );
	}

	if (arc->fileType() != arc->DBA_SIG) {
		fprintf( stderr, "%s is not a Datebook Archive, type=0x%08lx\n",
			name, arc->fileType() );
		delete arc;
		return( 1 );
	}

	// the next 4-bytes should be the number of datebook entries
	// multiplied by 15 (number of fields per entry)
	long num_entry = arc->readUlong();
	if (num_entry % FIELDS_PER_DATEBOOK_ENTRY != 0) {
		fprintf( stderr, "# datebook entries (%ld) not a multiple of %d\n",
			num_entry, FIELDS_PER_DATEBOOK_ENTRY );
	}
	num_entry /= FIELDS_PER_DATEBOOK_ENTRY;

	if (num_entry < 0 || num_entry > 1000000) {
		fprintf( stderr, "Unreasonable number of datebook entries: %ld\n",
			num_entry );
		delete arc;
		return( 1 );
	}
	if (verbose) {
		printf( "     num_entries=%ld\n", num_entry );
	}

	for( int i = 0; i < num_entry; i++ ) {
		Appt *a = datebook_entry( arc );
		if (a) {
			a->summarize( i+1 );
//			a->dump_vcal( i+1 );
//			a->dump_vcalendar( );
			
			// we're done with this one
			delete a;
		}
	}
	
	delete arc;
	return( 0 );
}

/*
 * routine:	nonewlines
 * 
 * purpose:	ensure that a string contains no new lines
 */
void nonewlines( char *str ) {
	
	// nothing to fix
	if (str == 0 || *str == 0)
		return;

	// forward pass, turn them all into spaces
	char *s;
	for( s = str; *s; s++ ) {
		if (*s == '\r' || *s == '\n')
			*s = ' ';
	}

	// backwards pass, loose them at end of line
	for( s--; s > str && *s == ' '; s-- ) {
		*s = 0;
	}
}

/*
 * routine:	datebook_entry
 *
 * purpose:	to read one datebook entry from a datebook archive
 *	
 * returns:	my own standard Appt object
 *
 * note:	there are numerous fields that I don't care about,
 *		but the DBA archive stream is such that I have to
 *		read them to find what comes after them. 
 */
Appt *datebook_entry( PalmArchive *pa ) {

	unsigned long rid = 0;	// start out initialized

/*
 * This macro performs a check that I do once for each field
 * just making sure that each field has the expected type.  If
 * it doesn't, we have a mal-formatted archive!
 */
#define	checkType(f,v) { \
	unsigned long ret = pa->readUlong();	\
	if (ret != v) {				\
		fprintf(stderr,			\
			"record %ld, field %s, type %ld != %d\n",	\
			 rid, f, ret, v );	\
		return( 0 );			\
	}					\
    }
	
	checkType( "record ID", 1 );
	rid = pa->readUlong();

	checkType( "status", 1 );
	unsigned long sts = pa->readUlong();

	checkType( "position", 1 );
	unsigned long pos = pa->readUlong();

	checkType( "start time", 3 );
	unsigned long startTime = pa->readUlong();

	checkType( "end time", 1 );
	unsigned long endTime = pa->readUlong();

	checkType( "description", 5 );
	(void) pa->readUlong();	// padding
	char *descr = pa->readCstring();
	if (descr)
		nonewlines( descr );

	checkType( "duration", 1 );
	unsigned long duration = pa->readUlong();

	checkType( "note", 5 );
	(void) pa->readUlong();	// padding
	char *note = pa->readCstring();
	if (note)
		nonewlines( note );

	checkType( "untimed", 6 );
	bool untimed  = pa->readUlong();

	checkType( "private", 6 );
	bool pvt = pa->readUlong();

	checkType( "category", 1 );
	unsigned long category = pa->readUlong();

	checkType( "alarm set", 6 );
	bool alarm_set = pa->readUlong();

	checkType( "alarm units", 1 );
	unsigned long alarm_units = pa->readUlong();

	checkType( "alarm type", 1 );
	unsigned long alarm_type = pa->readUlong();

	checkType( "repeat", 8 );
	unsigned short num_except = pa->readUshort();
	for( int i = 0; i < num_except; i++ ) {
		(void) pa->readUlong();
	}

	// repeat event class entry
	unsigned short flag = pa->readUshort();
	if (flag == 0xffff) {
		unsigned short tag = pa->readUshort();
		unsigned short len = pa->readUshort();
		char *classname = (char *) malloc( len + 1 );
		for( int i = 0; i < len; i++ ) {
			classname[i] = pa->readUbyte();
		}
		classname[len] = 0;

		// since I don't process these, they are worth noting
		fprintf(stderr, "WARNING: Found class name %s\n", classname);
	}

	if (flag != 0) {
		unsigned long brand = pa->readUlong();
		unsigned long interval = pa->readUlong();
		unsigned long enddate = pa->readUlong();
		unsigned long wstart = pa->readUlong();
		unsigned long day_x = 0;
		unsigned char day_mask = 0;
		unsigned long week_x = 0;
		unsigned long day_num = 0;
		unsigned long mon_x = 0;

		if (brand == 1 || brand == 2 || brand == 3) {
			day_x = pa->readUlong();
		}
		if (brand == 2) {
			day_mask = pa->readUbyte();
		}
		if (brand == 3) {
			week_x = pa->readUlong();
		}
		if (brand == 4 || brand == 5) {
			day_num = pa->readUlong();
		}
		if (brand == 5) {
			mon_x = pa->readUlong();
		}
	}
	
	// has this appointment been deleted
	if (sts == 0x04)
		return( 0 );

	// it is valid, so we can start filling in the new appointment
	Appt *thisappt = new Appt;
	thisappt->start_time = startTime;	// standard Unix time

	// sometimes they use duration rather than ending time
	if (startTime == endTime && duration > 0)
		thisappt->end_time = startTime + duration;
	else
		thisappt->end_time = endTime;

	if (descr) 
		thisappt->summary = descr;
	if (note)
		thisappt->description = note;

	thisappt->pvt = pvt;

	return( thisappt );
}
