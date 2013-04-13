/*
 * module:	PalmArchive.cpp
 *
 * purpose:	Reading Palm Archives
 *
 * note:	The trick with Palm archives is that they are
 *		do not have a traditional (static) in-memory
 *		structure.  Rather they are byte streams with
 *		embededd types and counts that tell you how to
 *		interpret the bytes that follow.
 */

#include "palmarchive.h"
#include <stdlib.h>

// sanity check limits
static const int MAX_CATEGORIES = 64;

/*
 * method: constructor (for an already open file)
 */
PalmArchive::PalmArchive( FILE *openfile ) {
	_file = openfile;
	init();
}

/*
 * method: constructor (with a specified file name)
 */
PalmArchive::PalmArchive( const char *filename ) {

	_file = fopen( filename, "r" );
	if (_file == NULL) {
		_errstr = "Unable to open file";
		return;
	}
	init();
}

void PalmArchive::init() {
	_errstr = 0;
	_filetype = 0;
	_filename = 0;
	_header = 0;
	_categories = 0;

	readHeader();
}

PalmArchive::~PalmArchive() {

	if (_file) {
		fclose( _file );
		_file = NULL;
	}

	if (_filename) {
		free( _filename );
		_filename = 0;
	}

	if (_header) {
		free( _header );
		_header = 0;
	}

	if (_categories) {
		for( int i = 0; i < _num_categories; i++ ) {
			if (_categories[i]) {
				free( _categories[i] );
				_categories[i] = 0;
			}
		}
		free( _categories );
		_categories = 0;
	}
}

/*
 * routine: readCstring
 *
 * purpose:
 *	to read a Cstring (len, string)
 *
 * returns:
 *	pointer to newly allocated string
 *	or zero
 */
char *PalmArchive::readCstring( ) {
	unsigned short len = readUbyte();
	if (len == 0)
		return( 0 );
	else if (len == 0xff) 
		len = readUshort();
	
	char *newstr = (char *) malloc( len+1 );
	int ret;
	if ((ret = fread( newstr, 1, len, _file )) != len ) {
fprintf(stderr,"Tried to read %d bytes, got %d\n", len, ret );
		_errstr = "Cstring short read";
		free( newstr );
		return( 0 );
	} else {
		newstr[len] = 0;
		return( newstr );
	}
}

/*
 * routine: readUlong
 *
 * purpose: to read a four-byte unsigned value
 *
 * returns:
 *	value (or -1);
 */
unsigned long PalmArchive::readUlong( ) {
	unsigned long value = 0;

	if (fread( &value, 4, 1, _file) != 1 ) {
		_errstr = "readUlong error";
		return( -1 );
	} else
		return( value );
}

/*
 * routine: readUshort
 *
 * purpose: to read a two-byte unsigned value
 *
 * returns:
 *	value (or -1);
 */
unsigned short PalmArchive::readUshort( ) {
	unsigned short value = 0;

	if (fread( &value, 2, 1, _file) != 1 ) {
		_errstr = "readUshort error";
		return( -1 );
	} else
		return( value );
}

/*
 * routine: readUbyte
 *
 * purpose: to read a one-byte unsigned value
 *
 * returns:
 *	value (or -1);
 */
unsigned char PalmArchive::readUbyte( ) {
	unsigned char value = 0;

	if (fread( &value, 1, 1, _file) != 1 ) {
		_errstr = "readUbyte error";
		return( -1 );
	} else
		return( value );
}

/*
 * routine: readHeader
 *
 * purpose:
 *	to read a generic Palm Archive Header
 *
 * return value:
 *	bool (success/failure)
 */
bool PalmArchive::readHeader( ) {

	_filetype = readUlong();
	if (_errstr)
		return( false );

	_filename = readCstring();
	if (_errstr)
		return( false );

	_header = readCstring();
	if (_errstr)
		return( false );

	// figure out how many categories there are, and read them in
	int freecat = readUlong();	// first free category
	_num_categories = readUlong();	// number of categories
	if (_num_categories > MAX_CATEGORIES) {
		_errstr = "too many categories";
		return( false );
	}

	_categories = (char **) malloc( _num_categories * sizeof (char **) );
	for( int i = 0; i < _num_categories; i++ ) {
		_categories[i] = readCategory();
	}
	if (_errstr)
		return( false );

	/*
	 * I don't understand the schema record stuff
	 * so, for now, I am ignoring it
	 */
	// I'm not sure what a schema is, and I'm pretty sure I don't care
	int rsrcid = readUlong();
	int fldsPerRow = readUlong();	// ???
	int posIndex = readUlong();	// ???
	int stsIndex = readUlong();	// ???
	int plcIndex = readUlong();	// ???
	int numfield = readUshort();	// number of schema fields
	for( int i = 0; i < numfield; i++ ) {
		(void) readUshort();
	}
	if (_errstr)
		return( false );
	
	// and now we should be at the good part
	return( true );
}

/*
 * routine: readCategory
 *
 * purpose:
 *	to read in a category description
 *	
 * returns:
 *	pointer to an allocated string for its long name
 */
char *PalmArchive::readCategory() {
	unsigned long catX = readUlong();
	unsigned long catID = readUlong();
	unsigned long dirty = readUlong();
	char *longname = readCstring();

	// I don't really care about the short names
	char *shortname = readCstring();
	if (shortname)
		free( shortname );

	return( longname );
}
