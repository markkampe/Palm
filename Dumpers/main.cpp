#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "palmarchive.h"

bool verbose = false;
bool whiny = false;
const char *format = 0;

struct option opts[] = {
		{"verbose", no_argument, 		0,	'v'},
		{"whiny", 	no_argument,		0,	'w'},
		{"format",	required_argument,	0,	'f'},
		{0, 0, 0, 0}
};

extern int process_datebook( PalmArchive *, const char *format );
extern int process_memos( PalmArchive *);
extern int process_todos( PalmArchive *);
extern int process_addrs( PalmArchive *);


int main( int argc, char **argv ) {
	int c;
	int optx = 0;
	while ((c = getopt_long(argc, argv, "vwf:", opts, &optx)) != -1) {
		switch(c) {
		case 'w':
			whiny = true;
			// fallsthrough
		case 'v':
			verbose = true;
			break;

		case 'f':
			format = optarg;
			break;
		}
	}
	int ret = 0;
	for( int i = optind; i < argc; i++ ) {
		// see if we can open this file as an archive
		PalmArchive *arc = new PalmArchive( argv[i] );
		if (arc->error() != 0) {
			fprintf( stderr, "Error (%s) initializing %s\n",
					arc->error(), argv[i] );
			ret |= 1;
		} else {
			if (arc->fileType() == arc->DBA_SIG) {
				ret = process_datebook(arc, format);
			} else if (arc->fileType() == arc->MEMO_SIG) {
				ret = process_memos(arc);
			} else if (arc->fileType() == arc->TODO_SIG) {
				ret = process_todos(arc);
			} else {
				fprintf(stderr, "%s is not a recognized archive, type=0x%08lx\n",
						argv[i], arc->fileType());
				ret = 1;
			}
		}
		delete arc;
	}

	return( ret );
}

