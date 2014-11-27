/*
 * Copyright (c) 2006 Stauffer Computer Consulting
 */
#ifndef lint
static char copyright[] = "Copyright (c) 2006 Stauffer Computer Consulting";
#endif

/***********************************************************************
 * COMPILE/DISASSEMBLE KFORTH PROGRAMS
 *
 *
 */
#include "stdafx.h"

/*
 * Return true if 'c' is a valid character inside of a kforth word
 */
static int is_kforth_word_char(int c)
{
	if( isspace(c) || strchr(";:{}", c) || iscntrl(c) )
		return 0;
	else
		return 1;
}

/*
 * All digits (with optional leading minus sign).
 */
static int is_kforth_operand(char *word)
{
	int len;
	char *w;

	ASSERT( word != NULL );

	len = 0;
	w = word;

	if( *w == '-' ) {
		len++;
		w++;
	}

	while( *w ) {
		if( isdigit(*w) ) {
			len++;
		}
		w++;
	}

	return ( strlen(word) == len );
}

/*
 * Return the opcode that corresponds to 'word' (or return -1).
 * (don't examine entry '0' because that is not really an instruction)
 */
static int lookup_opcode(KFORTH_OPERATIONS *kfops, char *word)
{
	int i;

	ASSERT( kfops != NULL );
	ASSERT( word != NULL );

	for(i=1; i<KFORTH_OPS_LEN; i++) {
		if( kfops[i].name == NULL )
			break;

		if( stricmp(kfops[i].name, word) == 0 ) {
			return i;
		}
	}
	return -1;
}

struct kforth_label_usage {
	int lineno;
	int cb;
	int pc;
	struct kforth_label_usage *next;
};

struct kforth_label {
	char	*name;
	int	lineno;
	int	cb;

	struct 	kforth_label_usage *usage;
	struct kforth_label *next;
};

static struct kforth_label *LabelList;

static struct kforth_label *create_label(char *word)
{
	struct kforth_label *label, *lp;

	ASSERT( word != NULL );

	label = (struct kforth_label*) CALLOC(1, sizeof(struct kforth_label));
	ASSERT( label != NULL );
	label->name = STRDUP(word);

	if( LabelList == NULL ) {
		LabelList = label;
	} else {
		for(lp=LabelList; lp->next; lp=lp->next)
			;
		lp->next = label;
	}

	return label;
}

static struct kforth_label *lookup_label(char *word)
{
	struct kforth_label *label;

	ASSERT( word != NULL );

	for(label=LabelList; label; label=label->next) {
		if( stricmp(label->name, word) == 0 )
			break;
	}

	return label;
}

static void add_label_usage(struct kforth_label *label, int lineno, int cb, int pc)
{
	struct kforth_label_usage *usage, *up;

	ASSERT( label != NULL );

	usage = (struct kforth_label_usage*) CALLOC(1, sizeof(struct kforth_label_usage));
	ASSERT( usage != NULL );

	usage->lineno = lineno;
	usage->cb = cb;
	usage->pc = pc;

	if( label->usage == NULL ) {
		label->usage = usage;
	} else {
		for(up=label->usage; up->next; up=up->next)
			;

		up->next = usage;
	}
}

static void delete_labels(void)
{
	struct kforth_label *label, *label_next;
	struct kforth_label_usage *usage, *usage_next;

	for(label=LabelList; label; label=label_next) {
		for(usage=label->usage; usage; usage=usage_next) {
			usage_next = usage->next;
			FREE(usage);
		}
		label_next = label->next;
		FREE(label->name);
		FREE(label);
	}

	LabelList = NULL;
}

static void compile_opcode(KFORTH_PROGRAM *kfp, int cb, int pc, int opcode, KFORTH_INTEGER operand)
{
	int i;

	ASSERT( kfp != NULL );
	ASSERT( cb >= 0 );
	ASSERT( pc >= 0 );
	ASSERT( opcode >= 0 && opcode < KFORTH_OPS_LEN  );

	/*
	 * Grow program to fit code block 'cb'.
	 */
	if( cb >= kfp->nblocks ) {
		kfp->block_len = (int*) REALLOC(kfp->block_len, sizeof(int*)*(cb+1));

		kfp->opcode = (unsigned char**)
				REALLOC(kfp->opcode, sizeof(unsigned char*) * (cb+1));

		kfp->operand = (KFORTH_INTEGER**)
				REALLOC(kfp->operand, sizeof(KFORTH_INTEGER*) * (cb+1));

		for(i=kfp->nblocks; i<cb+1; i++) {
			kfp->block_len[i] = 0;
			kfp->opcode[i] = (unsigned char*) CALLOC(0, sizeof(unsigned char));
			kfp->operand[i] = (KFORTH_INTEGER*) CALLOC(0, sizeof(KFORTH_INTEGER));
		}
		kfp->nblocks = cb+1;
	}

	/*
	 * Grow code block
	 */
	if( pc >= kfp->block_len[cb] ) {
		kfp->block_len[cb] = pc+1;

		kfp->opcode[cb] = (unsigned char*)
				REALLOC(kfp->opcode[cb], sizeof(unsigned char) * (pc+1) );

		kfp->operand[cb] = (KFORTH_INTEGER*)
				REALLOC(kfp->operand[cb], sizeof(KFORTH_INTEGER) * (pc+1) );
	}

	/*
	 * Set opcode/operand at (cb, pc)
	 */
	kfp->opcode[cb][pc] = opcode;
	kfp->operand[cb][pc] = operand;
	
}

/***********************************************************************
 * Compile the kforth program in the string: 'program_text' 
 */
KFORTH_PROGRAM *kforth_compile(const char *program_text, KFORTH_OPERATIONS *kfops, char *errbuf)
{
	KFORTH_PROGRAM *kfp;
	int lineno, in_comment, error;
	int save_cb, cb, pc, sp, opcode;
	KFORTH_INTEGER operand;
	const char *p;
	char word[1000], *w;
	int cb_stack[100], pc_stack[100];
	struct kforth_label *label;
	struct kforth_label_usage *usage;
	int next_code_block;

	ASSERT( program_text != NULL );
	ASSERT( kfops != NULL );
	ASSERT( errbuf != NULL );

	kfp = (KFORTH_PROGRAM*) CALLOC(1, sizeof(KFORTH_PROGRAM));
	ASSERT( kfp != NULL );

	kfp->nblocks = 1;
	kfp->block_len = (int*) CALLOC(1, sizeof(int));
	kfp->block_len[0] = 0;
	kfp->opcode = (unsigned char**) CALLOC(1, sizeof(unsigned char*));
	kfp->operand = (KFORTH_INTEGER**) CALLOC(1, sizeof(KFORTH_INTEGER*));
	kfp->opcode[0] = (unsigned char*) CALLOC(0, sizeof(unsigned char));
	kfp->operand[0] = (KFORTH_INTEGER*) CALLOC(0, sizeof(KFORTH_INTEGER));
	kfp->kfops = kfops;

	next_code_block = 0;
	sp = 0;
	cb = -1;
	pc = 0;
	lineno = 1;
	in_comment = 0;
	error = 0;
	p = program_text;
	while( *p != '\0' && !error ) {
		if( isspace(*p) || in_comment ) {
			if( *p == '\n' ) {
				in_comment = 0;
				lineno++;
			}
			p++;

		} else if( *p == ';' ) {
			in_comment = 1;
			p++;

		} else if( *p == '{' ) {
			cb_stack[sp] = cb;
			pc_stack[sp] = pc;
			sp++;

			cb = next_code_block++;
			pc = 0;

			p++;

		} else if( *p == '}' ) {
			sp--;
			if( sp < 0 ) {
				sprintf(errbuf, "Line: %d, too many close braces", lineno);
				error = 1;
			} else if( sp > 0 ) {
				save_cb = cb;
				cb = cb_stack[sp];
				pc = pc_stack[sp];

#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
				/* compile relative code block number */
				compile_opcode(kfp, cb, pc, 0, save_cb - cb);
#else
				/* compile absolute code block number */
				compile_opcode(kfp, cb, pc, 0, save_cb);
#endif

				pc++;
			}
			p++;

		} else if( iscntrl(*p) ) {
			sprintf(errbuf, "Line: %d, invalid char %d", lineno, *p);
			error = 1;

		} else {
			w = word;
			while( is_kforth_word_char(*p) ) {
				*w++ = *p++;
			}
			*w = '\0';

			if( *p == ':' ) {
				opcode = lookup_opcode(kfops, word);
				if( opcode >= 0 ) {
					sprintf(errbuf, "Line: %d, label '%s' clashes with instruction",
						lineno, word);
					error = 1;

				} else if( is_kforth_operand(word) ) {
					sprintf(errbuf, "Line: %d, numbers cannot be a label '%s'",
						lineno, word);
					error = 1;

				} else {
					p++;
					label = lookup_label(word);
					if( label == NULL ) {
						label = create_label(word);
						label->cb = next_code_block;
						label->lineno = lineno;
					} else {
						if( label->lineno != -1 ) {
							/* error: mutliply define labels */
							sprintf(errbuf,
								"Line: %d, symbol '%s' multiply defined",
								lineno, word);
							error = 1;
						} else {
							label->lineno = lineno;
							label->cb = next_code_block;
						}
					}
				}

			} else if( sp == 0 ) {
				sprintf(errbuf, "Line: %d, '%s' appears outside of a code block",
						lineno, word);
				error = 1;

			} else {
				opcode = lookup_opcode(kfops, word);
				if( opcode >= 0 ) {
					compile_opcode(kfp, cb, pc, opcode, 0);

				} else if( is_kforth_operand(word) ) {
					operand = _atoi64(word);
					compile_opcode(kfp, cb, pc, 0, operand);

				} else {
					label = lookup_label(word);
					if( label == NULL ) {
						label = create_label(word);
						label->lineno = -1;
						label->cb = 0;
						add_label_usage(label, lineno, cb, pc);
						compile_opcode(kfp, cb, pc, 0, 0);
					} else {
						if( label->lineno == -1 ) {
							add_label_usage(label, lineno, cb, pc);
						} else {
#ifdef KFORTH_RELATIVE_ADDRESSING_MODE
							/* compile relative code block number */
							compile_opcode(kfp, cb, pc, 0, label->cb - cb);
#else
							/* compile absolute code block number */
							compile_opcode(kfp, cb, pc, 0, label->cb);
#endif
						}
					}
				}
				pc++;
			}
		}
	}

	if( error ) {
		delete_labels();
		kforth_delete(kfp);
		return NULL;
	}

	if( sp > 0 ) {
		delete_labels();
		kforth_delete(kfp);
		sprintf(errbuf, "Line: %d, missing close braces", lineno);
		return NULL;
	}

	/*
	 * Make sure all labels are defined
	 */
	for(label=LabelList; label; label=label->next) {
		if( label->lineno == -1 ) {
			lineno = label->usage->lineno;
			sprintf(errbuf, "Line: %d, undefined label '%s'", lineno, label->name);
			kforth_delete(kfp);
			delete_labels();
			return NULL;
		}

		/*
		 * back patch all usage instances of label
		 */
		for(usage=label->usage; usage; usage=usage->next) {
			cb = usage->cb;
			pc = usage->pc;
			kfp->opcode[cb][pc] = 0;

#ifdef KFORTH_RELATIVE_CB
			/* compile relative code block number */
			kfp->operand[cb][pc] = label->cb - cb;
#else
			kfp->operand[cb][pc] = label->cb;
#endif
		}
	}

	delete_labels();

	return kfp;
}

/***********************************************************************
 * Delete the KFORTH program 'kfp'
 */
void kforth_delete(KFORTH_PROGRAM *kfp)
{
	int cb;

	ASSERT( kfp != NULL );

	for(cb=0; cb < kfp->nblocks; cb++ ) {
		FREE( kfp->opcode[cb] );
		FREE( kfp->operand[cb] );
	}

	FREE( kfp->opcode );
	FREE( kfp->operand );
	FREE( kfp->block_len );

	FREE( kfp );
}

static void append_to_string(char **str, int *size, int *len, char *buf)
{
	int buflen, new_size;

	ASSERT( str != NULL );
	ASSERT( *str != NULL );
	ASSERT( *size >= 0 );
	ASSERT( *len >= 0 );
	ASSERT( buf != NULL );

	buflen = (int) strlen(buf);

	if( (size-len) <= buflen ) {
		new_size = *size + 1000;
		*str = (char*) REALLOC(*str, new_size);
		*size = new_size;
	}

	strcat(*str, buf);
	*len += buflen;

}

/***********************************************************************
 * Convert the program 'kfp' into a readable string.
 * And also create a table (KFORTH_DISASSEMBLY_POS) that maps
 * character offsets in the string to 'cb' and 'pc'.
 *
 * 'width' is the maximum column width to break code blocks at.
 * A value of '80', for example would break lines whenever the next
 * intruction to be added would exceed this width.
 *
 * If 'want_cr' is non-zero then we terminate lines with \r\n instead of
 * just \n. (this is needed when populating windows CEdit controls fuckors!!!).
 *
 * HOW TO USE POS TABLE:
 *	The 'pos' table allows the caller to map a (cb, pc) to text contained
 *	int program_text. Simply scan the table (from 0 .. pos_len) looking for
 *	a matching (cb, pc) pair. When such an entry is found, you have
 *	character offsets for that item.
 *
 *	When pc equals "code block length" then
 *	this entry stores the offset to the closing curly brace.
 *
 *	When pc is -1 then this entry points to the label for the code block.
 *
 *
 */
KFORTH_DISASSEMBLY *kforth_disassembly_make(KFORTH_PROGRAM *kfp, int width, int want_cr)
{
	char buf[1000];
	char buf2[1000];
	char *nlstr;
	int cb, pc, opcode;
	int max_opcode;
	int line_length, i, len;
	KFORTH_DISASSEMBLY *result;

	char *program;
	int plen, psize;

	ASSERT( kfp != NULL );
	ASSERT( width >= 20 );

	if( want_cr ) {
		nlstr = "\r\n";
	} else {
		nlstr = "\n";
	}

	result = (KFORTH_DISASSEMBLY*) CALLOC(1, sizeof(KFORTH_DISASSEMBLY));
	ASSERT( result != NULL );

	result->program_text = NULL;

	/*
	 * Compute length of array
	 */
	result->pos_len = kfp->nblocks;
	for(cb=0; cb < kfp->nblocks; cb++) {
		result->pos_len += kfp->block_len[cb] + 1;
	}

	result->pos = (KFORTH_DISASSEMBLY_POS*) CALLOC(result->pos_len, sizeof(KFORTH_DISASSEMBLY_POS));
	ASSERT( result->pos != NULL );

	max_opcode = kforth_ops_max_opcode(kfp->kfops);

	psize = 1000;
	program = (char*) CALLOC(psize, sizeof(char));
	plen = 0;
	program[0] = '\0';
	i = 0;
	for(cb=0; cb < kfp->nblocks; cb++) {
		if( cb == 0 ) {
			sprintf(buf, "main");
		} else {
			sprintf(buf, "row%d", cb);
		}

		len = (int) strlen(buf);

		result->pos[i].cb	= cb;
		result->pos[i].pc	= -1;
		result->pos[i].start_pos= plen;
		result->pos[i].end_pos	= plen + len - 1;
		i++;

		sprintf(buf2, "%s:%s{%s    ", buf, nlstr, nlstr);
		append_to_string(&program, &psize, &plen, buf2);

		line_length = 4;
		for(pc=0; pc < kfp->block_len[cb]; pc++) {

			if( line_length >= width ) {

				sprintf(buf, "%s    ", nlstr);
				line_length = 4;

				append_to_string(&program, &psize, &plen, buf);
			}

			strcpy(buf, "  ");
			line_length += 2;
			append_to_string(&program, &psize, &plen, buf);

			opcode = kfp->opcode[cb][pc];
			if( opcode == 0 ) {
#ifndef __linux__
				sprintf(buf, "%I64d", kfp->operand[cb][pc]);
#else
				sprintf(buf, "%lld", kfp->operand[cb][pc]);
#endif

			} else if( opcode <= max_opcode ) {
				sprintf(buf, "%s", kfp->kfops[opcode].name);

			} else {
				ASSERT(0);
			}

			len = (int) strlen(buf);
			line_length += len;

			result->pos[i].cb	= cb;
			result->pos[i].pc	= pc;
			result->pos[i].start_pos= plen;
			result->pos[i].end_pos	= plen + len - 1;
			i++;

			append_to_string(&program, &psize, &plen, buf);
		}

		/*
		 * We are at the end of a code block, add closing '}' and add entry.
		 */
		sprintf(buf, " %s", nlstr);
		append_to_string(&program, &psize, &plen, buf);

		sprintf(buf, "}");
		len = (int) strlen(buf);

		result->pos[i].cb	= cb;
		result->pos[i].pc	= pc;
		result->pos[i].start_pos= plen;
		result->pos[i].end_pos	= plen + len - 1;
		i++;

		append_to_string(&program, &psize, &plen, buf);

		sprintf(buf, "%s%s", nlstr, nlstr);
		append_to_string(&program, &psize, &plen, buf);
	}

	ASSERT( i == result->pos_len );

	result->program_text = program;

	return result;
}

/***********************************************************************
 * Free a KFORTH_DISASSEMBLY object that was created from
 * 'kforth_disassembly_make()'
 *
 *
 */
void kforth_disassembly_delete(KFORTH_DISASSEMBLY *kfd)
{
	ASSERT( kfd != NULL );

	FREE(kfd->program_text);
	FREE(kfd->pos);
	FREE(kfd);
}


/*
 * return total number of instructions and numbers
 * that comprise the program 'kfp'.
 */
int kforth_program_length(KFORTH_PROGRAM *kfp)
{
	int i, len;

	ASSERT( kfp != NULL );

	len = 0;
	for(i=0; i < kfp->nblocks; i++) {
		len += kfp->block_len[i];
	}

	return len;
}

/*
 * Return memory size of 'kfp'.
 */
int kforth_program_size(KFORTH_PROGRAM *kfp)
{
	int i, size;

	ASSERT( kfp != NULL );

	size = 0;

	size += sizeof(KFORTH_PROGRAM);
	size += kfp->nblocks * sizeof(int);
	size += kfp->nblocks * sizeof(KFORTH_INTEGER*);
	size += kfp->nblocks * sizeof(unsigned char*);
	for(i=0; i < kfp->nblocks; i++) {
		size += kfp->block_len[i] * sizeof(KFORTH_INTEGER);
		size += kfp->block_len[i] * sizeof(unsigned char);
	}

	return size;
}

