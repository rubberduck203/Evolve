#ifndef _EVOLVE_SIMULATOR_H
#define _EVOLVE_SIMULATOR_H

/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */

/***********************************************************************
 * evolve_simulator:
 *
 * Interface for the evolve simulator. Clients wanting to
 * create a simulation, and simulate it, include this file.
 *
 */

/***********************************************************************
 * KFORTH COMMON
 */
#define KFORTH_OPS_LEN	250	/* maximum number of instructions supported */

typedef struct kforth_operations KFORTH_OPERATIONS;

typedef __int64 KFORTH_INTEGER;


/***********************************************************************
 * KFORTH PROGRAM
 */
typedef struct {
	int			nblocks;
	int			*block_len;
	unsigned char		**opcode;
	KFORTH_INTEGER		**operand;
	KFORTH_OPERATIONS	*kfops;
} KFORTH_PROGRAM;

/***********************************************************************
 * KFORTH DISASSEMBLY
 */
typedef struct {
	int	cb;
	int	pc;		/* -1 means label definition */
	int	start_pos;
	int	end_pos;
} KFORTH_DISASSEMBLY_POS;

typedef struct {
	char			*program_text;
	int			pos_len;
	KFORTH_DISASSEMBLY_POS	*pos;
} KFORTH_DISASSEMBLY;


/***********************************************************************
 * KFORTH MACHINE
 */
typedef struct {
	int	cb;
	int	pc;
} KFORTH_CALL_STACK_NODE;

typedef struct {
	KFORTH_INTEGER	value;
} KFORTH_DATA_STACK_NODE;

typedef struct kforth_stack_node {
	union {
		KFORTH_CALL_STACK_NODE	call;
		KFORTH_DATA_STACK_NODE	data;
	} u;
	struct kforth_stack_node *next;
	struct kforth_stack_node *prev;
} KFORTH_STACK_NODE;

typedef struct {
	int			terminated;
	int			call_stack_size;
	int			data_stack_size;
	int			cb;
	int			pc;
	KFORTH_INTEGER		R[10];		/* R0 - R9 */
	KFORTH_STACK_NODE	*call_sp;
	KFORTH_STACK_NODE	*data_sp;
	KFORTH_PROGRAM		*program;
	void			*client_data;
} KFORTH_MACHINE;


/***********************************************************************
 * KFORTH OPERATIONS
 */
typedef void (*KFORTH_FUNCTION)(KFORTH_MACHINE *kfm);

typedef struct kforth_operations {
	char			*name;
	KFORTH_FUNCTION		func;
} KFORTH_OPERATIONS;


/***********************************************************************
 * KFORTH MUTATION OPTIONS
 */
typedef struct {
	int	prob_mutate_codeblock;
	int	prob_duplicate;
	int	prob_delete;
	int	prob_insert;
	int	prob_transpose;
	int	prob_modify;
	int	max_opcodes;
	int	max_code_blocks;
	int	max_apply;
} KFORTH_MUTATE_OPTIONS;

#define PROBABILITY_SCALE	10000
#define MUTATE_MAX_APPLY_LIMIT	10		/* upper limit for the max_apply setting */

/***********************************************************************
 * RANDOM NUMBER GENERATOR
 */
#define	EVOLVE_DEG4	63
#define	EVOLVE_SEP4	1

typedef struct {
	int fidx;			/* front index */
	int ridx;			/* rear index */
	long state[ EVOLVE_DEG4 ];
} EVOLVE_RANDOM;


#define NOCOST_STACK	50			/* "no cost" stack elements */

typedef __int64			LONG_LONG;
typedef struct universe		UNIVERSE;
typedef struct organism		ORGANISM;
typedef struct cell		CELL;
typedef struct player		PLAYER;

/***********************************************************************
 * CELL
 *
 */
struct cell{
	char		color;		/* used by kill_dead_cells() algorithm */

	KFORTH_INTEGER	mood;
	KFORTH_INTEGER	message;
	KFORTH_MACHINE	*kfm;

	int		x;
	int		y;

	CELL		*next;
	ORGANISM	*organism;	/* pointer to my organism */
};

/***********************************************************************
 * ORGANISM
 *
 */
struct organism {
	LONG_LONG	id;		/* unique-id for this organism */
	LONG_LONG	parent1;	/* unique-id for this organism's parent */
	LONG_LONG	parent2;	/* unique-id for this organism's parent */
	int		generation;	/* generation from parent zero */
	int		energy;		/* amount of energy the organism has */
	int		age;		/* how old this organism is */
	int		strain;		/* what strain does this organism belong to? */
	int		oflags;		/* various organism flags */

	KFORTH_PROGRAM	*program;

	int		ncells;		/* number of cells */
	CELL		*cells;		/* linked list of cells in the organism */

	ORGANISM	*next;
	UNIVERSE	*universe;	/* what universe do I belong to? */
};

#define ORGANISM_FLAG_RADIOACTIVE	0x00000001		/* radioactive dye marker */
#define ORGANISM_FLAG_SEXONLY		0x00000002		/* organism only reproduces sexually */

/***********************************************************************
 * SPORE
 */
typedef struct spore {
	int		energy;
	int		strain;		/* strain of organism that created me */
	int		sflags;		/* various spore flags */
	LONG_LONG	parent;		/* parent-id that created me */
	KFORTH_PROGRAM *program;	/* program from parent */
} SPORE;

#define SPORE_FLAG_RADIOACTIVE		0x00000001		/* radioactive dye marker */

/***********************************************************************
 * GRID
 */
#define EVOLVE_MAX_BOUNDS	3000
#define EVOLVE_MIN_BOUNDS	100
#define EVOLVE_MAX_STRAINS	8

typedef enum {
	GT_BLANK,
	GT_BARRIER,
	GT_ORGANIC,
	GT_CELL,
	GT_SPORE,
	GT_PLAYER,
} GRID_TYPE;

typedef struct universe_grid {
	short type;			/* GRID_TYPE */
	union {
		int	energy;
		CELL	*cell;
		SPORE	*spore;
		PLAYER	*player;
	} u;
} UNIVERSE_GRID;

/***********************************************************************
 * KEYLIST: A handy data structure for clients of the UNIVERSE object
 * to associated key/value pairs with the simulation.
 */
#define KEYLIST_HASH_TABLE_SIZE		1000
typedef struct keylist_item {
	char		*key;
	char		*value;
	struct keylist_item *next;
} KEYLIST_ITEM;

typedef struct {
	KEYLIST_ITEM **table;
} KEYLIST;

/***********************************************************************
 * PLAYER
 */
typedef enum {
	PCMD_NOP,
	PCMD_OMOVE,
	PCMD_EAT,
	PCMD_EAT_ALL,
} PCMD_TYPE;

typedef struct {
	PCMD_TYPE	type;
	int		x;
	int		y;
} PLAYER_COMMAND;

struct player {
	int		terminated;		/* killed? */
	int		x;
	int		y;
	int		energy;
	int		age;
	PLAYER_COMMAND	cmd;
	UNIVERSE	*universe;
};

/***********************************************************************
 * UNIVERSE
 */
struct universe {
	unsigned long		seed;		/* random seed -- initially used */
	LONG_LONG		step;		/* how many steps */
	LONG_LONG		next_id;	/* next organism id to assign */
	int			norganism;	/* # of organisms in universe */
	LONG_LONG		nborn;		/* # of organisms born in this simulation */
	LONG_LONG		ndie;		/* # of organisms died in this simulation */

	EVOLVE_RANDOM		*er;		/* random number generator state */
	KFORTH_MUTATE_OPTIONS	*kfmo;		/* mutation probabilities */

	ORGANISM	*organisms;
	ORGANISM	*new_organisms;
	ORGANISM	*selected_organism;

	PLAYER		*player;

	int		width;
	int		height;
	UNIVERSE_GRID	*grid;

	KEYLIST		*keylist;		/* allow client associate key/value pairs with this sim */
};

typedef struct {
	int	energy;
	int	num_cells;
	int	num_instructions;
	int	call_stack_nodes;
	int	data_stack_nodes;
	int	num_organic;
	int	num_spores;
	int	num_sexual;	// # of organisms that have 2 parents

	int	spore_energy;
	int	organic_energy;

	int	grid_memory;
	int	cstack_memory;
	int	dstack_memory;
	int	program_memory;
	int	organism_memory;
	int	spore_memory;

	int	strain_population[EVOLVE_MAX_STRAINS];
	int	radioactive_population[EVOLVE_MAX_STRAINS];

} UNIVERSE_INFORMATION;

/*
 * 8 directions map as follows:
 *
 *	index	dir
 *	------	--------
 *	  0	( 0, -1)
 *	  1	( 1, -1)
 *	  2	( 1,  0)
 *	  3	( 1,  1)
 *	  4	( 0,  1)
 *	  5	(-1,  1)
 *	  6	(-1,  0)
 *	  7	(-1, -1)
 *
 */
typedef struct {
	int	what[8];
	int	where[8];
} CELL_VISION_DATA;

/***********************************************************************
 * PROTOTYPES:
 */

/*
 * kforth_mutate.cpp
 */
extern KFORTH_MUTATE_OPTIONS *kforth_mutate_options_make(
				int max_code_blocks,
				int max_apply,
				double prob_mutate_codeblock,
				double prob_duplicate,
				double prob_delete,
				double prob_insert,
				double prob_transpose,
				double prob_modify,
				int max_opcodes );

extern KFORTH_MUTATE_OPTIONS *kforth_mutate_options_copy(KFORTH_MUTATE_OPTIONS *kfmo);

extern void		kforth_mutate_options_defaults(KFORTH_MUTATE_OPTIONS *kfmo);

extern void		kforth_mutate_options_delete(KFORTH_MUTATE_OPTIONS *kfmo);

extern void		kforth_mutate(KFORTH_PROGRAM *kfp,
					KFORTH_MUTATE_OPTIONS *kfmo,
					EVOLVE_RANDOM *er);

extern KFORTH_PROGRAM	*kforth_merge(KFORTH_PROGRAM *kfp1, KFORTH_PROGRAM *kfp2);

extern KFORTH_PROGRAM	*kforth_merge_rnd(EVOLVE_RANDOM *er,
				KFORTH_PROGRAM *kfp1, KFORTH_PROGRAM *kfp2);

extern KFORTH_PROGRAM	*kforth_copy(KFORTH_PROGRAM *kfp);


/*
 * kforth_execute.cpp
 */
extern KFORTH_MACHINE	*kforth_machine_make(KFORTH_PROGRAM *kfp, void *client_data);
extern void		kforth_machine_delete(KFORTH_MACHINE *kfm);
extern KFORTH_MACHINE	*kforth_machine_copy(KFORTH_MACHINE *kfm);
extern void		kforth_machine_execute(KFORTH_MACHINE *kfm);
extern void		kforth_machine_reset(KFORTH_MACHINE *kfm);

extern KFORTH_OPERATIONS *kforth_ops_make(void);
extern void		kforth_ops_delete(KFORTH_OPERATIONS *kfops);
extern void		kforth_ops_add(KFORTH_OPERATIONS *kfops,
						char *name, KFORTH_FUNCTION func);
extern int		kforth_ops_max_opcode(KFORTH_OPERATIONS *kfops);

extern KFORTH_INTEGER	kforth_data_stack_pop(KFORTH_MACHINE *kfm);
extern void		kforth_data_stack_push(KFORTH_MACHINE *kfm, KFORTH_INTEGER value);
extern void		kforth_call_stack_push(KFORTH_MACHINE *kfm, int cb, int pc);

/*
 * kforth_compiler.cpp
 */
extern KFORTH_PROGRAM	*kforth_compile(const char *program_text,
					KFORTH_OPERATIONS *kfops, char *errbuf);
extern void		kforth_delete(KFORTH_PROGRAM *kfp);
extern int		kforth_program_length(KFORTH_PROGRAM *kfp);
extern int		kforth_program_size(KFORTH_PROGRAM *kfp);

extern KFORTH_DISASSEMBLY *kforth_disassembly_make(KFORTH_PROGRAM *kfp, int width, int want_cr);
extern void		kforth_disassembly_delete(KFORTH_DISASSEMBLY *kfd);

/*
 * cell.cpp
 */
extern void	Opcode_OMOVE(KFORTH_MACHINE *kfm);
extern void	Opcode_CMOVE(KFORTH_MACHINE *kfm);
extern void	Opcode_ROTATE(KFORTH_MACHINE *kfm);
extern void	Opcode_EAT(KFORTH_MACHINE *kfm);
extern void	Opcode_MAKE_SPORE(KFORTH_MACHINE *kfm);
extern void	Opcode_GROW(KFORTH_MACHINE *kfm);
extern void	Opcode_LOOK(KFORTH_MACHINE *kfm);
extern void	Opcode_WHAT(KFORTH_MACHINE *kfm);
extern void	Opcode_WHERE(KFORTH_MACHINE *kfm);
extern void	Opcode_NEAREST(KFORTH_MACHINE *kfm);
extern void	Opcode_FARTHEST(KFORTH_MACHINE *kfm);
extern void	Opcode_NEAREST2(KFORTH_MACHINE *kfm);
extern void	Opcode_FARTHEST2(KFORTH_MACHINE *kfm);
extern void	Opcode_MOOD(KFORTH_MACHINE *kfm);
extern void	Opcode_SET_MOOD(KFORTH_MACHINE *kfm);
extern void	Opcode_BROADCAST(KFORTH_MACHINE *kfm);
extern void	Opcode_SEND(KFORTH_MACHINE *kfm);
extern void	Opcode_RECV(KFORTH_MACHINE *kfm);
extern void	Opcode_ENERGY(KFORTH_MACHINE *kfm);
extern void	Opcode_AGE(KFORTH_MACHINE *kfm);
extern void	Opcode_NUM_CELLS(KFORTH_MACHINE *kfm);
extern void	Opcode_HAS_NEIGHBOR(KFORTH_MACHINE *kfm);
extern CELL	*Cell_Neighbor(CELL *cell, int xoffset, int yoffset);
extern void	Cell_Simulate(CELL *c);
extern void	Cell_delete(CELL *c);
extern void	Universe_CellVisionData(CELL *cell, CELL_VISION_DATA *cvd);

/*
 * organism.cpp
 */
extern int		Organism_Simulate(ORGANISM *o);
extern ORGANISM		*Organism_Make(int x, int y, int strain, int energy,
						const char *program_text, char *errbuf);
extern void		Organism_delete(ORGANISM *o);


/*
 * spore.cpp
 */
extern SPORE	*Spore_make(KFORTH_PROGRAM *program, int energy, LONG_LONG parent, int strain);
extern void	Spore_delete(SPORE *spore);
extern void	Spore_fertilize(UNIVERSE *u, ORGANISM *o, SPORE *spore,
					int x, int y, int energy);

/*
 * player.cpp
 */
extern void	Player_Simulate(PLAYER *p);
extern void	Player_Add(UNIVERSE *u, int x, int y);
extern void	Player_Remove(UNIVERSE *u);
extern PLAYER	*Player_Get(UNIVERSE *u);
extern void	Player_SetNextCommand(UNIVERSE *u, PLAYER_COMMAND *cmd);
extern PLAYER	*Player_make(UNIVERSE *u, int x, int y);
extern void	Player_delete(PLAYER *p);

/*
 * universe.cpp
 */
extern UNIVERSE		*Universe_Make(unsigned long seed, int width, int height, KFORTH_MUTATE_OPTIONS *kfmo);
extern void		Universe_Delete(UNIVERSE *u);
extern void		Universe_Simulate(UNIVERSE *u);
extern void		Universe_Information(UNIVERSE *u, UNIVERSE_INFORMATION *uinfo);


extern void		Universe_SetBarrier(UNIVERSE *u, int x, int y);
extern void		Universe_ClearBarrier(UNIVERSE *u, int x, int y);
extern GRID_TYPE	Universe_Query(UNIVERSE *u, int x, int y, UNIVERSE_GRID *ugrid);

extern void		Universe_SelectOrganism(UNIVERSE *u, ORGANISM *o);
extern void		Universe_ClearSelectedOrganism(UNIVERSE *u);
extern ORGANISM		*Universe_GetSelection(UNIVERSE *u);
extern ORGANISM		*Universe_DuplicateOrganism(ORGANISM *osrc);
extern ORGANISM		*Universe_CopyOrganism(UNIVERSE *u);
extern ORGANISM		*Universe_CutOrganism(UNIVERSE *u);
extern void		Universe_PasteOrganism(UNIVERSE *u, ORGANISM *o);
extern void		Universe_FreeOrganism(ORGANISM *o);

extern void		Universe_ClearTracers(UNIVERSE *u);
extern void		Universe_SetSporeTracer(SPORE *spore);
extern void		Universe_SetOrganismTracer(ORGANISM *organism);
extern void		Universe_ClearSporeTracer(SPORE *spore);
extern void		Universe_ClearOrganismTracer(ORGANISM *organism);

extern void		Universe_SetAttribute(UNIVERSE *u, const char *name, char *value);
extern void		Universe_GetAttribute(UNIVERSE *u, const char *name, char *value);

extern GRID_TYPE	Grid_GetPtr(UNIVERSE *u, int x, int y, UNIVERSE_GRID **ugrid);
extern GRID_TYPE	Grid_Get(UNIVERSE *u, int x, int y, UNIVERSE_GRID *ugrid);
extern void		Grid_Clear(UNIVERSE *u, int x, int y);
extern void		Grid_SetBarrier(UNIVERSE *u, int x, int y);
extern void		Grid_SetCell(UNIVERSE *u, CELL *cell);
extern void		Grid_SetOrganic(UNIVERSE *u, int x, int y, int energy);
extern void		Grid_SetSpore(UNIVERSE *u, int x, int y, SPORE *spore);
extern void		Grid_SetPlayer(UNIVERSE *u, int x, int y, PLAYER *player);

extern KFORTH_OPERATIONS *EvolveOperations(void);
extern int		EvolveMaxOpcodes(void);


/*
 * keylist.cpp
 */
extern KEYLIST		*KEYLIST_Make(void);
extern void		KEYLIST_Delete(KEYLIST *klist);
extern int		KEYLIST_Read(FILE *fp, KEYLIST *klist, char *errbuf);
extern int		KEYLIST_Write(FILE *fp, KEYLIST *klist, char *errbuf);
extern void		KEYLIST_Set(KEYLIST *klist, const char *key, const char *value);
extern void		KEYLIST_Get(KEYLIST *klist, const char *key, char *value);

/*
 * evolve_io.cpp
 */
extern UNIVERSE		*Universe_Read(const char *filename, char *errbuf);
extern int		Universe_Write(UNIVERSE *u, const char *filename, char *errbuf);


#endif
