/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * MUTATE/MERGE KFORTH PROGRAMS
 *
 *
 */
#include "stdafx.h"

/*
 * Instruction-level mutations will use XLEN as the maximum
 * instruction segment to manuipulate. (shorter code blocks
 * will use segments lengths smaller than XLEN)
 *
 */
#define XLEN 4

/*
 * return a random number between a and b. (including a and b)
 * (a and b must be integers)
 *
 * ASSERT( a <= b )
 */
#define CHOOSE(er,a,b)	( (sim_random(er) % ((b)-(a)+1) ) + (a) )

static void choose_instruction(EVOLVE_RANDOM *er, KFORTH_MUTATE_OPTIONS *kfmo,
					unsigned char *opcode, KFORTH_INTEGER *operand)
{
	long x;

	ASSERT( er != NULL );
	ASSERT( opcode != NULL );
	ASSERT( operand != NULL );

	x = CHOOSE(er, 0, PROBABILITY_SCALE);
	if( x < PROBABILITY_SCALE/2 ) {
		/*
		 * 1/2 of the time we chose a value
		 */
		*opcode		= 0;
		*operand	= CHOOSE(er, -99, 99);
	} else {
		/*
		 * 1/2 of the time we chose an instruction
		 */
		*opcode		= (unsigned char) CHOOSE(er, 1, kfmo->max_opcodes);
		*operand	= 0;
	}
}

/*
 * modify an existing instruction.
 *
 *	- modify the instuction (cb, pc)
 *	- (cb, pc) must be a valid instruction
 *	- pick a random number between -XLEN and XLEN. If 0, swap sign, otherwise add it to the
 *	  number.
 *	- if not a number, change instruction to something different (but not to a number)
 *
 * This keeps numbers as numbers and instructions as instructions.
 */
static void modify_single_instruction(EVOLVE_RANDOM *er, KFORTH_MUTATE_OPTIONS *kfmo,
				KFORTH_PROGRAM *kfp, int cb, int pc)
{
	int delta;

	ASSERT( er != NULL );
	ASSERT( kfmo != NULL );
	ASSERT( kfp != NULL );
	ASSERT( cb >= 0 && cb < kfp->nblocks );
	ASSERT( pc >= 0 && pc < kfp->block_len[cb] );

	if( kfp->opcode[cb][pc] == 0 ) {
		delta = CHOOSE(er, -XLEN, XLEN);
		if( delta == 0 ) {
			/*
			 * swap sign
		 	 */
			kfp->operand[cb][pc] = -kfp->operand[cb][pc];

		} else {
			/*
			 * Increment or Decrement by 1 - XLEN
			 */
			kfp->operand[cb][pc] += delta;
		}
	} else {
		kfp->opcode[cb][pc] = (unsigned char) CHOOSE(er, 1, kfmo->max_opcodes);
	}

}

/*
 * Pick random code block insert it into a random spot.
 */
static void duplicate_code_block(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, nblocks, pc, i, len;
	KFORTH_INTEGER *operand;
	unsigned char *opcode;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	nblocks = kfp->nblocks;

	if( nblocks == 0 )
		return;

	if( nblocks >= kfmo->max_code_blocks )
		return;

	/*
	 * Pick a code block to duplicate (and remember it)
	 */
	cb	= CHOOSE(er, 0, nblocks-1);
	len	= kfp->block_len[cb];
	opcode	= kfp->opcode[cb];
	operand	= kfp->operand[cb];

	/*
	 * Grow program to fit 1 more code block
	 */
	kfp->block_len = (int*)
			REALLOC(kfp->block_len, (nblocks+1) * sizeof(int));

	kfp->opcode = (unsigned char**)
			REALLOC(kfp->opcode, (nblocks+1) * sizeof(unsigned char*));

	kfp->operand = (KFORTH_INTEGER**)
			REALLOC(kfp->operand, (nblocks+1) * sizeof(KFORTH_INTEGER*));

	kfp->nblocks = nblocks+1;

	/*
	 * Shift all code blocks at 'cb'
	 */
	cb = CHOOSE(er, 0, nblocks);
	if( cb < nblocks ) {
		for(i=nblocks; i>cb; i--) {
			kfp->block_len[i]	= kfp->block_len[i-1];
			kfp->opcode[i]		= kfp->opcode[i-1];
			kfp->operand[i]		= kfp->operand[i-1];
		}
	}

	kfp->block_len[cb] = len;
	kfp->opcode[cb] = (unsigned char*) CALLOC(len, sizeof(unsigned char));
	kfp->operand[cb] = (KFORTH_INTEGER*) CALLOC(len, sizeof(KFORTH_INTEGER));

	for(pc=0; pc<len; pc++) {
		kfp->opcode[cb][pc] = opcode[pc];
		kfp->operand[cb][pc] = operand[pc];
	}
}

/*
 * Pick a random sequence of 1 - XLEN instructions and insert it at a random spot
 */
static void duplicate_instruction(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc, block_len, len, i;
	unsigned char opcode[XLEN];
	KFORTH_INTEGER operand[XLEN];

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb = CHOOSE(er, 0, kfp->nblocks-1);

	block_len = kfp->block_len[cb];

	if( block_len == 0 )
		return;

	len = CHOOSE(er, 1, ((block_len < XLEN) ? block_len : XLEN) );

	/*
	 * pick random instructions to duplicate
	 */
	pc = CHOOSE(er, 0, block_len-len);

	for(i=0; i<len; i++) {
		opcode[i] = kfp->opcode[cb][pc+i];
		operand[i] = kfp->operand[cb][pc+i];
	}

	/*
	 * Grow program to fit len more instructions in 'cb'
	 */
	kfp->opcode[cb] = (unsigned char*)
			REALLOC(kfp->opcode[cb], (block_len+len) * sizeof(unsigned char));

	kfp->operand[cb] = (KFORTH_INTEGER*)
			REALLOC(kfp->operand[cb], (block_len+len) * sizeof(KFORTH_INTEGER));

	kfp->block_len[cb] = block_len + len;

	/*
	 * make gap at 'pc' for len instructions
	 */
	pc = CHOOSE(er, 0, block_len);
	if( pc < block_len ) {
		for(i=block_len+len-1; i > pc+len-1; i--) {
			kfp->opcode[cb][i] = kfp->opcode[cb][i-len];
			kfp->operand[cb][i] = kfp->operand[cb][i-len];
		}
	}

	/*
	 * Insert copy of instruction
	 */
	for(i=0; i<len; i++){
		kfp->opcode[cb][pc+i] = opcode[i];
		kfp->operand[cb][pc+i] = operand[i];
	}

}

/*
 * Remove an entire code block
 */
static void delete_code_block(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, nblocks, i;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	/*
	 * Delete entire code block and move subsequent code blocks up.
	 */
	nblocks = kfp->nblocks;

	/*
	 * do not do this mutation if program only has 1 code block
	 */
	if( nblocks <= 1 )
		return;

	cb = CHOOSE(er, 0, nblocks-1);

	FREE( kfp->opcode[cb] );
	FREE( kfp->operand[cb] );

	for(i=cb; i<nblocks-1; i++) {
		kfp->block_len[i]	= kfp->block_len[i+1];
		kfp->opcode[i]		= kfp->opcode[i+1];
		kfp->operand[i]		= kfp->operand[i+1];
	}

	kfp->nblocks = nblocks-1;

}

/*
 * Delete 1 - XLEN instructions from a random place in a random code block
 */
static void delete_instruction(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc, block_len, len, i;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb = CHOOSE(er, 0, kfp->nblocks-1);

	block_len = kfp->block_len[cb];

	if( block_len == 0 )
		return;

	len = CHOOSE(er, 1, ((block_len < XLEN) ? block_len : XLEN) );

	pc = CHOOSE(er, 0, block_len - len);

	for(i=pc; i < block_len - len; i++) {
		kfp->opcode[cb][i] = kfp->opcode[cb][i+len];
		kfp->operand[cb][i] = kfp->operand[cb][i+len];
	}

	kfp->block_len[cb] = block_len - len;
}

/*
 * Insert a code block into program. New code block
 * will contain random instructions. Length of new code block
 * will be randomly between 0 and XLEN.
 */
static void insert_code_block(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc, nblocks, i, len;
	unsigned char opcode;
	KFORTH_INTEGER operand;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	/*
	 * Grow program to fit 1 more code block
	 */
	nblocks = kfp->nblocks;

	if( nblocks >= kfmo->max_code_blocks )
		return;

	kfp->block_len = (int*)
			REALLOC(kfp->block_len, (nblocks+1) * sizeof(int));

	kfp->opcode = (unsigned char**)
			REALLOC(kfp->opcode, (nblocks+1) * sizeof(unsigned char*));

	kfp->operand = (KFORTH_INTEGER**)
			REALLOC(kfp->operand, (nblocks+1) * sizeof(KFORTH_INTEGER*));

	kfp->nblocks = nblocks+1;

	if( nblocks == 0 ) {
		cb = 0;
	} else {
		cb = CHOOSE(er, 0, nblocks);
		if( cb < nblocks ) {
			for(i=nblocks; i>cb; i--) {
				kfp->block_len[i]	= kfp->block_len[i-1];
				kfp->opcode[i]		= kfp->opcode[i-1];
				kfp->operand[i]		= kfp->operand[i-1];
			}
		}
	}

	/*
	 * create code block at 'cb'
	 * Random length between 0 and XLEN.
	 */
	len = CHOOSE(er, 0, XLEN);

	kfp->block_len[cb]	= len;
	kfp->opcode[cb]		= (unsigned char*) CALLOC(len, sizeof(unsigned char));
	kfp->operand[cb]	= (KFORTH_INTEGER*) CALLOC(len, sizeof(KFORTH_INTEGER));

	for(pc=0; pc < len; pc++) {
		choose_instruction(er, kfmo, &opcode, &operand);
		kfp->opcode[cb][pc] = opcode;
		kfp->operand[cb][pc] = operand;
	}

}

/*
 * Pick random code block and insert some random instructions 1 - XLEN
 * into code block at a random spot.
 */
static void insert_instruction(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc, block_len, len, i;
	unsigned char opcode[XLEN];
	KFORTH_INTEGER operand[XLEN];

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb = CHOOSE(er, 0, kfp->nblocks-1);

	block_len = kfp->block_len[cb];

	/*
	 * Pick 1 to XLEN random instructions.
	 */
	len = CHOOSE(er, 1, XLEN);
	for(i=0; i<len; i++) {
		choose_instruction(er, kfmo, &opcode[i], &operand[i]);
	}

	/*
	 * Grow program to fit len more instructions in 'cb'
	 */
	kfp->opcode[cb] = (unsigned char*)
			REALLOC(kfp->opcode[cb], (block_len+len) * sizeof(unsigned char));

	kfp->operand[cb] = (KFORTH_INTEGER*)
			REALLOC(kfp->operand[cb], (block_len+len) * sizeof(KFORTH_INTEGER));

	kfp->block_len[cb] = block_len + len;

	/*
	 * make gap at 'pc' for len instructions
	 */
	pc = CHOOSE(er, 0, block_len);
	if( pc < block_len ) {
		for(i=block_len+len-1; i > pc+len-1; i--) {
			kfp->opcode[cb][i] = kfp->opcode[cb][i-len];
			kfp->operand[cb][i] = kfp->operand[cb][i-len];
		}
	}

	for(i=0; i < len; i++) {
		kfp->opcode[cb][pc+i] = opcode[i];
		kfp->operand[cb][pc+i] = operand[i];
	}
}

/*
 * Swap 2 randomly chosen code blocks
 */
static void transpose_code_block(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb1, cb2;
	unsigned char *save_opcode;
	KFORTH_INTEGER *save_operand;
	int save_len;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb1 = CHOOSE(er, 0, kfp->nblocks-1);
	cb2 = CHOOSE(er, 0, kfp->nblocks-1);

	if( cb1 == cb2 )
		return;

	save_opcode		= kfp->opcode[cb1];
	save_operand		= kfp->operand[cb1];
	save_len		= kfp->block_len[cb1];

	kfp->opcode[cb1]	= kfp->opcode[cb2];
	kfp->operand[cb1]	= kfp->operand[cb2];
	kfp->block_len[cb1]	= kfp->block_len[cb2];

	kfp->opcode[cb2]	= save_opcode;
	kfp->operand[cb2]	= save_operand;
	kfp->block_len[cb2]	= save_len;
}

/*
 * Pick 2 random instruction segments (1 to XLEN long) and swap them.
 */
static void transpose_instruction(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb1, pc1;
	int cb2, pc2;
	int block_len1, block_len2;
	int len, i;

	unsigned char save_opcode[XLEN];
	KFORTH_INTEGER save_operand[XLEN];

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb1 = CHOOSE(er, 0, kfp->nblocks-1);
	block_len1 = kfp->block_len[cb1];

	if( block_len1 == 0 )
		return;

	cb2 = CHOOSE(er, 0, kfp->nblocks-1);
	block_len2 = kfp->block_len[cb2];

	if( block_len2 == 0 )
		return;

	/*
	 * compute, len = MIN(XLEN, block_len1, block_len2)
	 */
	if( block_len1 < block_len2 ) {
		len = CHOOSE(er, 1, (block_len1 < XLEN) ? block_len1 : XLEN);
	} else {
		len = CHOOSE(er, 1, (block_len2 < XLEN) ? block_len2 : XLEN);
	}

	pc1 = CHOOSE(er, 0, block_len1-len);
	pc2 = CHOOSE(er, 0, block_len2-len);

	for(i=0; i<len; i++) {
		save_opcode[i] = kfp->opcode[cb1][pc1+i];
		save_operand[i] = kfp->operand[cb1][pc1+i];
	}

	for(i=0; i<len; i++) {
		kfp->opcode[cb1][pc1+i] = kfp->opcode[cb2][pc2+i];
		kfp->operand[cb1][pc1+i] = kfp->operand[cb2][pc2+i];
	}

	for(i=0; i<len; i++) {
		kfp->opcode[cb2][pc2+i] = save_opcode[i];
		kfp->operand[cb2][pc2+i] = save_operand[i];
	}

}

/*
 * Pick a random code block and modify all instructions in it
 * (all numbers are increased/decreased a tiny amount, instructions unchanged)
 */
static void modify_code_block(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb = CHOOSE(er, 0, kfp->nblocks-1);

	if( kfp->block_len[cb] == 0 )
		return;

	for(pc=0; pc < kfp->block_len[cb]; pc++) {
		if( kfp->opcode[cb][pc] == 0 ) {
			modify_single_instruction(er, kfmo, kfp, cb, pc);
		}
	}

}

/*
 * Pick a random starting instruction and modify 1 - XLEN sequential instructions.
 */
static void modify_instruction(KFORTH_PROGRAM *kfp, KFORTH_MUTATE_OPTIONS *kfmo, EVOLVE_RANDOM *er)
{
	int cb, pc, block_len, len, i;

	ASSERT( kfp != NULL );
	ASSERT( er != NULL );

	cb = CHOOSE(er, 0, kfp->nblocks-1);

	block_len = kfp->block_len[cb];

	if( block_len == 0 )
		return;

	len = CHOOSE(er, 1, ((block_len < XLEN) ? block_len : XLEN) );

	pc = CHOOSE(er, 0, block_len - len);

	for(i=0; i<len; i++) {
		modify_single_instruction(er, kfmo, kfp, cb, pc+i);
	}
}


/***********************************************************************
 * Using the mutation options 'kfmo' and
 * the random number generator 'er' make a tiny
 * random change to kfp.
 *
 * The first step is to determine if we will act at the instruction-level or
 * the codeblock-level. Once this determiniation is made, we check to
 * see if any of the 5 kinds of mutations will be applied (it may turn
 * out they we perfom NO mutations at all).
 *
 */
void kforth_mutate(KFORTH_PROGRAM *kfp,
			KFORTH_MUTATE_OPTIONS *kfmo,
			EVOLVE_RANDOM *er)
{
	long x;
	int mutate_code_block;
	int napply, i;

	ASSERT( kfp != NULL );
	ASSERT( kfmo != NULL );
	ASSERT( er != NULL );
	ASSERT( kfp->nblocks > 0 );

	if( kfmo->max_apply == 1 ) {
		napply = 1;

	} else if( kfmo->max_apply == 0 ) {
		return;

	} else {
		napply = CHOOSE(er, 1, kfmo->max_apply);
	}

	for(i=0; i < napply; i++) {
		/*
		 * Mutate at codeblock-level or instruction-level?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_mutate_codeblock )
			mutate_code_block = 1;
		else
			mutate_code_block = 0;

		/*
		 * Do Duplication?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_duplicate ) {
			if( mutate_code_block )
				duplicate_code_block(kfp, kfmo, er);
			else
				duplicate_instruction(kfp, kfmo, er);
		}

		/*
		 * Do Deletion?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_delete ) {
			if( mutate_code_block )
				delete_code_block(kfp, kfmo, er);
			else
				delete_instruction(kfp, kfmo, er);
		}

		/*
		 * Do Insertion?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_insert ) {
			if( mutate_code_block )
				insert_code_block(kfp, kfmo, er);
			else
				insert_instruction(kfp, kfmo, er);
		}

		/*
		 * Do Transposition?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_transpose ) {
			if( mutate_code_block )
				transpose_code_block(kfp, kfmo, er);
			else
				transpose_instruction(kfp, kfmo, er);
		}

		/*
		 * Do Modification?
		 */
		x = CHOOSE(er, 0, PROBABILITY_SCALE);
		if( x < kfmo->prob_modify ) {
			if( mutate_code_block )
				modify_code_block(kfp, kfmo, er);
			else
				modify_instruction(kfp, kfmo, er);
		}

	}

#if 0
	/* EXPERIMENTAL IDEA, NOT RELEASED TO PUBLIC YET */
	/*
	 * remove any empty code blocks at the end of the program.
	 * Incomming program (before mutations) shouldn't have any empty
	 * trailing code blocks. This will truncate just those
	 * that were added by the mutation operations.
	 *
	 */
	{
		int cb;

		cb = kfp->nblocks-1;
		while( cb >= 1 && kfp->block_len[cb] == 0 ) {
			FREE(kfp->opcode[cb]);
			FREE(kfp->operand[cb]);
			cb--;
		}
		kfp->nblocks = cb+1;
	}
	
#endif
	

}

/***********************************************************************
 *
 * Merge two kforth program. Use a 'mask' parameter to decide
 * which program (kfp1 or kfp2) donates a code block.
 *
 * The only the 16 lowest order bits in 'mask' are used.
 *
 * Bit 0 determines the first code block
 * bit 1 determine the second code block,
 * etc..
 *
 * If the 16-bits are exhausted, the mask is repeated.
 *
 * A bit value of 0, means that the code block from kfp1 is donated.
 * A bit value of 1, means that the code block from kfp2 is donated.
 *
 *
 * Example,
 *
 * If mask is 0000 0000 1011 0001
 *
 *	PROGRAM 1	PROGRAM 2	RESULT
 *	---------	---------	----------
 *	ABBA		BLAG		BLAG
 *	JUNK		CRAP		JUNK
 *	FOOBAR		GREPLINUX	FOOBAR
 *	STUFF		XXYYZZ		STUFF
 *	SUPERDUPER	FOOFOOFOO	FOOFOOFOO
 *	FOO12h		FOO666		FOO666
 *			HHIIGG		HHIIGG		<- donated from kfp2
 *			HHUUPP		HHUUPP
 *			JJJJJJ		JJJJJJ
 *
 * Create a new program from 'kfp1' and 'kfp2'. 
 * Even numbered code blocks are donated by PROGRAM 1.
 * Odd numbered code blocks are donated by PROGRAM 2.
 *
 * If programs have different number of code blocks, then
 * extra code blocks from the longer program are appended
 * to the new program.
 *
 * NOTE: kforth_merge(m, x, x) is allowed, and will return a copy of 'x'
 *
 */
static KFORTH_PROGRAM *kforth_merge_with_mask(int mask, KFORTH_PROGRAM *kfp1, KFORTH_PROGRAM *kfp2)
{
	KFORTH_PROGRAM *p, *kfp;
	int cb, pc, len;
	int bit, curmask;

	ASSERT( mask >= 0 && mask <= 0xFFFF );
	ASSERT( kfp1 != NULL );
	ASSERT( kfp2 != NULL );
	ASSERT( kfp1->kfops == kfp2->kfops );

	kfp = (KFORTH_PROGRAM*) CALLOC(1, sizeof(KFORTH_PROGRAM));

	kfp->nblocks = (kfp1->nblocks > kfp2->nblocks) ?
				kfp1->nblocks : kfp2->nblocks;

	kfp->block_len = (int*) CALLOC(kfp->nblocks, sizeof(int));
	kfp->opcode = (unsigned char**) CALLOC(kfp->nblocks, sizeof(unsigned char*));
	kfp->operand = (KFORTH_INTEGER**) CALLOC(kfp->nblocks, sizeof(KFORTH_INTEGER));
	kfp->kfops = kfp1->kfops;

	bit = 0;
	curmask = mask;
	for(cb=0; cb < kfp->nblocks; cb++) {
		
		if( (curmask & 0x0001) == 0 ) {
			if( cb < kfp1->nblocks )
				p = kfp1;
			else
				p = kfp2;
		} else {
			if( cb < kfp2->nblocks )
				p = kfp2;
			else
				p = kfp1;
		}

		len = p->block_len[cb];
		kfp->block_len[cb] = len;
		kfp->opcode[cb] = (unsigned char*) CALLOC(len, sizeof(unsigned char));
		kfp->operand[cb] = (KFORTH_INTEGER*) CALLOC(len, sizeof(KFORTH_INTEGER));

		for(pc=0; pc < len; pc++) {
			kfp->opcode[cb][pc] = p->opcode[cb][pc];
			kfp->operand[cb][pc] = p->operand[cb][pc];
		}

		curmask = curmask >> 1;
		bit += 1;
		if( bit >= 16 ) {
			curmask = mask;
		}
	}

	return kfp;
}

/***********************************************************************
 * Deterministic merge operation:
 *
 * (This was the OLD way of merging. A better way is to use
 * the random bit string)
 *
 * Merge using alternating code blocks. The mask used
 * is: 1010 1010 1010 1010
 *
 *	PROGRAM 1	PROGRAM 2	RESULT
 *	---------	---------	----------
 *	ABBA		BLAG		ABBA
 *	JUNK		CRAP		CRAP
 *	FOOBAR		GREPLINUX	FOOBAR
 *	STUFF		XXYYZZ		XXYYZZ
 *	SUPERDUPER	FOOFOOFOO	SUPERDUPER
 *	FOO12h		FOO666		FOO666
 *			HHIIGG		HHIIGG
 *			HHUUPP		HHUUPP
 *			JJJJJJ		JJJJJJ
 *
 * Create a new program from 'kfp1' and 'kfp2'. 
 * Even numbered code blocks are donated by PROGRAM 1.
 * Odd numbered code blocks are donated by PROGRAM 2.
 *
 * If programs have different number of code blocks, then
 * extra code blocks from the longer program are appended
 * to the new program.
 *
 * NOTE: kforth_merge(x, x) is allowed, and will return a copy of 'x'
 *
 */
KFORTH_PROGRAM *kforth_merge(KFORTH_PROGRAM *kfp1, KFORTH_PROGRAM *kfp2)
{
	ASSERT( kfp1 != NULL );
	ASSERT( kfp2 != NULL );
	ASSERT( kfp1->kfops == kfp2->kfops );

	return kforth_merge_with_mask(0xAAAA, kfp1, kfp2);
}

/***********************************************************************
 * Randomized Merge operation.
 *
 * Uses a random 16-bit pattern, which merges
 * using the each bit of the pattern. When
 *
 *	PROGRAM 1	PROGRAM 2	RESULT
 *	---------	---------	----------
 *	ABBA		BLAG		ABBA
 *	JUNK		CRAP		CRAP
 *	FOOBAR		GREPLINUX	FOOBAR
 *	STUFF		XXYYZZ		XXYYZZ
 *	SUPERDUPER	FOOFOOFOO	SUPERDUPER
 *	FOO12h		FOO666		FOO666
 *			HHIIGG		HHIIGG
 *			HHUUPP		HHUUPP
 *			JJJJJJ		JJJJJJ
 *
 * Create a new program from 'kfp1' and 'kfp2'. 
 *
 * When the bit is 0, program 1's code block is donated. When
 * the bit is 1 then program 2's code block gets donated.
 *
 * The bit pattern is repeated as needed to cover all code blocks.
 *
 * If programs have different number of code blocks, then
 * extra code blocks from the longer program are appended
 * to the new program.
 *
 * NOTE: kforth_merge(x, x) is allowed, and will return a copy of 'x'
 *
 */
KFORTH_PROGRAM *kforth_merge_rnd(EVOLVE_RANDOM *er, KFORTH_PROGRAM *kfp1, KFORTH_PROGRAM *kfp2)
{
	int mask;

	ASSERT( er != NULL );
	ASSERT( kfp1 != NULL );
	ASSERT( kfp2 != NULL );
	ASSERT( kfp1->kfops == kfp2->kfops );

	mask = CHOOSE(er, 0x0000, 0xFFFF);

	return kforth_merge_with_mask(mask, kfp1, kfp2);
}

/*
 * Make a copy of of 'kfp'
 * (Uses the fact that merge(x,x) is the
 * same as copying 'x').
 */
KFORTH_PROGRAM *kforth_copy(KFORTH_PROGRAM *kfp)
{
	ASSERT( kfp != NULL );

	return kforth_merge(kfp, kfp);
}

/*
 * Create a MUTATION_OPTIONS structure
 *
 * Inputs are:
 *	max_code_blocks:
 *		- mutated programs will never get mutated to bigger than this size.
 *		  (if they are already bigger than this, then they can still be mutated, just
 *		  never grow bigger. In other words no truncation on existing programs
 *		  is perfomed).
 *
 *	max_apply:
 *		- maximum number of times to apply mutations to a program. Pick
 *		a random count from 1.. max_apply, and apply the mutation algorithm
 *		that many times. If 0, disable mutations.
 *
 *	prob_mutate_codeblock
 *		- probability a code block will be mutated versus a single instruction
 *
 *	prob_duplicate
 *		- probability the duplicate mutation will take place
 *
 *	prob_delete
 *		- probability the delete mutation will take place
 *
 *	prob_insert
 *		- probability the insert mutation will take place
 *
 *	prob_transpose
 *		- probability the transpose mutation will take place
 *
 *	prob_modify
 *		- probability the modify mutation will take place
 *
 * All values are floating point (doubles) representing
 * probabilities (between 0.0 and 1.0)
 *
 * We support 5 types of mutations. Each type can be applied
 * at the code block level, or instruction level. That is
 * what the probability 'prob_mutate_codeblock' determines.
 *
 *
 */
KFORTH_MUTATE_OPTIONS *kforth_mutate_options_make(
			int max_code_blocks,
			int max_apply,
			double prob_mutate_codeblock,
			double prob_duplicate,
			double prob_delete,
			double prob_insert,
			double prob_transpose,
			double prob_modify,
			int max_opcodes )
{
	KFORTH_MUTATE_OPTIONS *kfmo;

	ASSERT( max_code_blocks > 0 );
	ASSERT( max_apply >= 0 && max_apply <= MUTATE_MAX_APPLY_LIMIT );
	ASSERT( prob_mutate_codeblock >= 0.0 && prob_mutate_codeblock <= 1.0 );
	ASSERT( prob_duplicate >= 0.0 && prob_duplicate <= 1.0 );
	ASSERT( prob_delete >= 0.0 && prob_delete <= 1.0 );
	ASSERT( prob_insert >= 0.0 && prob_insert <= 1.0 );
	ASSERT( prob_transpose >= 0.0 && prob_transpose <= 1.0 );
	ASSERT( prob_modify >= 0.0 && prob_modify <= 1.0 );
	ASSERT( max_opcodes > 0 && max_opcodes < KFORTH_OPS_LEN );

	kfmo = (KFORTH_MUTATE_OPTIONS *) CALLOC(1, sizeof(KFORTH_MUTATE_OPTIONS));
	ASSERT( kfmo != NULL );

	kfmo->max_code_blocks	= max_code_blocks;
	kfmo->max_apply		= max_apply;
	kfmo->prob_mutate_codeblock = (int) (prob_mutate_codeblock * PROBABILITY_SCALE);
	kfmo->prob_duplicate	= (int) (prob_duplicate * PROBABILITY_SCALE);
	kfmo->prob_delete	= (int) (prob_delete * PROBABILITY_SCALE);
	kfmo->prob_insert	= (int) (prob_insert * PROBABILITY_SCALE);
	kfmo->prob_transpose	= (int) (prob_transpose * PROBABILITY_SCALE);
	kfmo->prob_modify	= (int) (prob_modify * PROBABILITY_SCALE);
	kfmo->max_opcodes	= max_opcodes;

	return kfmo;
}

/*
 * Return a CALLOC'd copy of kfmo.
 */
KFORTH_MUTATE_OPTIONS *kforth_mutate_options_copy(KFORTH_MUTATE_OPTIONS *kfmo)
{
	KFORTH_MUTATE_OPTIONS *new_kfmo;

	ASSERT( kfmo != NULL );

	new_kfmo = (KFORTH_MUTATE_OPTIONS *) CALLOC(1, sizeof(KFORTH_MUTATE_OPTIONS));
	ASSERT( new_kfmo != NULL );

	*new_kfmo = *kfmo;

	return new_kfmo;
}

/*
 * These are the default kforth mutation rates
 *
 */
void kforth_mutate_options_defaults(KFORTH_MUTATE_OPTIONS *kfmo)
{
	KFORTH_MUTATE_OPTIONS *tmp;

	ASSERT( kfmo != NULL );

	tmp = kforth_mutate_options_make(100, 10, 0.25, 0.02, 0.06, 0.02, 0.02, 0.02,
						EvolveMaxOpcodes() );

	*kfmo = *tmp;

	kforth_mutate_options_delete(tmp);
}

void kforth_mutate_options_delete(KFORTH_MUTATE_OPTIONS *kfmo)
{
	ASSERT( kfmo != NULL );

	FREE( kfmo );
}
