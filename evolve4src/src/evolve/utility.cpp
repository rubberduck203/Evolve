/*
 * Copyright (c) 1991 Ken Stauffer, All Rights Reserved
 */

/***********************************************************************
 * utility.cpp
 *
 */

#include <stdafx.h>

/*
 * Return 1 for succes, 0 else
 *
 */
int StoreUniverse(LPCTSTR lpszPathName, UNIVERSE *u, char *errbuf)
{
	const char *filename;

	filename = lpszPathName;
	return Universe_Write(u, filename, errbuf);
}


UNIVERSE *LoadUniverse(LPCTSTR lpszPathName, char *errbuf)
{
	const char *filename;

	ASSERT( errbuf != NULL );

	filename = lpszPathName;
	return Universe_Read(filename, errbuf);
}

#define ROUND(x)	((int) (x))

static void create_barrier_point(UNIVERSE *u, int px, int py)
{
	int x, y;

	x = px;
	y = py;
	if( (x >= 0) && (x < u->width) && (y >= 0) && (y < u->height) )
		Universe_SetBarrier(u, x, y);

	x = px+1;
	y = py;
	if( (x >= 0) && (x < u->width) && (y >= 0) && (y < u->height) )
		Universe_SetBarrier(u, x, y);

	x = px-1;
	y = py;
	if( (x >= 0) && (x < u->width) && (y >= 0) && (y < u->height) )
		Universe_SetBarrier(u, x, y);

	x = px;
	y = py+1;
	if( (x >= 0) && (x < u->width) && (y >= 0) && (y < u->height) )
		Universe_SetBarrier(u, x, y);

	x = px;
	y = py-1;
	if( (x >= 0) && (x < u->width) && (y >= 0) && (y < u->height) )
		Universe_SetBarrier(u, x, y);
}

static void create_barrier_line(UNIVERSE *u, int p1x, int p1y, int p2x, int p2y)
{
	ASSERT( u != NULL );

	int x, y;
	double rise, run, m, b;
	int step;

	rise = (p2y - p1y);
	run = (p2x - p1x);

	if( run == 0.0 ) {
		// handle vertical line

		if( rise < 0.0 ) {
			step = -1;
		} else {
			step = 1;
		}

		y = p1y;
		while( y != p2y ) {
			create_barrier_point(u, p1x, y);
			y = y + step;
		}

	} else if( fabs(rise) <= fabs(run) ) {
			// iterate over x range
		m = rise / run;
		b = p1y - m * (p1x);

		if( run < 0 ) {
			step = -1;
		} else {
			step = 1;
		}

		x = p1x;
		while( x != p2x ) {
			y = ROUND(m*x + b);
			create_barrier_point(u, x, y);
			x = x + step;
		}

	} else {	// fabs(rise) > fabs(run)
			// iterate over y range
		m = rise / run;
		b = p1y - m * (p1x);

		if( rise < 0.0 ) {
			step = -1;
		} else {
			step = 1;
		}

		y = p1y;
		while( y != p2y ) {
			x = ROUND( (y-b) / m );
			create_barrier_point(u, x, y);
			y = y + step;
		}
	}
}


//
//	XCENTRE	X position of centre of ellipse in 
//		world coordinates.
//	YCENTRE	Same for Y position
//	MAJOR	Major axis of ellipse.
//	MINOR	Minor axis of ellipse.    
//	PA	Position angle of ellipse wrt. pos x-axis
//		in degrees.
//	STARTANG	Angle to start drawing ellipse (Degrees).
//	ENDANG	Angle to end drawing the ellipse (Degrees).
//	DELTA	Calculate plot points from STARTANG to ENDANG
//		in steps of DELTA degrees.
//
//Note:	The length of MAJOR can be smaller than the length of MINOR!
//
//Description:	Draw an ellipse with specifications (origin and axes) 
//		in world coordinates. The position angle of the major 
//		axis is wrt. the pos. x-axis. The ellipse is rotated 
//		counter-clockwise. The plotting starts at 'STARTANG'
//		degrees from the major axis and stops at 'ENDANG'
//		degrees from this major axis.
//
static void create_barrier_ellipse(UNIVERSE *u)
{
	double Pos_angle = 0.0;
	double Start_angle = 0.0;
	double End_angle = 360.0;
	double Rad = 0.017453292519943295769237;

	double delta;
	double center_x, center_y;	// center
	double major, minor;		// major axis and minor axis size

	double cosp, sinp;		// angles
	double cosa, sina;		// angles
	double alpha;			// used in polar coordinates
	double r;			// radius used in polar coords
	double denom;			// help variable
	double xell, yell;		// points of not rotated ellipse
	double x, y;			// each point of ellipse

	int curr_point_x, curr_point_y;
	int prev_point_x, prev_point_y;
	int first_point_x, first_point_y;

	bool first;
	double curve_angle;

	delta = 1.0;

	center_x = (u->width) / 2.0;
	center_y = (u->height) / 2.0;

	major = (u->width) / 2.0;
	minor = (u->height) / 2.0;

	cosp = cos( Pos_angle * Rad );
	sinp = sin( Pos_angle * Rad );

	first = true;
	alpha = Start_angle;
	curve_angle = 0.0;

	while( alpha <= End_angle ) {

		cosa = cos( alpha * Rad );
		sina = sin( alpha * Rad );
		denom = (minor*cosa * minor*cosa + major*sina * major*sina);
		if( denom == 0.0 ) {
			r = 0.0;
		} else {
			r = sqrt( minor*major * minor*major / denom );
		}

		xell = r * cosa;
		yell = r * sina;

		x = (xell * cosp - yell * sinp  + center_x);
		y = (xell * sinp + yell * cosp  + center_y);

		curr_point_x = ROUND(x);
		curr_point_y = ROUND(y);

		if( ! first ) {
			create_barrier_line(u, prev_point_x, prev_point_y, curr_point_x, curr_point_y);
		} else {
			first = false;
			first_point_x = curr_point_x;
			first_point_y = curr_point_y;
		}

		prev_point_x = curr_point_x;
		prev_point_y = curr_point_y;

		alpha = alpha + delta;

		curve_angle = curve_angle + 6;
		if( curve_angle > 360.0 ) {
			curve_angle = 0.0;
		}
	}

	create_barrier_line(u, prev_point_x, prev_point_y, first_point_x, first_point_y);
}

//
// the spiral path class. initialize with a starting
// point and a step value. Each time 'next' is called
// update the (x, y) point to be the next point in the spiral
// path.
//
//
typedef struct {
	int	x;
	int	y;
	int	step;

	int	dirx;
	int	diry;
	int	len;
	int	curlen;

} SPIRALPATH;

static SPIRALPATH *spiralpath_make(int x, int y, int step)
{
	SPIRALPATH *sp;

	sp = (SPIRALPATH *) CALLOC(1, sizeof(SPIRALPATH) );
	ASSERT( sp != NULL );

	sp->x		= x;
	sp->y		= y;
	sp->step	= step;

	sp->dirx	= 1;
	sp->diry	= 0;
	sp->len		= 1;

	sp->curlen	= sp->len;

	return sp;
}

/*
 * Compute next point in spiral path.
 */
static void spiralpath_next(SPIRALPATH *sp)
{
	int tmp;

	ASSERT( sp != NULL );

	if( sp->curlen > 0 ) {
		sp->curlen -= 1;
	} else {
		sp->len += 1;
		sp->curlen = sp->len;

		/*
		 * turn right
		 */
		tmp = sp->dirx;
		sp->dirx = sp->diry * -1;
		sp->diry = tmp;
	}

	sp->x += (sp->dirx * sp->step);
	sp->y += (sp->diry * sp->step);

}

static void spiralpath_delete(SPIRALPATH *sp)
{
	ASSERT( sp != NULL );
	FREE(sp);
}

/*
 * sexonly simulations will call this to create the
 * initial population.
 *
 * The organism 'o' will be cloned as needed. The organism 'o' will
 * be pasted as the first organism.
 *
 * total_energy is the amount of energy to divide up among the population.
 *
 *
 */
static void create_population(UNIVERSE *u, int population, int total_energy, ORGANISM *o)
{
	int energy_per_organism;
	int energy_remainder;
	int i;
	ORGANISM *no;
	SPIRALPATH *sp;

	ASSERT( u != NULL );
	ASSERT( population >= 1 && population <= 100 );
	ASSERT( total_energy > 0 );
	ASSERT( o != NULL );

	energy_per_organism = total_energy / population;
	energy_remainder = total_energy % population;

	if( energy_per_organism == 0 ) {
		population		= 1;
		energy_per_organism	= total_energy;
		energy_remainder	= 0;
	}

	sp = spiralpath_make(o->cells->x, o->cells->y, 5);

	for(i=0; i < population; i++) {

		if( i == 0 ) {
			no = o;
			no->energy = energy_per_organism + energy_remainder;
		} else {
			no = Universe_CopyOrganism(u);
			Universe_ClearSelectedOrganism(u);
			no->energy = energy_per_organism;
		}

		no->cells->x = sp->x;
		no->cells->y = sp->y;
		Universe_PasteOrganism(u, no);

		spiralpath_next(sp);
	}

	Universe_ClearSelectedOrganism(u);

	spiralpath_delete(sp);

}


/*
 * Create the universe from the properties that are
 * contained in the 'nuo' object.
 *
 * If only 1 strain was specified by the user, then
 * that starting organism goes right smack into the middle
 *
 * Otherwise, each strain is positioned around the place.
 *
 */
UNIVERSE *CreateUniverse(NewUniverseOptions *nuo, char *errbuf)
{
	int i, xpos, ypos;
	ORGANISM *o[8];
	UNIVERSE *u;
	int x[8], y[8];
	int wa, wb, wc;
	int ha, hb, hc;
	char buf[3000];
	CString source_code;
	FILE *fp;
	bool failed;
	int posi;
	size_t len;

	ASSERT( nuo != NULL );
	ASSERT( errbuf != NULL );

	if( nuo->num_strains == 0 ) {
		sprintf(errbuf, "No starting population was specified.");
		return NULL;
	}

	/*
 	 * Compute starting positions for each possible strain
	 */
	wa = nuo->width/4;
	wb = nuo->width/2;
	wc = nuo->width - nuo->width/4;

	ha = nuo->height/4;
	hb = nuo->height/2;
	hc = nuo->height - nuo->height/4;

	if( nuo->num_strains > 1 ) {
		x[0] = wb;
		y[0] = ha;

		x[1] = wb;
		y[1] = hc;

		x[2] = wa;
		y[2] = hb;

		x[3] = wc;
		y[3] = hb;

		x[4] = wa;
		y[4] = ha;

		x[5] = wc;
		y[5] = ha;

		x[6] = wa;
		y[6] = hc;

		x[7] = wc;
		y[7] = hc;
	} else {
		/*
		 * If just 1 strain in sim, then put it in the middle.
		 */
		for(i=0; i<8; i++) {
			x[i] = wb;
			y[i] = hb;
		}
	}

	for(i=0; i<8; i++) {
		o[i] = NULL;
	}

	posi = 0;
	failed = FALSE;
	for(i=0; i<8; i++) {
		if( ! nuo->has_strain[i] )
			continue;

		xpos = x[posi];
		ypos = y[posi];

		fp = fopen(nuo->filename[i], "r");
		if( fp == NULL ) {
			sprintf(errbuf, "%s: %s", nuo->filename[i], strerror(errno));
			failed = TRUE;
			break;
		}

		source_code = "";
		while( fgets(buf, sizeof(buf), fp) != NULL ) {
			len = strlen(buf)-1;
			if( buf[len-1] == '\n' )
				buf[ strlen(buf)-1 ] = '\0';
			strcat(buf, "\r\n");
			source_code += buf;
		}
		fclose(fp);

		o[i] = Organism_Make(xpos, ypos, i, nuo->energy[i], source_code, errbuf);
		if( o[i] == NULL ) {
			failed = TRUE;
			break;
		}

		if( nuo->sexonly[i] ) {
			o[i]->oflags |= ORGANISM_FLAG_SEXONLY;
		}

		posi++;
	}

	if( failed ) {
		for(i=0; i<8; i++) {
			if( o[i] == NULL ) {
				continue;
			}
			Organism_delete(o[i]);
		}
		return NULL;
	}

	u = Universe_Make(nuo->seed, nuo->width, nuo->height, &nuo->kfmo);
	ASSERT( u != NULL );

	if( nuo->want_barrier ) {
		create_barrier_ellipse(u);
	}

	for(i=0; i<8; i++) {
		if( o[i] == NULL )
			continue;

		create_population(u, nuo->population[i], nuo->energy[i], o[i]);
	}

	return u;
}

static void insert_commas(char *buf, char *str)
{
	char *p, *q;
	int i, len, new_len;

	len = (int) strlen(str);
	new_len = len + (len-1)/3;

	q = buf + new_len;
	*q-- = '\0';

	p = str + len;
	p--;

	i = 0;
	while( p >= str ) {
		if( (i % 3 == 0) && (i > 0) )
			*q-- = ',';

		*q-- = *p--;
		i++;
	}
}

/*
 * _int64 to ascii, with commas
 */
void lltoa_comma(char *buf, LONG_LONG value)
{
	char tmp[100];

	if( value < 0 ) {
		sprintf(tmp, "%I64d", -value);
		insert_commas(buf+1, tmp);
		buf[0] = '-';
	} else {
		sprintf(tmp, "%I64d", value);
		insert_commas(buf, tmp);
	}
}

/*
 * int to ascii, with commas
 */
void itoa_comma(char *buf, int value)
{
	char tmp[100];

	if( value < 0 ) {
		sprintf(tmp, "%d", -value);
		insert_commas(buf+1, tmp);
		buf[0] = '-';
	} else {
		sprintf(tmp, "%d", value);
		insert_commas(buf, tmp);
	}
}

int atoi_comma(const char *buf)
{
	char str[1000];
	const char *p;
	char *q;

	//
	// strip commas in 'buf' and place result in 'str'
	//
	p = buf;
	q = str;
	while( *p ) {
		if( *p != ',' )
			*q++ = *p;

		p++;
	}
	*q = '\0';

	return atoi(str);
}

LONG_LONG atoll_comma(const char *buf)
{
	char str[1000];
	const char *p;
	char *q;

	//
	// strip commas in 'buf' and place result in 'str'
	//
	p = buf;
	q = str;
	while( *p ) {
		if( *p != ',' )
			*q++ = *p;

		p++;
	}
	*q = '\0';

	return _atoi64(str);
}

void DDX_CommaValue(CDataExchange *pDX, int nIDC, int& value)
{
	char buf[1000];
	CString str;

	ASSERT( pDX != NULL );

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, str);
		value = atoi_comma(str);
	} else {
		itoa_comma(buf, value);
		str = buf;
		DDX_Text(pDX, nIDC, str);
	}

}

/*
 * Make sure value not negative. (i.e. 0 or more)
 * No upper bound is specified.
 */
void DDX_CommaValuePositive(CDataExchange *pDX, int nIDC, int& value)
{
	char buf[1000];
	CString str;

	ASSERT( pDX != NULL );

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, str);
		value = atoi_comma(str);
		if( value < 0 ) {
			value = 0;
		}
	} else {
		itoa_comma(buf, value);
		str = buf;
		DDX_Text(pDX, nIDC, str);
	}

}

/*
 * get/set value and make sure value is falls within min .. max (force
 * value to upper/lower bound if it falls outside this range)
 * strip commas, if present
 */
void DDX_RangeValue(CDataExchange *pDX, int nIDC, int& value, int minval, int maxval)
{
	char buf[1000];
	CString str;

	ASSERT( pDX != NULL );

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, str);
		value = atoi_comma(str);
		if( value < minval ) {
			value = minval;
		} else if( value > maxval ) {
			value = maxval;
		}
	} else {
		itoa_comma(buf, value);
		str = buf;
		DDX_Text(pDX, nIDC, str);
	}

}

void DDX_CommaValue(CDataExchange *pDX, int nIDC, LONG_LONG& value)
{
	char buf[1000];
	CString str;

	ASSERT( pDX != NULL );

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, str);
		value = atoll_comma(str);
	} else {
		lltoa_comma(buf, value);
		str = buf;
		DDX_Text(pDX, nIDC, str);
	}

}


void DDX_MutationRate(CDataExchange *pDX, int nIDC, int& value)
{
	double f;
	char tmp[100], buf[100], *p, *q;
	CString str;

	ASSERT( pDX != NULL );

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, str);
		strcpy(tmp, str);
		q = buf;
		for(p=tmp; *p; p++) {
			if( *p != ' ' && *p != '%' ) {
				*q++ = *p;
			}
		}
		*q = '\0';

		f = atof(buf);

		if( f > 100.0 ) {
			f = 100;
		} else if( f < 0.0 ) {
			f = 0.0;
		}

		value = (int) ( (f/100.0) * PROBABILITY_SCALE );

	} else {
		f = (double) value / (double) PROBABILITY_SCALE;
		f = f * 100.0;
		sprintf(buf, "%0.2f %%", f);
		str = buf;
		DDX_Text(pDX, nIDC, str);
	}

}

/*
 * True is 'str' is zero length or consists of all spaces.
 */
bool StringIsBlank(const char *str)
{
	const char *p;
	ASSERT( str != NULL );

	p = str;
	while( *p ) {
		if( ! isspace(*p) )
			return FALSE;
		p++;
	}

	return TRUE;
}

//
//
// Fill program filename, and parse special xy symbols.
//
//
// The control can specify filenames, plus prepend the filename with
// this flag:
//	"xy "		<- population = 1, sexonly = true
//
//	"xy*99 "	<- population = 99, sexonly = true
//
//
// Normal flags:
//
//	"xx "		<- normal simulation, population = 1
//	"xx*99"		<- normal simulation, population = 99
//
//
//
void DDX_ProgramFilename(CDataExchange *pDX, int nIDC, bool &sexonly, int &population, CString& filename)
{
	CString tmp;
	char buf[5000];
	char buf2[5000];
	char *p;

	if( pDX->m_bSaveAndValidate ) {
		DDX_Text(pDX, nIDC, tmp);

		strcpy(buf, tmp);
		if( _strnicmp(buf, "xy ", 3) == 0 ) {
			sexonly = true;
			population = 1;
			filename = buf+3;

		} else if( _strnicmp(buf, "xy*", 3) == 0 ) {
			strcpy(buf2, buf+3);
			p = strchr(buf2, ' ');
			if( p != NULL ) {
				*p = '\0';

				sexonly = true;
				population = atoi(buf2);
				if( population > 100 ) {
					population = 100;
				} else if( population <= 0 ) {
					population = 1;
				}
				filename = p+1;
			} else {
				sexonly = false;
				population = 1;
				filename = tmp;
			}

		} else if( _strnicmp(buf, "xx ", 3) == 0 ) {
			sexonly = false;
			population = 1;
			filename = buf+3;

		} else if( _strnicmp(buf, "xx*", 3) == 0 ) {
			strcpy(buf2, buf+3);
			p = strchr(buf2, ' ');
			if( p != NULL ) {
				*p = '\0';

				sexonly = false;
				population = atoi(buf2);
				if( population > 100 ) {
					population = 100;
				} else if( population <= 0 ) {
					population = 1;
				}
				filename = p+1;
			} else {
				sexonly = false;
				population = 1;
				filename = tmp;
			}

		} else {
			sexonly = false;
			population = 1;
			filename = tmp;
		}

	} else {
		if( sexonly ) {
			if( population == 1 ) {
				tmp = "xy " + filename;
				DDX_Text(pDX, nIDC, tmp);
			} else {
				sprintf(buf, "xy*%d %s", population, filename);
				tmp = buf;
				DDX_Text(pDX, nIDC, tmp);
			}
		} else {
			if( population == 1 ) {
				DDX_Text(pDX, nIDC, filename);
			} else {
				sprintf(buf, "xx*%d %s", population, filename);
				tmp = buf;
				DDX_Text(pDX, nIDC, tmp);
			}
		}
	}
}