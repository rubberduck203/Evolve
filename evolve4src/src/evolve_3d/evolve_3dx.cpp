/* ---------------------------------------------------------------------- */
// -----------------------------------
// Version: Hierarchical Display lists
// Antonio Ramires Fernandes
// www.lighthouse3d.com
// -----------------------------------

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "evolve_simulator.h"

#include <GL/glut.h>

#include "camera.h"
#include "textures.h"

#define ASSERT(x)	assert(x)

static void MakeCells(UNIVERSE *u, bool showall);

float angle=0.0;
float x=0.0f,y=10.75f,z=5.0f;
float lx=0.0f,ly=0.0f,lz=-1.0f;
float ratio=1.0;
int frame,time,timebase=0;
char s[30];

GLuint DLid;
GLuint barrierDL, organicDL, sporeDL, cellDL, celldeadDL;
GLuint all_barriers_DL = -1;
GLuint all_cells_DL = -1;

bool pause_sim;
int sim_steps = 1;
bool mouse_moved;
bool mouse_first_time = true;

UNIVERSE *universe;

CCamera Camera;

COGLTexture Tex1, Tex2, Tex3, Tex4, Tex5, Tex6;
COGLTexture TexFT, TexBK, TexLF, TexRT, TexUP, TexDN;

bool akey_flag, dkey_flag, wkey_flag, skey_flag, xkey_flag, ykey_flag;
bool ckey_flag, vkey_flag, fkey_flag, rkey_flag, mkey_flag, nkey_flag;
bool spkey_flag;
bool lf_key_flag, rt_key_flag, up_key_flag, dn_key_flag;

int scw, sch;

void changeSize(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	scw = w;
	sch = h;

	ratio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the clipping volume
#if 0
	gluPerspective(45,ratio,1,1000);
#else
	gluPerspective(45,ratio,1,5000);
	Camera.frustum.setCamInternals(45, ratio, 1, 5000);
#endif
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#if 0
	gluLookAt(x, y, z, 
		      x + lx, y + ly, z + lz,
			  0.0f,1.0f,0.0f);
#else
	float x, y, z;

	x = universe->width/2.0;
	y = 100.0;
	z = universe->height/2.0;

	Vec3 p(x, y, z);
	Vec3 l(x, y, z - 1.0);
	Vec3 u(0, 1, 0);

	gluLookAt(p.x, p.y, p.z, 
		      l.x, l.y, l.z,
			  u.x, u.y, u.z);

	Camera.frustum.setCamDef(p, l, u);
#endif


}

void initScene()
{
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);			// Enable Smooth Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// BEGIN CAMERA STUFF
	glLoadIdentity();
	Camera.Render();
	// END CAMERA STUFF

	glCallList(all_barriers_DL);
	glCallList(all_cells_DL);

#if 0
	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(s,"FPS:%4.2f", frame*1000.0/(time-timebase));
		glutSetWindowTitle(s);
		timebase = time;		
		frame = 0;
	}
#endif
	glutSwapBuffers();
}

void InitKeyFlags()
{
	akey_flag = false;
	dkey_flag = false;
	wkey_flag = false;
	skey_flag = false;
	xkey_flag = false;
	ykey_flag = false;
	ckey_flag = false;
	vkey_flag = false;
	fkey_flag = false;
	rkey_flag = false;
	mkey_flag = false;
	nkey_flag = false;
	spkey_flag = false;
	lf_key_flag = false;
	rt_key_flag = false;
	up_key_flag = false;
	dn_key_flag = false;
}


bool ProcessKeyFlags()
{
	bool changed;

	changed = false;

	if( akey_flag || lf_key_flag ) {
		Camera.StrafeRight(-2.0);
		changed = true;
	}

	if( dkey_flag || rt_key_flag ) {
		Camera.StrafeRight(2.0);
		changed = true;
	}

	if( wkey_flag || up_key_flag ) {
		Camera.MoveForward( -2.0 ) ;
		changed = true;
	}

	if( skey_flag || dn_key_flag ) {
		Camera.MoveForward( 2.0 ) ;
		changed = true;
	}

#if 0
	if( xkey_flag ) {
		Camera.RotateX(5.0);
		changed = true;
	}

	if( ykey_flag ) {
		Camera.RotateX(-5.0);
		changed = true;
	}

	if( ckey_flag ) {
		Camera.StrafeRight(-2.0);
		changed = true;
	}

	if( vkey_flag ) {
		Camera.StrafeRight(2.0);
		changed = true;
	}

	if( fkey_flag ) {
		Camera.MoveUpward(-0.3);
		changed = true;
	}

	if( rkey_flag ) {
		Camera.MoveUpward(0.3);
		changed = true;
	}

	if( mkey_flag ) {
		Camera.RotateZ(-5.0);
		changed = true;
	}

	if( nkey_flag ) {
		Camera.RotateZ(5.0);
		changed = true;
	}
#endif

	if( spkey_flag ) {
		Camera.MoveUpward(0.5);
		changed = true;
	}

	return changed;
}


//
// Special key down processing
//
void SpecialKeyDown(int key, int x, int y)
{
	switch( key ) {
	case GLUT_KEY_F1:
		if( pause_sim ) {
			sim_steps = 1;
			pause_sim = false;
		} else {
			pause_sim = true;
			MakeCells(universe, true);
		}
		break;

	case GLUT_KEY_LEFT:
		lf_key_flag = true;
		break;

	case GLUT_KEY_RIGHT:
		rt_key_flag = true;
		break;

	case GLUT_KEY_UP:
		up_key_flag = true;
		break;

	case GLUT_KEY_DOWN:
		dn_key_flag = true;
		break;
	}
}

void SpecialKeyUp(int key, int x, int y)
{
	switch( key ) {
	case GLUT_KEY_LEFT:
		lf_key_flag = false;
		break;

	case GLUT_KEY_RIGHT:
		rt_key_flag = false;
		break;

	case GLUT_KEY_UP:
		up_key_flag = false;
		break;

	case GLUT_KEY_DOWN:
		dn_key_flag = false;
		break;
	}
}

//
// CAMERA
//
void KeyDown(unsigned char key, int x, int y)
{

	switch (key) 
	{
	case 3:		// ^C
	case 27:	 // ESC
		exit(0);
		break;

	case 'a':
		akey_flag = true;
		break;

	case 'd':
		dkey_flag = true;
		break;

	case 'w':
		wkey_flag = true;
		break;

	case 's':
		skey_flag = true;
		break;

#if 0
	case 'x':
		xkey_flag = true;
		break;

	case 'y':
		ykey_flag = true;
		break;

	case 'c':
		ckey_flag = true;
		break;

	case 'v':
		vkey_flag = true;
		break;

	case 'f':
		fkey_flag = true;
		break;

	case 'r':
		rkey_flag = true;
		break;

	case 'm':
		mkey_flag = true;
		break;

	case 'n':
		nkey_flag = true;
		break;
#endif

	case ' ':
		spkey_flag = true;
		break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		sim_steps = (key - '0');
		break;

	}
}

void KeyUp(unsigned char key, int x, int y)
{
	switch(key) {
	case 'a':
		akey_flag = false;
		break;

	case 'd':
		dkey_flag = false;
		break;

	case 'w':
		wkey_flag = false;
		break;

	case 's':
		skey_flag = false;	
		break;

#if 0
	case 'x':
		xkey_flag = false;
		break;

	case 'y':
		ykey_flag = false;
		break;

	case 'c':
		ckey_flag = false;
		break;

	case 'v':
		vkey_flag = false;
		break;

	case 'f':
		fkey_flag = false;
		break;

	case 'r':
		rkey_flag = false;
		break;

	case 'm':
		mkey_flag = false;
		break;

	case 'n':
		nkey_flag = false;
		break;
#endif

	case ' ':
		spkey_flag = false;
		break;
	}
}

//
// When focus leaves this window, turn off simulator
// and reset any key down flags
//
void processMouseLeave(int state)
{
	if( state == GLUT_LEFT ) {
		InitKeyFlags();
		pause_sim = true;
		MakeCells(universe, true);

	} else if( state == GLUT_ENTERED ) {
		// do nothing
	}
}

void processMouseClicks(int button, int state, int x, int y)
{
	if( button == GLUT_LEFT_BUTTON ) {
		if( state == GLUT_DOWN ) {
			mouse_first_time = true;
		} else {
		}
	}
}

void processMouseMotion(int x, int y)
{
	static int prev_x, prev_y;
	float dx, dy;

	if( mouse_first_time ) {
		prev_x = x;
		prev_y = y;
		mouse_first_time = false;
		return;
	}

	dx = (prev_x - x) / 3.0;
	dy = (prev_y - y) / 3.0;

	prev_x = x;
	prev_y = y;

	Camera.RotateX(dy);
	Camera.RotateY(dx);
	mouse_moved = true;

}

void idleProc(void)
{
	int i;
	bool changed;

	changed = ProcessKeyFlags();

	if( ! pause_sim ) {
		for(i=0; i < sim_steps; i++) {
			Universe_Simulate(universe);
		}

		MakeCells(universe, false);
		changed = true;
	}

	if( changed || mouse_moved ) {
		mouse_moved = false;
		renderScene();
	}
}





void DrawCube(COGLTexture &tex)
{
	glEnable(GL_TEXTURE_2D);
	tex.SetActive();

	// front:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(-0.5,-0.5,0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(-0.5,0.5,0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(0.5,0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(0.5,-0.5,0.5);
	glEnd();

	// back:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(-0.5,-0.5,-0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(0.5,-0.5,-0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(0.5,0.5,-0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(-0.5,0.5,-0.5);
	glEnd();

	// top:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(-0.5,0.5,-0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(0.5,0.5,-0.5	);

		glTexCoord2f(1.0,1.0);
		glVertex3f(0.5,0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(-0.5,0.5,0.5);

#if 0
	// bottom:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(-0.5,-0.5,-0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(-0.5,-0.5,0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(0.5,-0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(0.5,-0.5,-0.5);
	glEnd();
#endif

	//left:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(-0.5,-0.5,-0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(-0.5,0.5,-0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(-0.5,0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(-0.5,-0.5,0.5);
	glEnd();

	// right:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,0.0);
		glVertex3f(0.5,-0.5,-0.5);

		glTexCoord2f(1.0,0.0);
		glVertex3f(0.5,-0.5,0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(0.5,0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(0.5,0.5,-0.5);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

//
// Draws a cube with two shaded, two one-colored and two textured faces
//
void DrawCell(void)
{
	DrawCube(Tex1);
}

void DrawCellDead(void)
{
	DrawCube(Tex3);
}

void DrawSpore(void)
{
	DrawCube(Tex2);
}

void DrawBarrier(void)
{
	DrawCube(Tex4);
}

void DrawOrganic(void)
{
	DrawCube(Tex5);
}

static void MakeBlocks(void)
{
	barrierDL = glGenLists(1);
	organicDL = glGenLists(1);
	sporeDL = glGenLists(1);
	cellDL = glGenLists(1);
	celldeadDL = glGenLists(1);

	glNewList(barrierDL, GL_COMPILE);
	DrawBarrier();
	glEndList();

	glNewList(organicDL, GL_COMPILE);
	DrawOrganic();
	glEndList();

	glNewList(sporeDL, GL_COMPILE);
	DrawSpore();
	glEndList();

	glNewList(cellDL, GL_COMPILE);
	DrawCell();
	glEndList();

	glNewList(celldeadDL, GL_COMPILE);
	DrawCellDead();
	glEndList();
	
}

//
//  front - TexFront
//
void Draw_Skybox(UNIVERSE *u)
{
	float x, y, z, width, height, length;
	float side;

#if 1
	if( u->width > u->height ) {
		side = u->width;
	} else {
		side = u->height;
	}

	width = side * 3.0;
	height = side * 3.0;
	length = side * 3.0;

	x = u->width / 2.0;
	y = height/5.0;
	z = u->height / 2.0;

#else
	width = u->width;
	height = 1000;
	length = u->height;
#endif

	// Center the Skybox around the given x,y,z position
	x = x - width  / 2;
	y = y - height / 2;
	z = z - length / 2;

	glEnable(GL_TEXTURE_2D);

	// Draw Front side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYFRONT]);
	TexFT.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		y+height,	z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width,	y+height,	z+length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width,	y,		z+length);
	glEnd();

	// Draw Back side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYBACK]);
	TexBK.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height, z); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
	glEnd();

	// Draw Left side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYLEFT]);
	TexLF.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z);	
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z+length); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z);		
	glEnd();

	// Draw Right side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYRIGHT]);
	TexRT.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height,	z+length); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height,	z);
	glEnd();

	// Draw Up side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYUP]);
	TexUP.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length); 
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x,		  y+height,	z+length);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x,		  y+height,	z);
	glEnd();

	// Draw Down side
	//glBindTexture(GL_TEXTURE_2D, SkyboxTexture[SKYDOWN]);
	TexDN.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x,		  y,		z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y,		z+length); 
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y,		z);
	glEnd();

	glDisable(GL_TEXTURE_2D);

}

static void MakeBarriers(UNIVERSE *u)
{
	int x, y;
	UNIVERSE_GRID ugrid;

	ASSERT( u != NULL );

	if( all_barriers_DL != -1 ) {
		glDeleteLists(all_barriers_DL, 1);
		all_barriers_DL = -1;
	}

	all_barriers_DL = glGenLists(1);

	glNewList(all_barriers_DL, GL_COMPILE);

	// Draw skybox
	Draw_Skybox(u);
	

	// Draw ground
	glEnable(GL_TEXTURE_2D);
	Tex6.SetActive();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(0.0f,		0.0f,	0.0f);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(u->width,		0.0f,	0.0f);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(u->width,		0.0f,	u->height);

		glTexCoord2f(0.0, 1.0);
		glVertex3f(0.0f,		0.0f,	u->height);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// draw barrier blocks
	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			Universe_Query(u, x, y, &ugrid);
			if( ugrid.type == GT_BARRIER ) {
				glPushMatrix();
				glTranslatef(x*1.0, 0.75f, y*1.0);
				glCallList(barrierDL);
				glPopMatrix();
			}
		}
	}

	glEndList();

}

static void MakeCells(UNIVERSE *u, bool showall)
{
	int x, y;
	UNIVERSE_GRID ugrid;
	ORGANISM *o;
	CELL *c;
	float posx, posy;
	bool in_range;
	Vec3 p;

	ASSERT( u != NULL );

#if 0
	if( all_cells_DL != -1 ) {
		glDeleteLists(all_cells_DL, 1);
		all_cells_DL = -1;
	}
#else
	if( all_cells_DL == -1 ) {
		all_cells_DL = glGenLists(1);
	}
#endif

	posx = Camera.GetPosition().x;
	posy = Camera.GetPosition().z;

	glNewList(all_cells_DL, GL_COMPILE);

	for(o=u->organisms; o; o=o->next) {
		for(c=o->cells; c; c=c->next) {

			p.x = c->x * 1.0;
			p.y = 0.75f;
			p.z = c->y * 1.0;

			if( ! showall ) {
				if( Camera.frustum.sphereInFrustum(p, 0.866025404) )
					in_range = true;
				else
					in_range = false;
			} else {
				in_range = true;
			}

			if( in_range ) {
				glPushMatrix();
				glTranslatef(p.x, p.y, p.z);
				if( c->kfm->terminated ) {
					glCallList(celldeadDL);
				} else {
					glCallList(cellDL);
				}
				glPopMatrix();
			}
		}
	}

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			Universe_Query(u, x, y, &ugrid);
			if( ugrid.type == GT_SPORE || ugrid.type == GT_ORGANIC ) {
				p.x = x * 1.0;
				p.y = 0.75;
				p.z = y * 1.0;

				if( ! showall ) {
					if( Camera.frustum.sphereInFrustum(p, 0.866025404) )
						in_range = true;
					else
						in_range = false;
				} else {
					in_range = true;
				}

				if( in_range ) {
					glPushMatrix();
					glTranslatef(p.x, p.y, p.z);
					if( ugrid.type == GT_SPORE ) {
						glCallList(sporeDL);
					} else if( ugrid.type == GT_ORGANIC ) {
						glCallList(organicDL);
					}
					glPopMatrix();
				}
			}
		}
	}

	glEndList();

}

void InitTextures(void)
{
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

#if 0
	Tex1.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_cell.bmp");
	Tex2.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_spore.bmp");
	Tex3.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_cell_dead.bmp");
	Tex4.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_barrier.bmp");
	Tex5.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_organic.bmp");
	Tex6.LoadFromFile("d:/stauffer/Development/opengl/snowman/tex_ground.bmp");

	TexFT.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_ft.bmp");
	TexBK.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_bk.bmp");
	TexLF.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_lf.bmp");
	TexRT.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_rt.bmp");
	TexUP.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_up.bmp");
	TexDN.LoadFromFile("d:/stauffer/Development/opengl/snowman/skybox_dn.bmp");
#else
	Tex1.LoadFromFile("tex_cell.bmp");
	Tex2.LoadFromFile("tex_spore.bmp");
	Tex3.LoadFromFile("tex_cell_dead.bmp");
	Tex4.LoadFromFile("tex_barrier.bmp");
	Tex5.LoadFromFile("tex_organic.bmp");
	Tex6.LoadFromFile("tex_ground.bmp");

	TexFT.LoadFromFile("skybox_ft.bmp");
	TexBK.LoadFromFile("skybox_bk.bmp");
	TexLF.LoadFromFile("skybox_lf.bmp");
	TexRT.LoadFromFile("skybox_rt.bmp");
	TexUP.LoadFromFile("skybox_up.bmp");
	TexDN.LoadFromFile("skybox_dn.bmp");
#endif

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}


int main(int argc, char **argv)
{
	UNIVERSE *u;
	char filename[1000], errbuf[1000];

	printf("\n");
	printf("Evolve 4.0 3D Explorer\n");
	printf("=======================\n");
	printf("Keys:\n");
	printf("    W or Up Arrow      -- move forward\n");
	printf("    S or Down Arrow    -- move backward\n");
	printf("    A or Left Arrow    -- strafe left\n");
	printf("    D or Right Arrow   -- strafe right\n");
	printf("    space bar          -- climb\n");
	printf("    F1                 -- toggle simulation on/off\n");
	printf("    1, 2, 3, ... 9     -- change number of simulation steps\n");
	printf("    ESC or ^C          -- exit program\n");
	printf("\n");
	printf("Mouse:\n");
	printf("    Press left mouse button and drag mouse to pan camera.\n");
	printf("\n");	
	printf("Input a filename to load, or just press [enter]\n");
	printf("to load a sample simulation file.\n");
	printf("\n");

	do {
		printf("\n");
		printf("Enter simulation filename [test.evolve]: ");
		gets(filename);

		if( filename[0] == '\0' ) {
			strcpy(filename, "test.evolve");
		}

		u = Universe_Read(filename, errbuf);

		if( u == NULL ) {
			printf("error: %s\n", errbuf);
			printf("\n");
			printf("Let's try again.\n");
		}

	} while( u == NULL );

	universe = u;

	Camera.set_universe(u);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(300,300);
	glutCreateWindow("Evolve 4.0 - 3d explorer");

	Camera.Move( F3dVector(u->width/2.0, 10.0, u->height/2.0 ) );
	Camera.MoveForward( 1.0 );

	initScene();
	InitTextures();

	MakeBlocks();
	MakeBarriers(u);
	MakeCells(u, true);

	InitKeyFlags();

	pause_sim = true;
	mouse_moved = false;
	sim_steps = 1;

	glutIgnoreKeyRepeat(1);

	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutMotionFunc(processMouseMotion);
	glutMouseFunc(processMouseClicks);
	glutEntryFunc(processMouseLeave);

	glutSpecialFunc(SpecialKeyDown);
	glutSpecialUpFunc(SpecialKeyUp);

	glutDisplayFunc(renderScene);
	glutIdleFunc(idleProc);

	glutReshapeFunc(changeSize);

	glutMainLoop();

	Universe_Delete(u);

	return(0);
}

