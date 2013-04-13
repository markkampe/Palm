/*
 * module:	appt.cpp
 *
 * purpose:	operations on generic appointments
 *
 */

#include <stdio.h>
#include <time.h>
#include "appt.h"

bool Appt::summarize( int apptnum ) {

	struct tm tmstart;
	struct tm tmend;

	gmtime_r( &start_time, &tmstart );
	gmtime_r( &end_time, &tmend );

	printf( "%5d: ", apptnum );

	printf("%04d/%02d/%02d", tmstart.tm_year+1900, tmstart.tm_mon + 1, tmstart.tm_mday );
	printf(" %02d:%02d:%02dZ-", tmstart.tm_hour, tmstart.tm_min, tmstart.tm_sec );

	if (tmend.tm_year != tmstart.tm_year ||
	    tmend.tm_mon != tmstart.tm_mon ||
	    tmend.tm_mday != tmstart.tm_mday) {
		printf("%04d/%02d/%02d ", 
			tmend.tm_year+1900, tmend.tm_mon + 1, tmend.tm_mday );
		printf(" %02d:%02d:%02dZ-", 
			tmend.tm_hour, tmend.tm_min, tmend.tm_sec );
	} else if (tmend.tm_hour != tmstart.tm_hour ||
	    tmend.tm_min != tmstart.tm_min ||
	    tmend.tm_sec != tmstart.tm_sec) {
		printf(" %02d:%02d:%02dZ-", 
			tmend.tm_hour, tmend.tm_min, tmend.tm_sec );
	}

	if (summary)
		printf(" %s", summary);
	else if (description)
		printf(" %s", description);

	printf("\n");

	return( true );
}

bool Appt::dump_vcalendar( ) {

	struct tm tm;

	printf("BEGIN:VEVENT\n");

	gmtime_r( &start_time, &tm );
	printf("DTSTART:%04d%02d%02dT%02d%02d%02dZ\n",
		tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec );

	gmtime_r( &end_time, &tm );
	printf("DTEND:%04d%02d%02dT%02d%02d%02dZ\n",
		tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec );

	if (summary)
		printf("SUMMARY:%s\n", summary);
	if (description)
		printf("DESCRIPTION:%s\n", description);
	if (pvt) 
		printf("CLASS:PRIVATE\n");

	printf("END VEVENT\n");

	return( true );
}

bool Appt::dump_vcal( int event_num ) {

	struct tm tmstart;
	struct tm tmend;

	printf("EVENT[%d]=", event_num);

	gmtime_r( &start_time, &tmstart );
	printf("START_T:%02d+%02d+%02d",
		tmstart.tm_hour, tmstart.tm_min, tmstart.tm_sec );

	gmtime_r( &end_time, &tmend );
	printf(";END_T:%02d+%02d+%02d",
		tmend.tm_hour, tmend.tm_min, tmend.tm_sec );

	printf(";START_D:%02d+%02d+%04d",
		tmstart.tm_mday, tmstart.tm_mon + 1, tmstart.tm_year+1900 );

	printf(";END_D:%02d+%02d+%04d",
		tmend.tm_mday, tmend.tm_mon + 1, tmend.tm_year+1900 );

	if (summary)
		printf(";NAME:%s", summary);
	if (description)
		printf(";NOTE:%s", description);

	printf("\n");

	return( true );
}
