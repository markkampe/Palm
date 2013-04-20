/*
 * module:	datebook.c
 *
 * purpose:	to process a Palm Datebook Archive
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "palmarchive.h"
#include "appt.h"

extern bool verbose;	// commentary on what we find
extern bool whiny;		// complaints about what we find

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
	
	checkType( "record ID", 1 );	// 1: record ID
	rid = pa->readUlong();

	checkType( "status", 1 );		// 2: appointment status
	unsigned long sts = pa->readUlong();
	Appt *thisappt = (sts == 0x04) ? 0 : new Appt;

	checkType( "position", 1 );		// 3: position ???
	unsigned long pos = pa->readUlong();

	checkType( "start time", 3 );	// 4: starting time
	unsigned long startTime = pa->readUlong();
	if (thisappt)
		thisappt->start_time = startTime;	// standard Unix time

	checkType( "end time", 1 );		// 5: ending time
	unsigned long endTime = pa->readUlong();

	checkType( "description", 5 );	// 6: description
	(void) pa->readUlong();	// padding
	char *descr = pa->readCstring();
	if (descr && thisappt) {
		nonewlines( descr );
		thisappt->summary = descr;
	}

	checkType( "duration", 1 );		// 7: duration
	unsigned long duration = pa->readUlong();
	// sometimes they use endtime, sometimes duration
	if (startTime == endTime && duration > 0)
		endTime = startTime + duration;
	if (thisappt)
		thisappt->end_time = endTime;

	checkType( "note", 5 );			// 8: note
	(void) pa->readUlong();	// padding
	char *note = pa->readCstring();
	if (note && thisappt) {
		nonewlines( note );
		thisappt->description = note;
	}

	checkType( "untimed", 6 );		// 9: untimed ???
	bool untimed  = pa->readUlong();
	if (untimed && thisappt)
		thisappt->allday = true;

	checkType( "private", 6 );		// 10: private appointment
	bool pvt = pa->readUlong();
	if (thisappt)
		thisappt->pvt = pvt;

	checkType( "category", 1 );		// 11: category ???
	unsigned long category = pa->readUlong();

	checkType( "alarm set", 6 );	// 12: alarm set for this appointment
	bool alarm_set = pa->readUlong();

	checkType( "alarm units", 1 );	// 13: how far in advance to give alarm
	unsigned long alarm_units = pa->readUlong();

	checkType( "alarm type", 1 );	// 14: alarm units (0->min, 1->hours, 2->days)
	unsigned long alarm_type = pa->readUlong();

	checkType( "repeat", 8 );		// 15: repeat information

	unsigned short num_except = pa->readUshort();	// 15a # exceptions
	unsigned long *excepts = 0;
	if (num_except > 0) {
		excepts = (unsigned long *) malloc(num_except * sizeof (unsigned long));
		for( int i = 0; i < num_except; i++ ) {
			excepts[i] = pa->readUlong();			// 15b exception entries
		}
	}

	// repeat event class entry
	unsigned short flag = pa->readUshort();			// 15c type of repeat event
	char *classname = 0;
	if (flag == 0xffff) {			// 15d class entries
		unsigned short tag = pa->readUshort();
		if (tag != 1) {
			fprintf(stderr, "ERROR - Exception class entry, tag (%d) != 1\n", tag);
			return(0);
		}
		unsigned short len = pa->readUshort();
		classname = (char *) malloc( len + 1 );
		for( int i = 0; i < len; i++ ) {
			classname[i] = pa->readUbyte();
		}
		classname[len] = 0;
		// these seem to be completely ignorable ???
	}

	if (flag != 0) {
		unsigned long brand = pa->readUlong();
		unsigned long interval = pa->readUlong();
		unsigned long enddate = pa->readUlong();
		unsigned long wstart = pa->readUlong();

		// each supported repetition type (brand) has different args
		if (brand < 1 || brand > 6) {
			fprintf(stderr, "fatal: unrecognized repetition brand: %ld", brand);
			exit( -1 );		// at this point, we have lost sync w/stream
		}

		// figure out what fields we expect to find
		const int has_day_x = 1;
		const int has_day_m = 2;
		const int has_day_n = 4;
		const int has_week_x = 8;
		const int has_mon_x = 16;
		unsigned char brandmask[] = {
				0,						// 0: undefined
				has_day_x,				// 1: daily
				has_day_x+has_day_m,	// 2: weekly, by days
				has_day_x+has_week_x,	// 3: monthly, by day
				has_day_n,				// 4: monthly, by date
				has_day_n+has_mon_x,	// 5: yearly, by date
				has_day_x				// 6: yearly, by day
		};

		unsigned long day_x = 0;
		if (brandmask[brand] & has_day_x) {
			day_x = pa->readUlong();
		}
		unsigned char day_mask = 0;
		if (brandmask[brand] & has_day_m) {
			day_mask = pa->readUbyte();
			if (day_mask == 0 || day_mask > 0x7f)
				fprintf(stdout, "WARNING - day mask = 0x%x", day_mask);
		}
		unsigned long week_x = 0;
		if (brandmask[brand] & has_week_x) {
			week_x = pa->readUlong();
		}
		unsigned long day_num = 0;
		if (brandmask[brand] & has_day_n) {
			day_num = pa->readUlong();
		}
		unsigned long mon_x = 0;
		if (brandmask[brand] & has_mon_x) {
			mon_x = pa->readUlong();
		}

		// unix time values for common intervals
		const int DAY = 24 * 60 * 60;

		// generate all the repetitions of this event
		time_t d = startTime;
		time_t next = d + DAY;		// default: day at a time
		while(d < enddate) {
			// deconstruct this into its date components
			struct tm tm;
			gmtime_r( &d, &tm );

			// see if this matches the specified pattern
			switch( brand ) {
			case 1: // daily ... parameter: interval
				break;

			case 2: // weekly by day ... parameters: day mask
				if ((day_mask & (1<<tm.tm_wday)) == 0)
					goto skip;
				// FIX - the above should probably be corrected for wstart
				//	     but I had no sample data from which to understand
				//		 its precise sense.
				break;

			case 3: // monthly by day ... parameters: day, week
				if (tm.tm_wday != day_x - 1)	// right day?
					goto skip;
				if (tm.tm_mday <= (week_x - 1) * 7)	// right week?
					goto skip;
				if (tm.tm_mday > week_x * 7)	// right week?
					goto skip;
				break;

			case 4: // monthly by date ... parameters: day number
				if (tm.tm_mday != day_num)
					goto skip;
				break;

			case 5: // annual by date ... parameters: month and day
				if (tm.tm_mon != mon_x)
					goto skip;
				if (tm.tm_mday != day_num)
					goto skip;
				break;

			case 6: // FIX - annual by day
					// 		I am totally unclear on exactly what this means
					//		and I had no sample data from which to understand it
				fprintf(stderr, "ERROR: annual by day repetition\n");
				d = enddate;
				goto skip;
			}

			// the date of the original event is not a repetition
			if (d == startTime)
				goto skip;

			// see if this date is on the exception list
			for( int i = 0; i < num_except; i++ ) {
					if (d >= excepts[i] && d < excepts[i] + DAY)
						goto skip;
			}

			// attach this date as a repetition instance
			thisappt->add(d);

			skip:	// try the next candidate
				d += DAY;
		}
	}

	if (excepts != 0)
		free(excepts);
	if (classname != 0)
		free(classname);

	return( thisappt );
}


/*
 * process a datebook archive
 */
int process_datebook( PalmArchive *arc, const char *format ) {

	const int FIELDS_PER_ENTRY	= 15;


	// make sure that it is, in fact, a datebook archive
	if (arc->fileType() != arc->DBA_SIG) {
		fprintf(stderr, "ERROR: file is not a DateBook Archive\n");
		return 1;
	}

	// make sure I understand it as such
	if (arc->fields_per_row() != FIELDS_PER_ENTRY) {
		fprintf(stderr, "ERROR: fields per row = %d, expected %d\n",
				arc->fields_per_row(), FIELDS_PER_ENTRY);
		return( 1 );
	}

	// the next 4-bytes should be the number of datebook entries
	// multiplied by 15 (number of fields per entry)
	long num_entry = arc->readUlong();
	if (num_entry % FIELDS_PER_ENTRY != 0) {
		fprintf( stderr, "# datebook entries (%ld) not a multiple of %d\n",
			num_entry, FIELDS_PER_ENTRY );
	}
	num_entry /= FIELDS_PER_ENTRY;

	if (num_entry < 0 || num_entry > 1000000) {
		fprintf( stderr, "Unreasonable number of datebook entries: %ld\n",
			num_entry );
		return( 1 );
	}

	int processed = 0;
	int discards = 0;

	if (format != 0 && strcmp(format, "vcalendar") == 0)
		Appt::header();

	for( int i = 0; i < num_entry; i++ ) {
		Appt *a = datebook_entry( arc );
		if (a) {
			if (format != 0 && strcmp(format, "vcalendar") == 0)
				a->dump_vcalendar();
			else
				a->summarize( i+1 );
			processed++;

			// we're done with this one
			delete a;
		} else {
			discards++;
		}
	}

	if (format != 0 && strcmp(format, "vcalendar") == 0)
		Appt::trailer();

	if (verbose) {
		fprintf(stderr, "expected %ld, processed %d, discarded %d\n",
				num_entry, processed, discards);
	}
	return( 0 );
}
