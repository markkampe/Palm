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
	bool	allday;

	Appt() {
		start_time = 0;
		end_time = 0;
		summary = 0;
		description = 0;
		pvt = 0;
		allday = 0;
		next = 0;
		last = 0;
	}

	~Appt();

	void add(time_t new_time);

	// vcalendar output functions
	static void header();
	bool dump_vcalendar( );
	static void trailer();

	// one line summary
	bool summarize( int );

   private:
	struct repetition {
		time_t start_time;
		struct repetition *next;
	} *next, *last;
};
