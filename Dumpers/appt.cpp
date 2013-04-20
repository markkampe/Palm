/*
 * module:	appt.cpp
 *
 * purpose:	operations on generic appointments
 *
 */

#include <stdio.h>
#include <time.h>
#include "appt.h"

 Appt::~Appt() {
		if (summary) {
			free( summary );
			summary = 0;
		}
		if (description) {
			free( description );
			description = 0;
		}
		while(next) {
			struct repetition *r = next;
			next = r->next;
			free(r);
		}
		last = 0;
	}

 /*
  * add another instance of an appointment
  */
 void Appt::add(time_t new_time) {
	 struct repetition *r = (struct repetition *) malloc( sizeof (struct repetition));
	 r->start_time = new_time;
	 r->next = 0;
	 if (last == 0)
		 next = r;
	 else
		 last->next = r;
	 last = r;
 }

 /*
  * produce a single line description of a single event instance
  */
void print_summary(
		int n,		// appointment number
		time_t st,	// starting time
		time_t et,	// ending time
		char *d 	// description
	) {
		if (n >= 0)	// appointment #, may be empty for repetitions
			printf( "%5d: ", n );
		else
			printf( "  -- : ");

		// starting date ... and perhaps time
		struct tm tmstart;
		gmtime_r( &st, &tmstart );
		printf("%04d/%02d/%02d",
				tmstart.tm_year+1900, tmstart.tm_mon + 1, tmstart.tm_mday );

		if (et != 0) {		// only if it is not all-day
			// starting time
			printf(" %02d:%02d:%02dZ",
					tmstart.tm_hour, tmstart.tm_min, tmstart.tm_sec );

			// print end date if it ends on a different day
			struct tm tmend;
			gmtime_r( &et, &tmend );
			if (tmend.tm_year != tmstart.tm_year ||
					tmend.tm_mon != tmstart.tm_mon ||
					tmend.tm_mday != tmstart.tm_mday) {
				printf("-%04d/%02d/%02d ",
						tmend.tm_year+1900, tmend.tm_mon + 1, tmend.tm_mday );
			}

			// print end time if it has a non-zero duration
			if (et != st)
				printf(" %02d:%02d:%02dZ",
					tmend.tm_hour, tmend.tm_min, tmend.tm_sec );
		}

		// print summary, or failing that, the description
		if (d != 0)
			printf(" %s", d);

		printf("\n");

		}
 /*
  * generate a one line summary of an appointment
  *    (which may have associated repetitions)
  */
bool Appt::summarize( int apptnum ) {

	char *descr = (summary != 0) ? summary : description;
	if (allday) {
		print_summary(apptnum, start_time, 0, descr);
		for( struct repetition *r = next; r != 0; r = r->next ) {
			print_summary(-1, r->start_time, 0, descr);
		}
	} else {
		long duration = end_time - start_time;
		print_summary(apptnum, start_time, end_time, descr);
				for( struct repetition *r = next; r != 0; r = r->next ) {
					print_summary(-1, r->start_time, r->start_time + duration, descr);
				}
	}

	return( true );
}

void Appt::header() {
	printf("BEGIN:VCALENDAR\n");
	printf("VERSION: 2.0\n");
}

void Appt::trailer() {
	printf("END:VCALENDAR\n");
}

/*
  * produce a vcal for a single event instance
  */
void print_vcal(
		time_t st,	// starting time
		time_t et,	// ending time
		char *sum,	// summary
		char *desc,	// description
		bool allday,// all day long
		bool pvt	// private appointment
	) {
	struct tm tm;

	printf("BEGIN:VEVENT\n");

	gmtime_r( &st, &tm );
	if (allday)
		printf("DTSTART;VALUE=DATE:%04d%02d%02d\n",
				tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday);
	else
		printf("DTSTART:%04d%02d%02dT%02d%02d%02dZ\n",
			tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec );

	gmtime_r( &et, &tm );
	if (allday)
		printf("DTEND;VALUE=DATE:%04d%02d%02d\n",
				tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday);
	else
		printf("DTEND:%04d%02d%02dT%02d%02d%02dZ\n",
			tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec );

	if (sum)
		printf("SUMMARY:%s\n", sum);
	if (desc)
		printf("DESCRIPTION:%s\n", desc);
	if (pvt) 
		printf("CLASS:PRIVATE\n");

	printf("END:VEVENT\n");
}

bool Appt::dump_vcalendar( ) {

	long duration = end_time - start_time;
	print_vcal(start_time, end_time, summary, description, allday, pvt);
	for( struct repetition *r = next; r != 0; r = r->next ) {
		print_vcal(r->start_time, r->start_time + duration,
				summary, description, allday, pvt );
	}

	return( true );
}
