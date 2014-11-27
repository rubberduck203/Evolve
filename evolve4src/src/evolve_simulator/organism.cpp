/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * ORGANISM OPERATIONS
 *
 *
 */
#include "stdafx.h"

/*
 * Color cells:
 *	Set the color field for all cells that
 *	touch 'cell'
 *
 */
#define COLOR_IT(c)	( (c!=NULL) && (!c->kfm->terminated) && (c->color==0) )

/*
 * visit every cell reachable by 'cell' in the north, south, east, west
 * direction and set its color field to 'color'.
 */
static void color_cells(CELL *cell, int color)
{
	CELL *north, *south, *east, *west;

	ASSERT( cell != NULL );

	north = Cell_Neighbor(cell, 0, -1);
	south = Cell_Neighbor(cell, 0, 1);
	east = Cell_Neighbor(cell, 1, 0);
	west = Cell_Neighbor(cell, -1, 0);

	cell->color = (char)color;

	if( COLOR_IT(north) )
		color_cells(north, color);

	if( COLOR_IT(south) )
		color_cells(south, color);

	if( COLOR_IT(east) )
		color_cells(east, color);

	if( COLOR_IT(west) )
		color_cells(west, color);
}

/*
 * Color all non-dead cells reachable from 'cell'
 */
static void color_all_cells(CELL *cell, int *color)
{
	CELL *north, *south, *east, *west;

	ASSERT( cell != NULL );
	ASSERT( color != NULL );

	north = Cell_Neighbor(cell, 0, -1);
	south = Cell_Neighbor(cell, 0, 1);
	east = Cell_Neighbor(cell, 1, 0);
	west = Cell_Neighbor(cell, -1, 0);

	if( COLOR_IT(north) ) {
		(*color)++;
		color_cells(north, *color);
	}

	if( COLOR_IT(south) ) {
		(*color)++;
		color_cells(south, *color);
	}

	if( COLOR_IT(east) ) {
		(*color)++;
		color_cells(east, *color);
	}

	if( COLOR_IT(west) ) {
		(*color)++;
		color_cells(west, *color);
	}
}


/*
 * Divide the organism into regions, based on
 * how the organism is divided into peices after
 * dead cells are removed.
 *
 * The largest region lives, smaller regions
 * will be converted to dead matter.
 *
 * If two or more regions "tie" for being the
 * largest, only one of them will remain.
 *
 */
#define MAX_REGIONS	100

static void kill_dead_cells(ORGANISM *o)
{
	UNIVERSE *u;
	int max_found;
	long max;
	int ndead, color, keep_color, i, energy;
	CELL *c, *prev, *nxt;
	int counts[ MAX_REGIONS ];

	ASSERT( o != NULL );

	u = o->universe;

	/*
	 * Set the graph coloring field to 0.
	 * (and count the dead)
	 */
	ndead = 0;
	for(c=o->cells; c; c=c->next) {
		c->color = 0;
		if( c->kfm->terminated ) {
			ndead += 1;
		}
	}

	if( ndead == 0 ) {
		return;
	}

	/*
	 * Divide organism into contigous regions.
	 * Each region is assigned a unique non-zero 'color' number.
	 */
	color = 0;
	for(c=o->cells; c; c=c->next) {
		if( c->kfm->terminated ) {
			color_all_cells(c, &color);
		}
	}

	/*
	 * Count the number of cells in each region.
	 */
	ASSERT( color < MAX_REGIONS );
	for(i=0; i < color; i++)
		counts[i] = 0;

	for(c=o->cells; c; c=c->next) {
		if( c->kfm->terminated )
			continue;

		/*
		 * every non-dead cell should have an non-zero color value
		 */
		ASSERT( c->color != 0 );

		/*
		 * Increment counter for this region
		 */
		counts[ c->color-1 ] += 1;
	}

	/*
	 * Find the largest region, set variable "keep_color" to that color.
	 */
	max = 0;
	max_found = 0;
	for(i=0; i < color; i++) {
		if( counts[i] > max ) {
			max_found = 1;
			keep_color = i+1;
			max = counts[i];
		}
	}

	/*
	 * Kill all cells that are not in 'keep_color'
	 */
	prev = NULL;
	for(c=o->cells; c; c=nxt) {
		nxt = c->next;

		if( max_found && c->color == keep_color ) {
			prev = c;
			continue;
		}

		if( prev == NULL )
			o->cells = nxt;
		else
			prev->next = nxt;
		/*
		 * Free the cell 'c'
		 */
		o->ncells -= 1;

		energy = (c->kfm->call_stack_size + c->kfm->data_stack_size) - NOCOST_STACK;
		if( energy < 0 ) {
			energy = 0;
		}

		if( energy > 0 ) {
			Grid_SetOrganic(u, c->x, c->y, energy);
		} else {
			Grid_Clear(u, c->x, c->y);
		}

		Cell_delete(c);
	}
}

/*
 * Put organic material at (x, y) with 'energy'
 *
 * If organic is already present at x, y, append
 * energy to it.
 *
 */
static void append_organic(UNIVERSE *u, int x, int y, int energy)
{
	GRID_TYPE type;
	UNIVERSE_GRID *ugp;

	type = Grid_GetPtr(u, x, y, &ugp);

	if( type == GT_ORGANIC ) {
		ugp->u.energy += energy;

	} else if( type == GT_BLANK ) {
		if( energy > 0 ) {
			ugp->type = GT_ORGANIC;
			ugp->u.energy = energy;
		}

	} else if( type == GT_CELL ) {
		if( energy > 0 ) {
			ugp->type = GT_ORGANIC;
			ugp->u.energy = energy;
		} else {
			ugp->type = GT_BLANK;
		}

	} else {
		ASSERT(0);
	}
}

/***********************************************************************
 * Simulate an organism, by simulating each cell.
 *
 * Return 1 if we are still alive. Else return 0.
 *
 */
int Organism_Simulate(ORGANISM *o)
{
	CELL *c, *nxt;
	UNIVERSE *u;
	int energy, energy_stack, energy_per_cell, energy_residue;
	int i;
	int first_x, first_y;

	ASSERT( o != NULL );
	ASSERT( o->ncells > 0 );

	o->age += 1;

	u = o->universe;

	/*
	 * This location will recieve ALL organism energy if
	 * organism's cells are all dead.
	 */
	first_x = o->cells->x;
	first_y = o->cells->y;

	/*
	 * Kill cells that are disconnected from main
	 * body mass. These are any cells destroyed by other
	 * organisms. (or died when their KFORTH program terminated).
	 */
	kill_dead_cells(o);

	/*
	 * Simulate each cell in the organism.
	 */
	for(c=o->cells; c && (o->energy > 0); c=c->next) {
		ASSERT( ! c->kfm->terminated );

		Cell_Simulate(c);
	}

	if( o->energy > 0 && o->ncells > 0 ) {
		/*
		 * we're still alive
		 */
		return 1;
	}

	/*
	 * We must be dead, so clean up organism and return 0.
	 */
	if( o->ncells > 0 ) {
		energy_per_cell = o->energy / o->ncells;
		energy_residue = o->energy % o->ncells;

		i = 0;
		for(c=o->cells; c; c=nxt) {
			nxt = c->next;

			if( i == 0 )
				energy = energy_per_cell + energy_residue;
			else
				energy = energy_per_cell;

			energy_stack = (c->kfm->data_stack_size + c->kfm->call_stack_size) - NOCOST_STACK;
			if( energy_stack < 0 ) {
				energy_stack = 0;
			}

			energy += energy_stack;

			append_organic(u, c->x, c->y, energy);

			Cell_delete(c);
			i++;
		}
		o->cells = NULL;
	} else {
		append_organic(u, first_x, first_y, o->energy);
	}

	return 0;
}

/***********************************************************************
 * Create a new organism.
 * We compile 'program_text' and create the first
 * organism with 1 cell at x, y.
 *
 */
ORGANISM *Organism_Make(int x, int y, int strain, int energy, const char *program_text, char *errbuf)
{
	ORGANISM *o;
	CELL *c;
	KFORTH_PROGRAM *kfp;
	KFORTH_MACHINE *kfm;

	ASSERT( x >= 0 );
	ASSERT( y >= 0 );
	ASSERT( strain >= 0 && strain < EVOLVE_MAX_STRAINS );
	ASSERT( energy > 0 );
	ASSERT( program_text != NULL );
	ASSERT( errbuf != NULL );

	kfp = kforth_compile(program_text, EvolveOperations(), errbuf);
	if( kfp == NULL )
		return NULL;

	o = (ORGANISM *) CALLOC(1, sizeof(ORGANISM));
	ASSERT( o != NULL );

	c = (CELL *) CALLOC(1, sizeof(CELL));
	ASSERT( c != NULL );

	/*
	 * Initialize organism
	 */
	o->strain = strain;
	o->energy = energy;
	o->ncells = 1;
	o->cells = c;
	o->next = NULL;
	o->universe = NULL;
	o->program = kfp;

	/*
	 * Initialize cell
	 */
	kfm = kforth_machine_make(kfp, c);
	c->kfm = kfm;
	c->x = x;
	c->y = y;
	c->next = NULL;
	c->organism = o;

	return o;
}

void Organism_delete(ORGANISM *o)
{
	ASSERT( o != NULL );

	kforth_delete(o->program);
	FREE(o);
}
