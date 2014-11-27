/***********************************************************************
 * evolve_3d.cpp
 *
 * Display an Evolve 4.0 simulation file in 3D. Allow
 * user to fly around and explore.
 *
 * Includes GAME MODE.
 *
 * This is hack code cobbled together from stuff found
 * on the internet. Praise Google!
 *
 *
 */

#include "stdafx.h"


/***********************************************************************
 *
 * Global data
 *
 */
float angle=0.0;
float x = 0.0f, y = 10.75f, z = 5.0f;
float lx = 0.0f, ly=0.0f, lz = -1.0f;
float ratio = 1.0;
int frame, time, timebase=0;
char s[30];

GLuint DLid;
GLuint barrierDL, organicDL, sporeDL, sporetDL, celldeadDL, celldeadtDL;
GLuint playerDL;
GLuint cell1DL,  cell2DL,  cell3DL,  cell4DL,  cell5DL,  cell6DL,  cell7DL,  cell8DL;
GLuint cell1tDL,  cell2tDL,  cell3tDL,  cell4tDL,  cell5tDL,  cell6tDL,  cell7tDL,  cell8tDL;
GLuint all_barriers_DL = -1;
GLuint all_cells_DL = -1;

bool pause_sim;
int sim_steps = 1;
bool mouse_moved;
bool mouse_first_time = true;

UNIVERSE *universe;

PLAYER_COMMAND cmd;

CCamera Camera;
ThirdPersonCamera GameCamera;
bool game_mode, game_mode_changed;
FrustumG frustum;
int shown_energy;

COGLTexture Tex1a, Tex1b, Tex1c, Tex1d, Tex1e, Tex1f, Tex1g, Tex1h;
COGLTexture Tex1at, Tex1bt, Tex1ct, Tex1dt, Tex1et, Tex1ft, Tex1gt, Tex1ht;
COGLTexture Tex2, Tex3, Tex4, Tex5, Tex6, Tex7;
COGLTexture Tex2t, Tex3t;
COGLTexture TexFT, TexBK, TexLF, TexRT, TexUP, TexDN;

bool akey_flag, dkey_flag, wkey_flag, skey_flag;
bool ckey_flag, spkey_flag;
bool lf_key_flag, rt_key_flag, up_key_flag, dn_key_flag;

bool delete_mode;
char delete_file[ 1000 ];

int scw, sch;

/***********************************************************************
 * Proto-types
 */
static void MakeCells(UNIVERSE *u, bool showall);

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
	gluPerspective(45, ratio, 1, 5000);
	frustum.setCamInternals(45, ratio, 1, 5000);

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

	frustum.setCamDef(p, l, u);
#endif


}

void initScene()
{
	glEnable(GL_DEPTH_TEST);

	// Enable Smooth Shading
	glShadeModel(GL_SMOOTH);

	// Really Nice Perspective Calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// BEGIN CAMERA STUFF

	glLoadIdentity();

	if( game_mode ) {
		PLAYER *p;
		p = Player_Get(universe);
		if( p != NULL ) {
			Vector3 pos;
			pos.x = p->x * 1.0;
			//pos.y = 0.75f;
			pos.y = 4.0f;
			pos.z = p->y * 1.0;
			GameCamera.setTargetPosition(pos);
			GameCamera.update(0.0);
#if 0
			//glViewport(0, 0, 800, 600);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMultMatrixf(&GameCamera.projMatrix[0][0]);
    
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMultMatrixf(&GameCamera.viewMatrix[0][0]);
#endif

			gluLookAt(
				GameCamera.p.x, GameCamera.p.y, GameCamera.p.z,
				GameCamera.l.x, GameCamera.l.y, GameCamera.l.z,
				GameCamera.u.x, GameCamera.u.y, GameCamera.u.z );

		       frustum.setCamDef(GameCamera.p, GameCamera.l, GameCamera.u);
		} else {
			// DEAD
		}

	} else {
		Camera.Render();
		gluLookAt(
			Camera.p.x, Camera.p.y, Camera.p.z,
			Camera.l.x, Camera.l.y, Camera.l.z,
			Camera.u.x, Camera.u.y, Camera.u.z );

		frustum.setCamDef(Camera.p, Camera.l, Camera.u);
	}

	// END CAMERA STUFF

	glCallList(all_barriers_DL);
	glCallList(all_cells_DL);

#if 0
	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(s, "FPS:%4.2f", frame*1000.0/(time-timebase));
		glutSetWindowTitle(s);
		timebase = time;
		frame = 0;
	}
#endif

#if 0
	if( game_mode ) {
		double dx, dy, angle;

		angle = GameCamera.heading();
		dx = GameCamera.l.x - GameCamera.p.x;
		dy = GameCamera.p.z - GameCamera.l.z;

		sprintf(s, "L: (%4.2f, %4.2f) angle: %4.2f",
			dx, dy, angle );
			glutSetWindowTitle(s);
	}

#endif
	if( game_mode ) {
		PLAYER *p;
		p = Player_Get(universe);
		if( p != NULL ) {
			if( p->energy != shown_energy ) {
				sprintf(s, "Evolve 4.0 - Game Mode, Energy: %d", p->energy);
				glutSetWindowTitle(s);
				shown_energy = p->energy;
			}
		}
	}

	glutSwapBuffers();
}

void InitKeyFlags()
{
	akey_flag = false;
	dkey_flag = false;
	wkey_flag = false;
	skey_flag = false;
	ckey_flag = false;
	spkey_flag = false;
	lf_key_flag = false;
	rt_key_flag = false;
	up_key_flag = false;
	dn_key_flag = false;

	game_mode = false;
	game_mode_changed = false;
}

bool ProcessKeyFlags_Normal()
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
		Camera.MoveForward( -2.0 );
		changed = true;
	}

	if( skey_flag || dn_key_flag ) {
		Camera.MoveForward( 2.0 );
		changed = true;
	}

	if( spkey_flag ) {
		Camera.MoveUpward(0.5);
		changed = true;
	}

	if( ckey_flag ) {
		Camera.MoveUpward(-0.5);
		changed = true;
	}

	return changed;
}

void kenny_forward(double heading, int &x, int &y)
{
	if( heading <= 22.5 ) {
		x = 0; y = -1;
	} else if( heading <= 67.5 ) {
		x = 1; y = -1;
	} else if( heading <= 112.5 ) {
		x = 1; y = 0;
	} else if( heading <= 157.5 ) {
		x = 1; y = 1;
	} else if( heading <= 202.5 ) {
		x = 0; y = 1;
	} else if( heading <= 247.5 ) {
		x = -1; y = 1;
	} else if( heading <= 292.5 ) {
		x = -1; y = 0;
	} else if( heading <= 337.5 ) {
		x = -1; y = -1;
	} else {
		x = 0; y = -1;
	}
}

void kenny_back(double heading, int &x, int &y)
{
	if( heading <= 22.5 ) {
		x = 0; y = 1;
	} else if( heading <= 67.5 ) {
		x = -1; y = 1;
	} else if( heading <= 112.5 ) {
		x = -1; y = 0;
	} else if( heading <= 157.5 ) {
		x = -1; y = -1;
	} else if( heading <= 202.5 ) {
		x = 0; y = -1;
	} else if( heading <= 247.5 ) {
		x = 1; y = -1;
	} else if( heading <= 292.5 ) {
		x = 1; y = 0;
	} else if( heading <= 337.5 ) {
		x = 1; y = 1;
	} else {
		x = 0; y = 1;
	}
}

void kenny_left(double heading, int &x, int &y)
{
	if( heading <= 22.5 ) {
		x = -1; y = 0;
	} else if( heading <= 67.5 ) {
		x = -1; y = -1;
	} else if( heading <= 112.5 ) {
		x = 0; y = -1;
	} else if( heading <= 157.5 ) {
		x = 1; y = -1;
	} else if( heading <= 202.5 ) {
		x = 1; y = 0;
	} else if( heading <= 247.5 ) {
		x = 1; y = 1;
	} else if( heading <= 292.5 ) {
		x = 0; y = 1;
	} else if( heading <= 337.5 ) {
		x = -1; y = 1;
	} else {
		x = -1; y = 0;
	}
}

void kenny_right(double heading, int &x, int &y)
{
	if( heading <= 22.5 ) {
		x = 1; y = 0;
	} else if( heading <= 67.5 ) {
		x = 1; y = 1;
	} else if( heading <= 112.5 ) {
		x = 0; y = 1;
	} else if( heading <= 157.5 ) {
		x = -1; y = 1;
	} else if( heading <= 202.5 ) {
		x = -1; y = 0;
	} else if( heading <= 247.5 ) {
		x = -1; y = -1;
	} else if( heading <= 292.5 ) {
		x = 0; y = -1;
	} else if( heading <= 337.5 ) {
		x = 1; y = -1;
	} else {
		x = 1; y = 0;
	}
}

bool ProcessKeyFlags_GameMode()
{
	bool changed;
	double heading;

	changed = false;

	heading = GameCamera.heading();

	if( akey_flag || lf_key_flag ) {
		// move player
		cmd.type = PCMD_OMOVE;
		kenny_left(heading, cmd.x, cmd.y);
		if( Player_Get(universe) != NULL ) {
			if( Player_Get(universe)->cmd.type == PCMD_NOP ) {
				Player_SetNextCommand(universe, &cmd);
				changed = true;
			}
		}
	}

	if( dkey_flag || rt_key_flag ) {
		// move player
		cmd.type = PCMD_OMOVE;
		kenny_right(heading, cmd.x, cmd.y);
		if( Player_Get(universe) != NULL ) {
			if( Player_Get(universe)->cmd.type == PCMD_NOP ) {
				Player_SetNextCommand(universe, &cmd);
				changed = true;
			}
		}
	}

	if( wkey_flag || up_key_flag ) {
		// move player
		cmd.type = PCMD_OMOVE;
		kenny_forward(heading, cmd.x, cmd.y);
		if( Player_Get(universe) != NULL ) {
			if( Player_Get(universe)->cmd.type == PCMD_NOP ) {
				Player_SetNextCommand(universe, &cmd);
				changed = true;
			}
		}
	}

	if( skey_flag || dn_key_flag ) {
		// move player
		cmd.type = PCMD_OMOVE;
		kenny_back(heading, cmd.x, cmd.y);
		if( Player_Get(universe) != NULL ) {
			if( Player_Get(universe)->cmd.type == PCMD_NOP ) {
				Player_SetNextCommand(universe, &cmd);
				changed = true;
			}
		}
	}

	if( spkey_flag ) {
		GameCamera.climb(0.25);
		changed = true;
	}

	if( ckey_flag ) {
		GameCamera.climb(-0.25);
		changed = true;
	}

	if( game_mode_changed ) {
		game_mode_changed = false;
		changed = true;
	}

	return changed;
}

bool ProcessKeyFlags()
{
	if( game_mode ) {
		return ProcessKeyFlags_GameMode();
	} else {
		return ProcessKeyFlags_Normal();
	}
}


//
// Special key down processing
//
void SpecialKeyDown(int key, int x, int y)
{
	switch( key ) {
	case GLUT_KEY_F1:
		if( pause_sim ) {
			pause_sim = false;
			sim_steps = 1;
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
		if( delete_mode ) {
			DeleteFile(delete_file);
		}
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

	case ' ':
		spkey_flag = true;
		break;

	case 'c':
		ckey_flag = true;
		break;

	case 'g':
		//
		// game mode toggle.
		//
		if( game_mode ) {
			game_mode = false;
			if( Player_Get(universe) != NULL ) {
				Player_Remove(universe);
			}
			glutSetCursor(GLUT_CURSOR_INHERIT);

			if( pause_sim ) {
				pause_sim = false;
				sim_steps = 1;
			}
			glutSetWindowTitle("Evolve 4.0 - 3D Explorer");

		} else {
			game_mode = true;
			if( Player_Get(universe) == NULL ) {
				Player_Add(universe, 0, 0);
			}
			glutSetCursor(GLUT_CURSOR_NONE);
			if( pause_sim ) {
				pause_sim = false;
			}
			sim_steps = 1;
			shown_energy = -1;
		}
		game_mode_changed = true;
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

	case ' ':
		spkey_flag = false;
		break;

	case 'c':
		ckey_flag = false;
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

		// Kent Dolan feature request, don't stop sim when
		// leaving window.
		//
		// pause_sim = true;


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
			if( game_mode ) {
				cmd.type = PCMD_EAT_ALL;
				if( Player_Get(universe) != NULL ) {
					Player_SetNextCommand(universe, &cmd);
				}
			}
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

	dx = (prev_x - x);
	dy = (prev_y - y);

	prev_x = x;
	prev_y = y;

	if( game_mode ) {
		GameCamera.rotate(dx/5.0, 0.0);
		GameCamera.update(1.0);

		//
		//		GameCamera.climb(-dy/10.0);
		//

		mouse_moved = true;
	} else {
		Camera.RotateX(dy / 3.0);
		Camera.RotateY(dx / 3.0);
		mouse_moved = true;
	}

}

void processPassiveMouseMotion(int x, int y)
{
	if( game_mode ) {
		processMouseMotion(x, y);

		if( x < scw/3 || x > (scw - scw/3) ) {
			glutWarpPointer(scw/2, sch/2);
			mouse_first_time = true;
		} else if( y < sch/3 || y > (sch - sch/3) ) {
			glutWarpPointer(scw/2, sch/2);
			mouse_first_time = true;
		}

	}
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
		glTexCoord2f(1.0,0.0);
		glVertex3f(-0.5,-0.5,0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(-0.5,0.5,0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(0.5,0.5,0.5);

		glTexCoord2f(0.0,0.0);
		glVertex3f(0.5,-0.5,0.5);
	glEnd();

	// back:
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-0.5,-0.5,-0.5);

		glTexCoord2f(1.0, 0.0);
		glVertex3f(0.5,-0.5,-0.5);

		glTexCoord2f(1.0, 1.0);
		glVertex3f(0.5,0.5,-0.5);

		glTexCoord2f(0.0, 1.0);
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
		glTexCoord2f(1.0,0.0);
		glVertex3f(-0.5,-0.5,-0.5);

		glTexCoord2f(1.0,1.0);
		glVertex3f(-0.5,0.5,-0.5);

		glTexCoord2f(0.0,1.0);
		glVertex3f(-0.5,0.5,0.5);

		glTexCoord2f(0.0,0.0);
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
void DrawCell_1(void)
{
	DrawCube(Tex1a);
}

void DrawCell_1t(void)
{
	DrawCube(Tex1at);
}

void DrawCell_2(void)
{
	DrawCube(Tex1b);
}

void DrawCell_2t(void)
{
	DrawCube(Tex1bt);
}

void DrawCell_3(void)
{
	DrawCube(Tex1c);
}

void DrawCell_3t(void)
{
	DrawCube(Tex1ct);
}

void DrawCell_4(void)
{
	DrawCube(Tex1d);
}

void DrawCell_4t(void)
{
	DrawCube(Tex1dt);
}

void DrawCell_5(void)
{
	DrawCube(Tex1e);
}

void DrawCell_5t(void)
{
	DrawCube(Tex1et);
}

void DrawCell_6(void)
{
	DrawCube(Tex1f);
}

void DrawCell_6t(void)
{
	DrawCube(Tex1ft);
}

void DrawCell_7(void)
{
	DrawCube(Tex1g);
}

void DrawCell_7t(void)
{
	DrawCube(Tex1gt);
}

void DrawCell_8(void)
{
	DrawCube(Tex1h);
}

void DrawCell_8t(void)
{
	DrawCube(Tex1ht);
}

void DrawCellDead(void)
{
	DrawCube(Tex3);
}

void DrawCellDeadt(void)
{
	DrawCube(Tex3t);
}

void DrawPlayer(void)
{
	DrawCube(Tex7);
}

void DrawSpore(void)
{
	DrawCube(Tex2);
}

void DrawSporet(void)
{
	DrawCube(Tex2t);
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
	cell1DL = glGenLists(1);
	cell2DL = glGenLists(1);
	cell3DL = glGenLists(1);
	cell4DL = glGenLists(1);
	cell5DL = glGenLists(1);
	cell6DL = glGenLists(1);
	cell7DL = glGenLists(1);
	cell8DL = glGenLists(1);
	celldeadDL = glGenLists(1);
	playerDL = glGenLists(1);

	sporetDL = glGenLists(1);
	cell1tDL = glGenLists(1);
	cell2tDL = glGenLists(1);
	cell3tDL = glGenLists(1);
	cell4tDL = glGenLists(1);
	cell5tDL = glGenLists(1);
	cell6tDL = glGenLists(1);
	cell7tDL = glGenLists(1);
	cell8tDL = glGenLists(1);
	celldeadtDL = glGenLists(1);

	glNewList(barrierDL, GL_COMPILE);
	DrawBarrier();
	glEndList();

	glNewList(organicDL, GL_COMPILE);
	DrawOrganic();
	glEndList();

	glNewList(sporeDL, GL_COMPILE);
	DrawSpore();
	glEndList();

	glNewList(sporetDL, GL_COMPILE);
	DrawSporet();
	glEndList();

	glNewList(cell1DL, GL_COMPILE);
	DrawCell_1();
	glEndList();

	glNewList(cell1tDL, GL_COMPILE);
	DrawCell_1t();
	glEndList();

	glNewList(cell2DL, GL_COMPILE);
	DrawCell_2();
	glEndList();

	glNewList(cell2tDL, GL_COMPILE);
	DrawCell_2t();
	glEndList();

	glNewList(cell3DL, GL_COMPILE);
	DrawCell_3();
	glEndList();

	glNewList(cell3tDL, GL_COMPILE);
	DrawCell_3t();
	glEndList();

	glNewList(cell4DL, GL_COMPILE);
	DrawCell_4();
	glEndList();

	glNewList(cell4tDL, GL_COMPILE);
	DrawCell_4t();
	glEndList();

	glNewList(cell5DL, GL_COMPILE);
	DrawCell_5();
	glEndList();

	glNewList(cell5tDL, GL_COMPILE);
	DrawCell_5t();
	glEndList();

	glNewList(cell6DL, GL_COMPILE);
	DrawCell_6();
	glEndList();

	glNewList(cell6tDL, GL_COMPILE);
	DrawCell_6t();
	glEndList();

	glNewList(cell7DL, GL_COMPILE);
	DrawCell_7();
	glEndList();

	glNewList(cell7tDL, GL_COMPILE);
	DrawCell_7t();
	glEndList();

	glNewList(cell8DL, GL_COMPILE);
	DrawCell_8();
	glEndList();

	glNewList(cell8tDL, GL_COMPILE);
	DrawCell_8t();
	glEndList();

	glNewList(celldeadDL, GL_COMPILE);
	DrawCellDead();
	glEndList();

	glNewList(celldeadtDL, GL_COMPILE);
	DrawCellDeadt();
	glEndList();

	glNewList(playerDL, GL_COMPILE);
	DrawPlayer();
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
	bool tracer;
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
		if( o->oflags & ORGANISM_FLAG_RADIOACTIVE ) {
			tracer = true;
		} else {
			tracer = false;
		}

		for(c=o->cells; c; c=c->next) {
			p.x = c->x * 1.0;
			p.y = 0.75f;
			p.z = c->y * 1.0;

			if( ! showall ) {
				if( frustum.sphereInFrustum(p, 0.866025404) )
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
					if( tracer ) {
						glCallList(celldeadtDL);
					} else {
						glCallList(celldeadDL);
					}
				} else {
					switch(c->organism->strain) {
					case 0:
						if( tracer )
							glCallList(cell1tDL);
						else
							glCallList(cell1DL);
						break;
					case 1:
						if( tracer )
							glCallList(cell2tDL);
						else
							glCallList(cell2DL);
						break;
					case 2:
						if( tracer )
							glCallList(cell3tDL);
						else
							glCallList(cell3DL);
						break;
					case 3:
						if( tracer )
							glCallList(cell4tDL);
						else
							glCallList(cell4DL);
						break;
					case 4:
						if( tracer )
							glCallList(cell5tDL);
						else
							glCallList(cell5DL);
						break;
					case 5:
						if( tracer )
							glCallList(cell6tDL);
						else
							glCallList(cell6DL);
						break;
					case 6:
						if( tracer )
							glCallList(cell7tDL);
						else
							glCallList(cell7DL);
						break;
					case 7:
						if( tracer )
							glCallList(cell8tDL);
						else
							glCallList(cell8DL);
						break;
					default:
						ASSERT(0);
						break;
					}

				}
				glPopMatrix();
			}
		}
	}

	for(x=0; x < u->width; x++) {
		for(y=0; y < u->height; y++) {
			Universe_Query(u, x, y, &ugrid);
			if( ugrid.type == GT_SPORE || ugrid.type == GT_ORGANIC
					|| ugrid.type == GT_PLAYER ) {
				p.x = x * 1.0;
				p.y = 0.75;
				p.z = y * 1.0;

				if( ! showall ) {
					if( frustum.sphereInFrustum(p, 0.866025404) )
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
						if( ugrid.u.spore->sflags & SPORE_FLAG_RADIOACTIVE ) {
							glCallList(sporetDL);
						} else {
							glCallList(sporeDL);
						}

					} else if( ugrid.type == GT_ORGANIC ) {
						glCallList(organicDL);
					} else if( ugrid.type == GT_PLAYER ) {
						glCallList(playerDL);
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

	Tex1a.LoadFromFile("tex_cell1.bmp");
	Tex1b.LoadFromFile("tex_cell2.bmp");
	Tex1c.LoadFromFile("tex_cell3.bmp");
	Tex1d.LoadFromFile("tex_cell4.bmp");
	Tex1e.LoadFromFile("tex_cell5.bmp");
	Tex1f.LoadFromFile("tex_cell6.bmp");
	Tex1g.LoadFromFile("tex_cell7.bmp");
	Tex1h.LoadFromFile("tex_cell8.bmp");

	Tex1at.LoadFromFile("tex_cell1t.bmp");
	Tex1bt.LoadFromFile("tex_cell2t.bmp");
	Tex1ct.LoadFromFile("tex_cell3t.bmp");
	Tex1dt.LoadFromFile("tex_cell4t.bmp");
	Tex1et.LoadFromFile("tex_cell5t.bmp");
	Tex1ft.LoadFromFile("tex_cell6t.bmp");
	Tex1gt.LoadFromFile("tex_cell7t.bmp");
	Tex1ht.LoadFromFile("tex_cell8t.bmp");

	Tex2.LoadFromFile("tex_spore.bmp");
	Tex2t.LoadFromFile("tex_sporet.bmp");

	Tex3.LoadFromFile("tex_cell_dead.bmp");
	Tex3t.LoadFromFile("tex_cell_deadt.bmp");

	Tex4.LoadFromFile("tex_barrier.bmp");

	Tex5.LoadFromFile("tex_organic.bmp");

	Tex6.LoadFromFile("tex_ground.bmp");

	Tex7.LoadFromFile("tex_player.bmp");

	TexFT.LoadFromFile("skybox_ft.bmp");
	TexBK.LoadFromFile("skybox_bk.bmp");
	TexLF.LoadFromFile("skybox_lf.bmp");
	TexRT.LoadFromFile("skybox_rt.bmp");
	TexUP.LoadFromFile("skybox_up.bmp");
	TexDN.LoadFromFile("skybox_dn.bmp");

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

#if 0
int main(int argc, char **argv)
#else
int _tmain(int argc, _TCHAR* argv[])
#endif
{
	UNIVERSE *u;
	char filename[1000], errbuf[1000];

	delete_mode = false;

	if( argc <= 1 ) {
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
				//strcpy(filename, "test.evolve");
				strcpy(filename, "../../test.evolve");
			}

			u = Universe_Read(filename, errbuf);

			if( u == NULL ) {
				printf("error: %s\n", errbuf);
				printf("\n");
				printf("Let's try again.\n");
			}

		} while( u == NULL );
	} else {
		/*
		 * Called from Evolve.exe
		 */

		strcpy(filename, argv[1]);
		u = Universe_Read(filename, errbuf);
		if( u == NULL ) {
			printf("\n\nerror: %s\n\n", errbuf);
			printf("\n");
			gets(filename);
			return 1;
		}

		if( argc == 3 && strcmp(argv[2], "/d") == 0 ) {
			strcpy(delete_file, filename);
			delete_mode = true;
		}
	}

	universe = u;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Evolve 4.0 - 3D Explorer");

	Camera.Move( F3dVector(u->width/2.0, 10.0, u->height/2.0 ) );
	Camera.MoveForward( 1.0 );

	scw = 800;
	sch = 600;

	GameCamera.perspective(
		ThirdPersonCamera::DEFAULT_FOVX,
		static_cast<float>(800) / static_cast<float>(600),
		1.0f,
		9000 * 2.0f);

	GameCamera.lookAt(
		Vector3(0.0f, 1.0 * 3.0f, 2.0 * 7.0f),
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f));

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
	glutPassiveMotionFunc(processPassiveMouseMotion);
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

#if 0
int _tmain(int argc, _TCHAR* argv[])
{
	return main(argc, NULL);
}

#endif

