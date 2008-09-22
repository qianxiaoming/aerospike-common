/*
*
* msg.h
* An independant message parser.
* This forms a generic way of parsing messages.
*
* A 'msg_desc' is a message descriptor. This will commonly be a singleton created
* by a unit once. 
* A 'msg' is a parsed-up and easy to read version of the message.
*/

#pragma once

#include "cf.h"

// NOTE: These values are actually used on the wire right now!type

typedef enum field_type_t {
	M_FT_INT32 = 1,
	M_FT_UINT32 = 2,
	M_FT_INT64 = 3,
	M_FT_UINT64 = 4,
	M_FT_STR = 5,
	M_FT_BUF = 6,
	M_FT_ARRAY = 7,
	M_FT_MESSAGE = 8,
} field_type;


typedef struct msg_field_desc_t {
	unsigned int		id;
	field_type 			type;
} msg_field_desc;

// TODO: consider that a msg_desc should have a human readable string
// Can play other interesting macro games to make sure we don't have 
// the problem of needing to add a length field to the message descriptions

typedef msg_field_desc msg_desc;

// This is a very simple linear system for representing a message
// Insert/read efficiency is paramount, but we think messages will tend
// to be compact without a lot of holes. If we expected sparse representation,
// we'd use a data structure better at sparse stuff

typedef struct msg_field_t {
	unsigned int 		id; // really probabaly don't need this in the represenation we have
	field_type 	type; 		// don't actually need this - but leave for faster access
	int 		field_len;  // used only for str and buf
	bool		is_valid;   // DEBUG - helps return errors on invalid types
	bool		is_set;     // keep track of whether the field was ever set
	bool		is_copy;    // if the 'str' or 'buf' is a copy, I must free
	union {
		uint32_t	ui32;
		int32_t		i32;
		uint64_t	ui64;
		int64_t     i64;
		char 		*str;
		byte		*buf;
		struct msg *m; // expansion - allows recursion - but how to describe?
	} u;
} msg_field;


typedef struct msg_t {
	int		 len; // number of elements in the field structure
	size_t   bytes_used;
	size_t	 bytes_alloc;
	bool     is_stack; // allocated on stack no need to free
	const msg_desc    *md;  // the message descriptor used to create this
	msg_field   f[];
} msg;


//
// msg_create - Initialize an empty message. You can pass in a stack buff,
// too. If everything fits, it stays. We use the msg_desc as a hint
// Slightly unusually, the 'md_sz' field is in bytes. This is a good shortcut
// to avoid terminator fields or anything similar
extern int msg_create(msg **m, const msg_desc *md, size_t md_sz, byte *stack_buf, size_t stack_buf_sz);

// msg_parse - parse a buffer into a message, which thus can be accessed
extern int msg_parse(msg *m, const byte *buf, const size_t buflen, bool copy);

// If you've received a little bit of a buffer, grab the size header.
// return = -2 means "not enough to tell yet"
extern int msg_get_size(size_t *size, const byte *buf, const size_t buflen);

// msg_tobuf - parse a message out into a buffer. Ret
extern int msg_fillbuf(const msg *m, byte *buf, size_t *buflen);

// msg_reset - after a message has been parsed, and the information consumed,
// reset all the internal pointers for another parse
extern void msg_reset(msg *m);

// Getters and setters

// Note! get_str and get_buf
// should probably be more complicated. To wit: both a 'copy' and 'dup'
// interface (where the 'copy' interface would return the length regardless,
// thus is also a 'getlet' method

extern int msg_get_uint32(const msg *m, int field_id, uint32_t *r);
extern int msg_get_int32(const msg *m, int field_id, int32_t *r);
extern int msg_get_uint64(const msg *m, int field_id, uint64_t *r);
extern int msg_get_int64(const msg *m, int field_id, int64_t *r);
extern int msg_get_str(const msg *m, int field_id, char **r, size_t *len, bool copy);  // this length is strlen+1, the allocated size
extern int msg_get_buf(const msg *m, int field_id, byte **r, size_t *len, bool copy);

extern int msg_set_uint32(msg *m, int field_id, const uint32_t v);
extern int msg_set_int32(msg *m, int field_id, const int32_t v);
extern int msg_set_uint64(msg *m, int field_id, const uint64_t v);
extern int msg_set_int64(msg *m, int field_id, const int64_t v);
extern int msg_set_str(msg *m, int field_id, const char *v, bool copy);
extern int msg_set_buf(msg *m, int field_id, const byte *v, size_t len, bool copy);

// A little routine that's good for testing
extern int msg_compare(const msg *m1, const msg *m2);

// And, finally, the destruction of a message
extern void msg_destroy(msg *m);


