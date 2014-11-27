/*
 * Copyright (c) 2007 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2007 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * CELL OPERATIONS:
 * In addition to the normal KFORTH language primitives, each cell
 * can execute one of these EVOLVE specific primitive operations:
 *
 *
 *	KFORTH		FUNCTION
 *	NAME		CALLED			DESCRIPTION
 *	--------------	------------------	-----------------------
 *	OMOVE		Opcode_OMOVE		move organism
 *	CMOVE		Opcode_CMOVE		move cell
 *	ROTATE		Opcode_ROTATE		rotate organism
 *	EAT		Opcode_EAT		eat any edible material adjecent to this cell.
 *	MAKE-SPORE	Opcode_MAKE_SPORE	release spore onto grid/fertilize existing spore
 *	GROW		Opcode_GROW		grow organism by 1 cell
 *
 * VISION:
 *	LOOK		Opcode_LOOK		set vision registers for all 8 directions
 *	WHAT		Opcode_WHAT		what did we see?
 *	WHERE		Opcode_WHERE		how far away did we see it?
 *	NEAREST		Opcode_NEAREST		what's the nearest thing seen?
 *	FARTHEST	Opcode_FARTHEST		what's the farthest thing seen?
 *	NEAREST2	Opcode_NEAREST2		what's the nearest thing seen?
 *	FARTHEST2	Opcode_FARTHEST2	what's the farthest thing seen?
 *
 * COMMUNICATE BETWEEN CELLS:
 *
 *	MOOD		Opcode_MOOD		get mood for cell at (xoffset, yoffset)
 *	MOOD!		Opcode_SET_MOOD		set our mood.
 *	BROADCAST	Opcode_BROADCAST	send message to ALL cells
 *	SEND		Opcode_SEND		send message to cell at (xoffset, yoffset)
 *	RECV		Opcode_RECV		get our message
 *
 * QUERY STATE OF ORGANISM:
 *
 *	ENERGY		Opcode_ENERGY		Energy left in organism
 *	AGE		Opcode_AGE		Age of organism
 *	NUM-CELLS	Opcode_NUM_CELLS	number of cells in organism
 *	HAS-NEIGHBOR	Opcode_HAS_NEIGHBOR	do we have a neighbor at x y?
 *
 * Normalized coordinates are coordinates that translate to
 * the values +1, 0, -1:
 *
 * BEFORE		NORMALIZED COORDINATES
 * (0, 0)		(0, 0)
 * (123, 0)		(+1, 0)
 * (123, -3939)		(+1, -1)
 * (-121, -3939)	(-1, -1)
 * (11, 22)		(+1, +1)
 *
 * Normalize coordinates are used for operations that act
 * on the grid location immediately surrounding the cell.
 *
 * 	(0,0)	<- no offset
 * 	(0,-1)	<- north location
 * 	(1,-1)	<- north east
 * 	(1,0)	<- east
 * 	(1,1)	<- south east
 * 	(0,1)	<- south
 * 	(-1,1)	<- south west
 * 	(-1,0)	<- west
 * 	(-1,-1)	<- north west
 *
 *	+-------+-------+-------+
 *	|       |       |       |
 *	|(-1,-1)| (0,-1)|(+1,-1)|
 *	|       |       |       |
 *	+-------+-------+-------+
 *	|       |       |       |
 *	| (-1,0)| (0,0) | (+1,0)|
 *	|       |       |       |
 *	+-------+-------+-------+
 *	|       |       |       |
 *	|(-1,+1)| (0,+1)|(+1,+1)|
 *	|       |       |       |
 *	+-------+-------+-------+
 *
 * There are 8 normalized locations surrounding every cell
 * on the grid.
 *
 * Operations that use normalized coordinates:
 *	OMOVE
 *	CMOVE
 *	EAT		(looks at all 8 locations)
 *	MAKE-SPORE
 *	GROW
 *	HAS_NEIGHBOR
 *
 */
#include "stdafx.h"


/*
 * VISION 'what' values. We use 1 bit position per what value
 * so these things can be OR'd together as a mask.
 */
#define VISION_MASK		0x7fff
#define VISION_TYPE_NONE	0
#define VISION_TYPE_CELL	1
#define VISION_TYPE_SPORE	2
#define VISION_TYPE_ORGANIC	4
#define VISION_TYPE_BARRIER	8

/*
 * Convert 'v' to:
 *	1	if v is > 0
 *	0	if v is = 0
 *	-1	if v is < 0
 */
#define NORMALIZE_OFFSET(v)	(  (v<0) ? -1 : ((v>0) ? 1 : 0)  )

static int grid_is_blank(UNIVERSE *u, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( u != NULL );

	if( x < 0 || x >= u->width )
		return 0;

	if( y < 0 || y >= u->height )
		return 0;

	type = Grid_Get(u, x, y, &ugrid);

	return (type == GT_BLANK);
}

/*
 * Can the organism move into this location?
 * This is routine is used by the OMOVE primitive to examine all destination
 * squares to see if this square is vacant (or contains one of our own cells).
 */
static int grid_can_moveto(UNIVERSE *u, ORGANISM *o, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	if( x < 0 || x >= u->width )
		return 0;

	if( y < 0 || y >= u->height )
		return 0;

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_BLANK )
		return 1;

	if( type == GT_CELL && ugrid.u.cell->organism == o )
		return 1;

	return 0;
}

/*
 * Return the cell located at (x,y) if it is part of organism 'o',
 * otherwise return NULL.
 */
static CELL *grid_has_our_cell(UNIVERSE *u, ORGANISM *o, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	if( x < 0 || x >= u->width )
		return NULL;

	if( y < 0 || y >= u->height )
		return NULL;

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_CELL ) {
		if( ugrid.u.cell->organism == o )
			return ugrid.u.cell;
	}

	return NULL;
}

/*
 * This routine looks along a simple line in one of the 8 directions.
 * The coordinates (xoffset, yoffset) form a normalized coordinate to
 * look along.
 *
 * Requires (xoffset, yoffset) are one of the 8 valid direction vectors.
 *
 * Always returns a non-zero 'what' and 'where'
 *
 * 'where' is always 1 or more.
 * 'what' will atleat BARRIER, even in a completely empty sim.
 *
 */
static void look_along_line(UNIVERSE *u, CELL *c, int xoffset, int yoffset, int *what, int *where)
{
	UNIVERSE_GRID ugrid;
	GRID_TYPE gt;
	int x, y, dist;

	ASSERT( u != NULL );
	ASSERT( c != NULL );
	ASSERT( xoffset >= -1 && xoffset <= 1 );
	ASSERT( yoffset >= -1 && yoffset <= 1 );
	ASSERT( !(xoffset == 0 && yoffset == 0) );
	ASSERT( what != NULL );
	ASSERT( where != NULL );

	x = c->x;
	y = c->y;

	x = x + xoffset;
	y = y + yoffset;
	dist = 1;

	while( (x >= 0) && (x < u->width)
				&& (y >= 0) && (y < u->height) ) {

		gt = Grid_Get(u, x, y, &ugrid);

		if( gt != GT_BLANK ) {
			if( gt == GT_CELL ) {
				if( ugrid.u.cell->organism != c->organism ) {
					*what = VISION_TYPE_CELL;
					*where = dist;
					return;
				}

			} else if( gt == GT_SPORE ) {
				*what = VISION_TYPE_SPORE;
				*where = dist;
				return;

			} else if( gt == GT_ORGANIC ) {
				*what = VISION_TYPE_ORGANIC;
				*where = dist;
				return;

			} else if( gt == GT_BARRIER ) {
				*what = VISION_TYPE_BARRIER;
				*where = dist;
				return;

			} else if( gt == GT_PLAYER ) {
				*what = VISION_TYPE_CELL;
				*where = dist;
				return;

			} else {
				ASSERT(0);
			}
		}

		x = x + xoffset;
		y = y + yoffset;
		dist += 1;
	}

	/*
	 * We hit the hard barrier surrounding the universe.
	 */
	*what = VISION_TYPE_BARRIER;
	*where = dist;
}

/*
 * Create a spore in the grid at (x,y).
 */
static void create_spore(UNIVERSE *u, ORGANISM *o, int x, int y, int energy)
{
	SPORE *spore;
	KFORTH_PROGRAM *program;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	program = kforth_copy(o->program);

	spore = Spore_make(program, energy, o->id, o->strain);

	if( o->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
		spore->sflags |= SPORE_FLAG_RADIOACTIVE;
	}

	Grid_SetSpore(u, x, y, spore);
}


/*
 * Organism 'o' is wanting to eat anything
 * in grid location (x,y)
 *
 */
static int eat(UNIVERSE *u, ORGANISM *o, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;
	SPORE *spore;
	CELL *eatc;
	ORGANISM *eato;
	PLAYER *eatp;
	int energy, energy1, energy2;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	if( x < 0 || x >= u->width )
		return 0;

	if( y < 0 || y >= u->height )
		return 0;

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_ORGANIC ) {
		energy = ugrid.u.energy;
		o->energy += energy;
		Grid_Clear(u, x, y);
		return energy;

	} else if( type == GT_SPORE ) {
		spore = ugrid.u.spore;
		energy = spore->energy;
		o->energy += energy;
		Grid_Clear(u, x, y);
		Spore_delete(spore);
		return energy;

	} else if( type == GT_CELL  ) {
		/*
		 * Energy transfer between living organisms
		 *
		 * 1. Make sure cell isn't one of ours
		 * 2. Make sure cell isn't already dead
		 * 3. Set cell kfm to terminated (this kills the cell)
		 * 4. convert cell stack nodes to energy and add to us.
		 * 5. add 1/n amount of energy from eaten organism.
		 *
		 */
		eatc = ugrid.u.cell;
		eato = eatc->organism;

		if( eato == o ) {
			return 0;
		}

		if( eatc->kfm->terminated ) {
			return 0;
		}

		energy1 = (eatc->kfm->call_stack_size + eatc->kfm->data_stack_size) - NOCOST_STACK;
		if( energy1 < 0 ) {
			energy1 = 0;
		}

		kforth_machine_reset(eatc->kfm);
		eatc->kfm->terminated = 1;

		ASSERT( eato->ncells > 0 );

		energy2 = (eato->energy / eato->ncells)
			+ (eato->energy % eato->ncells);

		eato->energy -= energy2;

		energy = energy1 + energy2;

		o->energy += energy;

		return energy;

	} else if( type == GT_PLAYER ) {
		eatp = ugrid.u.player;
		ASSERT( ! eatp->terminated );

		energy = eatp->energy;
		o->energy += energy;

		eatp->terminated = 1;
		eatp->energy = 0;
		Grid_Clear(u, eatp->x, eatp->y);

		return energy;

	} else {
		return 0;
	}
}


/***********************************************************************
 * KFORTH NAME:		OMOVE
 * STACK BEHAVIOR:	(x y -- r)
 *
 * Move organism in the direction specified by (x, y).
 *
 * If the move is successful r=1, else r=0.
 *
 * If data stack doesn't have 2 elements, then do nothing.
 *
 * RULES:
 *	1. Every desintation location must be blank. or
 *	2. destination location is one of our own cells
 *
 */
void Opcode_OMOVE(KFORTH_MACHINE *kfm)
{
	ORGANISM *o;
	CELL *cell, *ccurr;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int x, y, xoffset, yoffset;
	int failed;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	/*
	 * OMOVE of (0,0) always fails, helps look logic
	 * to evolve.
	 */
	if( xoffset == 0 && yoffset == 0 ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	failed = 0;
	for(ccurr=o->cells; ccurr; ccurr=ccurr->next) {
		x = ccurr->x + xoffset;
		y = ccurr->y + yoffset;

		if( ! grid_can_moveto(u, o, x, y) ) {
			failed = 1;
			break;
		}
	}

	if( failed ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	for(ccurr=o->cells; ccurr; ccurr=ccurr->next) {
		Grid_Clear(u, ccurr->x, ccurr->y);
	}

	for(ccurr=o->cells; ccurr; ccurr=ccurr->next) {
		x = ccurr->x + xoffset;
		y = ccurr->y + yoffset;

		ccurr->x = x;
		ccurr->y = y;

		Grid_SetCell(u, ccurr);
	}

	kforth_data_stack_push(kfm, 1);

}

/***********************************************************************
 * Mark all cells reachable from 'cell' in the 
 * in the N, S, E, W direction, and set color field to 1.
 *
 * Returns number of cells reached (including 'cell')
 *
 * (recursive version, which is slooooow)
 */
static int mark_reachable_cells_recur(CELL *cell)
{
	CELL *north, *south, *east, *west;
	int cnt;

	ASSERT( cell != NULL );

	cnt = 1;
	cell->color = 1;

	north = Cell_Neighbor(cell, 0, -1);
	south = Cell_Neighbor(cell, 0, 1);
	east = Cell_Neighbor(cell, 1, 0);
	west = Cell_Neighbor(cell, -1, 0);
	
	if( north != NULL && north->color == 0 )
		cnt += mark_reachable_cells_recur(north);

	if( south != NULL && south->color == 0 )
		cnt += mark_reachable_cells_recur(south);

	if( east != NULL && east->color == 0 )
		cnt += mark_reachable_cells_recur(east);

	if( west != NULL && west->color == 0 )
		cnt += mark_reachable_cells_recur(west);

	return cnt;
}

/*
 * Fixed stack structure for use in mark_reachable_cells()
 * (approx. 1.2 MB of RAM)
 */

#define MRC_STACK_SIZE (EVOLVE_MAX_BOUNDS * 100)

static struct {
	short x;
	short y;
} mrc_stack[ MRC_STACK_SIZE ];

static int mrc_sp;

static void mrc_empty_stack(void)
{
	mrc_sp = 0;
}

static void mrc_push(int x, int y)
{
	ASSERT( mrc_sp < MRC_STACK_SIZE );

	mrc_stack[mrc_sp].x = (short)x;
	mrc_stack[mrc_sp].y = (short)y;

	mrc_sp++;
}

static bool mrc_pop(int *x, int *y)
{
	ASSERT( x != NULL );
	ASSERT( y != NULL );

	if( mrc_sp > 0 ) {
		mrc_sp--;
		*x = mrc_stack[mrc_sp].x;
		*y = mrc_stack[mrc_sp].y;
		return true;
	} else {
		return false;
	}
}

/*
 * Return the cell located at (x,y) if it is part of organism 'o',
 * and color field is 0, otherwise return NULL.
 */
static CELL *mrc_get_cell(UNIVERSE *u, ORGANISM *o, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;

	ASSERT( u != NULL );
	ASSERT( o != NULL );

	if( x < 0 || x >= u->width )
		return NULL;

	if( y < 0 || y >= u->height )
		return NULL;

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_CELL ) {
		if( ugrid.u.cell->organism == o )
			if( ugrid.u.cell->color == 0 )
				return ugrid.u.cell;
	}

	return NULL;
}


/***********************************************************************
 * Visit each cell reachable in the N, S, E, W direction from 'cell'
 * and set color field to 1.
 *
 * Assumes that the color field has already been cleared prior
 * to calling this routine.
 *
 * On return, every cell reachable from 'cell' in a N, S, E, W direction will
 * have its color field set to 1.
 *
 * Returns the number of cells that were reached (including 'cell')
 *
 * (this is a non-recursive implementation, and is 1/7-th
 *	the running time of mark_reachable_cells_recur() )
 *
 */
static int mark_reachable_cells(CELL *cell)
{
	UNIVERSE *u;
	ORGANISM *o;
	CELL *c;
	int x, y, y1;
	int cnt;
	bool span_left, span_right;

	ASSERT( cell != NULL );

	o = cell->organism;
	u = o->universe;

	x = cell->x;
	y = cell->y;

	cnt = 0;

	mrc_empty_stack();
    
	mrc_push(x, y);
    
	while( mrc_pop(&x, &y) ) {
		y1 = y;

		while( mrc_get_cell(u, o, x, y1) ) {
			y1--;
		}

		y1++;

		span_left = false;
		span_right = false;

		while( c = mrc_get_cell(u, o, x, y1) ) {

			ASSERT( c != NULL && c->organism == o );

			c->color = 1;
			cnt += 1;

			if( !span_left && mrc_get_cell(u, o, x-1, y1) ) {
				mrc_push(x-1, y1);
				span_left = true;

			} else if( span_left && !mrc_get_cell(u, o, x-1, y1) ) {
				span_left = false;
			}

			if( !span_right && mrc_get_cell(u, o, x+1, y1) ) {
				mrc_push(x+1, y1);
				span_right = true;

			} else if( span_right && !mrc_get_cell(u, o, x+1, y1) ) {
				span_right = false;
			}
			y1++;
		}
	}

	return cnt;

}

/***********************************************************************
 * KFORTH NAME:		CMOVE
 * STACK BEHAVIOR:	(x y -- r)
 *
 * Move cell relative to organism. (x, y) specifies
 * a direction to move relative to current location.
 *
 * If successful r=1, else r=0.
 *
 * RULES:
 *	1. The destination location must be vacant
 *	2. After the move, the all cells must be connected in the N, S, E, or W direction
 *	   (a diagonal connection is not enough)
 *
 * Uses a scan line fill algorithm to analyse the organism after the cmove
 * to determine if its properly connected.
 */
void Opcode_CMOVE(KFORTH_MACHINE *kfm)
{
	CELL *cell, *c;
	ORGANISM *o;
	UNIVERSE *u;
	int x, y, xoffset, yoffset;
	int save_x, save_y;
	KFORTH_INTEGER value;
	int cnt;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	x = cell->x + xoffset;
	y = cell->y + yoffset;

	if( ! grid_is_blank(u, x, y) ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	/*
	 * Move cell to new location
	 */
	save_x = cell->x;
	save_y = cell->y;

	Grid_Clear(u, cell->x, cell->y);

	cell->x = x;
	cell->y = y;
	Grid_SetCell(u, cell);

	/*
	 * Make sure all cells in organism have at least 1 neighbor in the
	 * N, S, E, or W direction
	 */
	for(c=o->cells; c; c=c->next) {
		c->color = 0;
	}

	//cnt = mark_reachable_cells_recur(o->cells);
	cnt = mark_reachable_cells(o->cells);

	if( cnt != o->ncells ) {
		/*
		 * connectivity requirement is not true, so undo the move,
		 * and push 0 on data stack.
		 */
		Grid_Clear(u, cell->x, cell->y);
		cell->x = save_x;
		cell->y = save_y;
		Grid_SetCell(u, cell);
		kforth_data_stack_push(kfm, 0);
	} else {
		/* success! */
		kforth_data_stack_push(kfm, 1);
	}
}

/*
 * Compute new (x, y) coordinate for the input 'x' and 'y'
 * 
 * The rotation amount is given by 'n' (which is a number from -3 to 3).
 *
 * The origin (x, y) is given by 'origin_x' and 'origin_y'.
 *
 */
static void rotate(int n, int origin_x, int origin_y, int x, int y, int *newx, int *newy)
{
	int xoffset, yoffset;
	int new_xoffset, new_yoffset;

	ASSERT( newx != NULL );
	ASSERT( newy != NULL );

	xoffset = x - origin_x;
	yoffset = y - origin_y;

	switch(n) {
	case 0:
		new_xoffset = xoffset;
		new_yoffset = yoffset;
		break;

	case  1:
	case -3:
		new_xoffset = yoffset * -1;
		new_yoffset = xoffset *  1;
		break;

	case  2:
	case -2:
		new_xoffset = xoffset * -1;
		new_yoffset = yoffset * -1;
		break;

	case  3:
	case -1:
		new_xoffset = yoffset *  1;
		new_yoffset = xoffset * -1;
		break;

	default:
		ASSERT(0);
		break;
	}

	*newx = origin_x + new_xoffset;
	*newy = origin_y + new_yoffset;
}

/***********************************************************************
 * KFORTH NAME:		ROTATE
 * STACK BEHAVIOR:	( n -- r )
 *
 * Rotate organism.
 *
 *
 */
void Opcode_ROTATE(KFORTH_MACHINE *kfm)
{
	CELL *cell, *c;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int newx, newy;
	int failed;
	int n;

	if( kfm->data_stack_size < 1 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	value = value % 4;
	n = (int) value;

	if( n == 0 ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	/*
	 * translate all cells to new location, and
	 * make sure they are vacant.
	 */
	failed = 0;
	for(c=o->cells; c; c=c->next) {

		rotate(n, cell->x, cell->y, c->x, c->y, &newx, &newy);

		if( ! grid_can_moveto(u, o, newx, newy)  ) {
			failed = 1;
			break;
		}

	}

	if( failed ) {
		kforth_data_stack_push(kfm, 1);
		return;		
	}

	/*
 	 * Clear all cells at current location
	 */
	for(c=o->cells; c; c=c->next) {
		Grid_Clear(u, c->x, c->y);
	}

	/*
	 * The rotate operation can be performed, so
	 * repeat loop above, moving each cell.
	 */
	for(c=o->cells; c; c=c->next) {

		rotate(n, cell->x, cell->y, c->x, c->y, &newx, &newy);

		c->x = newx;
		c->y = newy;

		Grid_SetCell(u, c);
	}

	kforth_data_stack_push(kfm, 1);

}

/***********************************************************************
 * KFORTH NAME:		EAT
 * STACK BEHAVIOR:	( x y -- e )
 *
 * Attempt to eat something at the normalized (x,y) offset from this cell.
 * This will eat:
 *	* Spores
 *	* Organic material
 *	* Other Organisms
 *
 * 'e' will equal the amount of energy we acquired.
 *
 * If nothing was eaten, e=0.
 *
 */
void Opcode_EAT(KFORTH_MACHINE *kfm)
{
	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	int x, y, xoffset, yoffset, energy;
	KFORTH_INTEGER value;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	x = cell->x;
	y = cell->y;

	energy = eat(u, o, x+xoffset, y+yoffset);

	kforth_data_stack_push(kfm, energy);
	
}

/***********************************************************************
 * (NEW VERSION 4.7 -- 2nd spore creation can have 0 energy)
 *
 * KFORTH NAME:		MAKE-SPORE
 * STACK BEHAVIOR:	(x y e -- s)
 *
 * Create spore xoffset, yoffset relative to
 * this cell. The spore will be created if enough
 * energy exists.
 *
 * If a spore is already there, then we fertilize the
 * spore an create a new organism.
 *
 * Rules:
 *	1. (x,y) offset location must be vacant or another spore.
 *	2. Our energy must be equal to or more than 'e'
 *	3. New spore will possses 'e' energy
 *	4. We will lose 'e' energy.
 *	5. Must be 3 elements on the data stack
 *	6. If (x,y) contains a spore, we fertilize it and create
 *	   a new organism.
 *
 *	7. s=0 if we failed to make a spore. s=1 is we created a spore. s=-1 if
 *	   we fertilized an existing spore.
 *	8. e must be 1 or more.
 *	9. If a spore exists, it must be a spore matching our strain. If not then,
 *		s=0.
 *
 * This version allows the 2nd spore (the one that fertilizes the 1st spore) to
 * have 0 energy. This will encourage sexual reproduction.
 *
 *
 */
void Opcode_MAKE_SPORE(KFORTH_MACHINE *kfm)
{
	ORGANISM *o;
	CELL *cell;
	UNIVERSE *u;
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;
	KFORTH_INTEGER value;
	int x, y, xoffset, yoffset, energy;

	if( kfm->data_stack_size < 3 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	energy = (int) value;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	if( energy < 0 || energy > o->energy ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	x = cell->x + xoffset;
	y = cell->y + yoffset;

	if( x < 0 || x >= u->width ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	if( y < 0 || y >= u->height ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_BLANK ) {
		/*
		 * 1st spore, energy must be non-zero.
		 */
		if( energy == 0 ) {
			kforth_data_stack_push(kfm, 0);
			return;
		}
		o->energy -= energy;
		create_spore(u, o, x, y, energy);
		kforth_data_stack_push(kfm, 1);

	} else if( type == GT_SPORE && ugrid.u.spore->strain == o->strain ) {
		if( o->oflags & ORGANISM_FLAG_SEXONLY ) {
			if( ugrid.u.spore->parent != o->id ) {
				/*
				 * 2nd spore, energy can be zero.
				 *
				 * The organism has been flagged as
				 * "SEXONLY" meaning it can only
				 * fertilize spores that
				 * were created from other organisms.
				 *
				 */
				o->energy -= energy;
				Spore_fertilize(u, o, ugrid.u.spore, x, y, energy);
				kforth_data_stack_push(kfm, -1);
			} else {
				/*
				 * A SEXONLY organism cannot fertilize
				 * its own spore.
				 */
				kforth_data_stack_push(kfm, 0);
			}
		} else {
			/*
			 * 2nd spore, energy can be zero.
			 */
			o->energy -= energy;
			Spore_fertilize(u, o, ugrid.u.spore, x, y, energy);
			kforth_data_stack_push(kfm, -1);
		}

	} else {
		kforth_data_stack_push(kfm, 0);
	}

}

#ifdef GROW_USING_CB

/***********************************************************************
 * KFORTH NAME:		GROW
 * STACK BEHAVIOR:	(x y cb -- )
 *
 * Organism will grow by 1 cell. The new cell will be
 * placed at the normalized relative offset (x,y) from
 * the cell executing this instruction.
 *
 * Rules:
 *	1. the normalized location (x,y) must be vacant.
 *	2. The new cell inherits everything. The only difference is
 *	   its 's' on the stack will be -1.
 *	3. s=0 if grow failed. s=1 for the parent. S=-1 for the new cell.
 *	4. Organism must have enough energy to create new data/call stacks
 *	5. Must be connected to organism in N, S, E, W direction.
 *
 * 's' is set as follows:
 *	0	- failure to grow, parent cell (no new cell).
 *	1	- success, parent cell
 *	-1	- success, new cell
 *
 * The new cell is added to the FRONT of the linked list of cells
 * attached to the organism.
 *
 */
void Opcode_GROW(KFORTH_MACHINE *kfm)
{
	ORGANISM *o;
	CELL *cell, *ncell, *c;
	UNIVERSE *u;
	KFORTH_INTEGER value, cb;
	int x, y, xoffset, yoffset;
	int energy_needed, count;

	if( kfm->data_stack_size < 3 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	cb = kforth_data_stack_pop(kfm);

#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
	/* relative addressing mode */
	cb += kfm->cb;
#endif

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	x = cell->x + xoffset;
	y = cell->y + yoffset;

	if( ! grid_is_blank(u, x, y) ) {
		return;
	}

	energy_needed = (kfm->data_stack_size + kfm->call_stack_size) - NOCOST_STACK;
	if( energy_needed < 0 ) {
		energy_needed = 0;
	}

	if( o->energy < energy_needed ) {
		return;		
	}

	/*
	 * Make sure new cell location is touching organism
	 * in a North, South, East, West way
	 */
	count = 0;
	c = grid_has_our_cell(u, o, x+1,  y+0);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x-1,  y+0);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x+0,  y+1);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x+0, y-1);
	if( c != NULL )
		count++;

	if( count == 0 ) {
		return;
	}

	/*
	 * Create new cell:
	 */
	ncell = (CELL *) CALLOC(1, sizeof(CELL));
	ASSERT( ncell != NULL );

	*ncell = *cell;

	ncell->kfm	= kforth_machine_copy(cell->kfm);
	ncell->kfm->pc	+= 1;
	ncell->kfm->client_data = ncell;
	ncell->x	= x;
	ncell->y	= y;

	ncell->next = o->cells;
	o->cells = ncell;

	o->ncells	+= 1;
	o->energy	-= energy_needed;

	Grid_SetCell(u, ncell);

	if( cb >= 0 && cb < kfm->program->nblocks ) {
		ncell->kfm->cb = (int)cb;
		ncell->kfm->pc = 0;
	}

}
#else
/***********************************************************************
 * KFORTH NAME:		GROW
 * STACK BEHAVIOR:	(x y -- s)
 *
 * Organism will grow by 1 cell. The new cell will be
 * placed at the normalized relative offset (x,y) from
 * the cell executing this instruction.
 *
 * Rules:
 *	1. the normalized location (x,y) must be vacant.
 *	2. The new cell inherits everything. The only difference is
 *	   its 's' on the stack will be -1.
 *	3. s=0 if grow failed. s=1 for the parent. S=-1 for the new cell.
 *	4. Organism must have enough energy to create new data/call stacks
 *	5. Must be connected to organism in N, S, E, W direction.
 *
 * 's' is set as follows:
 *	0	- failure to grow, parent cell (no new cell).
 *	1	- success, parent cell
 *	-1	- success, new cell
 *
 * The new cell is added to the FRONT of the linked list of cells
 * attached to the organism.
 *
 */
void Opcode_GROW(KFORTH_MACHINE *kfm)
{
	ORGANISM *o;
	CELL *cell, *ncell, *c;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int x, y, xoffset, yoffset;
	int energy_needed, count;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	x = cell->x + xoffset;
	y = cell->y + yoffset;

	if( ! grid_is_blank(u, x, y) ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	/*
	 * We add 1, because the new cell will also
	 * have a '-1' on its data stack.
	 */
	energy_needed = (kfm->data_stack_size + kfm->call_stack_size + 1) - NOCOST_STACK;
	if( energy_needed < 0 ) {
		energy_needed = 0;
	}

	if( o->energy < energy_needed ) {
		kforth_data_stack_push(kfm, 0);
		return;		
	}

	/*
	 * Make sure new cell location is touching organism
	 * in a North, South, East, West way
	 */
	count = 0;
	c = grid_has_our_cell(u, o, x+1,  y+0);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x-1,  y+0);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x+0,  y+1);
	if( c != NULL )
		count++;

	c = grid_has_our_cell(u, o, x+0, y-1);
	if( c != NULL )
		count++;

	if( count == 0 ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	/*
	 * Create new cell:
	 */

	ncell = (CELL *) CALLOC(1, sizeof(CELL));
	ASSERT( ncell != NULL );

	*ncell = *cell;

	ncell->kfm	= kforth_machine_copy(cell->kfm);
	ncell->kfm->pc	+= 1;
	ncell->kfm->client_data = ncell;
	ncell->x	= x;
	ncell->y	= y;

	ncell->next = o->cells;
	o->cells = ncell;

	o->ncells	+= 1;
	o->energy	-= energy_needed;

	Grid_SetCell(u, ncell);

	/*
	 * Push 1 on Parent cell's data stack
	 */
	kforth_data_stack_push(kfm, 1);

	/*
	 * Push -1 on New cell's data stack
	 */
	kforth_data_stack_push(ncell->kfm, -1);
}
#endif

/***********************************************************************
 * KFORTH NAME:		LOOK
 * STACK BEHAVIOR:	(x y -- what where)
 *
 * Look along (x, y) and return a 'what' value and a 'where' value.
 *
 */
void Opcode_LOOK(KFORTH_MACHINE *kfm)
{
	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int xoffset, yoffset;
	int what, where;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	if( yoffset == 0 && xoffset == 0 ) {
		kforth_data_stack_push(kfm, 0);
		kforth_data_stack_push(kfm, 0);
		return;
	}

	look_along_line(u, cell, xoffset, yoffset, &what, &where);

	kforth_data_stack_push(kfm, what);
	kforth_data_stack_push(kfm, where);

}

/***********************************************************************
 * KFORTH NAME:		WHAT
 * STACK BEHAVIOR:	(x y -- type)
 *
 * Returns the 'type' of thing that we seen in direction (x, y). The
 * coordinates are normalized. If coordinates are (0, 0), then
 * a type of 0 is returned.
 *
 * Type is as follows:
 *
 *	0 - nothing
 *	1 - cell
 *	2 - spore
 *	4 - organic
 *	8 - barrier
 *
 */
void Opcode_WHAT(KFORTH_MACHINE *kfm)
{
	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int xoffset, yoffset;
	int what, where;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	if( yoffset == 0 && xoffset == 0 ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	look_along_line(u, cell, xoffset, yoffset, &what, &where);

	kforth_data_stack_push(kfm, what);
}

/***********************************************************************
 * KFORTH NAME:		WHERE
 * STACK BEHAVIOR:	(x y -- dist)
 *
 * Returns the distance value associated with the vision data for the
 * direction (x, y). The coordinates are normalized.
 *
 * A coordinate of (0,0) always returns 0.
 *
 */
void Opcode_WHERE(KFORTH_MACHINE *kfm)
{
	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int xoffset, yoffset;
	int what, where;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = NORMALIZE_OFFSET(value);

	value = kforth_data_stack_pop(kfm);
	xoffset = NORMALIZE_OFFSET(value);

	if( yoffset == 0 && xoffset == 0 ) {
		kforth_data_stack_push(kfm, 0);
		return;
	}

	look_along_line(u, cell, xoffset, yoffset, &what, &where);

	kforth_data_stack_push(kfm, where);
}

/*
 * Pick random number between 'a' and 'b' (includes 'a' and 'b')
 */
#define CHOOSE(er,a,b)	( (sim_random(er) % ((b)-(a)+1) ) + (a) )

/***********************************************************************
 *
 * This routine implements the logic for these instructions:
 *
 *	NEAREST		has_dist = false,  want_nearest = true
 *	FARTHEST	has_dist = false,  want_nearest = false
 *	NEAREST2	has_dist = true,   want_nearest = true
 *	FARTHEST2	has_dist = true,   want_nearest = false
 *
 */
static void generic_vision_search(KFORTH_MACHINE *kfm, bool has_dist, bool want_nearest)
{
	static const int xoffset[8] = {  0,  1,  1,  1,  0, -1, -1, -1 };
	static const int yoffset[8] = { -1, -1,  0,  1,  1,  1,  0, -1 };

	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;
	int i, mask, dist, dir;
	int best_where, best_dir;
	int what, where;
	bool found;

	ASSERT( kfm != NULL );

	if( has_dist ) {
		if( kfm->data_stack_size < 2 )
			return;

		value = kforth_data_stack_pop(kfm);
		dist = (int) value;

		value = kforth_data_stack_pop(kfm);
		mask = (int) (value & VISION_MASK);

	} else {
		if( kfm->data_stack_size < 1 )
			return;

		dist = EVOLVE_MAX_BOUNDS+1;

		value = kforth_data_stack_pop(kfm);
		mask = (int) (value & VISION_MASK);
	}

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	if( mask == 0 || dist <= 0 ) {
		kforth_data_stack_push(kfm, 0);
		kforth_data_stack_push(kfm, 0);
		return;
	}

	if( want_nearest ) {
		best_where = EVOLVE_MAX_BOUNDS + 1000;
	} else {
		best_where = -1;
	}

	found = false;

	/*
	 * Pick a random starting direction, then
	 * scan clock-wise.
	 */
	dir = CHOOSE(u->er, 0, 7);

	for(i=0; i<8; i++) {
		look_along_line(u, cell, xoffset[dir], yoffset[dir], &what, &where);

		ASSERT( where != 0 );
		ASSERT( what != 0 );

		if( (what & mask) && (where <= dist) ) {
			found = true;
			if( want_nearest ) {
				if( where < best_where ) {
					best_where = where;
					best_dir = dir;
				}
			} else {
				if( where > best_where ) {
					best_where = where;
					best_dir = dir;
				}
			}
		
		}
		dir += 1;
		if( dir > 7 )
			dir = 0;
	}

	if( found ) {
		kforth_data_stack_push(kfm, xoffset[best_dir]);
		kforth_data_stack_push(kfm, yoffset[best_dir]);
	} else {
		kforth_data_stack_push(kfm, 0);
		kforth_data_stack_push(kfm, 0);
	}

}

/***********************************************************************
 * KFORTH NAME:		NEAREST
 * STACK BEHAVIOR:	(mask -- x y)
 *
 * 	- Look infinitely in all 8 directions.
 *	- Return (x, y) direction that contains the smallest distance, and
 *	  matches 'mask'.
 *
 */
void Opcode_NEAREST(KFORTH_MACHINE *kfm)
{
	generic_vision_search(kfm, false, true);
}

/***********************************************************************
 * KFORTH NAME:		FARTHEST
 * STACK BEHAVIOR:	(mask -- x y)
 *
 * Look in all 8 directions and
 * find the (x, y) vision coordinate contains has
 * the farthest object matching the 'mask'.
 *
 */
void Opcode_FARTHEST(KFORTH_MACHINE *kfm)
{
	generic_vision_search(kfm, false, false);
}

/***********************************************************************
 * KFORTH NAME:		NEAREST2
 * STACK BEHAVIOR:	(mask dist -- x y)
 *
 * Find the (x, y) vision coordinate that cooresponds to the
 * vision data that contains the neareast object matching the 'mask'
 *
 * Use 'dist' to limit the radius of the search.
 *
 * If 'dist' is 0, 0r negative, then return (0,0).
 * If no 'mask' bits are set, return (0,0).
 *
 */
void Opcode_NEAREST2(KFORTH_MACHINE *kfm)
{
	generic_vision_search(kfm, true, true);	
}

/***********************************************************************
 * KFORTH NAME:		FARTHEST2
 * STACK BEHAVIOR:	(mask dist -- x y)
 *
 * Look in all 8 directions and
 * find the (x, y) vision coordinate contains has
 * the farthest object matching the 'mask'.
 *
 * Use 'dist' to limit the radius of the search.
 *
 * If 'dist' is 0, 0r negative, then return (0,0).
 * If no 'mask' bits are set, return (0,0).
 *
 */
void Opcode_FARTHEST2(KFORTH_MACHINE *kfm)
{
	generic_vision_search(kfm, true, false);
}


/***********************************************************************
 * KFORTH NAME:		MOOD
 * STACK BEHAVIOR:	(x y -- mood)
 *
 * Get mood of the cell that is located
 * at xoffset, yoffset relative to this cell
 *
 * mood will be set to the mood of the cell located at (x,y). If there
 * is no such cell, then mood=0.
 *
 */
void Opcode_MOOD(KFORTH_MACHINE *kfm)
{
	CELL *cell, *c;
	ORGANISM *o;
	UNIVERSE *u;
	int xoffset, yoffset;
	KFORTH_INTEGER value;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);
	yoffset = (int)value;

	value = kforth_data_stack_pop(kfm);
	xoffset = (int)value;

	c = Cell_Neighbor(cell, xoffset, yoffset);

	if( c != NULL ) {
		value = c->mood;
	} else {
		value = 0;
	}

	kforth_data_stack_push(kfm, value);
}

/***********************************************************************
 * KFORTH NAME:		MOOD!
 * STACK BEHAVIOR:	(m -- )
 *
 * Set our mood to 'm'.
 *
 */
void Opcode_SET_MOOD(KFORTH_MACHINE *kfm)
{
	CELL *cell;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;

	if( kfm->data_stack_size < 1 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);

	cell->mood = value;
}

/***********************************************************************
 * KFORTH NAME:		BROADCAST
 * STACK BEHAVIOR:	(m -- )
 *
 * Send message 'm' to every cell in the organism (including ourselves).
 *
 */
void Opcode_BROADCAST(KFORTH_MACHINE *kfm)
{
	CELL *cell, *ccurr;
	ORGANISM *o;
	UNIVERSE *u;
	KFORTH_INTEGER value;

	if( kfm->data_stack_size < 1 )
		return;

	cell = (CELL*) kfm->client_data;
	o = cell->organism;
	u = o->universe;

	value = kforth_data_stack_pop(kfm);

	for(ccurr=o->cells; ccurr; ccurr=ccurr->next) {
		ccurr->message = value;
	}
}

/***********************************************************************
 * KFORTH NAME:		SEND
 * STACK BEHAVIOR:	(m x y -- )
 *
 * 3 values are on the stack. X offset and Y offset
 * identify a cell relative to this one. Send the
 * message to that organism.
 *
 */
void Opcode_SEND(KFORTH_MACHINE *kfm)
{
	CELL *cell, *c;
	int xoffset, yoffset;
	KFORTH_INTEGER value, message;

	if( kfm->data_stack_size < 3 )
		return;

	cell = (CELL*) kfm->client_data;

	value = kforth_data_stack_pop(kfm);
	yoffset = (int)value;

	value = kforth_data_stack_pop(kfm);
	xoffset = (int)value;

	message = kforth_data_stack_pop(kfm);

	c = Cell_Neighbor(cell, xoffset, yoffset);
	if( c != NULL ) {
		c->message = message;
	}
}

/***********************************************************************
 * KFORTH NAME:		RECIEVE
 * STACK BEHAVIOR:	( -- m)
 *
 * Push contents of recieve buffer onto stack
 */
void Opcode_RECV(KFORTH_MACHINE *kfm)
{
	CELL *cell;

	cell = (CELL*) kfm->client_data;

	kforth_data_stack_push(kfm, cell->message);
}

/***********************************************************************
 * KFORTH NAME:		ENERGY
 * STACK BEHAVIOR:	( -- n)
 *
 * Push organism energy amount onto stack
 */
void Opcode_ENERGY(KFORTH_MACHINE *kfm)
{
	CELL *cell;

	cell = (CELL*) kfm->client_data;

	kforth_data_stack_push(kfm, cell->organism->energy);
}

/***********************************************************************
 * KFORTH NAME:		AGE
 * STACK BEHAVIOR:	( -- n)
 *
 * Push organism age onto stack
 */
void Opcode_AGE(KFORTH_MACHINE *kfm)
{
	CELL *cell;

	cell = (CELL*) kfm->client_data;

	kforth_data_stack_push(kfm, cell->organism->age);
}

/***********************************************************************
 * KFORTH NAME:		NUM-CELLS
 * STACK BEHAVIOR:	( -- n)
 *
 * Get number of cells in organism onto stack.
 *
 * Push organism 'ncells' onto stack.
 */
void Opcode_NUM_CELLS(KFORTH_MACHINE *kfm)
{
	CELL *cell;

	cell = (CELL*) kfm->client_data;

	kforth_data_stack_push(kfm, cell->organism->ncells);
}

/***********************************************************************
 * KFORTH NAME:		HAS-NEIGHBOR
 * STACK BEHAVIOR:	(x y -- s)
 *
 * Do we have a neighbor cell at x, y offsets?
 *
 * Offsets are NOT normalized.
 *
 */
void Opcode_HAS_NEIGHBOR(KFORTH_MACHINE *kfm)
{
	CELL *cell, *c;
	int xoffset, yoffset;
	KFORTH_INTEGER value;

	if( kfm->data_stack_size < 2 )
		return;

	cell = (CELL*) kfm->client_data;

	value = kforth_data_stack_pop(kfm);
	yoffset = (int)value;

	value = kforth_data_stack_pop(kfm);
	xoffset = (int)value;

	c = Cell_Neighbor(cell, xoffset, yoffset);
	if( c != NULL ) {
		kforth_data_stack_push(kfm, 1);
	} else {
		kforth_data_stack_push(kfm, 0);
	}
}

/* ********************************************************************** */

/*
 * Get a neighboring cell for 'cell'. Return
 * NULL if cell has no neighbor.
 *
 */
CELL *Cell_Neighbor(CELL *cell, int xoffset, int yoffset)
{
	UNIVERSE_GRID grid;
	UNIVERSE *u;
	int x, y;
	GRID_TYPE type;

	ASSERT( cell != NULL );

	u = cell->organism->universe;

	x = cell->x + xoffset;
	y = cell->y + yoffset;

	if( x < 0 || x >= u->width )
		return NULL;

	if( y < 0 || y >= u->height )
		return NULL;

	type = Grid_Get(u, x, y, &grid);
	if( type == GT_CELL ) {
		if( grid.u.cell->organism == cell->organism ) {
			return grid.u.cell;
		}
	}

	return NULL;
}


/***********************************************************************
 * Simulate Cell
 *
 * We execute the KFORTH engine 1 execution step.
 * The change in stack size, will alter the amount of
 * energy we have.
 *
 * If a cell terminates, it dies.
 *
 * We compute the change in the stack usage of executing
 * the cell's KFORTH_MACHINE 1 step. This amount of stack
 * usage will decrease the amount of energy the organism
 * has. If we use more stack space than energy in the organism,
 * then we kill the cell and restore the cells stack usage BEFORE
 * to the organisms energy.
 *
 */
void Cell_Simulate(CELL *c)
{
	ORGANISM *o;
	int before, after, usage;

	ASSERT( c != NULL );
	ASSERT( ! c->kfm->terminated );

	o = c->organism;

	before = (c->kfm->call_stack_size + c->kfm->data_stack_size) - NOCOST_STACK;
	if( before < 0 ) {
		before = 0;
	}

	kforth_machine_execute(c->kfm);

	after = (c->kfm->call_stack_size + c->kfm->data_stack_size) - NOCOST_STACK;
	if( after < 0 ) {
		after = 0;
	}

	usage = (after - before);

	if( usage <= o->energy ) {
		o->energy -= usage;
	} else {
		kforth_machine_reset(c->kfm);
		c->kfm->terminated = 1;
		o->energy += before;
	}

}

void Cell_delete(CELL *c)
{
	ASSERT( c != NULL );

	kforth_machine_delete(c->kfm);
	FREE(c);
}

/***********************************************************************
 *
 * The GUI wil want to perform a virtual "look" operation to display
 * the current visual information for a cell.
 *
 * This looks in all 8 direction surrounding cell 'c' and
 * stored the 'what' and 'where' values in 'cvd'.
 *
 */
void Universe_CellVisionData(CELL *cell, CELL_VISION_DATA *cvd)
{
	ORGANISM *o;
	UNIVERSE *u;

	ASSERT( cell != NULL );
	ASSERT( cvd != NULL );

	o = cell->organism;
	u = o->universe;

	look_along_line(u, cell,  0, -1, &cvd->what[0], &cvd->where[0]);
	look_along_line(u, cell,  1, -1, &cvd->what[1], &cvd->where[1]);
	look_along_line(u, cell,  1,  0, &cvd->what[2], &cvd->where[2]);
	look_along_line(u, cell,  1,  1, &cvd->what[3], &cvd->where[3]);
	look_along_line(u, cell,  0,  1, &cvd->what[4], &cvd->where[4]);
	look_along_line(u, cell, -1,  1, &cvd->what[5], &cvd->where[5]);
	look_along_line(u, cell, -1,  0, &cvd->what[6], &cvd->where[6]);
	look_along_line(u, cell, -1, -1, &cvd->what[7], &cvd->where[7]);
}
