/*
 * module:	appt.h
 *
 * purpose:	generic appointments and their output
 *		in various useful forms
 */

#include <time.h>
#include <stdlib.h>

// appointment structure used in memory
class Appt {
   public:
	time_t	start_time;
	time_t	end_time;
	char	*summary;
	char	*description;
	bool	pvt;

	Appt() {
		start_time = 0;
		end_time = 0;
		summary = 0;
		description = 0;
		pvt = 0;
	}

	~Appt() {
		if (summary) {
			free( summary );
			summary = 0;
		}
		if (description) {
			free( description );
			description = 0;
		}
	}

	bool dump_vcalendar( );		// vcalendar
	bool dump_vcal( int  );		// vcal
	bool summarize( int );		// one liner
};
