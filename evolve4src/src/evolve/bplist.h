#pragma once

//
// This structure can be used to form a linked list
// of breakpoints.
//

typedef struct bplist {
	int cb;
	int pc;
	struct bplist *next;
} BPLIST;

