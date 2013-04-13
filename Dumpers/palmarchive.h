/*
 * module:	PalmArchive.h
 *
 * purpose:	Reading Palm Archives
 *
 * note:	The trick with Palm archives is that they are
 *		do not have a traditional (static) in-memory
 *		structure.  Rather they are byte streams with
 *		embededd types and counts that tell you how to
 *		interpret the bytes that follow.
 */
#include <stdio.h>

class PalmArchive {

   public:
	PalmArchive( FILE *openfile );
	PalmArchive( const char *filename );
	~PalmArchive();
	
	// basic data read routines
	unsigned long	 readUlong();
	unsigned short	 readUshort();
	unsigned char	 readUbyte();
	char 		*readCstring();


	// information about this archive
	const char	*error()	{ return( _errstr ); }
	unsigned long	 fileType()	{ return( _filetype ); }
	const char 	*fileName()	{ return( _filename ); }
	const char	*headerString()	{ return( _header ); }
	const char	*category(int i){ 
		if (i < 0 || i >= _num_categories)
			return( "NONE" );
		else
			return( _categories[i] );
	}

	// known archive types
	static const unsigned long DBA_SIG = 0x44420100UL;
	static const unsigned long ADDR_SIG = 0x41420100UL;
	static const unsigned long MEMO_SIG = 0x4d500100UL;
	static const unsigned long TODO_SIG = 0x54440100UL;

	const char	*typeName() {
		switch( _filetype ) {
		   case DBA_SIG:
			return( "Datebook" );

		   case ADDR_SIG:
			return( "Addressbook" );

		   case MEMO_SIG:
			return( "Memopad" );

		   case TODO_SIG:
			return( "Todolist" );
		}

		return( "???" );
	}


   private:
	// read routines for internal types
	void		 init();
	bool		 readHeader();
	char		 *readCategory();

	FILE	*_file;
	unsigned long _filetype;
	const char *_errstr;
	char	*_filename;
	char	*_header;
	int	_num_categories;
	char	**_categories;
};
