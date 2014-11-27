/*
 * Copyright (c) 1994 Photon Systems, All Rights Reserved
 */

/***********************************************************************
 * Photon Ascii READER module.
 */
#include "stdafx.h"

#include <setjmp.h>

#ifndef TRUE
#define TRUE			1
#endif

#ifndef FALSE
#define FALSE			0
#endif

#ifndef BUFSIZ
#define BUFSIZ			1024
#endif

#define arraylen(x)		(sizeof(x)/sizeof(x[0]))

extern char *strdup();


typedef struct dlist {
        void*           p;
        struct dlist*   prev;
        struct dlist*   next;
} DLIST;

typedef void (*DlistFreeProcType)(void*);
typedef int (*DlistCmpProcType)(void*, void*);

enum {
	DEF_STRUCT,
	DEF_ARRAY,
	DEF_COMPONENT
};

typedef struct {
	char		*name;
	int		type;
	DLIST		*components;
	char		*index;
	char		*default_value;
} DEFINITION;

typedef struct _instance {
	int n;
	union {
		struct _instance *components;
		struct _instance *vec;
		char	*data;
	} u;
	struct _instance *next;
} INSTANCE;

typedef struct {
	char		filename[ BUFSIZ ];
	FILE		*fp;
	DLIST		*definitions;
	DLIST		*old_definitions;
	int		lineno;
	int		ungettoken;
	int		token;
	char		tokenbuf[ BUFSIZ ];
	char		error[ BUFSIZ ];
} CONFIG_FILE;

typedef struct {
	CONFIG_FILE	*cf;
	DEFINITION	*definition;
	INSTANCE	*instance;
} CONFIG_INSTANCE;

#define	T_STRING	300
#define T_TOKEN		301
#define T_EOF		302
#define T_IDENT		303
#define T_STRUCT	304
#define T_INTEGER	305

#define ET_ARG		300
#define ET_INTEGER	301
#define ET_IDENT	302
#define ET_EOF		303

static struct {
	char *name;
	int token;
} keywords[] = {
	{"struct",	T_STRUCT},
};

static DEFINITION *parse_definition(CONFIG_FILE *, char *);

static int etoken_arg;
static char etoken_ident[ BUFSIZ ];

static char cf_errorbuf[ BUFSIZ ];
static jmp_buf jmpbuf;


/**********************************************************************
 * Creates an allocated DLIST structure.
 *
 */
static DLIST *dlist_new(void *p)
{
	DLIST *newp;

	newp = (DLIST *) malloc( sizeof(DLIST) );

	if( newp == NULL) {
		return NULL;
	}

	newp->p = p;
	newp->prev = NULL;
	newp->next = NULL;
	return newp;
}

/**********************************************************************
 * Adds data to the end of the list.
 * The newly added element is returned. NULL on error.
 *
 */
static DLIST *dlist_add(DLIST **head, void *p)
{
	DLIST *curr;

	if( *head == NULL ) {
		*head = dlist_new(p);
		return *head;
	} else {
		for (curr = *head; curr->next; curr = curr->next)
		    ;

		curr->next = dlist_new(p);
		if( curr->next == NULL )
			return NULL;
		curr->next->prev = curr;
		return curr->next;
	}
}

/**********************************************************************
 * Frees the whole list.
 */
static void dlist_destroy(DLIST **head, void (*destroy)(void*))
{
	DLIST *curr, *tmp;

	if( *head == NULL )
		return;

	if( (*head)->next == NULL ) {
		if(destroy)
			(*destroy)((*head)->p);
		free(*head);
	} else {
		curr = *head;
		while( curr ) {
			curr = (tmp=curr)->next;
			if(destroy)
				(*destroy)(tmp->p);
			free(tmp);
		}
	}
	*head = NULL;
}

/**********************************************************************
 *	Removes data from the list.
 *
 */
static DLIST *dlist_delete(DLIST **head, void* p, void (*destroy)(void*),
							int (*comp)(void*, void*))
{
	DLIST *curr;
	DLIST *rtn;

	if (*head == NULL)
		return NULL;

	if(comp) {
		if( (*comp)(p, (*head)->p) == 0 ) {
			if( destroy )
				(*destroy)((*head)->p);
			rtn = (*head)->next;
			free(*head);

			if(rtn)
				rtn->prev = NULL;

			*head = rtn;
			return rtn;
		}

		for(curr=*head; curr; curr=curr->next) {
			if( (*comp)(p, curr->p) == 0 ) {
				curr->prev->next = curr->next;
			    
				if (curr->next)
					curr->next->prev = curr->prev;
			    
				rtn = curr->next;
				if( destroy )
					(*destroy)(curr->p);
				free(curr);
				return rtn;
			}
		}
		return NULL;
	}

	if( (*head)->p == p) {
		if( destroy )
			(*destroy)((*head)->p);
		rtn = (*head)->next;
		free(*head);

		if(rtn)
			rtn->prev = NULL;

		*head = rtn;
		return rtn;
	}

	for(curr=*head; curr; curr=curr->next) {
		if(curr->p == p) {
			curr->prev->next = curr->next;

			if (curr->next)
				curr->next->prev = curr->prev;

			rtn = curr->next;
			if( destroy )
				(*destroy)(curr->p);
			free(curr);
			return rtn;
		}
	}
	return NULL;
}


/**********************************************************************
 * Search the list for an element. Return the the DLIST pointer
 * to the element found. If the compare function is NULL, then
 * the element whose has the same data pointer as 'p' is
 * returned.
 *
 */
static DLIST *dlist_search(DLIST *head, void *p, int (*comp)(void*, void*))
{
	DLIST *curr;

	if( comp ) {
		for(curr=head; curr; curr=curr->next)
			if( (*comp)(p,curr->p) == 0 )
				return curr;
	} else {
		for(curr=head; curr; curr=curr->next)
			if(p == curr->p)
				return curr;
	}

	return NULL;

}

/**********************************************************************
 * Count the number of elements in the list 'head'.
 *
 */
static int dlist_count(DLIST *head)
{
	int i;

	i = 0;
	while( head ) {
		head = head->next;
		i++;
	}

	return i;
}

/***********************************************************************
 * Format an error into the cf_errorbuf buffer, and
 * long jump to a known place.
 *
 * If the first character of fmt is '*', it is a warning.
 */
static void error(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsprintf(cf_errorbuf, fmt, ap);
	va_end(ap);

	longjmp(jmpbuf, 1);
}

static INSTANCE *new_inst(void)
{
	INSTANCE *inst;

	inst = (INSTANCE*) malloc( sizeof(INSTANCE) );
	if( inst == NULL )
		error("out of memory");

	inst->u.components = NULL;
	inst->next = NULL;
	return inst;
}

static void add_inst_components(INSTANCE *inst, INSTANCE *i)
{
	INSTANCE *curr;

	for(curr=inst->u.components; curr; curr=curr->next) {
		if( curr->next == NULL ) {
			curr->next = i;
			return;
		}
	}
	inst->u.components = i;
}

static void free_definition(DEFINITION *def)
{
	dlist_destroy(&def->components, (DlistFreeProcType)free_definition);
	if( def->index )
		free(def->index);
	if( def->default_value )
		free(def->default_value);
	free(def->name);
	free(def);
}

static void free_instance(DEFINITION *def, INSTANCE *inst)
{
	INSTANCE *i, *nxt;
	DEFINITION newd, *d;
	DLIST *curr;
	int j;

	switch(def->type) {
	case DEF_STRUCT:
		i = inst->u.components;
		for(curr=def->components; curr; curr=curr->next) {
			nxt = i->next;
			free_instance((DEFINITION*)curr->p, i);
			i = nxt;
		}
		free(inst);
		break;

	case DEF_ARRAY:
		d = (DEFINITION*)def->components->p;
		if( !strcmp(d->name, "[]") ) {
			newd = *d;
		} else {
			newd = *def;
			newd.type = DEF_STRUCT;
		}
		for(j=0; j<inst->n; j++) {
			i = new_inst();
			*i = inst->u.vec[j];
			free_instance(&newd, i);
		}
		free(inst->u.vec);
		free(inst);
		break;

	case DEF_COMPONENT:
		free(inst->u.data);
		free(inst);
		break;
	}
}

static int ident_cmp(char *a, char *b)
{
	return stricmp(a, b);
}

/************************************************************************/
static char *get_substrings(char *p, DLIST **ss)
{
	char buf[ BUFSIZ ];
	char *q;
	int brace;

	/* skip '{' */
	p++;

	for(;;) {
		q = buf;
		brace = 0;
		while( (*p && *p != ',' && *p != '}') || brace ) {
			if( *p == '{' )
				brace++;
			if( *p == '}' )
				brace--;
			*q++ = *p++;
		}
		*q = '\0';

		q = strdup(buf);
		if( q == NULL )
			return NULL;
		dlist_add(ss, q);

		if( *p == '}' )
			break;
		else if( *p == ',' )
			p++;
		else
			break;
	}

	if( *p == '\0' )
		return NULL;

	return p+1;
}

/***********************************************************************
 * This function expands the string 'string' into many pieces.
 * It looks for the special syntax '{... , ... , ...}'
 * and builds new strings when this is encountered.
 *
 *	"BLAH.{A,B,CX}"
 *
 * becomes list:
 *
 *	[ "BLAH.A", "BLAH.B", "BLAH.CX" ]
 *
 * curly braces cannot be nested
 *
 * RETURNS:
 *	The number of expansions.
 *	0 syntax error, or empty expansion. "{}"
 *	-1 out of memory.
 */
int expand_strings(char *string, DLIST **result)
{
	char *p, *q;
	char head[ BUFSIZ ];
	char tail[ BUFSIZ ];
	char buf[ BUFSIZ ];
	DLIST *expansions, *ss, *newp, *curr, *curr2;
	int added;

	*result = NULL;

	expansions = NULL;
	p = strdup(string);
	if( p == NULL )
		return -1;
	dlist_add(&expansions, p);

	do {
		newp = NULL;
		added = FALSE;
		for(curr=expansions; curr; curr=curr->next) {
			p=(char*)curr->p;
			q = head;
			while( *p && *p != '{' ) {
				*q++ = *p++;
				continue;
			}
			*q = '\0';

			if( *p != '{' ) {
				dlist_add(&newp, strdup((char*)curr->p));
				continue;
			}

			ss = NULL;
			p = get_substrings(p, &ss);
			if( p == NULL )
				return 0;
			q = tail;
			while( *p ) {
				*q++ = *p++;
			}
			*q = '\0';
			for(curr2=ss; curr2; curr2=curr2->next) {
				sprintf(buf, "%s%s%s", head, (char*)curr2->p, tail);
				p = strdup(buf);
				if( p == NULL )
					return -1;
				dlist_add(&newp, p);
			}
			dlist_destroy(&ss, (DlistFreeProcType) free);
			added = TRUE;
		}
		dlist_destroy(&expansions, (DlistFreeProcType) free);
		expansions = newp;
	} while( added );

	*result = expansions;

	return dlist_count(*result);
}

static int is_string(int c)
{
	if( c == '\t' )
		return TRUE;

	if( iscntrl(c) )
		return FALSE;

	if( c == '"' )
		return FALSE;

	if( c < 0 )
		return FALSE;

	return TRUE;
}

static int is_token(int c)
{
	if( iscntrl(c) )
		return FALSE;

	if( isspace(c) )
		return FALSE;

	if( strchr("#{}[]=", c) )
		return FALSE;

	if( c < 0 )
		return FALSE;

	return TRUE;
}

/*
 * Determine if the string is an identifier.
 */
static int is_ident(char *tok)
{
	char *p;

	if( !isalpha(tok[0]) )
		return FALSE;

	for(p=tok+1; *p; p++) {
		if( !isalnum(*p) && strchr("_-", *p)==NULL )
			return FALSE;
	}
	return TRUE;
}

/*
 * Determine if the string is all digites.
 */
static int is_integer(char *tok)
{
	char *p;

	for(p=tok; *p; p++) {
		if( !isdigit(*p) )
			return FALSE;
	}
	return TRUE;
}

static void ungettoken(CONFIG_FILE *cf, int t)
{
	cf->ungettoken = TRUE;
	cf->token = t;
}

/***********************************************************************
 * Get a token from the input stream.
 *
 * RETURNS:
 *	<token>			<tokenbuf>
 *	T_STRING		"a thing in quotes"
 *	T_IDENT			token forms a valid identifier.
 *	T_INTEGER		a string of all digits.
 *	T_TOKEN			a token that is not an IDENT or INTEGER
 *	T_EOF			nothing. ("EOF")
 *	'{'			{
 *	'}'			}
 *	T_STRUCT		struct
 *	'['			[
 *	']'			]
 */
static int gettoken(CONFIG_FILE *cf)
{
	char buf[ 2000 ], *p;
	int c, i, count;

	if( cf->ungettoken ) {
		cf->ungettoken = FALSE;
		return cf->token;
	}

	for(;;) {
		switch( c = fgetc(cf->fp) ) {
		case -1:
			strcpy(cf->tokenbuf, "EOF");
			return T_EOF;

		case '#':
			while( (c=fgetc(cf->fp)) != EOF ) {
				if( c == '\n' ) {
					cf->lineno++;
					break;
				}
			}
			break;

		case '\n':
			cf->lineno++;
			break;

		case '\014':	/* ^L */
		case '\015':	/* CR */
		case '\t':	/* tab */
		case ' ':	/* space */
			break;

		case '=':
		case '{':
		case '}':
		case '[':
		case ']':
			cf->tokenbuf[0] = c;
			cf->tokenbuf[1] = '\0';
			return c;

		case '"':
			p = buf;
			while( is_string(c=fgetc(cf->fp)) ) {
				if( p - buf > sizeof(buf) )
					error("string too large");

				if( c != '\\' ) {
					*p++ = c;
					continue;
				}
				switch( (c=fgetc(cf->fp)) ) {
				case -1:
					error("EOF in string");

				case '"': *p++ = '"'; break;
				case '\'': *p++ = '\''; break;
				case 'a': *p++ = '\a'; break;
				case 'b': *p++ = '\b'; break;
				case 'f': *p++ = '\f'; break;
				case 'r': *p++ = '\r'; break;
				case 'v': *p++ = '\v'; break;
				case 'n': *p++ = '\n'; break;
				case 't': *p++ = '\t'; break;
				case '\\': *p++ = '\\'; break;
				default:
				   if( c >= '0' && c <= '7' ) {
					count = 0;
					i = 0;
					do {
						i <<= 3;
						i += c-'0';
						count++;
						c = fgetc(cf->fp);
					} while(c >= '0' && c <= '7' && count < 3);
					ungetc(c, cf->fp);
					if( i == 0 )
						error("null not allowed in string");
					*p++ = i;
				   } else
					error("invalid escape sequence in string");
				}
			}
			*p = '\0';
			if( c == -1 ) error("EOF in string");
			if( c != '"' ) error("invalid character in string");
			strcpy(cf->tokenbuf, buf);
			return T_STRING;

		default:
			p = buf;
			*p++ = c;
			while( is_token((c=fgetc(cf->fp))) ) {
				if( p - buf > sizeof(buf) )
					error("token too large");
					*p++ = c;
			}
			*p = '\0';
			strcpy(cf->tokenbuf, buf);

			if( c != -1 )
				ungetc(c, cf->fp);

			/* check to see if token is a keyword */
			for(i=0; i<arraylen(keywords); i++) {
				if( !ident_cmp(keywords[i].name, cf->tokenbuf) )
					return keywords[i].token;
			}

			if( is_ident(cf->tokenbuf) )
				return T_IDENT;

			if( is_integer(cf->tokenbuf) )
				return T_INTEGER;

			return T_TOKEN;
		}
	}
}

/**********************************************************************
 * GRAMMAR FOR A STRUCTURE DEFINITION:
 *
 *	base-definition	==>	  STRUCT definition
 *
 *	definition	==>	  IDENT { component-list }
 *				| IDENT [ indexes ] { component-list }
 *
 *	component-list	==>	  component
 *				| component-list component
 *
 *	indexes		==>	  IDENT
 *				| indexes IDENT
 *
 *	component	==>	  IDENT
 *				| IDENT = data-token
 *				| definition
 *	
 *	data-token	==>	  IDENT
 *				| STRING
 *				| INTEGER
 *				| TOKEN
 *
 * Examples,
 *
 *
 * struct FOO {
 *	NAME
 * }
 *
 * struct EMPLOYEE {
 *	FIRST_NAME
 *	MIDDLE_NAME
 *	LAST_NAME
 * }
 *
 * struct POLYGON {
 *	COLORS[N] {
 *		RED
 *		GREEN
 *		BLUE
 *	}
 *
 *	POINT_LIST[N] {
 *		POINT {
 *			X Y Z
 *		}
 *	}
 * }
 *
 * struct COORDINATES[N] {
 *	X
 *	Y
 *	FLAG = "off"
 *	VERSION = 2.0
 *	PIXEL_OPS[N M] {
 *		XOR AND NOR OR
 *	}
 * }
 *
 *
 *
 */

static DEFINITION *search_def_list(DLIST *head, char *name)
{
	DLIST *curr;

	for(curr=head; curr; curr=curr->next)
		if( !ident_cmp(name, ((DEFINITION*)curr->p)->name) )
			return (DEFINITION*)curr->p;

	return NULL;
}

/***********************************************************************
 * The '[' has been parsed, this function parses a list of index
 * variables, followed by the closing ']'.
 *
 */
static DLIST *parse_indexes(CONFIG_FILE *cf)
{
	int t;
	DLIST *head, *curr;

	head = NULL;

	for(;;) {
		t = gettoken(cf);
		if( t != T_IDENT )
			break;

		curr = dlist_search(head, cf->tokenbuf, (DlistCmpProcType)ident_cmp);
		if( curr )
			error("duplicate index name %s", cf->tokenbuf);

		dlist_add(&head, strdup(cf->tokenbuf));
	}

	if( t != ']' )
		error("'%s' unexpected in index list", cf->tokenbuf);

	if( head == NULL )
		error("empty index list [] is illegal");

	return head;
}

/***********************************************************************
 * Once the initial '{' is parsed, this function is called to
 * parse the componen-list and the ending '}'.
 *
 */
static DLIST *parse_component_list(CONFIG_FILE *cf)
{
	int t;
	char component_name[ BUFSIZ ];
	DEFINITION *def, *d;
	DLIST *head;

	head = NULL;

	for(;;) {
		t = gettoken(cf);
		if( t != T_IDENT )
			break;

		strcpy(component_name, cf->tokenbuf);

		t = gettoken(cf);
		if( t == T_IDENT || t == '}' ) {
			ungettoken(cf, t);

			d = search_def_list(head, component_name);
			if( d )
				error("%s duplicate component", component_name);

			def = (DEFINITION *)malloc( sizeof(DEFINITION) );
			if( def == NULL )
				error("out of memory");
			def->name = strdup(component_name);
			def->type = DEF_COMPONENT;
			def->components = NULL;
			def->index = NULL;
			def->default_value = NULL;
			dlist_add(&head, def);

		} else if( t == '=' ) {
			/* parse a data-token */
			t = gettoken(cf);
			if( t == T_STRING || t == T_TOKEN || t == T_IDENT
					|| t == T_INTEGER ) {
				d = search_def_list(head, component_name);
				if( d )
					error("%s duplicate component", component_name);

				def = (DEFINITION *)malloc( sizeof(DEFINITION) );
				if( def == NULL )
					error("out of memory");
				def->name = strdup(component_name);
				def->type = DEF_COMPONENT;
				def->components = NULL;
				def->index = NULL;
				def->default_value = strdup(cf->tokenbuf);
				dlist_add(&head, def);
			} else
				error("'%s' unexpected after =", cf->tokenbuf);

		} else if( t == '[' || t == '{' ) {
			ungettoken(cf, t);
			def = parse_definition(cf, component_name);
			def->name = strdup(component_name);
			dlist_add(&head, def);

		} else
			break;
	}

	if( t != '}' )
		error("syntax error in definition");

	if( head == NULL )
		error("empty structure { } is illegal");

	return head;
}

/***********************************************************************
 * Once an identifier for a structure is parsed, this
 * functions parses the rest of the definition.
 */
static DEFINITION *parse_definition(CONFIG_FILE *cf, char *struct_name)
{
	int t;
	DEFINITION *def, *d, *save_def;
	DLIST *clist;
	DLIST *index_list, *curr;

	t = gettoken(cf);
	if( t == '{' ) {
		/*
		 * non-array structure.
		 */
		clist = parse_component_list(cf);

		def = (DEFINITION*) malloc( sizeof(DEFINITION) );
		if( def == NULL )
			error("out of memory");
		def->type = DEF_STRUCT;
		def->default_value = NULL;
		def->components = clist;
		def->index = NULL;

	} else if( t == '[' ) {
		/*
		 * Array structure.
		 */
		index_list = parse_indexes(cf);
		t = gettoken(cf);
		if( t != '{' )
			error("syntax error in structure %s", struct_name);

		clist = parse_component_list(cf);

		save_def = def = (DEFINITION*) malloc( sizeof(DEFINITION) );
		if( def == NULL )
			error("out of memory");
		def->type = DEF_ARRAY;
		def->default_value = NULL;
		def->components = NULL;
		def->index = (char*)index_list->p;

		for(curr=index_list->next; curr; curr=curr->next) {
			d = (DEFINITION*) malloc( sizeof(DEFINITION) );
			if( d == NULL )
				error("out of memory");
			d->name = strdup("[]");
			d->type = DEF_ARRAY;
			d->default_value = NULL;
			d->components = NULL;
			d->index = (char*)curr->p;
			dlist_add(&def->components, d);
			def = d;
		}
		def->components = clist;
		dlist_destroy(&index_list, NULL);

		def = save_def;

	} else
		error("unexpected %s in structure definition for %s",
				cf->tokenbuf, struct_name);

	return def;
}

/***********************************************************************
 * The initial "struct" keyword has been parsed. This
 * function parses the remainder of the definition.
 */
static DEFINITION *parse_base_definition(CONFIG_FILE *cf)
{
	int t;
	char struct_name[ BUFSIZ ];
	DEFINITION *def;

	t = gettoken(cf);
	if( t != T_IDENT )
		error("%s unexpected after 'struct' keyword", cf->tokenbuf);

	strcpy(struct_name, cf->tokenbuf);

	def = parse_definition(cf, struct_name);
	def->name = strdup(struct_name);

	return def;
}

static char *parse_data(CONFIG_FILE *cf)
{
	int t;

	t = gettoken(cf);

	if( t == T_TOKEN || t == T_IDENT || t == T_STRING || t == T_INTEGER ) {
		return strdup(cf->tokenbuf);

	} else if( t == T_STRUCT ) {
		error("struct keyword unexpected. (use quotes to escape 'struct' keyword)");
		return NULL; /* NOTREACHED */

	} if( t == T_EOF ) {
		error("premature EOF");
		return NULL; /* NOTREACHED */

	} else {
		error("syntax error");
		return NULL; /* NOTREACHED */
	}

}

/*
 * Multiple dimensional arrays are supported. The '{'
 * only works for the FIRST dimension.
 */
static INSTANCE *parse_instance(CONFIG_FILE *cf, DEFINITION *def,
					DLIST **count_head, DLIST *cp)
{
	DLIST *curr, *head;
	DLIST *chead;
	INSTANCE *i, *inst, *ia;
	DEFINITION newd, *d;
	char *p;
	short count;
	int j, t;

	switch( def->type ) {
	case DEF_STRUCT:
		inst = new_inst();
		for(curr=def->components; curr; curr=curr->next) {
			i = parse_instance(cf, (DEFINITION*)curr->p, count_head, NULL);
			add_inst_components(inst, i);
		}
		return inst;

	case DEF_ARRAY:
		if( cp == NULL )
			t = gettoken(cf);
		else
			t = T_INTEGER;

		if( t == T_INTEGER ) {
			if( cp == NULL ) {
				count = atoi(cf->tokenbuf);
				cp = dlist_add(count_head, (void*)count);
			} else
				count = (short)cp->p;

			d = (DEFINITION*)def->components->p;
			if( !strcmp(d->name, "[]") )
				newd = *d;
			else {
				newd = *def;
				newd.type = DEF_STRUCT;
			}
			inst = new_inst();
			inst->n = count;
			ia = (INSTANCE*)malloc( count * sizeof(INSTANCE) );
			inst->u.vec = ia;

			for(j=0; j<count; j++) {
				i = parse_instance(cf, &newd, count_head, cp->next);
				ia[j] = *i;
				free(i);
			}
			return inst;
		} else if( t == '{' ) {
			d = (DEFINITION*)def->components->p;
			if( !strcmp(d->name, "[]") )
				newd = *d;
			else {
				newd = *def;
				newd.type = DEF_STRUCT;
			}
			inst = new_inst();

			head = NULL;
			chead = NULL;
			for(;;) {
				t = gettoken(cf);
				if( t == '}' )
					break;
				else
					ungettoken(cf, t);

				i = parse_instance(cf, &newd, &chead, chead);
				dlist_add(&head, i);
			}
			dlist_destroy(&chead, NULL);
			count = dlist_count(head);
			dlist_add(count_head, (void*)count);
			inst->n = count;
			ia = (INSTANCE*)malloc( count * sizeof(INSTANCE) );
			inst->u.vec = ia;

			for(j=0, curr=head; j<count; j++, curr=curr->next)
				ia[j] = *(INSTANCE*)curr->p;
			dlist_destroy(&head, (DlistFreeProcType)free);
			return inst;
		} else
			error("Missing array count or {");
		return NULL;

	case DEF_COMPONENT:
		inst = new_inst();
		if( def->default_value ) {
			inst->u.data = strdup(def->default_value);
		} else {
			p = parse_data(cf);
			inst->u.data = p;
		}
		return inst;

	default:
		return NULL;
	}
}

/***********************************************************************
 * Parse a instance of a definition.
 *
 */
static CONFIG_INSTANCE *parse_base_instance(CONFIG_FILE *cf)
{
	int t;
	CONFIG_INSTANCE *ci;
	DEFINITION *def;
	DLIST *count_head;

	t = gettoken(cf);
	if( t != T_IDENT )
		error("structure name expected");

	def = search_def_list(cf->definitions, cf->tokenbuf);
	if( def == NULL )
		error("instance %s has not been defined", cf->tokenbuf);

	ci = (CONFIG_INSTANCE*) malloc( sizeof(CONFIG_INSTANCE) );
	if( ci == NULL )
		error("out of memory");

	ci->cf = cf;
	ci->definition = def;

	count_head = NULL;
	ci->instance = parse_instance(cf, def, &count_head, NULL);
	dlist_destroy(&count_head, NULL);

	return ci;
}

/***********************************************************************
 *	etoken		etoken_ident		etoken_arg
 *	ET_IDENT	the identifier
 *	ET_ARG					argument number
 *	ET_INTEGER				hardcoded number
 *	'.'
 *	'['
 *	']'
 */
static int expr_gettoken(char **string)
{
	char *p, *q;
	int c, i;

	p = *string;

	for(;;) {
		switch( (c = *p) ) {
		case '\0':
			return ET_EOF;

		case ' ':
		case '\t':
			p++;
			break;

		case '%':
			p++;
			i = 0;
			while( isdigit(*p) ) {
				i *= 10;
				i += (*p)-'0';
				p++;
			}
			etoken_arg = i;
			*string = p;
			return ET_ARG;

		case '.':
		case '[':
		case ']':
			*string = p+1;
			return c;

		default:
			q = etoken_ident;
			while( (isalnum(*p) || strchr("_-", *p)) && *p )
				*q++ = *p++;
			*q = '\0';

			if( is_ident(etoken_ident) ) {
				*string = p;
				return ET_IDENT;
			} else if( is_integer(etoken_ident) ) {
				etoken_arg = atoi(etoken_ident);
				*string = p;
				return ET_INTEGER;
			} else
				error("invalid identifier %s", etoken_ident);
		}
	}
}

/***********************************************************************
 * GRAMMAR FOR EXPRESSIONS:
 *	An expression is presented by the caller of the Phascii_Get()
 *	routine to extract data from an instance.
 *	eg.
 *	IDENT [ %0 ][ %1 ][ %2 ] . IDENT
 *	IDENT [ 12 ] . IDENT
 *	IDENT . IDENT . IDENT
 *
 *        expression      ==>       IDENT expr-tail
 *
 *        expr-tail       ==>       . IDENT
 *                                | . IDENT expr-tail
 *                                | [ ARG ] expr-tail
 *                                | [ INTEGER ] expr-tail
 */

/***********************************************************************
 * This function parses an expression string, and returns
 * the data item index.
 * If an error in parses occurs, an error string is returned.
 *
 * RETURNS:
 *	>= 0	- got array count.
 *	< 0	- got datum string.
 */
static int parse_expression(char **datum, CONFIG_INSTANCE *ci,
							char *expr, int *args)
{
	char *p;
	DEFINITION *def, *d;
	INSTANCE *inst, *i;
	int array_count, idx, got_array, t, type, found;
	DLIST *curr;
	INSTANCE *curr2;

	p = expr;

	t = expr_gettoken(&p);
	if( t != ET_IDENT )
		error("syntax error");

	if( ident_cmp(etoken_ident, ci->definition->name) )
		error("identifier %s does not match instance name", etoken_ident);

	def = ci->definition;
	inst = ci->instance;

	got_array = FALSE;
	array_count = -1;

	for(;;) {
		t = expr_gettoken(&p);
		if( t == '.' ) {
			t = expr_gettoken(&p);
			if( t != ET_IDENT )
				error("syntax error");

			type = (got_array) ? DEF_STRUCT : def->type;
			got_array = FALSE;

			switch( type ) {
			case DEF_STRUCT:
				/* search component list for name */
				curr=def->components;
				curr2=inst->u.components;
				found = FALSE;
				while( curr ) {
					d = (DEFINITION*)curr->p;
					i = curr2;
					if( !ident_cmp(etoken_ident, d->name) ) {
						found = TRUE;
						break;
					}
					curr=curr->next;
					curr2=curr2->next;
				}
				/*
				 * This error is not that serious, because
				 * the input file may not have a complete
				 * schema.
				 */
				if( !found ) {
					sprintf(cf_errorbuf, "*no such component %s.%s",
						def->name, etoken_ident);
					return -2; /* invalid component */
				}

				def = d;
				inst = i;
				break;

			case DEF_ARRAY:
				d = def;
				i = inst;
				found = FALSE;
				for(;;) {
					if( !ident_cmp(etoken_ident, d->index) ) {
						found = TRUE;
						break;
					}
					d = (DEFINITION*)(d->components->p);
					if( strcmp(d->name, "[]") != 0 )
						break;
					i = &(i->u.vec[0]);
				}
				if( !found ) {
					error("no such index %s.%s",
						def->name, etoken_ident);
				} else {
					array_count = i->n;
					t = expr_gettoken(&p);
					if( t != ET_EOF )
						error("syntax error");
					goto out;
				}
				break;

			case DEF_COMPONENT:
				error("cannot apply '.%s' to %s",
						etoken_ident, def->name);
			}

		} else if( t == '[' ) {
			if( def->type != DEF_ARRAY )
				error("illegal use of '[]' on %s", def->name);

			t = expr_gettoken(&p);
			if( t == ET_ARG )
				idx = args[ etoken_arg ];
			else if( t == ET_INTEGER )
				idx = etoken_arg;
			else
				error("syntax error in '%s[]'", def->name);

			t = expr_gettoken(&p);
			if( t != ']' )
				error("syntax error in '%s[]'", def->name);

			if( idx >= inst->n )
				error("%s[%d] exceeds array size of %d",
					def->name, idx, inst->n);

			inst = &inst->u.vec[ idx ];

			d = (DEFINITION*)def->components->p;
			if( !strcmp(d->name, "[]") )
				def = d;

			got_array = TRUE;
		} else if( t == ET_EOF )
			break;
		else
			error("syntax error");
	}
out:
	if( array_count >= 0 )
		return array_count;

	if( def->type == DEF_ARRAY )
		error("illegal evaluation to array");

	if( def->type == DEF_STRUCT )
		error("illegal evaluation to a structure");

	/*
	 * Fetch the data, and return.
	 */
	*datum = inst->u.data;
	return -1;
}

/***********************************************************************
 * Check that the two words: "PHOTON" "ASCII" appear on the first line,
 * and "PHOTON" appears BEFORE "ASCII" and the line is a comment.
 * (case doesn't matter)
 */
static int check_magic(char *buf)
{
	char *p;
	int got_photon, got_ascii;

	p = buf;
	while( *p == ' ' || *p == '\t' ) p++;

	if( *p != '#' )
		return FALSE;

	got_photon = -1;
	got_ascii = -1;

	for(p=buf; *p; p++) {
		if( got_photon == -1 && !strnicmp(p, "PHOTON", 6) )
			got_photon = (int) (p-buf);

		if( got_ascii == -1 && got_photon>=0 && !strnicmp(p, "ASCII", 5) )
			got_ascii = (int) (p-buf);
	}

	return got_ascii >= 0 && got_photon >= 0 && got_photon < got_ascii;
}

int Phascii_FileIsPhotonAscii(const char *filename)
{
	FILE *fp;
	char buf[ BUFSIZ ];
	int n;
	int is_ascii;

	fp = fopen(filename, "r");
	if( fp == NULL ) {
		return 0;
	}

	fgets(buf, sizeof(buf), fp);

	n = check_magic(buf);
	if( n == FALSE ) {
		is_ascii = 0;
	} else {
		is_ascii = 1;
	}

	fclose(fp);

	return is_ascii;
}

/***********************************************************************
 * Open a config filename. 'mode' is currently always "r".
 *
 * RETURNS:
 *	NULL - failure.
 *	else a config file handle.
 */
PHASCII_FILE Phascii_Open(const char *filename, char *mode)
{
	CONFIG_FILE *cf;
	FILE *fp;
	char buf[ BUFSIZ ];
	int n;

	fp = fopen(filename, "r");
	if( fp == NULL ) {
		sprintf(cf_errorbuf, "%s: %s", filename, strerror(errno));
		return NULL;
	}

	fgets(buf, sizeof(buf), fp);

	n = check_magic(buf);
	if( n == FALSE ) {
		fclose(fp);
		sprintf(cf_errorbuf, "%s: Line 1, PHOTON ASCII Header missing.",
				filename);
		return NULL;
	}

	cf = (CONFIG_FILE*) malloc( sizeof(CONFIG_FILE) );
	if( cf == NULL ) {
		fclose(fp);
		sprintf(cf_errorbuf, "out of memory");
		return NULL;
	}

	strcpy(cf->filename, filename);
	cf->fp = fp;
	cf->lineno = 2;
	cf->ungettoken = FALSE;
	cf->error[0] = '\0';
	cf->definitions = NULL;
	cf->old_definitions = NULL;

	return (PHASCII_FILE) cf;
}

/***********************************************************************
 * Close the PHASCII_FILE.
 */
void Phascii_Close(PHASCII_FILE phf)
{
	CONFIG_FILE *cf;
	
	cf = (CONFIG_FILE*) phf;

	fclose(cf->fp);
	dlist_destroy(&cf->definitions, (DlistFreeProcType) free_definition);
	dlist_destroy(&cf->old_definitions, (DlistFreeProcType) free_definition);
	free(cf);
}

/************************************************************************
 * Return the error string from the last error.
 * If cfg_file is NULL, then return error string from last
 * failed Phascii_Open() call.
 */
char *Phascii_Error(PHASCII_FILE phf)
{
	CONFIG_FILE *cf;

	cf = (CONFIG_FILE*) phf;

	if( cf )
		return cf->error;
	else
		return cf_errorbuf;
}

/***********************************************************************
 * Remains for backward compatibility with the older version
 * of this library.
 */
char *Phascii_GetError(void)
{
	return Phascii_Error(NULL);
}

/************************************************************************
 * Returns TRUE if the instance 'ci' has the name 'name'.
 */
int Phascii_IsInstance(PHASCII_INSTANCE instance, char *name)
{
	CONFIG_INSTANCE *ci;
	
	ci = (CONFIG_INSTANCE*) instance;

	return ident_cmp(ci->definition->name, name) == 0;
}

/***********************************************************************
 * Returns TRUE if EOF has been reached in the input, else
 * returns FALSE. If an error occured since the last Phascii_GetInstance(),
 * then Phascii_Eof() will not return TRUE.
 *
 * Call this function to determine if a Phascii_GetInstance() returned
 * NULL because EOF or because of a parse error in the input.
 */
int Phascii_Eof(PHASCII_FILE phf)
{
	CONFIG_FILE *cf;

	cf = (CONFIG_FILE*) phf;

	if( cf->error[0] )
		return FALSE;
	else
		return feof( cf->fp );
}

/************************************************************************
 * Parse input until an instance is parsed, return that instance.
 *
 * RETURNS:
 *	NULL - an error occured, or EOF was detected.
 *	else a config instance is returned.
 */
PHASCII_INSTANCE Phascii_GetInstance(PHASCII_FILE phf)
{
	CONFIG_FILE *cf;
	CONFIG_INSTANCE *ci;
	DEFINITION *def, *d;
	char buf[ BUFSIZ ];
	int t, found;
	DLIST *curr;

	cf = (CONFIG_FILE*) phf;

	/*
	 * Clear any possible errors.
	 */
	cf->error[0] = '\0';

	/*
	 * Setup error handler.
	 */
	if( setjmp(jmpbuf) ) {
		sprintf(buf, "%s: Line: %d, %s.", cf->filename, cf->lineno, cf_errorbuf);
		strcpy(cf->error, buf);
		return NULL;
	}

	ci = NULL;
	for(;;) {
		t = gettoken(cf);
		if( t == T_STRUCT ) {
			def = parse_base_definition(cf);

			/*
			 * See if we are creating a new definition.
			 */
			found = FALSE;
			for(curr=cf->definitions; curr; curr=curr->next) {
				d = (DEFINITION*)curr->p;
				if( !ident_cmp(d->name, def->name) ) {
					found = TRUE;
					break;
				}
			}
			if( found ) {
				/*
				 * It is possible that the caller has a 'ci'
				 * which points to the definition we are going to
				 * be updating. This is rare, but it is easy
				 * enough to protect ourselves against.
				 * Just move the definition to another
				 * list.
				 */
				dlist_delete(&cf->definitions, d, NULL, NULL);
				dlist_add(&cf->old_definitions, d);
			}
			dlist_add(&cf->definitions, def);

		} else if( t == T_IDENT ) {
			ungettoken(cf, t);
			ci = parse_base_instance(cf);
			break;

		} else if( t == T_EOF ) {
			break;

		} else {
			error("missing definition or instance at or near '%s'", cf->tokenbuf);
		}
	}
	return (PHASCII_INSTANCE *) ci;
}

/***********************************************************************
 * De-allocate memory associated with this instance.
 */
void Phascii_FreeInstance(PHASCII_INSTANCE ph_inst)
{
	CONFIG_INSTANCE *ci;

	ci = (CONFIG_INSTANCE *) ph_inst;

	free_instance(ci->definition, ci->instance);

	free(ci);
}

/***********************************************************************
 * This is the primary function of the Phascii routines.
 *
 * Usage:
 *	n = Phascii_Get(ci, "expression", i, j, k, "%d %s %f", &m, buf, &f);
 *
 *	ci	- A config instance previously returned by Phascii_GetInstance()
 *
 *	"expression"
 *		- A expression describing what data to retrieve.
 *
 *	i, j, k
 *		- Integer's substituted for %0 %1 %2 occuring in expression.
 *
 *	"%d %s %f"
 *		- A scanf-like string for converting the data items
 *		  into the variables.
 *
 *	m, buf, f
 *		- The variables that will be loaded with the data values.
 *
 * RETURNS:
 *	Returns the number of items converted. If this
 *	is less than the number you wanted, then a conversion error
 *	occured, and Phascii_Error() can be called to retrieve the error
 *	messages.
 *
 */
int Phascii_Get(PHASCII_INSTANCE ph_entry, char *fmt, ...)
{
	CONFIG_INSTANCE *ci;
	CONFIG_FILE *cf;
	va_list ap;
	char *p, *conv;
	int max, n, i, t, *args, num_gotten, conv_arg, found;
	char buf[ BUFSIZ ];
	char *datap, *data, *match;
	DLIST *fmt_head, *curr;
	void *ptr;
	int no_such_component;

	num_gotten = 0;
	args = NULL;
	fmt_head = NULL;

	ci = (CONFIG_INSTANCE*) ph_entry;
	cf = ci->cf;
	
	n = expand_strings(fmt, &fmt_head);
	if( n == 0 ) {
		strcpy(cf->error, "illegal {} expression");
		return 0;
	} else if( n < 0 ) {
		strcpy(cf->error, "out of memory");
		return 0;
	}

	/*
	 * Setup error handler.
	 */
	if( setjmp(jmpbuf) ) {
		if( args )
			free(args);

		if( fmt_head )
			dlist_destroy(&fmt_head, (DlistFreeProcType)free);

		if( cf_errorbuf[0] == '*' ) {
			/*
			 * A non-serious error occured.
			 */
			sprintf(cf->error, "*Phascii_Get(): %s", cf_errorbuf);
		} else
			sprintf(cf->error, "Phascii_Get(): %s", cf_errorbuf);
		return num_gotten;
	}
	
	va_start(ap, fmt);
	
	/*
	 * Determine maximum number of %0 ... %N values.
	 */
	max = -1;
	for(curr=fmt_head; curr; curr=curr->next) {
		p = (char *)curr->p;
		for(;;) {
			t = expr_gettoken(&p);
			if( t == ET_ARG ) {
				if( etoken_arg > max )
					max = etoken_arg;
			} else if( t == ET_EOF )
				break;
		}
	}

	/*
	 * Fetch values for the %0...%N arguments.
	 */
	if( max >= 0 ) {
		args = (int *) malloc( sizeof(int) * (max+1) );
		for(i=0; i<max+1; i++)
			args[i] = va_arg(ap, int);
	} else
		args = NULL;

	/*
	 * Parse each expression, and perform conversion.
	 */	
	num_gotten = 0;
	conv = va_arg(ap, char*);
	p = conv;
	for(curr=fmt_head; curr; curr=curr->next) {
		no_such_component = FALSE;
		n = parse_expression(&data, ci, (char*)curr->p, args);
		if( n >= 0 ) {
			/* array size request */
			sprintf(buf, "%d", n);
			datap = buf;
		} else if( n == -1 ) {
			/* data request */
			datap = data;
		} else {
			/* error: no such component */
			sprintf(cf->error, "*Phascii_Get(): %s", cf_errorbuf);
			no_such_component = TRUE;
			buf[0] = '\0';
			datap = buf;
		}

		while( *p == ' ' || *p == '\t' )
			p++;

		if( *p != '%' && *p )
			error("bad %% conversion string");

		if( *p == '\0' )
			error("not enough conversion %%'s");

		/*
		 * Handle:
		 *	%*s		conv_arg = int in arg list.
		 *	%12s		conv_arg = 12
		 *	%s		conv_arg = length of string.
		 *
		 *	%*e		conv_arg = int in arg list.
		 *	%12e		conv_arg = 12
		 *	%e		illegal.
		 */
		p++;
		if( *p == '*' ) {
			conv_arg = va_arg(ap, int);
			p++;
		} else if( isdigit(*p) ) {
			conv_arg = 0;
			do {
				conv_arg *= 10;
				conv_arg += (*p) - '0';
				p++;
			} while( isdigit(*p) );
		} else {
			if( *p == 'e' )
				error("illegal: %%e requires a number argument");
			else
				conv_arg = (int) strlen(datap);
		}

		ptr = va_arg(ap, void*);

		/*
		 * Write data into variable...
		 */
		if( !no_such_component) {
		switch( *p ) {
		case 'c':
			*((char*)ptr) = datap[0];
			break;

		case 'f':
			*((float*)ptr) = (float) atof( datap );
			break;
			
		case 'l':
			p++;
			switch (*p) {
			case 'f':
				*((double*) ptr) = (double) atof( datap );
				break;

			case 'd':
				*((long*) ptr) = (long) atol( datap );
				break;

			case 'u':
				*((unsigned long*) ptr) = (unsigned long) atol( datap );
				break;

			case 'l': /* long long - 64-bit integer */
				*((__int64*) ptr) = _atoi64( datap );
				break;

			default:
				error("invalid conversion %%l%c", *p);
				break;
			}
			break;
			
		case 'h':
			p++;
			switch (*p) {
			case 'f':
				*((float*) ptr) = (float) atof( datap );
				break;

			case 'd':
				*((short*) ptr) = (short) atol( datap );
				break;

			case 'u':
				*((unsigned short*) ptr) = (unsigned short) atol( datap );
				break;

			default:
				error("invalid conversion %%l%c", *p);
				break;
			}
			break;
			
		case 'd':
			*((int*) ptr) = (int) atoi( datap );
			break;

		case 'u':
			*((unsigned int*) ptr) = (unsigned int) atoi( datap );
			break;

		case 's':
			strncpy((char*) ptr, datap, conv_arg);
			*((char*)ptr + conv_arg) = '\0';
			break;

		case 'e':
			/*
			 * ENUMERATION FORMAT: "4e"
			 *	[ "S1", "S2", ..., "S4", &i ]
			 *
			 * ENUMERATION FORMAT: "*e"
			 *	[ 4, "S1", "S2", ..., "S4", &i ]
			 *
			 * If no string matches, %e will yeild atoi(datap) or 0.
			 */
			if( conv_arg == 0 )
				error("%%e must have a length specifier");

			i = 0;
			found = -1;
			match = (char*)ptr;
			do {
				if( !ident_cmp(match, datap) )
					found = i;

				match = va_arg(ap, char*);
				i++;
			} while( i<conv_arg );
			ptr = (void*)match;
			if( found >= 0 )
				*((int*)ptr) = found;
			else {
				if( is_integer(datap) ) {
					*((int*)ptr) = atoi(datap);
				} else
					*((int*)ptr) = 0;
			}
			break;

		default:
			error("invalid conversion %%c", *p);
		}}
		p++; /* skip over conversion character */

		if( no_such_component == FALSE )
			num_gotten++;
	}
	va_end( ap );

	if( args )
		free(args);

	if( fmt_head )
		dlist_destroy(&fmt_head, free);
	
	return num_gotten;
}

/***********************************************************************
 * Phascii_Makestring(char *):
 *
 * This function is for people WRITTING out photon ascii files. Often
 * a string it to be generated, this function makes it easy to
 * build a string that is SAFE for reading again.
 *
 * Create a string that contains the same information
 * as buf, except the new string is surrounded by double quotes, and
 * the following characters will be converted to proper config escape
 * sequences:
 *
 *	THIS CHAR IS		CONVERT TO THIS SEQUENCE
 *	double quote (")	\"
 *	back slash (\)		\\
 *	bell			\a
 *	backspace		\b
 *	carrage return		\r
 *	new page		\v
 *	newline			\n
 *	tab			\t
 *
 * If the input string contains bogus characters, then the output
 * string may not be readable by the Phascii_XXXX() routines.
 */
char *Phascii_MakeString(char *buf)
{
	char *p, *q, *newp;
	int count;

	count = 0;
	for(p=buf; *p; p++) {
		count++;
		if( *p == '\a' ) count++;
		else if( *p == '\b' ) count++;
		else if( *p == '\f' ) count++;
		else if( *p == '\n' ) count++;
		else if( *p == '\r' ) count++;
		else if( *p == '\t' ) count++;
		else if( *p == '\v' ) count++;
		else if( *p == '"' ) count++;
		else if( *p == '\\' ) count++;
	}

	newp = (char*) malloc( count+3 );

	p=buf;
	q=newp;
	*q++ = '"';
	while( *p ) {
		if( strchr("\a\b\f\n\r\t\v\"\\", *p) ) {
			*q++ = '\\';

			switch( *p ) {
			case '\a':	*q++ = 'a'; break;
			case '\b':	*q++ = 'b'; break;
			case '\f':	*q++ = 'f'; break;
			case '\n':	*q++ = 'n'; break;
			case '\r':	*q++ = 'r'; break;
			case '\t':	*q++ = 't'; break;
			case '\v':	*q++ = 'v'; break;
			case '"':	*q++ = '"'; break;
			case '\\':	*q++ = '\\'; break;
			}
			p++;
		} else
			*q++ = *p++;
	}
	*q++ = '"';
	*q = '\0';

	return newp;
}

static void indexes_add(PHASCII_INDEXES **head, char *name)
{
	PHASCII_INDEXES *new_node, *node;

	new_node = (PHASCII_INDEXES*) malloc( sizeof(PHASCII_INDEXES) );
	new_node->name = name;
	new_node->next = NULL;

	if( *head == NULL ) {
		*head = new_node;
	} else {
		for(node=*head; node->next; node=node->next)
			;

		node->next = new_node;
	}
}

static PHASCII_INDEXES *get_schema_indexes(DEFINITION *def)
{
	PHASCII_INDEXES *head;
	DEFINITION *d;

	head = NULL;
	indexes_add(&head, strdup(def->index));

	d = (DEFINITION*)def->components->p;

	while( !strcmp(d->name, "[]") ) {
		indexes_add(&head, strdup(d->index));
		d = (DEFINITION*) d->components->p;
	}
	return head;
}

static void schema_add(PHASCII_SCHEMA **head, PHASCII_SCHEMA *new_node)
{
	PHASCII_SCHEMA *node;

	new_node->next = NULL;

	if( *head == NULL ) {
		*head = new_node;
	} else {
		for(node=*head; node->next; node=node->next)
			;

		node->next = new_node;
	}
}


/***********************************************************************
 * Extract the schema for a config instance. expr is
 * an expression that is evaluated, and a linked list
 * of components will be returned. If the expression
 * is invalid or evaluates to a component, then
 * NULL is returned, else a linked list of components
 * for the array is returned.
 *
 * EXPRESSION GRAMMER:
 *	expression	==>	  IDENT expr-tail
 *				| NULL
 *
 *	expr-tail	==>	  . IDENT
 *				| . IDENT expr-tail
 *
 * EXAMPLES:
 *	WELL.UWI			- returns NULL.
 *	EMPLOYEE.NAME.FIRST		- return NULL.
 *	EMPLOYEE.NAME			- (FIRST MIDDLE LAST)
 *	EMPLOYEE			- (NAME SALARY BDATE AGE TITLE)
 *
 * RETURNS:
 *	NULL - expression error, or expression is a component.
 *		cf->error will be filled with proper error string or by empty.
 *	else - a linked list of PHASCII_SCHEMA records.
 *
 */
PHASCII_SCHEMA *Phascii_GetSchema(PHASCII_INSTANCE ph_entry, char *expr)
{
	CONFIG_FILE *cf;
	CONFIG_INSTANCE *ci;
	char *p;
	DLIST *curr;
	DEFINITION *def, *d, *next;
	PHASCII_SCHEMA *head, *cs;
	int t, found;

	ci = (CONFIG_INSTANCE*) ph_entry;

	cf = ci->cf;
	def = ci->definition;
	cf->error[0] = '\0';

	/* empty expression */
	if( expr == NULL || expr[0] == '\0' ) {
		head = NULL;
		cs = (PHASCII_SCHEMA *) malloc( sizeof(PHASCII_SCHEMA) );
		cs->name = strdup(def->name);
		switch( def->type ) {
		case DEF_STRUCT:
			cs->type = PHASCII_STRUCT;
			cs->indexes = NULL;
			break;

		case DEF_ARRAY:
			cs->type = PHASCII_ARRAY;
			cs->indexes = get_schema_indexes(def);
			break;

		case DEF_COMPONENT:
			/* should never happen */
			cs->type = PHASCII_COMPONENT;
			cs->indexes = NULL;
			break;
		}
		schema_add(&head, cs);
		return head;
	}

	p = expr;

	t = expr_gettoken(&p);
	if( t != ET_IDENT ) {
		sprintf(cf->error, "syntax error");
		return NULL;
	}

	if( ident_cmp(etoken_ident, def->name) ) {
		sprintf(cf->error, "identifier %s does not match instance name",
								etoken_ident);
		return NULL;
	}

	for(;;) {
		t = expr_gettoken(&p);
		if( t == ET_EOF )
			break;

		if( t != '.' ) {
			sprintf(cf->error, "syntax error");
			return NULL;
		}

		t = expr_gettoken(&p);
		if( t != ET_IDENT ) {
			sprintf(cf->error, "syntax error");
			return NULL;
		}

		switch( def->type ) {
			case DEF_STRUCT:
				/* search component list for name */
				found = FALSE;
				curr=def->components;
				while( curr ) {
					d = (DEFINITION*)curr->p;
					if( !ident_cmp(etoken_ident, d->name) ) {
						found = TRUE;
						break;
					}
					curr=curr->next;
				}
				if( !found ) {
					sprintf(cf->error, "no such component %s.%s",
						def->name, etoken_ident);
					return NULL;
				}

				def = d;
				break;

			case DEF_ARRAY:
				d = def;
				for(;;) {
					next = (DEFINITION*)d->components->p;
					if( strcmp(next->name, "[]") != 0 )
						break;
					d = next;
				}
				def = d;
				/* search component list for name */
				found = FALSE;
				curr=def->components;
				while( curr ) {
					d = (DEFINITION*)curr->p;
					if( !ident_cmp(etoken_ident, d->name) ) {
						found = TRUE;
						break;
					}
					curr=curr->next;
				}
				if( !found ) {
					sprintf(cf->error, "no such component %s.%s",
						def->name, etoken_ident);
					return NULL;
				}

				def = d;
				break;

			case DEF_COMPONENT:
				break;
		}
	}

	if( def->type == DEF_ARRAY ) {
		d = def;
		for(;;) {
			next = (DEFINITION*)d->components->p;
			if( strcmp(next->name, "[]") != 0 )
				break;
			d = next;
		}
		def = d;
	}

	head = NULL;
	for(curr=def->components; curr; curr=curr->next) {
		d = (DEFINITION*)curr->p;
		cs = (PHASCII_SCHEMA *) malloc( sizeof(PHASCII_SCHEMA) );
		cs->name = strdup(d->name);
		switch( d->type ) {
		case DEF_STRUCT:
			cs->type = PHASCII_STRUCT;
			cs->indexes = NULL;
			break;

		case DEF_ARRAY:
			cs->type = PHASCII_ARRAY;
			cs->indexes = get_schema_indexes(d);
			break;

		case DEF_COMPONENT:
			cs->type = PHASCII_COMPONENT;
			cs->indexes = NULL;
			break;
		}
		schema_add(&head, cs);
	}

	return head;
}

static void config_schema_free(PHASCII_SCHEMA *cs)
{
	PHASCII_INDEXES *node, *nxt;

	if( cs->name )
		free(cs->name);

	for(node=cs->indexes; node; node=nxt) {
		nxt = node->next;
		if( node->name )
			free(node->name);
		free(node);
	}
	free(cs);
}

void Phascii_FreeSchema(PHASCII_SCHEMA *head)
{
	PHASCII_SCHEMA *node, *nxt;

	for(node=head; node; node=nxt) {
		nxt = node->next;
		config_schema_free(node);
	}
}

int Phascii_Lineno(PHASCII_FILE phf)
{
	CONFIG_FILE *cf;

	cf = (CONFIG_FILE*) phf;

	return cf->lineno;
}
