/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * READ/WRITE operations (ASCII VERSION)
 *
 * This module reads/writes objects to files.
 * The format of files are "PHOTON ASCII".
 *
 * If you are generating these files the UNIVERSE entity must be first
 * as the reader attaches all other records to the UNIVERSE record.
 * Except for this, order shouldn't matter.
 *
 */
#include "stdafx.h"

static char *prolog[] = {
	"# PHOTON ASCII",
	"",
	"struct ORGANIC[N] {",
	"	X",
	"	Y",
	"	ENERGY",
	"}",
	"",
	"struct BARRIER[N] {",
	"	X",
	"	Y",
	"}",
	"",
	"struct ER {",
	"	FIDX",
	"	RIDX",
	"	STATE[N] {",
	"		VALUE",
	"	}",
	"}",
	"",
	"struct KFMO {",
	"	MAX_CODE_BLOCKS",
	"	MAX_APPLY",
	"	PROB_MUTATE_CODEBLOCK",
	"	PROB_DUPLICATE",
	"	PROB_DELETE",
	"	PROB_INSERT",
	"	PROB_TRANSPOSE",
	"	PROB_MODIFY",
	"}",
	"",
	"struct SPORE {",
	"	X",
	"	Y",
	"	ENERGY",
	"	PARENT",
	"	STRAIN",
	"	SFLAGS",
	"	PROGRAM[N] {",
	"		TEXT_LINE",
	"	}",
	"}",
	"",
	"struct CELL {",
	"	ORGANISM_ID",
	"	X",
	"	Y",
	"	MOOD",
	"	MESSAGE",
	"",
	"	MACHINE {",
	"		TERMINATED",
	"		CB",
	"		PC",
	"		R[N] {",
	"			VALUE",
	"		}",
	"",
	"		CALL_STACK[N] {",
	"			CB",
	"			PC",
	"		}",
	"",
	"		DATA_STACK[N] {",
	"			VALUE",
	"		}",
	"	}",
	"}",
	"",
	"struct ORGANISM {",
	"	ORGANISM_ID",
	"	STRAIN",
	"	OFLAGS",
	"	PARENT1",
	"	PARENT2",
	"	GENERATION",
	"	ENERGY",
	"	AGE",
	"	PROGRAM[N] {",
	"		TEXT_LINE",
	"	}",
	"}",
	"",
	"struct KEYLIST {",
	"	KEY",
	"	VALUE",
	"}",
	""
	"struct PLAYER {",
	"	TERMINATED",
	"	X",
	"	Y",
	"	ENERGY",
	"	AGE",
	"	CMD {",
	"		TYPE",
	"		X",
	"		Y",
	"	}",
	"}",
	"",
	"struct UNIVERSE {",
	"	SEED",
	"	STEP",
	"	NEXT_ID",
	"	NBORN",
	"	NDIE",
	"	WIDTH",
	"	HEIGHT",
	"}",
	"",
};

/* ***********************************************************************
   ***********************************************************************
   ****************************   WRITE ROUTINES   ************************
   ***********************************************************************
   *********************************************************************** */

static int getline(char **inp, char *buf)
{
	char *p, *q;

	ASSERT( inp != NULL );
	ASSERT( buf != NULL );

	p = *inp;
	q = buf;
	while( *p != '\0' && *p != '\n' ) {
		*q++ = *p++;
	}
	*q = '\0';

	if( *p == '\n' )
		*p++;

	*inp = p;

	if( *p == '\0' )
		return 0;
	else
		return 1;
}

/*
 * Write the header information for the
 * PHOTON ASCII file.
 */
static void write_prolog(FILE *fp)
{
	int i;

	ASSERT( fp != NULL );

	for(i=0; i < sizeof(prolog)/sizeof(prolog[0]); i++) {
		fprintf(fp, "%s\n", prolog[i]);
	}
}

static void write_spore(FILE *fp, int x, int y, SPORE *spore)
{
	KFORTH_DISASSEMBLY *kfd;
	char buf[5000], *p;

	ASSERT( fp != NULL );
	ASSERT( spore != NULL );

	fprintf(fp,
#ifndef __linux__
		"SPORE %d %d %d %I64d %d %d\n",
#else
		"SPORE %d %d %d %lld %d %d\n",
#endif
				x, y, spore->energy,
				spore->parent, spore->strain,
				spore->sflags);

	kfd = kforth_disassembly_make(spore->program, 80, 0);

	fprintf(fp, "  {  # program\n");

	p = kfd->program_text;
	while( getline(&p, buf) ) {
		fprintf(fp, "\t\"%s\"\n", buf);
	}
	fprintf(fp, "  }\n");

	kforth_disassembly_delete(kfd);

	fprintf(fp, "\n");
}

/*
 * Write out each spore as a SPORE photon ascii instance.
 */
static void write_spores(FILE *fp, UNIVERSE *u)
{
	int x, y;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_Get(u, x, y, &ugrid);

			if( type == GT_SPORE ) {
				write_spore(fp, x, y, ugrid.u.spore);
			}
		}
	}
}

/*
 * Write out barrier elements
 */
static void write_barriers(FILE *fp, UNIVERSE *u)
{
	int x, y, n;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	fprintf(fp, "\n");

	n = 0;
	fprintf(fp, "BARRIER {\n");

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_Get(u, x, y, &ugrid);
			if( type != GT_BARRIER )
				continue;

			if( n >= 500 ) {
				n = 0;
				fprintf(fp, "}\n");
				fprintf(fp, "BARRIER {\n");
			}

			fprintf(fp, "\t%d\t%d\n", x, y);
			n += 1;
		}
	}

	fprintf(fp, "}\n");
	fprintf(fp, "\n");

}

/*
 * Write out the keylist Each key/value pair
 * produced one of these records.
 */
static void write_keylist(FILE *fp, UNIVERSE *u)
{
	KEYLIST *keylist;
	KEYLIST_ITEM *curr;
	int i;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	keylist = u->keylist;

	for(i=0; i < KEYLIST_HASH_TABLE_SIZE; i++) {
		if( keylist->table[i] == NULL )
			continue;

		for(curr=keylist->table[i]; curr; curr=curr->next) {
			fprintf(fp, "KEYLIST \"%s\" \"%s\"\n", curr->key, curr->value);
		}
	}

}

/*
 * Write out organic material
 */
static void write_organic(FILE *fp, UNIVERSE *u)
{
	int x, y, n;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	fprintf(fp, "\n");

	n = 0;
	fprintf(fp, "ORGANIC {\n");

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Grid_Get(u, x, y, &ugrid);
			if( type != GT_ORGANIC )
				continue;

			if( n >= 500 ) {
				n = 0;
				fprintf(fp, "}\n");
				fprintf(fp, "ORGANIC {\n");
			}

			fprintf(fp, "\t%d\t%d\t%d\n", x, y, ugrid.u.energy);
			n += 1;
		}
	}

	fprintf(fp, "}\n");
	fprintf(fp, "\n");
}

void write_evolve_random(FILE *fp, EVOLVE_RANDOM *er)
{
	int i, n;

	ASSERT( fp != NULL );
	ASSERT( er != NULL );

	fprintf(fp, "ER %d %d %d\n", er->fidx, er->ridx, EVOLVE_DEG4);

	n = 0;
	for(i=0; i < EVOLVE_DEG4; i++) {
		if( n >= 4 ) {
			n = 0;
			fprintf(fp, "\n");
		}

		fprintf(fp, "\t%ld", er->state[i]);
		n++;
	}
	fprintf(fp, "\n\n");
}

static void write_kforth_mutate_options(FILE *fp, KFORTH_MUTATE_OPTIONS *kfmo)
{
	ASSERT( fp != NULL );
	ASSERT( kfmo != NULL );

	fprintf(fp, "\n");
	fprintf(fp, "KFMO\n");
	fprintf(fp, "\t%d\n", kfmo->max_code_blocks);
	fprintf(fp, "\t%d\n", kfmo->max_apply);
	fprintf(fp, "\t%d\n", kfmo->prob_mutate_codeblock);
	fprintf(fp, "\t%d\n", kfmo->prob_duplicate);
	fprintf(fp, "\t%d\n", kfmo->prob_delete);
	fprintf(fp, "\t%d\n", kfmo->prob_insert);
	fprintf(fp, "\t%d\n", kfmo->prob_transpose);
	fprintf(fp, "\t%d\n", kfmo->prob_modify);

	fprintf(fp, "\n");
}

static void write_cell(FILE *fp, LONG_LONG organism_id, CELL *c)
{
	KFORTH_MACHINE *kfm;
	KFORTH_STACK_NODE *sp;

	ASSERT( fp != NULL );
	ASSERT( c != NULL );

	fprintf(fp,
#ifndef __linux__
		"CELL %I64d %d %d\n",
#else
		"CELL %lld %d %d\n",
#endif
			organism_id,
			c->x,
			c->y);

	fprintf(fp,
#ifndef __linux__
		"\t%I64d %I64d\n",
#else
		"\t%lld %lld\n",
#endif
			c->mood, c->message);

	kfm = c->kfm;
	fprintf(fp, "\t%d %d %d\n",
			kfm->terminated,
			kfm->cb,
			kfm->pc);

	fprintf(fp,
#ifndef __linux__
			"\t{ %I64d %I64d %I64d %I64d %I64d\n",
#else
			"\t{ %lld %lld %lld %lld %lld\n",
#endif
			kfm->R[0], kfm->R[1], kfm->R[2], kfm->R[3], kfm->R[4]);

	fprintf(fp,
#ifndef __linux__
			"\t  %I64d %I64d %I64d %I64d %I64d }\n",
#else
			"\t  %lld %lld %lld %lld %lld }\n",
#endif
			kfm->R[5], kfm->R[6], kfm->R[7], kfm->R[8], kfm->R[9]);

	/*
	 * call stack
	 */
	for(sp=kfm->call_sp; sp && sp->prev; sp=sp->prev)
		;

	fprintf(fp, "\t{\n");
	while( sp ) {
		fprintf(fp, "\t\t%d %d\n", sp->u.call.cb, sp->u.call.pc);
		sp = sp->next;
	}
	fprintf(fp, "\t}\n");

	/*
	 * data stack
	 */
	for(sp=kfm->data_sp; sp && sp->prev; sp=sp->prev)
		;

	fprintf(fp, "\t{\n");
	while( sp ) {
		fprintf(fp,
#ifndef __linux__
			"\t\t%I64d\n",
#else
			"\t\t%lld\n",
#endif
				sp->u.data.value);
		sp = sp->next;
	}
	fprintf(fp, "\t}\n\n");

}

static void write_organism(FILE *fp, ORGANISM *o)
{
	KFORTH_DISASSEMBLY *kfd;
	char buf[5000], *p;
	CELL *c;

	ASSERT( fp != NULL );
	ASSERT( o != NULL );

	fprintf(fp,
#ifndef __linux__
		"ORGANISM %I64d %d %d %I64d %I64d %d %d %d\n",
#else
		"ORGANISM %lld %d %d %lld %lld %d %d %d\n",
#endif
			o->id,
			o->strain,
			o->oflags,
			o->parent1,
			o->parent2,
			o->generation,
			o->energy,
			o->age);

	kfd = kforth_disassembly_make(o->program, 80, 0);

	fprintf(fp, "  {  # program\n");

	p = kfd->program_text;
	while( getline(&p, buf) ) {
		fprintf(fp, "\t\"%s\"\n", buf);
	}
	fprintf(fp, "  }\n");

	kforth_disassembly_delete(kfd);

	fprintf(fp, "\n");

	for(c=o->cells; c; c=c->next) {
		write_cell(fp, o->id, c);
	}
}

static void write_organisms(FILE *fp, UNIVERSE *u)
{
	ORGANISM *o;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	for(o=u->organisms; o; o=o->next) {
		write_organism(fp, o);
	}
}

static void write_player(FILE *fp, UNIVERSE *u)
{
	PLAYER *player;

	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	if( u->player == NULL )
		return;

	player = u->player;

	fprintf(fp, "PLAYER %d %d %d %d %d %d %d %d\n",
		player->terminated,
		player->x,
		player->y,
		player->energy,
		player->age,
		player->cmd.type,
		player->cmd.x,
		player->cmd.y);

	fprintf(fp, "\n");

}

static void write_universe(FILE *fp, UNIVERSE *u)
{
	ASSERT( fp != NULL );
	ASSERT( u != NULL );

	fprintf(fp,
#ifndef __linux__
			"UNIVERSE %lu %I64d %I64d %I64d %I64d %d %d\n\n",
#else
			"UNIVERSE %lu %lld %lld %lld %lld %d %d\n\n",
#endif
			u->seed,
			u->step,
			u->next_id,
			u->nborn,
			u->ndie,
			u->width,
			u->height);
}


/***********************************************************************
 * Write the entire state of the universe to 'filename'
 *
 */
int Universe_WriteAscii(UNIVERSE *u, const char *filename, char *errbuf)
{
	FILE *fp;

	ASSERT( u != NULL );
	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	fp = fopen(filename, "w");
	if( fp == NULL ) {
		sprintf(errbuf, "%s: %s", filename, strerror(errno));
		return 0;
	}

	write_prolog(fp);
	write_universe(fp, u);
	write_evolve_random(fp, u->er);
	write_kforth_mutate_options(fp, u->kfmo);
	write_barriers(fp, u);
	write_keylist(fp, u);
	write_organic(fp, u);
	write_spores(fp, u);
	write_organisms(fp, u);
	write_player(fp, u);

	fclose(fp);
	return 1;
}

/* ***********************************************************************
   ***********************************************************************
   ****************************   READ ROUTINES   ************************
   ***********************************************************************
   *********************************************************************** */

static int read_universe(PHASCII_INSTANCE pi, UNIVERSE **pu, char *errmsg)
{
	UNIVERSE *u;
	int n, x, y;

	ASSERT( pi != NULL );
	ASSERT( pu != NULL );
	ASSERT( errmsg != NULL );

	if( *pu != NULL ) {
		sprintf(errmsg, "multiple UNIVERSE instances not allowed");
		return 0;		
	}

	u = (UNIVERSE *) CALLOC(1, sizeof(UNIVERSE));
	ASSERT( u != NULL );

	n = Phascii_Get(pi, "UNIVERSE.SEED", "%ul", &u->seed);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.SEED");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.STEP", "%ll", &u->step);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.STEP");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.NEXT_ID", "%ll", &u->next_id);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.NEXT_ID");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.NBORN", "%ll", &u->nborn);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.NBORN");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.NDIE", "%ll", &u->ndie);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.NDIE");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.WIDTH", "%d", &u->width);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.WIDTH");
		FREE(u);
		return 0;
	}

	n = Phascii_Get(pi, "UNIVERSE.HEIGHT", "%d", &u->height);
	if( n != 1 ) {
		sprintf(errmsg, "missing UNIVERSE.HEIGHT");
		FREE(u);
		return 0;
	}

	if( u->width < 0 || u->width > EVOLVE_MAX_BOUNDS ) {
		sprintf(errmsg, "UNIVERSE.WIDTH out of bounds");
		FREE(u);
		return 0;
	}

	if( u->height < 0 || u->height > EVOLVE_MAX_BOUNDS ) {
		sprintf(errmsg, "UNIVERSE.HEIGHT out of bounds");
		FREE(u);
		return 0;
	}

	u->keylist = KEYLIST_Make();

	u->grid = (UNIVERSE_GRID*) CALLOC( u->width * u->height, sizeof(UNIVERSE_GRID) );
	ASSERT( u->grid != NULL );

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			Grid_Clear(u, x, y);
		}
	}

	*pu = u;
	return 1;
}


/*
 * Read the ER instance and build an EVOLVE_RANDOM object and
 * attach it to UNIVERSE 'u'.
 */
static int read_er(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int n, i, num, fidx, ridx;
	long state;
	EVOLVE_RANDOM *er;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before ER instance");
		return 0;
	}

	if( u->er != NULL ) {
		sprintf(errmsg, "multiple ER instances not allowed");
		return 0;
	}

	n = Phascii_Get(pi, "ER.FIDX", "%d", &fidx);
	if( n != 1 ) {
		sprintf(errmsg, "missing ER.FIDX");
		return 0;
	}

	n = Phascii_Get(pi, "ER.RIDX", "%d", &ridx);
	if( n != 1 ) {
		sprintf(errmsg, "missing ER.RIDX");
		return 0;
	}

	n = Phascii_Get(pi, "ER.STATE.N", "%d", &num);
	if( n != 1 ) {
		sprintf(errmsg, "missing ER.STATE.N");
		return 0;
	}

	if( num != EVOLVE_DEG4 ) {
		sprintf(errmsg, "ER.STATE.N = %d should be %d", num, EVOLVE_DEG4);
		return 0;
	}

	er = sim_random_make(123);

	er->fidx = fidx;
	er->ridx = ridx;

	for(i=0; i < n; i++) {
		n = Phascii_Get(pi, "ER.STATE[%0].VALUE", i, "%ld", &state);

		if( n != 1 ) {
			sprintf(errmsg, "ER.STATE[%d].VALUE missing", i);
			return 0;
		}

		er->state[i] = state;
	}

	u->er = er;

	return 1;
}

static int read_kfmo(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int n;
	KFORTH_MUTATE_OPTIONS *kfmo;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before KFMO instance");
		return 0;
	}

	if( u->kfmo != NULL ) {
		sprintf(errmsg, "multiple KFMO instances not allowed");
		return 0;
	}

	kfmo = kforth_mutate_options_make(1, 1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1);
	kforth_mutate_options_defaults(kfmo);

	n = Phascii_Get(pi, "KFMO.MAX_CODE_BLOCKS", "%d", &kfmo->max_code_blocks);
	if( n != 1 ) {
		/*
		 * This is a new thing in version 8 files, so this error may
		 * happen when reading older files, so no error here.
		 */
	}

	n = Phascii_Get(pi, "KFMO.MAX_APPLY", "%d", &kfmo->max_apply);
	if( n != 1 ) {
		/*
		 * This is a new thing in version 9 files, so this error may
		 * happen when reading older files, so no error here.
		 */
	}

	n = Phascii_Get(pi, "KFMO.PROB_MUTATE_CODEBLOCK", "%d", &kfmo->prob_mutate_codeblock);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_MUTATE_CODEBLOCK");
		FREE(kfmo);
		return 0;
	}

	n = Phascii_Get(pi, "KFMO.PROB_DUPLICATE", "%d", &kfmo->prob_duplicate);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_DUBLICATE");
		FREE(kfmo);
		return 0;
	}

	n = Phascii_Get(pi, "KFMO.PROB_DELETE", "%d", &kfmo->prob_delete);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_DELETE");
		FREE(kfmo);
		return 0;
	}

	n = Phascii_Get(pi, "KFMO.PROB_INSERT", "%d", &kfmo->prob_insert);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_INSERT");
		FREE(kfmo);
		return 0;
	}

	n = Phascii_Get(pi, "KFMO.PROB_TRANSPOSE", "%d", &kfmo->prob_transpose);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_TRANSPOSE");
		FREE(kfmo);
		return 0;
	}

	n = Phascii_Get(pi, "KFMO.PROB_MODIFY", "%d", &kfmo->prob_modify);
	if( n != 1 ) {
		sprintf(errmsg, "missing KFMO.PROB_MODIFY");
		FREE(kfmo);
		return 0;
	}

	u->kfmo = kfmo;

	return 1;
}

static int read_keylist(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	char key[ 1000 ];
	char value[ 1000 ];
	int n;

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before KEYLIST instance");
		return 0;
	}

	n = Phascii_Get(pi, "KEYLIST.KEY", "%s", key);
	if( n != 1 ) {
		sprintf(errmsg, "missing KEYLIST.KEY");
		return 0;
	}

	n = Phascii_Get(pi, "KEYLIST.VALUE", "%s", value);
	if( n != 1 ) {
		sprintf(errmsg, "missing KEYLIST.KEY");
		return 0;
	}

	KEYLIST_Set(u->keylist, key, value);

	return 1;
}

static int read_organic(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int n, i, num;
	int x, y, energy;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before ORGANIC instance");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANIC.N", "%d", &num);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANIC.N");
		return 0;
	}

	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "ORGANIC[%0].X", i, "%d", &x);
		if( n != 1 ) {
			sprintf(errmsg, "ORGANIC[].X missing");
			return 0;
		}

		n = Phascii_Get(pi, "ORGANIC[%0].Y", i, "%d", &y);
		if( n != 1 ) {
			sprintf(errmsg, "ORGANIC[].Y missing");
			return 0;
		}

		n = Phascii_Get(pi, "ORGANIC[%0].ENERGY", i, "%d", &energy);
		if( n != 1 ) {
			sprintf(errmsg, "ORGANIC[].ENERGY missing");
			return 0;
		}

		if( x < 0 || x >= u->width ) {
			sprintf(errmsg, "ORGANIC[%d].X = %d, out of bounds", i, x);
			return 0;
		}

		if( y < 0 || y >= u->height ) {
			sprintf(errmsg, "ORGANIC[%d].Y = %d, out of bounds", i, y);
			return 0;
		}

		if( energy < 0 ) {
			sprintf(errmsg, "ORGANIC[%d].ENERGY = %d, negative", i, energy);
			return 0;
		}

		Grid_SetOrganic(u, x, y, energy);
	}

	return 1;
}

static int read_barrier(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int n, i, num;
	int x, y;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before BARRIER instance");
		return 0;
	}

	n = Phascii_Get(pi, "BARRIER.N", "%d", &num);
	if( n != 1 ) {
		sprintf(errmsg, "missing BARRIER.N");
		return 0;
	}

	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "BARRIER[%0].X", i, "%d", &x);
		if( n != 1 ) {
			sprintf(errmsg, "BARRIER[].X missing");
			return 0;
		}

		n = Phascii_Get(pi, "BARRIER[%0].Y", i, "%d", &y);
		if( n != 1 ) {
			sprintf(errmsg, "BARRIER[].Y missing");
			return 0;
		}

		if( x < 0 || x >= u->width ) {
			sprintf(errmsg, "BARRIER[%d].X = %d, out of bounds", i, x);
			return 0;
		}

		if( y < 0 || y >= u->height ) {
			sprintf(errmsg, "BARRIER[%d].Y = %d, out of bounds", i, y);
			return 0;
		}

		Grid_SetBarrier(u, x, y);
	}

	return 1;
}

static int read_spore(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	char buf[5000];
	int n, num, i, len;
	int x, y, energy, strain, sflags;
	LONG_LONG parent;
	SPORE *spore;
	KFORTH_PROGRAM *kfp;
	char *program_text;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before SPORE instance");
		return 0;
	}

	n = Phascii_Get(pi, "SPORE.X", "%d", &x);
	if( n != 1 ) {
		sprintf(errmsg, "missing SPORE.X");
		return 0;
	}

	if( x < 0 || x >= u->width ) {
		sprintf(errmsg, "SPORE.X = %d, out of bounds", x);
		return 0;
	}

	n = Phascii_Get(pi, "SPORE.Y", "%d", &y);
	if( n != 1 ) {
		sprintf(errmsg, "missing SPORE.Y");
		return 0;
	}

	if( y < 0 || y >= u->height ) {
		sprintf(errmsg, "SPORE.Y = %d, out of bounds", y);
		return 0;
	}

	n = Phascii_Get(pi, "SPORE.ENERGY", "%d", &energy);
	if( n != 1 ) {
		sprintf(errmsg, "missing SPORE.ENERGY");
		return 0;
	}

	n = Phascii_Get(pi, "SPORE.PARENT", "%ll", &parent);
	if( n != 1 ) {
		sprintf(errmsg, "missing SPORE.PARENT");
		return 0;
	}

	strain = 0;
	Phascii_Get(pi, "SPORE.STRAIN", "%d", &strain);

	sflags = 0;
	Phascii_Get(pi, "SPORE.SFLAGS", "%d", &sflags);

	n = Phascii_Get(pi, "SPORE.PROGRAM.N", "%d", &num);
	if( n != 1 ) {
		sprintf(errmsg, "missing SPORE.PROGRAM.N");
		return 0;
	}

	/*
	 * Compute length
	 */
	len = 0;
	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "SPORE.PROGRAM[%0].TEXT_LINE", i, "%s", buf);
		if( n != 1 ) {
			sprintf(errmsg, "missing SPORE.PROGRAM[].TEXT_LINE");
			return 0;
		}

		len += (int) strlen(buf) + 1;
	}
	len += 1;

	program_text = (char*) CALLOC(len, sizeof(char));
	ASSERT( program_text != NULL );

	program_text[0] = '\0';
	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "SPORE.PROGRAM[%0].TEXT_LINE", i, "%s", buf);
		ASSERT( n == 1 );

		strcat(program_text, buf);
		strcat(program_text, "\n");
	}


	kfp = kforth_compile(program_text, EvolveOperations(), errmsg);
	if( kfp == NULL ) {
		FREE(program_text);
		return 0;
	}
	FREE(program_text);

	spore = (SPORE *) CALLOC(1, sizeof(SPORE));
	ASSERT( spore != NULL );

	spore->energy = energy;
	spore->parent = parent;
	spore->strain = strain;
	spore->sflags = sflags;
	spore->program = kfp;

	Grid_SetSpore(u, x, y, spore);

	return 1;
}

static int read_organism(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	char buf[5000];
	int n, num, i, len;
	LONG_LONG organism_id, parent1, parent2;
	int generation, energy, age, strain, oflags;
	ORGANISM *o, *curr;
	KFORTH_PROGRAM *kfp;
	char *program_text;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before ORGANISM instance");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.ORGANISM_ID", "%ll", &organism_id);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.ORGANISM_ID");
		return 0;
	}

	strain = 0;
	Phascii_Get(pi, "ORGANISM.STRAIN", "%d", &strain);

	oflags = 0;
	Phascii_Get(pi, "ORGANISM.OFLAGS", "%d", &oflags);

	n = Phascii_Get(pi, "ORGANISM.PARENT1", "%ll", &parent1);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.PARENT1");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.PARENT2", "%ll", &parent2);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.PARENT2");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.GENERATION", "%d", &generation);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.GENERATION");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.ENERGY", "%d", &energy);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.ENERGY");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.AGE", "%d", &age);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.AGE");
		return 0;
	}

	n = Phascii_Get(pi, "ORGANISM.PROGRAM.N", "%d", &num);
	if( n != 1 ) {
		sprintf(errmsg, "missing ORGANISM.PROGRAM.N");
		return 0;
	}

	/*
	 * Compute length
	 */
	len = 0;
	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "ORGANISM.PROGRAM[%0].TEXT_LINE", i, "%s", buf);
		if( n != 1 ) {
			sprintf(errmsg, "missing ORGANISM.PROGRAM[].TEXT_LINE");
			return 0;
		}

		len += (int) strlen(buf) + 1;
	}
	len += 1;

	program_text = (char*) CALLOC(len, sizeof(char));
	ASSERT( program_text != NULL );

	program_text[0] = '\0';
	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "ORGANISM.PROGRAM[%0].TEXT_LINE", i, "%s", buf);
		ASSERT( n == 1 );

		strcat(program_text, buf);
		strcat(program_text, "\n");
	}

	kfp = kforth_compile(program_text, EvolveOperations(), errmsg);
	if( kfp == NULL ) {
		FREE(program_text);
		return 0;
	}
	FREE(program_text);

	o = (ORGANISM *) CALLOC(1, sizeof(ORGANISM));
	ASSERT( o != NULL );

	o->id		= organism_id;
	o->strain	= strain;
	o->oflags	= oflags;
	o->parent1	= parent1;
	o->parent2	= parent2;
	o->generation	= generation;
	o->energy	= energy;
	o->age		= age;
	o->program	= kfp;
	o->universe	= u;

	/*
	 * Attach organism to universe
	 * (preserve order in 'u->organisms' list)
	 */
	u->norganism	+= 1;

	if( u->organisms == NULL ) {
		u->organisms = o;

	} else {
		for(curr=u->organisms; curr->next; curr=curr->next)
			;

		curr->next = o;
	}

	return 1;
}

static int read_player(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int n;
	int x, y, terminated, energy, age, cmd_type, cmd_x, cmd_y;
	PLAYER *player;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before PLAYER instance");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.TERMINATED", "%d", &terminated);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.TERMINATED");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.X", "%d", &x);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.X");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.Y", "%d", &y);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.Y");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.AGE", "%d", &age);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.AGE");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.ENERGY", "%d", &energy);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.ENERGY");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.CMD.TYPE", "%d", &cmd_type);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.CMD.TYPE");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.CMD.X", "%d", &cmd_x);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.CMD.X");
		return 0;
	}

	n = Phascii_Get(pi, "PLAYER.CMD.Y", "%d", &cmd_y);
	if( n != 1 ) {
		sprintf(errmsg, "missing PLAYER.CMD.Y");
		return 0;
	}

	player = Player_make(u, x, y);
	player->terminated	= terminated;
	player->energy		= energy;
	player->age		= age;
	player->cmd.type	= (PCMD_TYPE)cmd_type;
	player->cmd.x		= cmd_x;
	player->cmd.y		= cmd_y;

	u->player = player;
	Grid_SetPlayer(u, x, y, player);

	return 1;

}

/*
 * Read a cell and attach to organism.
 *
 */
static int read_cell(PHASCII_INSTANCE pi, UNIVERSE *u, char *errmsg)
{
	int i, n, num;
	int organism_id, cb, pc;
	KFORTH_INTEGER value;
	ORGANISM *o, *ocurr;
	CELL *c, *ccurr;
	KFORTH_MACHINE *kfm;

	ASSERT( pi != NULL );
	ASSERT( errmsg != NULL );

	if( u == NULL ) {
		sprintf(errmsg, "a UNIVERSE instance must appear before CELL instance");
		return 0;
	}

	n = Phascii_Get(pi, "CELL.ORGANISM_ID", "%d", &organism_id);
	if( n != 1 ) {
		sprintf(errmsg, "missing CELL.ORGANISM_ID");
		return 0;
	}

	c = (CELL*) CALLOC(1, sizeof(CELL));
	ASSERT( c != NULL );

	n = Phascii_Get(pi, "CELL.X", "%d", &c->x);
	if( n != 1 ) {
		FREE(c);
		sprintf(errmsg, "missing CELL.X");
		return 0;
	}

	if( c->x < 0 || c->x >= u->width ) {
		sprintf(errmsg, "CELL.X = %d, out of bounds", c->x);
		return 0;
	}

	n = Phascii_Get(pi, "CELL.Y", "%d", &c->y);
	if( n != 1 ) {
		FREE(c);
		sprintf(errmsg, "missing CELL.Y");
		return 0;
	}

	if( c->y < 0 || c->y >= u->height ) {
		sprintf(errmsg, "CELL.Y = %d, out of bounds", c->y);
		return 0;
	}

	n = Phascii_Get(pi, "CELL.MOOD", "%ll", &c->mood);
	if( n != 1 ) {
		FREE(c);
		sprintf(errmsg, "missing CELL.MOOD");
		return 0;
	}

	n = Phascii_Get(pi, "CELL.MESSAGE", "%ll", &c->message);
	if( n != 1 ) {
		FREE(c);
		sprintf(errmsg, "missing CELL.MOOD");
		return 0;
	}

	kfm = (KFORTH_MACHINE *) CALLOC(1, sizeof(KFORTH_MACHINE));
	ASSERT( kfm != NULL );

	n = Phascii_Get(pi, "CELL.MACHINE.TERMINATED", "%d", &kfm->terminated);
	if( n != 1 ) {
		FREE(c);
		kforth_machine_delete(kfm);
		sprintf(errmsg, "missing CELL.MACHINE.TERMINATED");
		return 0;
	}

	n = Phascii_Get(pi, "CELL.MACHINE.CB", "%d", &kfm->cb);
	if( n != 1 ) {
		FREE(c);
		kforth_machine_delete(kfm);
		sprintf(errmsg, "missing CELL.MACHINE.CB");
		return 0;
	}

	n = Phascii_Get(pi, "CELL.MACHINE.PC", "%d", &kfm->pc);
	if( n != 1 ) {
		FREE(c);
		kforth_machine_delete(kfm);
		sprintf(errmsg, "missing CELL.MACHINE.PC");
		return 0;
	}

	/*
	 * Read REGISTER array
	 */
	for(i=0; i<10; i++) {
		n = Phascii_Get(pi, "CELL.MACHINE.R[%0].VALUE", i, "%ll", &kfm->R[i]);
		if( n != 1 ) {
			FREE(c);
			kforth_machine_delete(kfm);
			sprintf(errmsg, "missing CELL.MACHINE.R[%d].VALUE", i);
			return 0;
		}
	}

	/*
	 * Read CALL_STACK
	 */
	n = Phascii_Get(pi, "CELL.MACHINE.CALL_STACK.N", "%d", &num);
	if( n != 1 ) {
		FREE(c);
		kforth_machine_delete(kfm);
		sprintf(errmsg, "missing CELL.MACHINE.CALL_STACK.N");
		return 0;
	}

	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "CELL.MACHINE.CALL_STACK[%0].CB", i, "%d", &cb);
		if( n != 1 ) {
			FREE(c);
			kforth_machine_delete(kfm);
			sprintf(errmsg, "missing CELL.MACHINE.CALL_STACK[%d].CB", i);
			return 0;
		}

		n = Phascii_Get(pi, "CELL.MACHINE.CALL_STACK[%0].PC", i, "%d", &pc);
		if( n != 1 ) {
			FREE(c);
			kforth_machine_delete(kfm);
			sprintf(errmsg, "missing CELL.MACHINE.CALL_STACK[%d].PC", i);
			return 0;
		}

		kforth_call_stack_push(kfm, cb, pc);
	}


	/*
	 * Read DATA_STACK
	 */
	n = Phascii_Get(pi, "CELL.MACHINE.DATA_STACK.N", "%d", &num);
	if( n != 1 ) {
		FREE(c);
		kforth_machine_delete(kfm);
		sprintf(errmsg, "missing CELL.MACHINE.DATA_STACK.N");
		return 0;
	}

	for(i=0; i < num; i++) {
		n = Phascii_Get(pi, "CELL.MACHINE.DATA_STACK[%0].VALUE", i, "%ll", &value);
		if( n != 1 ) {
			FREE(c);
			kforth_machine_delete(kfm);
			sprintf(errmsg, "missing CELL.MACHINE.DATA_STACK[%d].VALUE", i);
			return 0;
		}
		kforth_data_stack_push(kfm, value);
	}

	/*
	 * Find organism
	 */
	o = NULL;
	for(ocurr=u->organisms; ocurr; ocurr=ocurr->next) {
		if( ocurr->id == organism_id ) {
			o = ocurr;
			break;
		}
	}

	if( o == NULL ) {
		sprintf(errmsg, "ORGANISM %d not found", organism_id);
		FREE(c);
		kforth_machine_delete(kfm);
		return 0;
	}

	kfm->client_data	= c;
	kfm->program		= o->program;
	c->kfm			= kfm;
	c->organism		= o;

	/*
	 * Attach cell to organism
	 */
	o->ncells += 1;
	if( o->cells == NULL ) {
		o->cells = c;
	} else {
		for(ccurr=o->cells; ccurr->next; ccurr=ccurr->next)
			;
		ccurr->next = c;
	}

	Grid_SetCell(u, c);

	return 1;
}

/***********************************************************************
 * Read a universe from 'filename'.
 *
 * We're reading photon ascii. So the basic algorithm is to read
 * 'Photon Ascii Instances' and then switch on the type and read the fields.
 * Ignore unknown instances.
 *
 * RETURNS:
 *	On Success, a universe object is returned.
 *	On Failure, NULL is returned and errbuf contains a error message.
 *
 */
UNIVERSE *Universe_ReadAscii(const char *filename, char *errbuf)
{
	PHASCII_FILE phf;
	PHASCII_INSTANCE pi;
	char errmsg[1000];
	int success;
	UNIVERSE *u;

	ASSERT( filename != NULL );
	ASSERT( errbuf != NULL );

	phf = Phascii_Open(filename, "r");
	if( phf == NULL ) {
		strcpy(errbuf, Phascii_GetError());
		return NULL;
	}

	u = NULL;

	while( pi = Phascii_GetInstance(phf) ) {

		if( Phascii_IsInstance(pi, "ORGANIC") ) {
			success = read_organic(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "BARRIER") ) {
			success = read_barrier(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "ER") ) {
			success = read_er(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "KFMO") ) {
			success = read_kfmo(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "KEYLIST") ) {
			success = read_keylist(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "SPORE") ) {
			success = read_spore(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "CELL") ) {
			success = read_cell(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "ORGANISM") ) {
			success = read_organism(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "PLAYER") ) {
			success = read_player(pi, u, errmsg);

		} else if( Phascii_IsInstance(pi, "UNIVERSE") ) {
			success = read_universe(pi, &u, errmsg);

		} else {
			success = 1;
		}

		Phascii_FreeInstance(pi);

		if( ! success ) {
			sprintf(errbuf, "%s", errmsg);
			Phascii_Close(phf);
			return NULL;
		}
	}

	if( ! Phascii_Eof(phf) ) {
		sprintf(errbuf, "%s\n", Phascii_Error(phf));
		Phascii_Close(phf);
		return NULL;
	}

	Phascii_Close(phf);

	if( u == NULL ) {
		sprintf(errbuf, "%s: No UNIVERSE instance", filename);
		return NULL;
	}

	return u;
}
