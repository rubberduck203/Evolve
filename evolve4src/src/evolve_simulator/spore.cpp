/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * SPORE:
 *
 */
#include "stdafx.h"

SPORE *Spore_make(KFORTH_PROGRAM *program, int energy, LONG_LONG parent, int strain)
{
	SPORE *spore;

	ASSERT( program != NULL );
	ASSERT( energy > 0 );

	spore = (SPORE *) CALLOC(1, sizeof(SPORE));
	ASSERT( spore != NULL );

	spore->program = program;
	spore->energy = energy;
	spore->parent = parent;
	spore->strain = strain;

	return spore;
}

void Spore_delete(SPORE *spore)
{
	ASSERT( spore != NULL );

	kforth_delete(spore->program);
	FREE(spore);
}

/*
 * Fertilize the spore:
 */
void Spore_fertilize(UNIVERSE *u, ORGANISM *o, SPORE *spore,
					int x, int y, int energy)
{
	KFORTH_PROGRAM *program;
	ORGANISM *no;
	CELL *nc;

	ASSERT( u != NULL );
	ASSERT( o != NULL );
	ASSERT( spore != NULL );
	ASSERT( o->strain == spore->strain );
	ASSERT( energy >= 0 );

#if 0
	/* OLD WAY OF MERGING, NO LONGER USED */
	program = kforth_merge(o->program, spore->program);
#else
	program = kforth_merge_rnd(u->er, o->program, spore->program);
#endif
	kforth_mutate(program, u->kfmo, u->er);

	no = (ORGANISM *) CALLOC(1, sizeof(ORGANISM));
	ASSERT( no != NULL );

	nc = (CELL *) CALLOC(1, sizeof(CELL));
	ASSERT( nc != NULL );

	nc->kfm		= kforth_machine_make(program, nc);
	nc->x		= x;
	nc->y		= y;
	nc->organism	= no;
	nc->next	= NULL;

	no->strain	= spore->strain;
	no->id		= u->next_id++;
	no->parent1	= spore->parent;
	no->parent2	= o->id;
	no->generation	= o->generation + 1;
	no->energy	= spore->energy + energy;
	no->age		= 0;
	no->program	= program;

	no->ncells	= 1;
	no->cells	= nc;

	no->next	= NULL;
	no->universe	= u;

	/*
	 * If organism or spore is radioactive, then new organism is too
	 */
	if( (spore->sflags & SPORE_FLAG_RADIOACTIVE)
				|| (o->oflags & ORGANISM_FLAG_RADIOACTIVE) ) {

		no->oflags |= ORGANISM_FLAG_RADIOACTIVE;

	}

	if( o->oflags & ORGANISM_FLAG_SEXONLY ) {
		no->oflags |= ORGANISM_FLAG_SEXONLY;
	}

	Grid_SetCell(u, nc);

	no->next		= u->new_organisms;
	u->new_organisms	= no;

	u->nborn++;

	Spore_delete(spore);
}
