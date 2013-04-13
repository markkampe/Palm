/*
 * datebook.h
 *
 *	format of a Palm Datebook Archive 
 *
 */

struct dba_category {
	long index;
	long id;
	long dirty_flag;
	// Cstring	longname
	// Cstring	shortname
};

struct dba_header {
	long version;
	
	long filename;
	long tablestring;
	long cat_next_free;
	long cat_count;
	// 
	struct dba_category categories[0];
};

struct dba_schemas {
	long resource;
	long per_row;
	long pos_record_id;
	long pos_status;
	long pos_placement;
	short field_count;
	short schema_fields[0];
};

struct dba_repeat {
	short	num_exceptions;
	long	exception[];
}

struct dba_entry {
	long	ftype_rid;	// s.b. 1
	long	record_id;
	long	ftype_sts;	// s.b. 1
	long	status;
	long	ftype_pos;	// s.b. 1
	long	position;
	long	ftype_start;	// s.b. 3
	long	start;		// seconds since 1/1/70 GMT
	long	ftype_end;	// s.b. 1
	long	end;		// seconds since 1/1/70 GMT
	long	ftype_pad;	// s.b. 5
	long	pad1
	long	description;
	long	ftype_dur;	// s.b. 1
	long	duration
	long	note;	
	long	ftype_untimed;	// s.b. 6
	long	untimed;
	long	ftype_private;	// s.b. 6
	long	private;
	long	ftype_cat;	// s.b. 1
	long	category;
	long	ftype_alarmset;	// s.b. 6
	long	alarmset;
	long	ftype_al_unit;	// s.b.1
	long	alarm_units;
	long	ftype_al_adv;	// s.b. 1
	long	alarm_type;
	long	ftype_repeat;	// s.b. 8
	struct dba_repeat repeats;
};

struct dba_entries {
	long num_entries; 	// times 15
	struct dba_entry entries[0];
};
	
