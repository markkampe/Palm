/*
 * memo.cpp
 *
 * 	process a memo palm archive
 *
 * 	... abandoned because the archives from my palm were not this format
 * 	but rather 'cafebabe'
 */

#include "palmarchive.h"

int process_memos( PalmArchive *arc) {

		const int FIELDS_PER_ENTRY	= 6;

		// make sure that it is, in fact, a datebook archive
		if (arc->fileType() != arc->MEMO_SIG) {
			fprintf(stderr, "ERROR: file is not a Memo Archive\n");
			return 1;
		}

		// make sure I understand it as such
		if (arc->fields_per_row() != FIELDS_PER_ENTRY) {
			fprintf(stderr, "ERROR: fields per row = %d, expected %d\n",
					arc->fields_per_row(), FIELDS_PER_ENTRY);
			return( 1 );
		}

		// the next 4-bytes should be the number of datebook entries
		// multiplied by 6 (number of fields per entry)
		long num_entry = arc->readUlong();
		if (num_entry % FIELDS_PER_ENTRY != 0) {
			fprintf( stderr, "# entries (%ld) not a multiple of %d\n",
				num_entry, FIELDS_PER_ENTRY );
		}
		num_entry /= FIELDS_PER_ENTRY;

		if (num_entry < 0 || num_entry > 1000000) {
			fprintf( stderr, "Unreasonable number of entries: %ld\n",
				num_entry );
			return( 1 );
		}

		int processed = 0;
		int discards = 0;

		return( 0 );
}
