/*
 * Copyright (c) 2007 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2007 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * BINARY VERSION 8 READ/WRITE operations.
 *
 * This module reads/writes objects to files.
 *
 * What's new from version 7 to 8?
 * ===============================
 *	* changed vision instructions
 *
 */
#include "stdafx.h"

/*                               0123456789012345678901234567890 */
#define EVOLVE_BINARY_MAGIC	"EVOLVE_BINARY_FORMAT:V08     "
#define EVOLVE_BINARY_MAGIC_LEN	30

static int fwrite_UNIVERSE(UNIVERSE *u, FILE *fp)
{
	int n;

	ASSERT( u != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_unsigned_long(&u->seed, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&u->step, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&u->next_id, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&u->norganism, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&u->nborn, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&u->ndie, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&u->width, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&u->height, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_UNIVERSE(UNIVERSE *u, FILE *fp)
{
	int n;

	ASSERT( u != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_unsigned_long(&u->seed, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&u->step, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&u->next_id, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&u->norganism, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&u->nborn, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&u->ndie, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&u->width, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&u->height, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_EVOLVE_RANDOM(EVOLVE_RANDOM *er, FILE *fp)
{
	int n;

	ASSERT( er != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&er->fidx, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&er->ridx, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_array_long(er->state, EVOLVE_DEG4, fp);
	if( n != EVOLVE_DEG4 )
		return n;

	return 1;
}

static int fread_EVOLVE_RANDOM(EVOLVE_RANDOM *er, FILE *fp)
{
	int n;

	ASSERT( er != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_int(&er->fidx, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&er->ridx, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_array_long(er->state, EVOLVE_DEG4, fp);
	if( n != EVOLVE_DEG4 )
		return n;

	return 1;
}

static int fwrite_KFORTH_MUTATE_OPTIONS(KFORTH_MUTATE_OPTIONS *kfmo, FILE *fp)
{
	int n;

	ASSERT( kfmo != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&kfmo->max_code_blocks, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_mutate_codeblock, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_duplicate, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_delete, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_insert, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_transpose, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->prob_modify, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfmo->max_opcodes, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_KFORTH_MUTATE_OPTIONS(KFORTH_MUTATE_OPTIONS *kfmo, FILE *fp)
{
	int n;

	ASSERT( kfmo != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_int(&kfmo->max_code_blocks, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_mutate_codeblock, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_duplicate, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_delete, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_insert, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_transpose, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->prob_modify, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfmo->max_opcodes, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_KFORTH_MACHINE(KFORTH_MACHINE *kfm, FILE *fp)
{
	int n;

	ASSERT( kfm != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&kfm->terminated, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfm->call_stack_size, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfm->data_stack_size, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfm->cb, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&kfm->pc, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_array_LONG_LONG(kfm->R, 10, fp);
	if( n != 10 )
		return 0;

	return 1;
}

static int fread_KFORTH_MACHINE(KFORTH_MACHINE *kfm, FILE *fp)
{
	int n;

	ASSERT( kfm != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_int(&kfm->terminated, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfm->call_stack_size, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfm->data_stack_size, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfm->cb, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&kfm->pc, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_array_LONG_LONG(kfm->R, 10, fp);
	if( n != 10 )
		return 0;

	return 1;
}

static int fwrite_KFORTH_STACK_NODE_C(KFORTH_STACK_NODE *curr, FILE *fp)
{
	int n;

	ASSERT( curr != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&curr->u.call.cb, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&curr->u.call.pc, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_KFORTH_STACK_NODE_C(KFORTH_STACK_NODE *curr, FILE *fp)
{
	int n;

	ASSERT( curr != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_int(&curr->u.call.cb, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&curr->u.call.pc, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_KFORTH_STACK_NODE_D(KFORTH_STACK_NODE *curr, FILE *fp)
{
	int n;

	ASSERT( curr != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_LONG_LONG(&curr->u.data.value, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_KFORTH_STACK_NODE_D(KFORTH_STACK_NODE *curr, FILE *fp)
{
	int n;

	ASSERT( curr != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_LONG_LONG(&curr->u.data.value, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_CELL(CELL *c, FILE *fp)
{
	int n;

	ASSERT( c != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_LONG_LONG(&c->mood, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&c->message, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&c->x, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&c->y, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_CELL(CELL *c, FILE *fp)
{
	int n;

	ASSERT( c != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_LONG_LONG(&c->mood, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&c->message, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&c->x, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&c->y, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_ORGANISM(ORGANISM *o, FILE *fp)
{
	int n;

	ASSERT( o != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_LONG_LONG(&o->id, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&o->parent1, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&o->parent2, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&o->generation, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&o->energy, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&o->age, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&o->strain, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&o->ncells, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_ORGANISM(ORGANISM *o, FILE *fp)
{
	int n;

	ASSERT( o != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_LONG_LONG(&o->id, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&o->parent1, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&o->parent2, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&o->generation, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&o->energy, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&o->age, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&o->strain, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&o->ncells, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fwrite_SPORE(SPORE *spore, FILE *fp)
{
	int n;

	ASSERT( spore != NULL );
	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&spore->energy, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_int(&spore->strain, fp);
	if( n != 1 )
		return n;

	n = evbio_fwrite_LONG_LONG(&spore->parent, fp);
	if( n != 1 )
		return n;

	return 1;
}

static int fread_SPORE(SPORE *spore, FILE *fp)
{
	int n;

	ASSERT( spore != NULL );
	ASSERT( fp != NULL );

	n = evbio_fread_int(&spore->energy, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_int(&spore->strain, fp);
	if( n != 1 )
		return n;

	n = evbio_fread_LONG_LONG(&spore->parent, fp);
	if( n != 1 )
		return n;

	return 1;
}

/***********************************************************************
 * Binary WRITE routines.
 *
 *
 *
 *
 */
static bool write_magic(FILE *fp)
{
	int n;

	ASSERT( fp != NULL );

	n = evbio_fwrite_array_char((unsigned char*)EVOLVE_BINARY_MAGIC, EVOLVE_BINARY_MAGIC_LEN, fp);
	if( n == EVOLVE_BINARY_MAGIC_LEN )
		return true;
	else
		return false;

}

static bool read_magic(FILE *fp)
{
	int n;
	char buf[1000];

	ASSERT( fp != NULL );

	n = evbio_fread_array_char((unsigned char*)buf, EVOLVE_BINARY_MAGIC_LEN, fp);
	if( n != EVOLVE_BINARY_MAGIC_LEN ) {
		return false;
	}

	if( strcmp(EVOLVE_BINARY_MAGIC, buf) == 0 ) {
		return true;
	} else {
		return false;
	}
}


static bool write_evolve_random(FILE *fp, EVOLVE_RANDOM *er)
{
	int n;

	ASSERT( fp != NULL );
	ASSERT( er != NULL );

	n = fwrite_EVOLVE_RANDOM(er, fp);
	if( n != 1 ) {
		return false;
	}

	return true;
}

static EVOLVE_RANDOM *read_evolve_random(FILE *fp)
{
	EVOLVE_RANDOM *er;
	int n;

	ASSERT( fp != NULL );

	er = (EVOLVE_RANDOM *) CALLOC(1, sizeof(EVOLVE_RANDOM));
	ASSERT( er != NULL );

	n = fread_EVOLVE_RANDOM(er, fp);
	if( n != 1 ) {
		FREE(er);
		return NULL;
	}

	return er;
}

static bool write_mutate_options(FILE *fp, KFORTH_MUTATE_OPTIONS *kfmo)
{
	int n;

	ASSERT( fp != NULL );
	ASSERT( kfmo != NULL );

	n = fwrite_KFORTH_MUTATE_OPTIONS(kfmo, fp);
	if( n != 1 ) {
		return false;
	}

	return true;
}

static KFORTH_MUTATE_OPTIONS *read_mutate_options(FILE *fp)
{
	KFORTH_MUTATE_OPTIONS *kfmo;
	int n;

	ASSERT( fp != NULL );

	kfmo = (KFORTH_MUTATE_OPTIONS *) CALLOC(1, sizeof(KFORTH_MUTATE_OPTIONS));
	ASSERT( kfmo != NULL );

	n = fread_KFORTH_MUTATE_OPTIONS(kfmo, fp);
	if( n != 1 ) {
		FREE(kfmo);
		return NULL;
	}

	return kfmo;
}

static bool write_universe(FILE *fp, UNIVERSE *u)
{
	int n;
	bool success;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = fwrite_UNIVERSE(u, fp);
	if( n != 1 ) {
		return false;
	}

	success = write_evolve_random(fp, u->er);
	if( ! success ) {
		return false;
	}

	success = write_mutate_options(fp, u->kfmo);
	if( ! success ) {
		return false;
	}

	success = evbio_write_keylist(fp, u->keylist);
	if( ! success ) {
		return false;
	}

	return true;
}

static UNIVERSE *read_universe(FILE *fp)
{
	UNIVERSE *u;
	int n;
	int x, y;

	ASSERT( fp != NULL );

	u = (UNIVERSE *) CALLOC(1, sizeof(UNIVERSE));
	ASSERT( u != NULL );

	n = fread_UNIVERSE(u, fp);
	if( n != 1 ) {
		FREE(u);
		return NULL;
	}

	u->er = read_evolve_random(fp);
	if( u->er == NULL ) {
		FREE(u);
		return NULL;
	}

	u->kfmo = read_mutate_options(fp);
	if( u->kfmo == NULL ) {
		FREE(u->er);
		FREE(u);
		return NULL;
	}

	u->keylist = evbio_read_keylist(fp);
	if( u->keylist == NULL ) {
		FREE(u->er);
		FREE(u->kfmo);
		FREE(u);
		return NULL;
	}

	u->organisms = NULL;
	u->new_organisms = NULL;
	u->selected_organism = NULL;

	u->grid = (UNIVERSE_GRID *) CALLOC( u->width * u->height, sizeof(UNIVERSE_GRID));
	ASSERT( u->grid != NULL );

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			Grid_Clear(u, x, y);
		}
	}

	return u;
}

static bool write_program(FILE *fp, KFORTH_PROGRAM *kfp)
{
	int n;
	int i;
	int len;
	unsigned char *opcodes;
	KFORTH_INTEGER *operands;

	ASSERT( fp != NULL );
	ASSERT( kfp != NULL );

	n = evbio_fwrite_int(&kfp->nblocks, fp);
	if( n != 1 ) {
		return false;
	}

	n = evbio_fwrite_array_int(kfp->block_len, kfp->nblocks, fp);
	if( n != kfp->nblocks ) {
		return false;
	}

	for(i=0; i < kfp->nblocks; i++) {
		len = kfp->block_len[i];
		opcodes = kfp->opcode[i];
		operands = kfp->operand[i];

		n = evbio_fwrite_array_char(opcodes, len, fp);
		if( n != len ) {
			return false;
		}

		n = evbio_fwrite_array_LONG_LONG(operands, len, fp);
		if( n != len ) {
			return false;
		}
	}

	return true;
}

static KFORTH_PROGRAM *read_program(FILE *fp)
{
	KFORTH_PROGRAM *kfp;
	unsigned char *opcodes;
	KFORTH_INTEGER *operands;
	int i, len;
	int n;

	ASSERT( fp != NULL );

	kfp = (KFORTH_PROGRAM *) CALLOC(1, sizeof(KFORTH_PROGRAM));
	ASSERT( kfp != NULL );

	n = evbio_fread_int(&kfp->nblocks, fp);
	if( n != 1 ) {
		FREE(kfp);
		return NULL;
	}

	kfp->block_len = (int *) CALLOC(kfp->nblocks, sizeof(int));
	ASSERT( kfp->block_len != NULL );

	n = evbio_fread_array_int(kfp->block_len, kfp->nblocks, fp);
	if( n != kfp->nblocks ) {
		FREE(kfp->block_len);
		FREE(kfp);
		return NULL;
	}

	kfp->opcode = (unsigned char**) CALLOC(kfp->nblocks, sizeof(unsigned char*));
	kfp->operand = (KFORTH_INTEGER**) CALLOC(kfp->nblocks, sizeof(KFORTH_INTEGER*));

	for(i=0; i < kfp->nblocks; i++) {
		len = kfp->block_len[i];

		opcodes = (unsigned char *) CALLOC(len, sizeof(unsigned char));
		ASSERT( opcodes != NULL );

		operands = (KFORTH_INTEGER *) CALLOC(len, sizeof(KFORTH_INTEGER));
		ASSERT( opcodes != NULL );

		n = evbio_fread_array_char(opcodes, len, fp);
		if( n != len ) {
			FREE(kfp);
			return NULL;
		}

		n = evbio_fread_array_LONG_LONG(operands, len, fp);
		if( n != len ) {
			FREE(kfp);
			return NULL;
		}

		kfp->opcode[i] = opcodes;
		kfp->operand[i] = operands;
	}

	kfp->kfops = EvolveOperations();

	return kfp;
}

static bool write_machine(FILE *fp, KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *curr;
	int n;

	ASSERT( fp != NULL );
	ASSERT( kfm != NULL );

	n = fwrite_KFORTH_MACHINE(kfm, fp);
	if( n != 1 ) {
		return false;
	}

	for(curr=kfm->call_sp; curr; curr=curr->prev) {
		n = fwrite_KFORTH_STACK_NODE_C(curr, fp);
		if( n != 1 ) {
			return false;
		}
	}

	for(curr=kfm->data_sp; curr; curr=curr->prev) {
		n = fwrite_KFORTH_STACK_NODE_D(curr, fp);
		if( n != 1 ) {
			return false;
		}
	}

	return true;
}

static KFORTH_MACHINE *read_machine(FILE *fp)
{
	KFORTH_MACHINE *kfm;
	KFORTH_STACK_NODE *curr, *prev;
	int i, n;

	ASSERT( fp != NULL );

	kfm = (KFORTH_MACHINE *) CALLOC(1, sizeof(KFORTH_MACHINE));
	ASSERT( kfm != NULL );

	n = fread_KFORTH_MACHINE(kfm, fp);
	if( n != 1 ) {
		FREE(kfm);
		return NULL;
	}
	kfm->call_sp = NULL;
	kfm->data_sp = NULL;

	prev = NULL;
	for(i=0; i < kfm->call_stack_size; i++) {
		curr = (KFORTH_STACK_NODE *) CALLOC(1, sizeof(KFORTH_STACK_NODE));
		ASSERT( curr != NULL );

		n = fread_KFORTH_STACK_NODE_C(curr, fp);
		if( n != 1 ) {
			FREE(kfm);
			return NULL;
		}

		curr->next = NULL;
		curr->prev = NULL;

		if( prev == NULL ) {
			kfm->call_sp = curr;
		} else {
			prev->prev = curr;
			curr->next = prev;
		}

		prev = curr;
	}

	prev = NULL;
	for(i=0; i < kfm->data_stack_size; i++) {
		curr = (KFORTH_STACK_NODE *) CALLOC(1, sizeof(KFORTH_STACK_NODE));
		ASSERT( curr != NULL );

		n = fread_KFORTH_STACK_NODE_D(curr, fp);
		if( n != 1 ) {
			FREE(kfm);
			return NULL;
		}

		curr->next = NULL;
		curr->prev = NULL;

		if( prev == NULL ) {
			kfm->data_sp = curr;
		} else {
			prev->prev = curr;
			curr->next = prev;
		}

		prev = curr;
	}

	kfm->client_data = NULL;
	kfm->program = NULL;

	return kfm;
}

static bool write_cell(FILE *fp, CELL *c)
{
	int n;
	bool success;

	ASSERT( fp != NULL );
	ASSERT( c != NULL );

	n = fwrite_CELL(c, fp);
	if( n != 1 )
		return false;

	success = write_machine(fp, c->kfm);
	if( ! success ) {
		return false;
	}

	return true;
}

static CELL *read_cell(FILE *fp)
{
	CELL *c;
	int n;

	ASSERT( fp != NULL );

	c = (CELL *) CALLOC(1, sizeof(CELL));
	ASSERT( c != NULL );

	n = fread_CELL(c, fp);
	if( n != 1 ) {
		FREE(c);
		return NULL;
	}

	c->kfm = read_machine(fp);
	if( c->kfm == NULL ) {
		FREE(c);
		return NULL;
	}

	c->kfm->client_data = c;
	c->next = NULL;
	c->organism = NULL;

	return c;
}

static bool write_organism(FILE *fp, ORGANISM *o)
{
	CELL *curr;
	bool success;
	int n;

	ASSERT( fp != NULL );
	ASSERT( o != NULL );

	n = fwrite_ORGANISM(o, fp);
	if( n != 1 ) {
		return false;
	}

	success = write_program(fp, o->program);
	if( ! success ) {
		return false;
	}

	for(curr=o->cells; curr; curr=curr->next) {
		success = write_cell(fp, curr);
		if( ! success ) {
			return false;
		}
	}

	return true;
}

static ORGANISM *read_organism(FILE *fp)
{
	ORGANISM *o;
	CELL *c, *prev;
	int i, n;

	ASSERT( fp != NULL );

	o = (ORGANISM *) CALLOC(1, sizeof(ORGANISM));
	ASSERT( o != NULL );

	n = fread_ORGANISM(o, fp);
	if( n != 1 ) {
		FREE(o);
		return NULL;
	}

	o->next = NULL;
	o->universe = NULL;

	o->program = read_program(fp);
	if( o->program == NULL ) {
		FREE(o);
		return NULL;
	}

	prev = NULL;
	for(i=0; i < o->ncells; i++) {
		c = read_cell(fp);
		if( c == NULL ) {
			FREE(o);
			return NULL;
		}
		c->kfm->program = o->program;
		c->organism = o;

		if( prev == NULL ) {
			o->cells = c;
		} else {
			prev->next = c;
		}
		prev = c;
	}

	return o;

}

static bool write_organisms(FILE *fp, UNIVERSE *u)
{
	ORGANISM *curr;
	bool success;
	int n;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = evbio_fwrite_int(&u->norganism, fp);
	if( n != 1 ) {
		return false;
	}

	for(curr=u->organisms; curr; curr=curr->next) {
		success = write_organism(fp, curr);
		if( ! success ) {
			return false;
		}
	}

	return true;
}

static bool read_organisms(FILE *fp, UNIVERSE *u)
{
	ORGANISM *prev, *o;
	CELL *c;
	int count;
	int i, n;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = evbio_fread_int(&count, fp);
	if( n != 1 ) {
		return false;
	}

	if( u->norganism != count ) {
		return false;
	}

	prev = NULL;
	for(i=0; i < count; i++) {
		o = read_organism(fp);
		if( o == NULL ) {
			return false;
		}

		o->universe = u;

		if( prev == NULL ) {
			u->organisms = o;
		} else {
			prev->next = o;
		}

		for(c=o->cells; c; c=c->next) {
			Grid_SetCell(u, c);
		}

		prev = o;
	}

	return true;
}

static bool write_spore(FILE *fp, int x, int y, SPORE *spore)
{
	int n;
	bool success;

	ASSERT( fp != NULL );
	ASSERT( spore != NULL );

	n = evbio_fwrite_int(&x, fp);
	if( n != 1 )
		return false;

	n = evbio_fwrite_int(&y, fp);
	if( n != 1 )
		return false;

	n = fwrite_SPORE(spore, fp);
	if( n != 1 )
		return false;

	success = write_program(fp, spore->program);
	if( ! success )
		return false;

	return true;
}

static bool write_spores(FILE *fp, UNIVERSE *u)
{
	UNIVERSE_GRID *ugp;
	GRID_TYPE type;
	SPORE *spore;
	int x, y, count;
	bool success;
	int n;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	count = 0;
	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_SPORE )
				count++;
		}
	}

	n = evbio_fwrite_int(&count, fp);
	if( n != 1 )
		return false;

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_SPORE ) {
				spore = ugp->u.spore;
				success = write_spore(fp, x, y, spore);
				if( ! success ) {
					return false;
				}
			}
		}
	}

	return true;
}

static bool read_spores(FILE *fp, UNIVERSE *u)
{
	SPORE *spore;
	int i, n;
	int x, y;
	int count;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = evbio_fread_int(&count, fp);
	if( n != 1 ) {
		return false;
	}

	for(i=0; i < count; i++) {
		spore = (SPORE *) CALLOC(1, sizeof(SPORE));
		ASSERT( spore != NULL );

		n = evbio_fread_int(&x, fp);
		if( n != 1 ) {
			return false;
		}

		n = evbio_fread_int(&y, fp);
		if( n != 1 ) {
			return false;
		}

		n = fread_SPORE(spore, fp);
		if( n != 1 ) {
			return false;
		}

		spore->program = read_program(fp);
		if( spore->program == NULL ) {
			return false;
		}

		Grid_SetSpore(u, x, y, spore);
	}

	return true;

}

static bool write_organic(FILE *fp, int x, int y, int energy)
{
	int n;

	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&x, fp);
	if( n != 1 )
		return false;

	n = evbio_fwrite_int(&y, fp);
	if( n != 1 )
		return false;

	n = evbio_fwrite_int(&energy, fp);
	if( n != 1 )
		return false;

	return true;
}

static bool write_organics(FILE *fp, UNIVERSE *u)
{
	UNIVERSE_GRID *ugp;
	GRID_TYPE type;
	int x, y, count;
	bool success;
	int n;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	count = 0;
	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_ORGANIC )
				count++;
		}
	}

	n = evbio_fwrite_int(&count, fp);
	if( n != 1 )
		return false;

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_ORGANIC ) {
				success = write_organic(fp, x, y, ugp->u.energy);
				if( ! success ) {
					return false;
				}
			}
		}
	}

	return true;
}

static bool read_organics(FILE *fp, UNIVERSE *u)
{
	int i, n;
	int x, y, energy;
	int count;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = evbio_fread_int(&count, fp);
	if( n != 1 ) {
		return false;
	}

	for(i=0; i < count; i++) {
		n = evbio_fread_int(&x, fp);
		if( n != 1 ) {
			return false;
		}

		n = evbio_fread_int(&y, fp);
		if( n != 1 ) {
			return false;
		}

		n = evbio_fread_int(&energy, fp);
		if( n != 1 ) {
			return false;
		}

		Grid_SetOrganic(u, x, y, energy);
	}

	return true;
}

static bool write_barrier(FILE *fp, int x, int y)
{
	int n;

	ASSERT( fp != NULL );

	n = evbio_fwrite_int(&x, fp);
	if( n != 1 )
		return false;

	n = evbio_fwrite_int(&y, fp);
	if( n != 1 )
		return false;

	return true;
}

static bool write_barriers(FILE *fp, UNIVERSE *u)
{
	UNIVERSE_GRID *ugp;
	GRID_TYPE type;
	int x, y, count;
	bool success;
	int n;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	count = 0;
	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_BARRIER )
				count++;
		}
	}

	n = evbio_fwrite_int(&count, fp);
	if( n != 1 )
		return false;

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_GetPtr(u, x, y, &ugp);
			if( type == GT_BARRIER ) {
				success = write_barrier(fp, x, y);
				if( ! success ) {
					return false;
				}
			}
		}
	}

	return true;
}

static bool read_barriers(FILE *fp, UNIVERSE *u)
{
	int i, n;
	int x, y;
	int count;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	n = evbio_fread_int(&count, fp);
	if( n != 1 ) {
		return false;
	}

	for(i=0; i < count; i++) {
		n = evbio_fread_int(&x, fp);
		if( n != 1 ) {
			return false;
		}

		n = evbio_fread_int(&y, fp);
		if( n != 1 ) {
			return false;
		}

		Grid_SetBarrier(u, x, y);
	}

	return true;
}

/***********************************************************************
 * Write the entire state of the universe to 'filename'
 *
 */
int Universe_WriteBinary8(UNIVERSE *u, const char *filename, char *errbuf)
{
	FILE *fp;
	bool success;

	ASSERT( u != NULL );
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	fp = fopen(filename, "wb");
	if( fp == NULL ) {
		sprintf(errbuf, "%s: %s", filename, strerror(errno));
		return 0;
	}

	success = write_magic(fp);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write magic number (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	success = write_universe(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write universe (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	success = write_organisms(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write organisms (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	success = write_spores(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write spores (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	success = write_organics(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write organic (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	success = write_barriers(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot write barriers (%s)", filename, strerror(errno));
		fclose(fp);
		return 0;
	}

	fclose(fp);

	return 1;
}

UNIVERSE *Universe_ReadBinary8(const char *filename, char *errbuf)
{
	UNIVERSE *u;
	FILE *fp;
	bool success;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	fp = fopen(filename, "rb");
	if( fp == NULL ) {
		sprintf(errbuf, "%s: %s", filename, strerror(errno));
		return NULL;
	}

	success = read_magic(fp);
	if( ! success ) {
		sprintf(errbuf, "%s: bad magic number", filename);
		fclose(fp);
		return NULL;
	}

	u = read_universe(fp);
	if( u == NULL ) {
		sprintf(errbuf, "%s: cannot read universe (%s)", filename, strerror(errno));
		fclose(fp);
		return NULL;
	}

	success = read_organisms(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot read organisms (%s)", filename, strerror(errno));
		fclose(fp);
		return NULL;
	}

	success = read_spores(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot read spores (%s)", filename, strerror(errno));
		fclose(fp);
		return NULL;
	}

	success = read_organics(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot read organics (%s)", filename, strerror(errno));
		fclose(fp);
		return NULL;
	}

	success = read_barriers(fp, u);
	if( ! success ) {
		sprintf(errbuf, "%s: cannot read barriers (%s)", filename, strerror(errno));
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	return u;
}

int Universe_Migrate_8to9(const char *filename, char *errbuf)
{
	UNIVERSE *u;
	int n;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	u = Universe_ReadBinary8(filename, errbuf);
	if( u == NULL ) {
		sprintf(errbuf, "%s", errbuf);
		return 0;
	}

	/*
	 * Version 8 files use the max_apply of 1, so set this default
	 */
	u->kfmo->max_apply = 1;

	n = Universe_WriteBinary9(u, filename, errbuf);
	if( !n ) {
		sprintf(errbuf, "%s", errbuf);
		return 0;
	}

	Universe_Delete(u);

	return 1;
}


