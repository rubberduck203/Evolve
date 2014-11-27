/*
 * Copyright (c) 2007 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2007 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * PLAYER OPERATIONS
 *
 * These operations are used to implement "game mode" in which the
 * user controls an organism inside of the simulation.
 *
 * INVARIANT:
 * The total energy never changes for a simulation. That is
 * adding and removing the player entity does not change the total
 * energy in the simulation.
 *
 * The 'Player' entity is a user controllable cell which can be
 * placed anywhere on a vacant grid location.
 *
 * It can be seen and eaten by other organisms. We keep track of its age
 * and energy.
 */

#include "stdafx.h"

#define NORMALIZE_OFFSET(v)	(  (v<0) ? -1 : ((v>0) ? 1 : 0)  )

/*
 * Player eats whatever is at grid location (x, y)
 * Return total amount of energy eaten.
 *
 * Implements the same algorithm as eat() from cell.c
 *
 */
static int player_eat(UNIVERSE *u, PLAYER *p, int x, int y)
{
	GRID_TYPE type;
	UNIVERSE_GRID ugrid;
	SPORE *spore;
	CELL *eatc;
	ORGANISM *eato;
	int energy, energy1, energy2;

	ASSERT( u != NULL );
	ASSERT( p != NULL );

	if( x < 0 || x >= u->width )
		return 0;

	if( y < 0 || y >= u->height )
		return 0;

	type = Grid_Get(u, x, y, &ugrid);

	if( type == GT_ORGANIC ) {
		energy = ugrid.u.energy;
		p->energy += energy;
		Grid_Clear(u, x, y);
		return energy;

	} else if( type == GT_SPORE ) {
		spore = ugrid.u.spore;
		energy = spore->energy;
		p->energy += energy;
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

		p->energy += energy;

		return energy;

	} else if( type == GT_PLAYER ) {
		ASSERT(0);
		return 0;

	} else {
		return 0;
	}
}


/***********************************************************************
 * Simulate the player object, by executing the PLAYER_COMMAND.
 *
 * If no player exists in the simulation, then return.
 *
 * The player basically has 2 things they can do. EAT and MOVE.
 * In both cases they must specify and (x, y) vector. The GUI
 * interface will be responsible for translating user input into
 * logical commands.
 *
 */
void Player_Simulate(PLAYER *p)
{
	UNIVERSE *u;
	UNIVERSE_GRID ugrid;
	GRID_TYPE type;
	int x, y, xoffset, yoffset;

	ASSERT( p != NULL );
	ASSERT( ! p->terminated );

	p->age += 1;

	u = p->universe;

	switch( p->cmd.type ) {
	case PCMD_NOP:
		/*
		 * no operation:
		 */
		break;

	case PCMD_OMOVE:
		/*
		 * Move player object to the new location, specified
		 * by the vector (pc->x, pc->y). If blocked, don't move.
		 *
		 * Perhaps implement "sliding behavior"????
		 */
		xoffset = NORMALIZE_OFFSET( p->cmd.x );
		yoffset = NORMALIZE_OFFSET( p->cmd.y );

		x = p->x + xoffset;
		y = p->y + yoffset;

		if( x < 0 || x >= u->width )
			break;

		if( y < 0 || y >= u->height )
			break;

		type = Grid_Get(u, x, y, &ugrid);
		if( type != GT_BLANK )
			break;

		Grid_Clear(u, p->x, p->y);

		p->x = x;
		p->y = y;
		Grid_SetPlayer(u, x, y, p);
		break;

	case PCMD_EAT:
		/*
		 * directional "eat" instruction
		 */
		xoffset = NORMALIZE_OFFSET( p->cmd.x );
		yoffset = NORMALIZE_OFFSET( p->cmd.y );

		if( xoffset != 0 || yoffset != 0 ) {
			// vector cannot be (0, 0)
			player_eat(u, p, p->x + xoffset, p->y + yoffset);
		}
		break;

	case PCMD_EAT_ALL:
		/*
		 * execute "eat" instruction for all cells surrounding
		 * us.
		 */
		player_eat(u, p, p->x+1, p->y+1 );
		player_eat(u, p, p->x+1, p->y+0 );
		player_eat(u, p, p->x+1, p->y-1 );

		player_eat(u, p, p->x-1, p->y+1 );
		player_eat(u, p, p->x-1, p->y );
		player_eat(u, p, p->x-1, p->y-1 );

		player_eat(u, p, p->x, p->y+1 );
		player_eat(u, p, p->x, p->y-1 );
		break;

	default:
		ASSERT(0);
	}

	/*
	 * Reset command to be no operation.
	 */
	p->cmd.type = PCMD_NOP;
	p->cmd.x = 0;
	p->cmd.y = 0;
}

/*
 * Create a player object with default values set.
 */
PLAYER *Player_make(UNIVERSE *u, int x, int y)
{
	PLAYER *player;

	ASSERT( u  != NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );

	player = (PLAYER *) CALLOC(1, sizeof(PLAYER));
	ASSERT( player != NULL );

	player->terminated = 0;
	player->x = x;
	player->y = y;
	player->energy = 0;
	player->age = 0;
	player->cmd.type = PCMD_NOP;
	player->cmd.x = 0;
	player->cmd.y = 0;
	player->universe = u;

	return player;
}

/*
 * Delete a player object.
 */
void Player_delete(PLAYER *p)
{
	ASSERT( p != NULL );
	FREE(p);
}

/*
 * Add player to universe. Assumes player does not already exist.
 *
 * x and y are the location
 *
 * Player will be given 0 units of energy.
 *
 * The next player command will be the NOOP.
 *
 * (x, y) must be valid coordinates and not contain anything else.
 *
 */
void Player_Add(UNIVERSE *u, int x, int y)
{
#ifndef NDEBUG
	UNIVERSE_GRID ugrid;
#endif
	PLAYER *player;

	ASSERT( u != NULL );
	ASSERT( u->player == NULL );
	ASSERT( x >= 0 && x < u->width );
	ASSERT( y >= 0 && y < u->height );
	ASSERT( Grid_Get(u, x, y, &ugrid) == GT_BLANK );

	player = Player_make(u, x, y);
	u->player = player;
	Grid_SetPlayer(u, x, y, player);
}


/*
 * Remove player from universe simulation object. All energy for player
 * is converted to organic energy in the form
 * of a white square at the current location of
 * the player.
 *
 */
void Player_Remove(UNIVERSE *u)
{
	PLAYER *player;

	ASSERT( u != NULL );
	ASSERT( u->player != NULL );
	ASSERT( u->player->terminated == 0 );

	player = u->player;
	u->player = NULL;

	if( player->energy > 0 ) {
		Grid_SetOrganic(u, player->x, player->y, player->energy);
	} else {
		Grid_Clear(u, player->x, player->y);
	}
	Player_delete(player);
}

/*
 * Return player object.
 * Returns NULL is no player is set.
 */
PLAYER *Player_Get(UNIVERSE *u)
{
	ASSERT( u != NULL );

	return u->player;
}

/*
 * Set the next command to be executed by the player object.
 *
 * When Universe_Simulate(u) is called the player will execute this
 * command first, before any other organisms.
 *
 */
void Player_SetNextCommand(UNIVERSE *u, PLAYER_COMMAND *cmd)
{
	ASSERT( u != NULL );
	ASSERT( cmd != NULL );
	ASSERT( u->player != NULL );

	u->player->cmd = *cmd;
}

