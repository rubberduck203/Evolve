/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * EXECUTE KFORTH PROGRAMS
 *
 *
 */
#include "stdafx.h"

/*
 * Allocate a stack node (can be for data or call stack)
 */
static KFORTH_STACK_NODE *kforth_stack_node_make(void)
{
	KFORTH_STACK_NODE *node;

	node = (KFORTH_STACK_NODE*) CALLOC(1, sizeof(KFORTH_STACK_NODE));
	ASSERT( node != NULL );

	return node;
}

static void kforth_stack_node_delete(KFORTH_STACK_NODE *node)
{
	ASSERT( node != NULL );

	FREE( node );
}

/*
 * Push node onto 'sp'. sp is updated to point to the new top element.
 */
static void kforth_stack_push(KFORTH_STACK_NODE **sp, KFORTH_STACK_NODE *node)
{
	ASSERT( sp != NULL );
	ASSERT( node != NULL );

	node->next = NULL;
	node->prev = *sp;

	if( *sp != NULL ) {
		ASSERT( (*sp)->next == NULL );
		(*sp)->next = node;
	}
	*sp = node;
}

/*
 * Remove top element from stack. Requires
 * the stack not be empty.
 *
 * 'sp' is updated to point to the previous 'top' element
 * (or NULL if the stack becomes empty).
 *
 */
static KFORTH_STACK_NODE *kforth_stack_pop(KFORTH_STACK_NODE **sp)
{
	KFORTH_STACK_NODE *top;

	ASSERT( sp != NULL );
	ASSERT( *sp != NULL );

	top = *sp;
	*sp = (*sp)->prev;
	if( *sp != NULL )
		(*sp)->next = NULL;
	top->prev = NULL;

	return top;
}

KFORTH_INTEGER kforth_data_stack_pop(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	KFORTH_INTEGER value;

	ASSERT( kfm != NULL );
	ASSERT( kfm->data_sp != NULL );

	node = kforth_stack_pop(&kfm->data_sp);
	value = node->u.data.value;
	kforth_stack_node_delete(node);
	kfm->data_stack_size -= 1;

	return value;
}

void kforth_data_stack_push(KFORTH_MACHINE *kfm, KFORTH_INTEGER value)
{
	KFORTH_STACK_NODE *node;

	ASSERT( kfm != NULL );

	node = kforth_stack_node_make();
	node->u.data.value = value;
	kforth_stack_push(&kfm->data_sp, node);
	kfm->data_stack_size += 1;
}

void kforth_call_stack_push(KFORTH_MACHINE *kfm, int cb, int pc)
{
	KFORTH_STACK_NODE *node;

	ASSERT( kfm != NULL );

	node = kforth_stack_node_make();
	node->u.call.cb = cb;
	node->u.call.pc = pc;
	kforth_stack_push(&kfm->call_sp, node);
	kfm->call_stack_size += 1;
}

/***********************************************************************
 * Create a kforth machine object.
 * Stacks are empty, and execution is setup to
 * begin at code block 0 (cb=0), instruction 0 (pc=0).
 *
 */
KFORTH_MACHINE *kforth_machine_make(KFORTH_PROGRAM *kfp, void *client_data)
{
	KFORTH_MACHINE *kfm;

	ASSERT( kfp != NULL );

	kfm = (KFORTH_MACHINE*) CALLOC(1, sizeof(KFORTH_MACHINE));
	ASSERT( kfm != NULL );

	kfm->program = kfp;
	kfm->client_data = client_data;

	return kfm;
}

/***********************************************************************
 * Delete a KFORTH machine. Delete any stack space occupied
 * by the machine.
 * The program associated with the machine is NOT delete'd
 * however as the program could be in use by another
 * machine. (multi-celled creatures for example)
 *
 */
void kforth_machine_delete(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;

	ASSERT( kfm != NULL );

	while( kfm->call_sp != NULL ) {
		node = kforth_stack_pop(&kfm->call_sp);
		kforth_stack_node_delete(node);
	}
				
	while( kfm->data_sp != NULL ) {
		node = kforth_stack_pop(&kfm->data_sp);
		kforth_stack_node_delete(node);
	}

	FREE( kfm );
}

/***********************************************************************
 * Return a complete copy of 'kfm' (including stacks).
 *
 * NOTE: The returned copy refers to the same 'client_data' and 'program' as 'kfm'.
 *
 */
KFORTH_MACHINE *kforth_machine_copy(KFORTH_MACHINE *kfm)
{
	KFORTH_MACHINE *kfm2;
	KFORTH_STACK_NODE *sp, *sp2, *prev;

	ASSERT( kfm != NULL );

	kfm2 = (KFORTH_MACHINE *) CALLOC(1, sizeof(KFORTH_MACHINE));
	ASSERT( kfm2 != NULL );

	*kfm2		= *kfm;
	kfm2->call_sp	= NULL;
	kfm2->data_sp	= NULL;

	/*
	 * Copy call stack
	 */
	prev = NULL;
	for(sp=kfm->call_sp; sp; sp=sp->prev) {
		sp2 = kforth_stack_node_make();
		sp2->u.call = sp->u.call;

		if( prev == NULL ) {
			kfm2->call_sp = sp2;
		} else {
			prev->prev = sp2;
			sp2->next = prev;
		}

		prev = sp2;
	}

	/*
	 * Copy data stack
	 */
	prev = NULL;
	for(sp=kfm->data_sp; sp; sp=sp->prev) {
		sp2 = kforth_stack_node_make();
		sp2->u.data = sp->u.data;

		if( prev == NULL ) {
			kfm2->data_sp = sp2;
		} else {
			prev->prev = sp2;
			sp2->next = prev;
		}

		prev = sp2;
	}

	return kfm2;
}

/***********************************************************************
 * Execute the KFORTH machine 1 execution step.
 * If program finished, set kfm->terminated.
 *
 * This routine requires that the program has
 * not previously terminated.
 *
 */
void kforth_machine_execute(KFORTH_MACHINE *kfm)
{
	int cb, pc;
	int nblocks, block_len;
	int opcode;
	KFORTH_FUNCTION func;
	KFORTH_STACK_NODE *node;

	ASSERT( kfm != NULL );
	ASSERT( kfm->terminated == 0 );

	cb = kfm->cb;
	pc = kfm->pc;

	nblocks = kfm->program->nblocks;
	ASSERT( cb < nblocks );

	block_len = kfm->program->block_len[cb];

	if( pc >= block_len ) {
		/*
		 * return from code block
		 */
		if( kfm->call_sp != NULL ) {
			node = kforth_stack_pop(&kfm->call_sp);
			kfm->call_stack_size -= 1;
			ASSERT( kfm->call_stack_size >= 0 );

			kfm->cb = node->u.call.cb;
			kfm->pc = node->u.call.pc+1;

			kforth_stack_node_delete(node);

		} else {
#ifdef GROW_USING_CB
			// not true when GROW calls a cb.
#else
			ASSERT( cb == 0 );
#endif


			kfm->terminated = 1;
		}

	} else {
		/*
		 * Normal instruction
		 */
		opcode = kfm->program->opcode[cb][pc];
		ASSERT( opcode >= 0 && opcode < KFORTH_OPS_LEN );

		func = kfm->program->kfops[opcode].func;

		(*func)(kfm);

		kfm->pc = kfm->pc + 1;
	}
}

/***********************************************************************
 * Reset the kforth machine so that is can restart
 * execution of the program.
 *
 */
void kforth_machine_reset(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	int i;

	ASSERT( kfm != NULL );

	kfm->terminated = 0;

	while( kfm->call_sp != NULL ) {
		node = kforth_stack_pop(&kfm->call_sp);
		kforth_stack_node_delete(node);
	}

	while( kfm->data_sp != NULL ) {
		node = kforth_stack_pop(&kfm->data_sp);
		kforth_stack_node_delete(node);
	}

	kfm->call_stack_size = 0;
	kfm->data_stack_size = 0;

	kfm->cb = 0;
	kfm->pc = 0;

	for(i=0; i<10; i++)
		kfm->R[i] = 0;
}


/* ----------------------------------------------------------------------
 * KFORTH BUILT-IN OPCODES
 * Each function below may be called during execution of a KFORTH program.
 * The KFORTH_OPERATIONS table is indexed by the opcode, and
 * one of these functions is called.
 *
 * These operations form the CORE set of KFORTH operators that exist
 * in any KFORTH implementation. Additional operators
 * may be added by other implementations.
 *
 */

/*
 * Push Operand value on data stack
 *
 * This is opcode 0, and is created by the compiler when
 * a numeric literal is encountered in the input.
 *
 * ( -- a )
 */
static void kfop_push(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	int cb, pc;

	node = kforth_stack_node_make();

	cb = kfm->cb;
	pc = kfm->pc;

	node->u.data.value = kfm->program->operand[cb][pc];

	kforth_stack_push(&kfm->data_sp, node);
	kfm->data_stack_size += 1;
}

/*
 * Remove a value from the data stack. If
 * Stack is empty do nothing.
 * ( a -- )
 */
static void kfop_pop(KFORTH_MACHINE *kfm)
{
	if( kfm->data_sp != NULL ) {
		kforth_data_stack_pop(kfm);
	}
}

/*
 * Call operation: Call code block
 * appearing on top of data stack. If code block number
 * is invalid, do nothing.
 *
 * ( a -- )
 *
 */
static void kfop_call(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	KFORTH_INTEGER cb;

	if( kfm->data_sp ) {
		cb = kforth_data_stack_pop(kfm);

#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
		/* relative addressing mode */
		cb += kfm->cb;
#endif

		if( cb >= 0 && cb < kfm->program->nblocks ) {
			node = kforth_stack_node_make();
			node->u.call.cb = kfm->cb;
			node->u.call.pc = kfm->pc;
			kforth_stack_push(&kfm->call_sp, node);
			kfm->call_stack_size += 1;
			/* 'pc' will get incremented to 0, inside the main execution loop */
			kfm->pc = -1;
			kfm->cb = (int)cb;
		}
	}
}

/*
 * Call code block 'cb' if value is non-zero. Else
 * do nothing.
 *
 * (value cb --)
 */
static void kfop_if(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	KFORTH_INTEGER value, cb;

	if( kfm->data_stack_size >= 2 ) {
		cb = kforth_data_stack_pop(kfm);

#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
		/* relative addressing mode */
		cb += kfm->cb;
#endif

		value = kforth_data_stack_pop(kfm);

		if( value != 0 ) {
			if( cb >= 0 && cb < kfm->program->nblocks ) {
				node = kforth_stack_node_make();
				node->u.call.cb = kfm->cb;
				node->u.call.pc = kfm->pc;
				kforth_stack_push(&kfm->call_sp, node);
				kfm->call_stack_size += 1;
				/* 'pc' will get incremented to 0, inside the main execution loop */
				kfm->pc = -1;
				kfm->cb = (int)cb;
			}
		}
	}
}

/*
 * Call code block 'cb1' if value is non-zero. Else
 * call code block 'cb2'. If code block 1 or 2 is invalid,
 * assume it calls an empty NO-OP block.
 *
 * (value cb1 cb2 --)
 */
static void kfop_ifelse(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *node;
	KFORTH_INTEGER value, cb1, cb2;

	if( kfm->data_stack_size >= 3 ) {
		cb2 = kforth_data_stack_pop(kfm);
		cb1 = kforth_data_stack_pop(kfm);

#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
		/* relative addressing mode */
		cb1 += kfm->cb;
		cb2 += kfm->cb;
#endif

		value = kforth_data_stack_pop(kfm);

		if( value != 0 ) {
			if( cb1 >= 0 && cb1 < kfm->program->nblocks ) {
				node = kforth_stack_node_make();
				node->u.call.cb = kfm->cb;
				node->u.call.pc = kfm->pc;
				kforth_stack_push(&kfm->call_sp, node);
				kfm->call_stack_size += 1;
				/* 'pc' will get incremented to 0, inside the main execution loop */
				kfm->pc = -1;
				kfm->cb = (int)cb1;
			}
		} else {
			if( cb2 >= 0 && cb2 < kfm->program->nblocks ) {
				node = kforth_stack_node_make();
				node->u.call.cb = kfm->cb;
				node->u.call.pc = kfm->pc;
				kforth_stack_push(&kfm->call_sp, node);
				kfm->call_stack_size += 1;
				/* 'pc' will get incremented to 0, inside the main execution loop */
				kfm->pc = -1;
				kfm->cb = (int)cb2;
			}
		}
	}
}

/*
 * Loop to start of code block if n is non-zero.
 * ( n -- )
 */
static void kfop_loop(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		if( value != 0 ) {
			kfm->pc = -1;
		}
	}
}

/*
 * Exit code block (if n non-zero)
 * ( n -- )
 */
static void kfop_exit(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		if( value != 0 ) {
			kfm->pc = kfm->program->block_len[kfm->cb];
		}
	}
}

/*
 * ( a -- a a )
 */
static void kfop_dup(KFORTH_MACHINE *kfm)
{
	if( kfm->data_sp != NULL ) {
		kforth_data_stack_push(kfm, kfm->data_sp->u.data.value);
	}
}

/*
 * ( a b -- b a )
 */
static void kfop_swap(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER tmp;

	if( kfm->data_stack_size >= 2 ) {
		tmp = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value =
			kfm->data_sp->prev->u.data.value;

		kfm->data_sp->prev->u.data.value = tmp;
	}
}

/*
 * ( a b -- a b a )
 */
static void kfop_over(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kfm->data_sp->prev->u.data.value;
		kforth_data_stack_push(kfm, value);
	}
}

/*
 * ( a b c -- b c a )
 */
static void kfop_rot(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *na, *nb, *nc;
	KFORTH_INTEGER a, b, c;

	if( kfm->data_stack_size >= 3 ) {
		nc = kfm->data_sp;
		nb = nc->prev;
		na = nb->prev;

		c = nc->u.data.value;
		b = nb->u.data.value;
		a = na->u.data.value;

		na->u.data.value = b;
		nb->u.data.value = c;
		nc->u.data.value = a;
	}
}

/*
 * ( a b c -- c a b )
 */
static void kfop_reverse_rot(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *na, *nb, *nc;
	KFORTH_INTEGER a, b, c;

	if( kfm->data_stack_size >= 3 ) {
		nc = kfm->data_sp;
		nb = nc->prev;
		na = nb->prev;

		c = nc->u.data.value;
		b = nb->u.data.value;
		a = na->u.data.value;

		na->u.data.value = c;
		nb->u.data.value = a;
		nc->u.data.value = b;
	}
}


/*
 * ( ... v2 v1 v0 n -- ... v2 v1 v0 vn )
 *
 * pick numbering begins with 0.
 *
 */
static void kfop_pick(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *sp;
	KFORTH_INTEGER n;
	int i;

	if( kfm->data_sp != NULL ) {
		n = kforth_data_stack_pop(kfm);
		if( (n >= 0) && (n <= kfm->data_stack_size-1) ) {
			sp = kfm->data_sp;
			for(i=0; i<n; i++) {
				sp = sp->prev;
			}
			kforth_data_stack_push(kfm, sp->u.data.value);
		}
	}
}

/*
 * ( n -- n n | 0 ) duplicate only if non-zero
 */
static void kfop_dup_if(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		if( value ) {
			kforth_data_stack_push(kfm, value);
		} else {
			kfm->data_sp->u.data.value = 0;
		}
	}
}

/*
 * Swap 2 pairs
 *
 * ( a  b  c  d --  c  d  a  b )
 *   |  |  |  |     |  |  |  |
 *   |  |  |  |     |  |  |  |
 *   n1 n2 n3 n4    n1 n2 n3 n4
 */
static void kfop_2swap(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *n1, *n2, *n3, *n4;
	KFORTH_INTEGER a, b, c, d;

	if( kfm->data_stack_size >= 4 ) {
		n4 = kfm->data_sp;
		n3 = n4->prev;
		n2 = n3->prev;
		n1 = n2->prev;

		a = n1->u.data.value;
		b = n2->u.data.value;
		c = n3->u.data.value;
		d = n4->u.data.value;

		n1->u.data.value = c;
		n2->u.data.value = d;
		n3->u.data.value = a;
		n4->u.data.value = b;
	}
}

/*
 * ( a b c d -- a b c d a b)
 */
static void kfop_2over(KFORTH_MACHINE *kfm)
{
	KFORTH_STACK_NODE *sp;
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 4 ) {
		sp = kfm->data_sp;
		sp = sp->prev;
		sp = sp->prev;
		b = sp->u.data.value;
		sp = sp->prev;
		a = sp->u.data.value;

		kforth_data_stack_push(kfm, a);
		kforth_data_stack_push(kfm, b);
	}
}

/*
 * ( a b -- a b a b )
 */
static void kfop_2dup(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		a = kfm->data_sp->prev->u.data.value;
		b = kfm->data_sp->u.data.value;
		kforth_data_stack_push(kfm, a);
		kforth_data_stack_push(kfm, b);
	}
}

/*
 * ( a b -- )
 */
static void kfop_2pop(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size >= 2 ) {
		kforth_data_stack_pop(kfm);
		kforth_data_stack_pop(kfm);
	}
}

/*
 * ( a b -- b )
 */
static void kfop_nip(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kforth_data_stack_pop(kfm);
		kfm->data_sp->u.data.value = value;
	}
}

/*
 * ( a b -- b a b)
 */
static void kfop_tuck(KFORTH_MACHINE *kfm)
{
	if( kfm->data_stack_size >= 2 ) {
		kfop_swap(kfm);
		kfop_over(kfm);
	}
}

/*
 * ( n -- n+1 )
 */
static void kfop_increment(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value+1;
	}
}

/*
 * ( n -- n-1 )
 */
static void kfop_decrement(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value-1;
	}
}

/*
 * ( n -- n+2 )
 */
static void kfop_increment2(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value+2;
	}
}

/*
 * ( n -- n-2 )
 */
static void kfop_decrement2(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value-2;
	}
}

/*
 * ( n -- n/2 )
 */
static void kfop_half(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value/2;
	}
}

/*
 * ( n -- n*2 )
 */
static void kfop_double(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = value*2;
	}
}

/*
 * ( n -- ABS(n) )
 */
static void kfop_abs(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		if( value < 0 ) {
			value = -value;
		}
		kfm->data_sp->u.data.value = value;
	}
}

/*
 * If n is negative, do nothing.
 * ( n -- SQRT(n) )
 */
static void kfop_sqrt(KFORTH_MACHINE *kfm)
{
	double d;
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		if( value >= 0 ) {
			d = (double)value;
			d = sqrt(d);
			kfm->data_sp->u.data.value = (KFORTH_INTEGER)d;
		}
	}
}

/*
 * ( a b -- a+b )
 */
static void kfop_plus(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kforth_data_stack_pop(kfm);
		kfm->data_sp->u.data.value += value;
	}
}

/*
 * ( a b -- a-b )
 */
static void kfop_minus(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kforth_data_stack_pop(kfm);
		kfm->data_sp->u.data.value -= value;
	}
}

/*
 * ( a b -- a*b )
 */
static void kfop_multiply(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kforth_data_stack_pop(kfm);
		kfm->data_sp->u.data.value *= value;
	}
}

/*
 * if b==0, do nothing.
 * ( a b -- a/b )
 */
static void kfop_divide(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kfm->data_sp->u.data.value;
		if( value != 0 ) {
			value = kforth_data_stack_pop(kfm);
			kfm->data_sp->u.data.value /= value;
		}
	}
}

/*
 * if b==0, do nothing.
 * ( a b -- a%b )
 */
static void kfop_modulos(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kfm->data_sp->u.data.value;
		if( value != 0 ) {
			value = kforth_data_stack_pop(kfm);
			kfm->data_sp->u.data.value %= value;
		}
	}
}

/*
 * if b==0, do nothing.
 * ( a b -- a%b a/b )
 */
static void kfop_divmod(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kfm->data_sp->u.data.value;
		if( b != 0 ) {
			b = kforth_data_stack_pop(kfm);
			a = kforth_data_stack_pop(kfm);
			kforth_data_stack_push(kfm, a%b);
			kforth_data_stack_push(kfm, a/b);
		}
	}
}

/*
 * ( a -- -a )
 */
static void kfop_negate(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = -value;
	}
}

/*
 * ( a b -- -a -b )
 */
static void kfop_2negate(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_stack_size >= 2 ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = -value;

		value = kfm->data_sp->prev->u.data.value;
		kfm->data_sp->prev->u.data.value = -value;
	}
}

/*
 * ( a b -- a==b )
 */
static void kfop_eq(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a==b);
	}
}

/*
 * ( a b -- a!=b )
 */
static void kfop_ne(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a!=b);
	}
}

/*
 * ( a b -- a<b )
 */
static void kfop_lt(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a<b);
	}
}

/*
 * ( a b -- a>b )
 */
static void kfop_gt(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a>b);
	}
}

/*
 * ( a b -- a<=b )
 */
static void kfop_le(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a<=b);
	}
}

/*
 * ( a b -- a>=b )
 */
static void kfop_ge(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a>=b);
	}
}

/*
 * ( a -- a==0 )
 */
static void kfop_equal_zero(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (value == 0);
	}
}

/*
 * ( a b -- a|b )
 */
static void kfop_or(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a|b);
	}
}

/*
 * ( a b -- a&b )
 */
static void kfop_and(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a&b);
	}
}

/*
 * ( a -- !a )
 */
static void kfop_not(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = !value;
	}
}

/*
 * ( a -- ~a )
 */
static void kfop_invert(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = ~value;
	}
}

/*
 * ( a b -- a^b )
 */
static void kfop_xor(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a^b);
	}
}

/*
 * ( a b -- MIN(a,b) )
 */
static void kfop_min(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a<b) ? a : b;
	}
}

/*
 * ( a b -- MAX(a,b) )
 */
static void kfop_max(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kfm->data_sp->u.data.value;
		kfm->data_sp->u.data.value = (a>b) ? a : b;
	}
}

/*
 * ( -- CB )   ; current code block number
 */
static void kfop_cb(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->cb);
}

/*
 * Fetch register 'n'. If 'n' is not a valid
 * register, then do nothing.
 *
 * ( n -- Rn )
 */
static void kfop_rn(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kfm->data_sp->u.data.value;
		if( value >= 0 && value <= 9 ) {
			value = kfm->R[value];
			kfm->data_sp->u.data.value = value;
		}
	}
}

static void kfop_r0(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[0]);
}

static void kfop_r1(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[1]);
}

static void kfop_r2(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[2]);
}

static void kfop_r3(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[3]);
}

static void kfop_r4(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[4]);
}

static void kfop_r5(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[5]);
}

static void kfop_r6(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[6]);
}

static void kfop_r7(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[7]);
}

static void kfop_r8(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[8]);
}

static void kfop_r9(KFORTH_MACHINE *kfm)
{
	kforth_data_stack_push(kfm, kfm->R[9]);
}

/*
 *
 * ( val n -- )
 */
static void kfop_set_rn(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value, n;

	if( kfm->data_stack_size >= 2 ) {
		n = kfm->data_sp->u.data.value;
		if( n >= 0 && n <= 9 ) {
			n = kforth_data_stack_pop(kfm);
			value = kforth_data_stack_pop(kfm);
			kfm->R[n] = value;
		}
	}
}

static void kfop_set_r0(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[0] = value;
	}
}

static void kfop_set_r1(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[1] = value;
	}
}

static void kfop_set_r2(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[2] = value;
	}
}

static void kfop_set_r3(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[3] = value;
	}
}

static void kfop_set_r4(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[4] = value;
	}
}

static void kfop_set_r5(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[5] = value;
	}
}

static void kfop_set_r6(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[6] = value;
	}
}

static void kfop_set_r7(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[7] = value;
	}
}

static void kfop_set_r8(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[8] = value;
	}
}

static void kfop_set_r9(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER value;

	if( kfm->data_sp != NULL ) {
		value = kforth_data_stack_pop(kfm);
		kfm->R[9] = value;
	}
}

/*
 * ( n -- SIGN(n) )
 */
static void kfop_sign(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER n;

	if( kfm->data_stack_size >= 1 ) {
		n = kfm->data_sp->u.data.value;

		if( n > 0 )
			n = 1;
		else if( n < 0 )
			n = -1;
		else
			n = 0;

		kfm->data_sp->u.data.value = n;
	}
}

/*
 * These constants used for pack/unpack instructions
 * for some reason I cannot get the full positive range, so
 * the min/max values are defined as:
 *
 *		MIN		MAX
 *	int	-2^31-1		2^31-1		(pack2/unpack2)
 *
 *	short	-2^15-1		2^15-1		(pack4/unpack4)
 *
 */
#define MAX_INT32	 2147483647
#define MIN_INT32	-2147483647

#define MAX_INT16	 32767
#define MIN_INT16	-32767

/*
 * If incomming value is too big or too little (for a short)
 * then peg value at max or min.
 */
static KFORTH_INTEGER pack16(KFORTH_INTEGER x)
{
	if( x > MAX_INT16 ) {
		return MAX_INT16;

	} else if( x < MIN_INT16 ) {
		return MIN_INT16;

	} else {
		return x;
	}
}

/*
 * If incomming value is too big or too little (for an int)
 * then peg value at max or min.
 */
static KFORTH_INTEGER pack32(KFORTH_INTEGER x)
{
	if( x > MAX_INT32 ) {
		return MAX_INT32;

	} else if( x < MIN_INT32 ) {
		return MIN_INT32;

	} else {
		return x;
	}
}


/*
 * ( a b -- n )
 */
static void kfop_pack2(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b, n;

	if( kfm->data_stack_size >= 2 ) {
		b = kforth_data_stack_pop(kfm);
		a = kforth_data_stack_pop(kfm);

		a = (unsigned int) pack32(a);
		b = (unsigned int) pack32(b);

		n = (a << 32) | (b);

		kforth_data_stack_push(kfm, n);
	}
}

/*
 * ( a b c d -- n )
 */
static void kfop_pack4(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b, c, d, n;

	if( kfm->data_stack_size >= 4 ) {
		d = kforth_data_stack_pop(kfm);
		c = kforth_data_stack_pop(kfm);
		b = kforth_data_stack_pop(kfm);
		a = kforth_data_stack_pop(kfm);

		a = (unsigned short) pack16(a);
		b = (unsigned short) pack16(b);
		c = (unsigned short) pack16(c);
		d = (unsigned short) pack16(d);

		n = (a << 48) | (b << 32) | (c << 16) | (d);

		kforth_data_stack_push(kfm, n);
	}
}


/*
 * ( n -- a b )
 */
static void kfop_unpack2(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b, n;

	if( kfm->data_stack_size >= 1 ) {
		n = kforth_data_stack_pop(kfm);

		a = (int) (n >> 32);
		b = (int) (n & 0xffffffff);

		kforth_data_stack_push(kfm, a);
		kforth_data_stack_push(kfm, b);
	}
}

/*
 * ( n -- a b c d )
 */
static void kfop_unpack4(KFORTH_MACHINE *kfm)
{
	KFORTH_INTEGER a, b, c, d, n;

	if( kfm->data_stack_size >= 1 ) {
		n = kforth_data_stack_pop(kfm);

		a = (short) (n >> 48);
		b = (short) ((n >> 32) & 0xffff);
		c = (short) ((n >> 16) & 0xffff);
		d = (short) ((n) & 0xffff);

		kforth_data_stack_push(kfm, a);
		kforth_data_stack_push(kfm, b);
		kforth_data_stack_push(kfm, c);
		kforth_data_stack_push(kfm, d);
	}

}

/***********************************************************************
 * Create a KFORTH_OPERATIONS table, and
 * add all the KFORTH_OPERATION
 * initialize with the KFORTH primitives.
 *
 * This table can be extended with additional primitives depending
 * on the application of KFORTH to a particular problem domain.
 *
 * These primitives always exist in the KFORTH language.
 *
 * KFORTH LANGUAGE PRIMITIVES:
 *
 *
 * Name		Function	Description
 * ---------	-----------	------------------------------------------
 *
 * <any number>	kfop_push	Push <any number> on data stack.
 * pop		kfop_pop	( n -- )
 *
 * call		kfop_call	( code-block -- )		call code block
 * if		kfop_if		( expr code-block -- )
 * ifelse	kfop_ifelse	( expr true-block false-block -- )
 * ?loop	kfop_loop	( n -- )	goto start of code block
 * ?exit	kfop_exit	( n -- )	goto end of code block
 *
 * dup		kfop_dup	( a -- a a )
 * swap		kfop_swap	( a b -- b a )
 * over		kfop_over	( a b  -- a b a )
 * rot		kfop_rot	( a b c -- b c a )
 * -rot		kfop_reverse_rot ( a b c -- c a b )
 * pick		kfop_pick	( ... v2 v1 v0 n -- ... v2 v1 v0 vn )
 * ?dup		kfop_dup_if	( n -- n n | 0 ) duplicate only if non-zero
 * 2swap	kfop_2swap	( a b c d --  c d a b )
 * 2over	kfop_2over	( a b c d -- a b c d a b)
 * 2dup		kfop_2dup	( a b -- a b a b )
 * 2pop		kfop_2pop	( a b  -- )
 * nip		kfop_nip	( a b -- b )
 * tuck		kfop_tuck	( a b -- b a b)
 * 1+		kfop_increment	( n -- n+1 )
 * 1-		kfop_decrement	( n -- n-1 )
 * 2+		kfop_increment2	( n -- n+2 )
 * 2-		kfop_decrement2	( n -- n-2 )
 * 2/		kfop_half	( n -- n/2 )
 * 2*		kfop_double	( n -- n*2 )
 * abs		kfop_abs	( n -- abs(n) )
 * sqrt		kfop_sqrt	( n -- sqrt(n) )
 * +		kfop_plus	( a b -- a+b )
 * -		kfop_minus	( a b -- a-b )
 * *		kfop_multiply	( a b -- a*b )
 * /		kfop_divide	( a b -- a/b )
 * mod		kfop_modulos	( a b -- a%b )
 * /mod		kfop_divmod	( a b -- a%b a/b )
 * negate	kfop_negate	( n -- -n )
 * 2negate	kfop_2negate	( a b -- -a -b )
 * =		kfop_eq		( a b -- EQ(a,b) )
 * <>		kfop_ne		( a b -- NE(a,b) )
 * <		kfop_lt		( a b -- LT(a,b) )
 * >		kfop_gt		( a b -- GT(a,b) )
 * <=		kfop_le		( a b -- LE(a,b) )
 * >=		kfop_ge		( a b -- GE(a,b) )
 * 0=		kfop_equal_zero	( n -- EQ(n,0) )
 * or		kfop_or		( a b -- a|b )	logical (and bitwise) or
 * and		kfop_and	( a b -- a&b )	logical (and bitwise) and
 * not		kfop_not	( n -- !n )	logical not
 * invert	kfop_invert	( n -- ~n )	bitwise not
 * xor		kfop_xor	( a b -- a^b )	bitwise XOR
 * min		kfop_min	( a b -- min(a,b) )
 * max		kfop_max	( a b  -- max(a,b) )
 *
 * CB		kfop_cb		( -- CB )	get code block number
 * Rn		kfop_rn		( n -- Rn )	get register n
 * R0		kfop_r0		(   -- R0 )	get register r0
 * R1		kfop_r1		(   -- R1 )	get register r1
 * R2		kfop_r2		(   -- R2 )	get register r2
 * R3		kfop_r3		(   -- R3 )	get register r3
 * R4		kfop_r4		(   -- R4 )	get register r4
 * R5		kfop_r5		(   -- R5 )	get register r5
 * R6		kfop_r6		(   -- R6 )	get register r6
 * R7		kfop_r7		(   -- R7 )	get register r7
 * R8		kfop_r8		(   -- R8 )	get register r8
 * R9		kfop_r9		(   -- R9 )	get register r9
 *
 * Rn!		kfop_set_rn	( val n -- )	set Register n to val
 * R0!		kfop_set_r0	( val -- )	set regiser 0 to val
 * R1!		kfop_set_r1	( val -- )	set regiser 1 to val
 * R2!		kfop_set_r2	( val -- )	set regiser 2 to val
 * R3!		kfop_set_r3	( val -- )	set regiser 3 to val
 * R4!		kfop_set_r4	( val -- )	set regiser 4 to val
 * R5!		kfop_set_r5	( val -- )	set regiser 5 to val
 * R6!		kfop_set_r6	( val -- )	set regiser 6 to val
 * R7!		kfop_set_r7	( val -- )	set regiser 7 to val
 * R8!		kfop_set_r8	( val -- )	set regiser 8 to val
 * R9!		kfop_set_r9	( val -- )	set regiser 9 to val
 * sign		kfop_sign	( n -- SIGN(n) ) sign of 'n' (returns: -1, 0, 1)
 * pack2	kfop_pack2	( a b -- n )	combine 2 32-bit ints into a single 'n'
 * unpack2	kfop_unpack2	( n -- a b )	extracts 2 32-bit values from a single 'n'
 * pack4	kfop_pack2	( a b c d -- n )	combine 4 16-bit ints into a single 'n'
 * unpack4	kfop_unpack2	( n -- a b c d )	extracts 4 16-bit values from a single 'n'
 *
 *
 */
KFORTH_OPERATIONS *kforth_ops_make(void)
{
	KFORTH_OPERATIONS *kfops;

	kfops = (KFORTH_OPERATIONS*) CALLOC(KFORTH_OPS_LEN, sizeof(KFORTH_OPERATIONS));

	ASSERT( kfops != NULL );

	/*
	 * the first entry (opcde=0) is special.
	 * as 9999 is not a valid operator name, but it
	 * is added here, without using kforth_ops_add().
 	 *
 	 * Instruction 0 is created by the compiler
	 * when it compiles any number.
	 */
	kfops[0].name = "9999";
	kfops[0].func = kfop_push;

	kforth_ops_add(kfops, "call",		kfop_call);
	kforth_ops_add(kfops, "if",		kfop_if);
	kforth_ops_add(kfops, "ifelse",		kfop_ifelse);
	kforth_ops_add(kfops, "?loop",		kfop_loop);
	kforth_ops_add(kfops, "?exit",		kfop_exit);

	kforth_ops_add(kfops, "pop",		kfop_pop);
	kforth_ops_add(kfops, "dup",		kfop_dup);
	kforth_ops_add(kfops, "swap",		kfop_swap);
	kforth_ops_add(kfops, "over",		kfop_over);
	kforth_ops_add(kfops, "rot",		kfop_rot);

	kforth_ops_add(kfops, "pick",		kfop_pick);
	kforth_ops_add(kfops, "?dup",		kfop_dup_if);
	kforth_ops_add(kfops, "-rot",		kfop_reverse_rot);
	kforth_ops_add(kfops, "2swap",		kfop_2swap);
	kforth_ops_add(kfops, "2over",		kfop_2over);
	kforth_ops_add(kfops, "2dup",		kfop_2dup);
	kforth_ops_add(kfops, "2pop",		kfop_2pop);
	kforth_ops_add(kfops, "nip",		kfop_nip);
	kforth_ops_add(kfops, "tuck",		kfop_tuck);
	kforth_ops_add(kfops, "1+",		kfop_increment);
	kforth_ops_add(kfops, "1-",		kfop_decrement);
	kforth_ops_add(kfops, "2+",		kfop_increment2);
	kforth_ops_add(kfops, "2-",		kfop_decrement2);
	kforth_ops_add(kfops, "2/",		kfop_half);
	kforth_ops_add(kfops, "2*",		kfop_double);
	kforth_ops_add(kfops, "abs",		kfop_abs);
	kforth_ops_add(kfops, "sqrt",		kfop_sqrt);
	kforth_ops_add(kfops, "+",		kfop_plus);
	kforth_ops_add(kfops, "-",		kfop_minus);
	kforth_ops_add(kfops, "*",		kfop_multiply);
	kforth_ops_add(kfops, "/",		kfop_divide);
	kforth_ops_add(kfops, "mod",		kfop_modulos);
	kforth_ops_add(kfops, "/mod",		kfop_divmod);
	kforth_ops_add(kfops, "negate",		kfop_negate);
	kforth_ops_add(kfops, "2negate",	kfop_2negate);
	kforth_ops_add(kfops, "=",		kfop_eq);
	kforth_ops_add(kfops, "<>",		kfop_ne);
	kforth_ops_add(kfops, "<",		kfop_lt);
	kforth_ops_add(kfops, ">",		kfop_gt);
	kforth_ops_add(kfops, "<=",		kfop_le);
	kforth_ops_add(kfops, ">=",		kfop_ge);
	kforth_ops_add(kfops, "0=",		kfop_equal_zero);
	kforth_ops_add(kfops, "or",		kfop_or);
	kforth_ops_add(kfops, "and",		kfop_and);
	kforth_ops_add(kfops, "not",		kfop_not);
	kforth_ops_add(kfops, "invert",		kfop_invert);
	kforth_ops_add(kfops, "xor",		kfop_xor);
	kforth_ops_add(kfops, "min",		kfop_min);
	kforth_ops_add(kfops, "max",		kfop_max);

	kforth_ops_add(kfops, "CB",		kfop_cb);
	kforth_ops_add(kfops, "Rn",		kfop_rn);
	kforth_ops_add(kfops, "R0",		kfop_r0);
	kforth_ops_add(kfops, "R1",		kfop_r1);
	kforth_ops_add(kfops, "R2",		kfop_r2);
	kforth_ops_add(kfops, "R3",		kfop_r3);
	kforth_ops_add(kfops, "R4",		kfop_r4);
	kforth_ops_add(kfops, "R5",		kfop_r5);
	kforth_ops_add(kfops, "R6",		kfop_r6);
	kforth_ops_add(kfops, "R7",		kfop_r7);
	kforth_ops_add(kfops, "R8",		kfop_r8);
	kforth_ops_add(kfops, "R9",		kfop_r9);

	kforth_ops_add(kfops, "Rn!",		kfop_set_rn);
	kforth_ops_add(kfops, "R0!",		kfop_set_r0);
	kforth_ops_add(kfops, "R1!",		kfop_set_r1);
	kforth_ops_add(kfops, "R2!",		kfop_set_r2);
	kforth_ops_add(kfops, "R3!",		kfop_set_r3);
	kforth_ops_add(kfops, "R4!",		kfop_set_r4);
	kforth_ops_add(kfops, "R5!",		kfop_set_r5);
	kforth_ops_add(kfops, "R6!",		kfop_set_r6);
	kforth_ops_add(kfops, "R7!",		kfop_set_r7);
	kforth_ops_add(kfops, "R8!",		kfop_set_r8);
	kforth_ops_add(kfops, "R9!",		kfop_set_r9);

	kforth_ops_add(kfops, "sign",		kfop_sign);
	kforth_ops_add(kfops, "pack2",		kfop_pack2);
	kforth_ops_add(kfops, "unpack2",	kfop_unpack2);
	kforth_ops_add(kfops, "pack4",		kfop_pack4);
	kforth_ops_add(kfops, "unpack4",	kfop_unpack4);

	return kfops;
}

/***********************************************************************
 * Free a KFORTH_OPERATIONS table.
 *
 */
void kforth_ops_delete(KFORTH_OPERATIONS *kfops)
{
	ASSERT( kfops != NULL );

	FREE(kfops);
}

/***********************************************************************
 * A valid kforth operator name:
 *	1. Does not include ';' (clashes with comments)
 *	2. Does not include ':'	(clashes with labels)
 *	3. Does not include '{'	or '}' (clashes with braces)
 *	4. Does not include white space
 *	5. Does not include control characters
 *	6. Cannot clash with a number '999' '123', '0', '-12'
 *	7. Does not have a length of 0.
 *
 */
static int kforth_valid_operator_name(char *name)
{
	char *p;
	int digits, len, leading_minus;

	ASSERT( name != NULL );

	if( name[0] == '-' )
		leading_minus = 1;
	else
		leading_minus = 0;

	digits = 0;
	len = 0;
	for(p=name; *p; p++) {
		if( *p == ':' )
			return 0;

		if( *p == ';' )
			return 0;

		if( *p == '{' )
			return 0;

		if( *p == '}' )
			return 0;

		if( isspace(*p) )
			return 0;

		if( iscntrl(*p) )
			return 0;

		if( isdigit(*p) )
			digits++;

		len++;
	}

	if( len == 0 )
		return 0;

	if( (digits > 0) && (leading_minus + digits) == len )
		return 0;

	return 1;

}

/***********************************************************************
 * This routine requires that the 'kfops' table not be full,
 * and that 'name' has not already been used.
 * And that 'name' is a valid KFORTH operator name.
 *
 */
void kforth_ops_add(KFORTH_OPERATIONS *kfops, char *name, KFORTH_FUNCTION func)
{
	int i;
	int found_empty_slot;

	ASSERT( kfops != NULL );
	ASSERT( name != NULL );
	ASSERT( func != NULL );

	ASSERT( kforth_valid_operator_name(name) );

	found_empty_slot = 0;
	for(i=0; i < KFORTH_OPS_LEN; i++) {
		if( kfops[i].name == NULL ) {
			found_empty_slot = 1;
			break;
		}

		ASSERT( stricmp(name, kfops[i].name) != 0 );
	}

	ASSERT( found_empty_slot );

	kfops[i].name	= name;
	kfops[i].func	= func;
}

/*
 * Return the maximum defined opcode for 'kfops'
 *
 * Valid opcodes will be: 0 .. max_opcode
 *
 * max_opcode will be less than or equal to KFORTH_OPS_LEN
 *
 */
int kforth_ops_max_opcode(KFORTH_OPERATIONS *kfops)
{
	int i, found_empty_slot;

	ASSERT( kfops != NULL );

	found_empty_slot = 0;
	for(i=0; i < KFORTH_OPS_LEN; i++) {
		if( kfops[i].name == NULL ) {
			found_empty_slot = 1;
			break;
		}
	}

	ASSERT( found_empty_slot );
	ASSERT( i >= 0 );

	return i-1;
}
