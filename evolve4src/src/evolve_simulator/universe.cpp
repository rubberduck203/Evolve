/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * UNIVERSE OPERATIONS
 *
 *
 */
#include "stdafx.h"

#define GET_GRID(u, x, y)	(   &(u)->grid[(y)*(u)->width + (x)]   )

#if 0
/*
 * Debug routine to compute total enerergy in the universe.
 */
int assert_total_energy(UNIVERSE *u)
{
	UNIVERSE_GRID ugrid;
	GRID_TYPE type;
	CELL *cell;
	ORGANISM *o;
	PLAYER *player;
	SPORE *spore;
	int x, y;
	int energy;

	ASSERT( u != NULL );

	energy = 0;

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Universe_Query(u, x, y, &ugrid);

			if( type == GT_ORGANIC ) {
				energy += ugrid.u.energy;
			}

			if( type == GT_SPORE ) {
				spore = ugrid.u.spore;
				energy += spore->energy;
			}

			if( type == GT_CELL ) {
				cell = ugrid.u.cell;
				energy += cell->kfm->data_stack_size;
				energy += cell->kfm->call_stack_size;
			}

			if( type == GT_PLAYER ) {
				player = ugrid.u.player;
				energy += player->energy;
			}

		}
	}

	for(o=u->organisms; o; o=o->next) {
		energy += o->energy;
	}

	for(o=u->new_organisms; o; o=o->next) {
		energy += o->energy;
	}

	return energy;
}
#endif

GRID_TYPE Grid_GetPtr(UNIVERSE *u, int x, int y, UNIVERSE_GRID **ugrid)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( ugrid != NULL );

	grid = GET_GRID(u, x, y);

	*ugrid = grid;

	return (GRID_TYPE) grid->type;
}

GRID_TYPE Grid_Get(UNIVERSE *u, int x, int y, UNIVERSE_GRID *ugrid)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( ugrid != NULL );

	grid = GET_GRID(u, x, y);

	*ugrid = *grid;

	return (GRID_TYPE) grid->type;
}

void Grid_Clear(UNIVERSE *u, int x, int y)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_BLANK;
	grid->u.energy	= 0;
}

void Grid_SetBarrier(UNIVERSE *u, int x, int y)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_BARRIER;
	grid->u.energy	= 0;
}

void Grid_SetCell(UNIVERSE *u, CELL *cell)
{
	int x, y;
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( cell != NULL );
	ASSERT( cell->x >= 0 && cell->x < u->width );
	ASSERT( cell->y >= 0 && cell->y < u->height );

	x = cell->x;
	y = cell->y;

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_CELL;
	grid->u.cell	= cell;
}

void Grid_SetOrganic(UNIVERSE *u, int x, int y, int energy)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( energy >= 0 );

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_ORGANIC;
	grid->u.energy	= energy;
}

void Grid_SetSpore(UNIVERSE *u, int x, int y, SPORE *spore)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( spore != NULL );

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_SPORE;
	grid->u.spore	= spore;
}

void Grid_SetPlayer(UNIVERSE *u, int x, int y, PLAYER *player)
{
	UNIVERSE_GRID *grid;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( player != NULL );

	grid		= GET_GRID(u, x, y);
	grid->type	= GT_PLAYER;
	grid->u.player	= player;
}

/***********************************************************************
 * Create a blank universe. A blank universe contains
 * no objects on it grid. the grid is defined as width x height.
 *
 * The random seed is initialized to 'seed'.
 *
 */
UNIVERSE *Universe_Make(unsigned long seed, int width, int height, KFORTH_MUTATE_OPTIONS *kfmo)
{
	UNIVERSE *u;

	ASSERT( width >= EVOLVE_MIN_BOUNDS && width <= EVOLVE_MAX_BOUNDS );
	ASSERT( height >= EVOLVE_MIN_BOUNDS && height <= EVOLVE_MAX_BOUNDS );
	ASSERT( kfmo != NULL );

	u = (UNIVERSE *) CALLOC(1, sizeof(UNIVERSE));
	ASSERT( u != NULL );

	u->seed = seed;
	u->er = sim_random_make(seed);

	u->next_id = 1;

	u->width = width;
	u->height = height;

	u->grid = (UNIVERSE_GRID*) CALLOC(width * height, sizeof(UNIVERSE_GRID));
	ASSERT( u->grid != NULL );

	u->kfmo = kforth_mutate_options_copy(kfmo);

	u->player = NULL;

	u->keylist = KEYLIST_Make();

	return u;
}

/*
 * Free all the memory associated with an organism.
 */
static void complete_organism_free(ORGANISM *o)
{
	CELL *c, *nxt;

	if( o->cells ) {
		ASSERT( o->ncells > 0 );

		for(c=o->cells; c; c=nxt) {
			nxt = c->next;
			Cell_delete(c);
		}
	}

	Organism_delete(o);
}

/***********************************************************************
 * Free all memory associated with a universe 'u' object.
 *
 */
void Universe_Delete(UNIVERSE *u)
{
	ORGANISM *curr, *nxt;
	UNIVERSE_GRID *ugp;
	int x, y;

	ASSERT( u != NULL );

	for(curr=u->new_organisms; curr; curr=nxt) {
		nxt = curr->next;
		complete_organism_free(curr);
	}

	for(curr=u->organisms; curr; curr=nxt) {
		nxt = curr->next;
		complete_organism_free(curr);
	}

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			ugp = GET_GRID(u, x, y);
			if( ugp->type == GT_SPORE ) {
				Spore_delete(ugp->u.spore);
			}
		}
	}

	sim_random_delete(u->er);
	kforth_mutate_options_delete(u->kfmo);
	KEYLIST_Delete(u->keylist);
	FREE(u->grid);
	FREE(u);
}

/***********************************************************************
 * Simulate the universe 1 step.
 *
 */
void Universe_Simulate(UNIVERSE *u)
{
	ORGANISM *curr, *prev, *nxt;
	int alive;

	ASSERT( u != NULL );

	if( u->player != NULL ) {
		Player_Simulate(u->player);
	}

	prev = NULL;
	for(curr = u->organisms; curr; curr=nxt) {
		nxt = curr->next;

		alive = Organism_Simulate(curr);
		if( ! alive ) {
			if( prev != NULL ) {
				prev->next = curr->next;
			} else {
				u->organisms = curr->next;
			}

			if( curr == u->selected_organism ) {
				u->selected_organism = NULL;
			}

			/*
			 * Free the ORGANISM node (including the program).
			 * All the cells were free'd inside of Organism_Simulate().
			 */
			ASSERT(curr->cells == NULL);

			Organism_delete(curr);

			u->ndie += 1;
			u->norganism -= 1;
		} else {
			prev = curr;
		}
	}

	/*
	 * Append new organisms, to the end of the Organism list.
	 * 'prev' points to the last node in the Organism list.
	 */
	if( prev != NULL ) {
		ASSERT( prev->next == NULL );
		for(curr=u->new_organisms; curr; curr=curr->next) {
			u->norganism += 1;
		}
		prev->next = u->new_organisms;
	} else {
		u->organisms = u->new_organisms;
	}
	u->new_organisms = NULL;

	if( (u->player != NULL) && u->player->terminated ) {
		/*
		 * player object got eaten, cleanup
		 */
		Player_delete(u->player);
		u->player = NULL;
	}

	/*
	 * Increment step.
	 */
	u->step += 1;
}

/***********************************************************************
 * Calculate infomration about the universe
 *
 */
void Universe_Information(UNIVERSE *u, UNIVERSE_INFORMATION *uinfo)
{
	UNIVERSE_GRID ugrid;
	GRID_TYPE type;
	CELL *cell;
	ORGANISM *o;
	PLAYER *player;
	SPORE *spore;
	KFORTH_PROGRAM *kfp;
	int x, y, e;

	ASSERT( u != NULL );
	ASSERT( uinfo != NULL );

	memset(uinfo, 0, sizeof(UNIVERSE_INFORMATION));

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Universe_Query(u, x, y, &ugrid);

			if( type == GT_ORGANIC ) {
				uinfo->energy += ugrid.u.energy;
				uinfo->num_organic++;
				uinfo->organic_energy += ugrid.u.energy;
			}

			if( type == GT_SPORE ) {
				spore = ugrid.u.spore;
				kfp = spore->program;
				uinfo->num_instructions += kforth_program_length(kfp);
				uinfo->energy += spore->energy;
				uinfo->num_spores++;
				uinfo->spore_memory += sizeof(SPORE);
				uinfo->program_memory += kforth_program_size(kfp);
				uinfo->spore_energy += spore->energy;
			}

			if( type == GT_CELL ) {
				cell = ugrid.u.cell;
				uinfo->call_stack_nodes += cell->kfm->call_stack_size;
				uinfo->data_stack_nodes += cell->kfm->data_stack_size;
				e = cell->kfm->call_stack_size + cell->kfm->data_stack_size - NOCOST_STACK;
				if( e < 0 ) {
					e = 0;
				}
				uinfo->energy += e;
				uinfo->num_cells++;
			}

			if( type == GT_PLAYER ) {
				player = ugrid.u.player;
				uinfo->energy += player->energy;
			}

			uinfo->grid_memory += sizeof(UNIVERSE_GRID);
		}
	}

	for(o=u->organisms; o; o=o->next) {
		kfp = o->program;
		uinfo->num_instructions += kforth_program_length(kfp);

		uinfo->energy += o->energy;

		uinfo->organism_memory += sizeof(ORGANISM);
		uinfo->organism_memory += o->ncells * (sizeof(CELL) + sizeof(KFORTH_MACHINE));
		uinfo->program_memory += kforth_program_size(kfp);

		if( o->parent1 != o->parent2 )
			uinfo->num_sexual += 1;

		ASSERT( o->strain >= 0 && o->strain < EVOLVE_MAX_STRAINS );

		uinfo->strain_population[o->strain] += 1;

		if( o->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
			uinfo->radioactive_population[o->strain] += 1;
		}
	}

	uinfo->cstack_memory = uinfo->call_stack_nodes * sizeof(KFORTH_STACK_NODE);
	uinfo->dstack_memory = uinfo->data_stack_nodes * sizeof(KFORTH_STACK_NODE);
}

/***********************************************************************
 * Create a barrier (if grid location is blank).
 *
 */
void Universe_SetBarrier(UNIVERSE *u, int x, int y)
{
	UNIVERSE_GRID *g;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );

	g = GET_GRID(u, x, y);

	if( g->type == GT_BLANK ) {
		g->type = GT_BARRIER;
	}
}

/***********************************************************************
 * Clear a barrier (if grid location is a barrier).
 *
 */
void Universe_ClearBarrier(UNIVERSE *u, int x, int y)
{
	UNIVERSE_GRID *g;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );

	g = GET_GRID(u, x, y);

	if( g->type == GT_BARRIER ) {
		g->type = GT_BLANK;
	}

}

/***********************************************************************
 * Query an x,y position in the universe.
 *
 * Returns the grid type, and any associated data is returned in 'ugrid'
 *
 */
GRID_TYPE Universe_Query(UNIVERSE *u, int x, int y, UNIVERSE_GRID *ugrid)
{
	UNIVERSE_GRID *g;

	ASSERT( u != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( ugrid != NULL );

	g = GET_GRID(u, x, y);

	*ugrid = *g;

	return (GRID_TYPE) g->type;
}

/***********************************************************************
 * These routines keep track of a single organism, and allow
 * the caller to remove it, make a copy, or insert an organism
 * into another universe.
 *
 * If the selected organism is removed from universe, the selection
 * is also removed.
 * 
 *	Universe_SelectOrganism(UNIVERSE *, ORGANISM *);
 *		Remembers an organism. If the organism is not
 *		part of the universe, then this routine ASSERT's
 *
 *	Universe_GetSelection(UNIVERSE *);
 *		Returns the currently selected organism. NULL  is returned
 *		if no selection exists.
 *
 *	Universe_CopyOrganism(UNIVERSE *);
 *		Makes a complete copy of the organism and returns it.
 *		(The copy is detatched from the universe).
 *
 *	Universe_CutOrganism(UNIVERSE *);
 *		Removes the currently selected organism from the universe, and returns
 *		a pointer to the organism.
 *
 *	Universe_PasteOrganism(UNIVERSE *, ORGANISM *);
 *		Inserts the organism into the universe. If the organism is already
 *		in the universe, then nothing is done.
 *	
 *	Universe_FreeOrganism(ORGANISM *);
 *		Free's all the memory associated with the organism.
 *
 */

/***********************************************************************
 * 'o' must be a valid organism inside of 'u'.
 * Flags this organism as selected. It will remain the
 * selected organism until:
 *
 *	1. Selection is cleared
 *	2. Another organism is selected
 *	3. Organism dies during simulation
 *
 */
void Universe_SelectOrganism(UNIVERSE *u, ORGANISM *o)
{
	ORGANISM *curr;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	/*
	 * Make sure the organism is in our universe.
	 */
	for(curr=u->organisms; curr; curr=curr->next) {
		if( o == curr ) {
			u->selected_organism = o;
			return;
		}
	}
	ASSERT(0);
}

/***********************************************************************
 * Clears the selection (if any)
 *
 */
void Universe_ClearSelectedOrganism(UNIVERSE *u)
{
	ASSERT( u != NULL );

	u->selected_organism = NULL;
}

/***********************************************************************
 * Returns the selected organism (if any).
 *
 */
ORGANISM *Universe_GetSelection(UNIVERSE *u)
{
	ASSERT( u != NULL );

	return u->selected_organism;
}

/***********************************************************************
 * Make a copy of 'osrc'
 */
ORGANISM *Universe_DuplicateOrganism(ORGANISM *osrc)
{
	ORGANISM *odst;
	CELL *cprev, *csrc, *cdst;

	ASSERT( osrc != NULL );

	odst = (ORGANISM *) CALLOC(1, sizeof(ORGANISM) );
	ASSERT( odst != NULL );

	*odst = *osrc;

	odst->next = NULL;
	odst->universe = NULL;

	/*
	 * Copy the program;
	 */
	odst->program = kforth_copy(osrc->program);

	/*
	 * Copy the cells.
	 */
	cprev = NULL;
	for(csrc=osrc->cells; csrc; csrc=csrc->next) {
		cdst = (CELL *) CALLOC(1, sizeof(CELL) );
		ASSERT(cdst);

		*cdst = *csrc;

		cdst->kfm = kforth_machine_copy(csrc->kfm);
		cdst->kfm->program = odst->program;
		cdst->kfm->client_data = cdst;
		cdst->next = NULL;
		cdst->organism = odst;

		if( cprev == NULL ) {
			odst->cells = cdst;
		} else {
			cprev->next = cdst;
		}

		cprev = cdst;
	}

	return odst;
}

/***********************************************************************
 * Copy Selected Organism and returns it.
 * The returned organism is not attached to any UNIVERSE.
 *
 */
ORGANISM *Universe_CopyOrganism(UNIVERSE *u)
{
	ORGANISM *odst, *osrc;

	ASSERT( u != NULL );
	ASSERT( u->selected_organism != NULL );

	osrc = u->selected_organism;

	odst = Universe_DuplicateOrganism(osrc);

	return odst;
}

/***********************************************************************
 * Removes selected organism from universe and returns the
 * selection.
 *
 */
ORGANISM *Universe_CutOrganism(UNIVERSE *u)
{
	ORGANISM *curr, *prev;
	CELL *cell;

	ASSERT( u != NULL );
	ASSERT( u->selected_organism != NULL );

	prev = NULL;
	for(curr=u->organisms; curr; curr=curr->next) {
		if( curr == u->selected_organism ) {
			if( prev == NULL ) {
				u->organisms = curr->next;
			} else {
				prev->next = curr->next;
			}
			curr->next = NULL;
			curr->universe = NULL;
			u->selected_organism = NULL;
			u->norganism -= 1;

			for(cell=curr->cells; cell; cell=cell->next) {
				Grid_Clear(u, cell->x, cell->y);
			}
			return curr;
		}

		prev = curr;
	}
	ASSERT(0);
	return NULL;
}

/***********************************************************************
 * Insert organism into universe.
 *
 * The organism 'o' will be set as the selected organism.
 *
 */
void Universe_PasteOrganism(UNIVERSE *u, ORGANISM *o)
{
	CELL *cell;
	int x, y;
	int origin_x, origin_y;
	int tmp_x, tmp_y;
	int good_cells;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;
	char key[100];
	int start_paste_x, start_paste_y, paste_dir;

	ASSERT( u != NULL );
	ASSERT( o != NULL );
	ASSERT( o->next == NULL );
	ASSERT( o->universe == NULL );

	/*
	 * Add the organism.
	 */
	o->id		= u->next_id++;
	o->universe	= u;
	o->next		= u->organisms;
	u->organisms	= o;
	u->norganism	+= 1;

	u->selected_organism = o;

	sprintf(key, "HadStrain%d", o->strain);
	Universe_SetAttribute(u, key, "Y");

#if 0
	good_cells = 0;
	for(cell=o->cells; cell; cell=cell->next) {
		if( cell->x < 0 || cell->x > u->width )
			continue;

		if( cell->y < 0 || cell->y > u->height )
			continue;

		type = Grid_Get(u, cell->x, cell->y, &ugrid);
		if( type == GT_BLANK )
			good_cells += 1;
	}

	/*
	 * All cells are located on blank squares
	 */
	if( good_cells == o->ncells ) {
		for(cell=o->cells; cell; cell=cell->next) {
			Grid_SetCell(u, cell);
		}
		return;
	}
#endif

	/*
	 * Normalize the coordinates in the organism
	 * First cell is (0,0) and all other
	 * cells have their (x,y) coordinates
	 * adjusted relative to that.
	 */
	origin_x = o->cells->x;
	origin_y = o->cells->y;

	for(cell=o->cells; cell; cell=cell->next) {
		cell->x = cell->x - origin_x;
		cell->y = cell->y - origin_y;
	}

	/*
	 * determine where to begin looking for a good spot to paste organism.
	 */	
	if( (origin_x < u->width) && (origin_y < u->height) ) {
		start_paste_x = origin_x;
		start_paste_y = origin_y;
	} else {
		start_paste_x = u->width/2;
		start_paste_y = u->height/2;
	}

	/*
	 * Scan the universe for a spot to insert
	 * the organism.
	 */
	for(paste_dir=0; paste_dir <= 7; paste_dir++) {
		x = start_paste_x;
		y = start_paste_y;

		while( x < u->width && y < u->height ) {

			good_cells = 0;
			for(cell=o->cells; cell; cell=cell->next) {
				tmp_x = x + cell->x;
				tmp_y = y + cell->y;

				if( tmp_x < 0 || tmp_x >= u->width )
					break;

				if( tmp_y < 0 || tmp_y >= u->height )
					break;

				type = Grid_Get(u, tmp_x, tmp_y, &ugrid);

				if( type != GT_BLANK )
					break;

				good_cells += 1;
			}

			if( good_cells == o->ncells ) {
				for(cell=o->cells; cell; cell=cell->next) {
					cell->x = cell->x + x;
					cell->y = cell->y + y;

					Grid_SetCell(u, cell);
				}
				return;
			}

			switch( paste_dir ) {
			case 0:    x += 5; y += 5;   break;
			case 1:    x -= 5; y -= 5;   break;
			case 2:    x += 5; y -= 5;   break;
			case 3:    x -= 5; y += 5;   break;
			case 4:    x += 0; y += 5;   break;
			case 5:    x += 0; y -= 5;   break;
			case 6:    x += 5; y += 0;   break;
			case 7:    x -= 5; y += 0;   break;
			default:
				ASSERT(0);
			}
		}
	}

	/*
	 * If we get here, then we failed to find
	 * a vacant spot to paste. In this case we silently fail.
	 *
	 * TODO: Clean up. remove  organism, etc..
	 */
}

/***********************************************************************
 * Free an organism that was copied, or cut out of a universe.
 *
 */
void Universe_FreeOrganism(ORGANISM *o)
{
	ASSERT( o != NULL );
	ASSERT( o->next == NULL );
	ASSERT( o->universe == NULL );

	complete_organism_free(o);
}

void Universe_ClearTracers(UNIVERSE *u)
{
	int x, y;
	UNIVERSE_GRID ugrid;
	GRID_TYPE type;

	ASSERT( u != NULL );

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			type = Universe_Query(u, x, y, &ugrid);
			if( type == GT_BLANK )
				continue;

			if( type == GT_SPORE ) {
				Universe_ClearSporeTracer(ugrid.u.spore);

			} else if( type == GT_CELL ) {
				Universe_ClearOrganismTracer(ugrid.u.cell->organism);
			}
		}
	}

}

void Universe_SetSporeTracer(SPORE *spore)
{
	ASSERT( spore != NULL );

	spore->sflags |= SPORE_FLAG_RADIOACTIVE;
}

void Universe_SetOrganismTracer(ORGANISM *organism)
{
	ASSERT( organism != NULL );

	organism->oflags |= ORGANISM_FLAG_RADIOACTIVE;
}

void Universe_ClearSporeTracer(SPORE *spore)
{
	ASSERT( spore != NULL );

	spore->sflags &= ~SPORE_FLAG_RADIOACTIVE;
}

void Universe_ClearOrganismTracer(ORGANISM *organism)
{
	ASSERT( organism != NULL );

	organism->oflags &= ~ORGANISM_FLAG_RADIOACTIVE;
}

/*
 * These routines are just wrappers for the actual KEYLIST routine.
 */

/*
 * This routine associates the string "value" with the keyword "name".
 * If name already exists in the keylist, then the new value is
 * assigned to it. If name doesn't exist in the keylist, then a new
 * entry is added.
 *
 */
void Universe_SetAttribute(UNIVERSE *u, const char *name, char *value)
{
	ASSERT( u != NULL );
	ASSERT( name != NULL );
	ASSERT( value != NULL );

	KEYLIST_Set(u->keylist, name, value);
}

/* 
 * Get the string associated with the keyword "name". If "name" is
 * not in the keylist, then an empty string will be returned for value.
 */
void Universe_GetAttribute(UNIVERSE *u, const char *name, char *value)
{
	ASSERT( u != NULL );
	ASSERT( name != NULL );
	ASSERT( value != NULL );

	KEYLIST_Get(u->keylist, name, value);
}

/***********************************************************************
 *
 * Return the global KFORTH_OPERATIONS table.
 * The returned table is a static variable, and does
 * not need to be free'd. This table contains the ORGANISM/CELL operations
 * as well as the core KFORTH operations.
 *
 * When a new instruction needs to be added, add it to this
 * list.
 *
 * This is a "once" function meaning it computes the 
 * KFORTH_OPERATIONS table once, then returns it
 * on subsequent calls to this function.
 *
 * NOTE:
 * The reason we store this data in a static array is
 * so that this "once" function can be used without worrying
 * about freeing it. This is because many entities will be referring to it, and
 * it is impossible to assign ownership to any 1 client of this data structure.
 *
 */
KFORTH_OPERATIONS *EvolveOperations(void)
{
	static int first_time = 1;
	static KFORTH_OPERATIONS kfops_table[ KFORTH_OPS_LEN ];
	KFORTH_OPERATIONS *kfops;
	int i;

	if( first_time ) {
		first_time = 0;
		kfops = kforth_ops_make();
		kforth_ops_add(kfops,	"CMOVE",	Opcode_CMOVE);
		kforth_ops_add(kfops,	"OMOVE",	Opcode_OMOVE);
		kforth_ops_add(kfops,	"ROTATE",	Opcode_ROTATE);
		kforth_ops_add(kfops,	"EAT",		Opcode_EAT);
		kforth_ops_add(kfops,	"MAKE-SPORE",	Opcode_MAKE_SPORE);
		kforth_ops_add(kfops,	"GROW",		Opcode_GROW);
		kforth_ops_add(kfops,	"LOOK",		Opcode_LOOK);
		kforth_ops_add(kfops,	"WHAT",		Opcode_WHAT);
		kforth_ops_add(kfops,	"WHERE",	Opcode_WHERE);
		kforth_ops_add(kfops,	"NEAREST",	Opcode_NEAREST);
		kforth_ops_add(kfops,	"FARTHEST",	Opcode_FARTHEST);
		kforth_ops_add(kfops,	"NEAREST2",	Opcode_NEAREST2);
		kforth_ops_add(kfops,	"FARTHEST2",	Opcode_FARTHEST2);
		kforth_ops_add(kfops,	"MOOD",		Opcode_MOOD);
		kforth_ops_add(kfops,	"MOOD!",	Opcode_SET_MOOD);
		kforth_ops_add(kfops,	"BROADCAST",	Opcode_BROADCAST);
		kforth_ops_add(kfops,	"SEND",		Opcode_SEND);
		kforth_ops_add(kfops,	"RECV",		Opcode_RECV);
		kforth_ops_add(kfops,	"ENERGY",	Opcode_ENERGY);
		kforth_ops_add(kfops,	"AGE",		Opcode_AGE);
		kforth_ops_add(kfops,	"NUM-CELLS",	Opcode_NUM_CELLS);
		kforth_ops_add(kfops,	"HAS-NEIGHBOR",	Opcode_HAS_NEIGHBOR);

		/*
		 * copy to static table. (so clients of this data structure
		 * won't be required to free it)
		 */
		for(i=0; i < KFORTH_OPS_LEN; i++) {
			kfops_table[i] = kfops[i];
		}

		kforth_ops_delete(kfops);
	}

	return kfops_table;
}

/*
 * Return maximum opcodes of the 'EvolveOperations()' table.
 * This is a "once" function, meaning it computes the
 * value the first time then returns value after that.
 */
int EvolveMaxOpcodes(void)
{
	static int first_time = 1;
	static int max_opcodes;

	if( first_time ) {
		first_time = 0;
		max_opcodes = kforth_ops_max_opcode( EvolveOperations() );
	}

	return max_opcodes;
}

