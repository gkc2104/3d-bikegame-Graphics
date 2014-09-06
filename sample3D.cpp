//	This is a sample program that illustrates OpenGL and GLUT in 3D. It
//	renders a picture of 36 snowmen. The camera can be moved by dragging
//	the mouse. The camera moves forward by hitting the up-arrow key and
//	back by moving the down-arrow key. Hit ESC, 'q' or 'Q' to exit.
//
//	Warning #1: This program uses the function glutSpecialUpFunc, which
//	may not be available in all GLUT implementations. If your system
//	does not have it, you can comment this line out, but the up arrow
//	processing will not be correct.
//----------------------------------------------------------------------

#include <stdlib.h> 
#include <math.h> 
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include "imageloader.h"
#include "vec3f.h"

// The following works for both linux and MacOS
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)
	
// escape key (for exit)
#define ESC 27


using namespace std;
//----------------------------------------------------------------------
// Global variables
//
// The coordinate system is set up so that the (x,y)-coordinate plane
// is the ground, and the z-axis is directed upwards. The y-axis points
// to the north and the x-axis points to the east.
//
// The values (x,y) are the current camera position. The values (lx, ly)
// point in the direction the camera is looking. The variables angle and
// deltaAngle control the camera's angle. The variable deltaMove
// indicates the amount of incremental motion for the camera with each
// redraw cycle. The variables isDragging and xDragStart are used to
// monitor the mouse when it drags (with the left button down).
//----------------------------------------------------------------------

// Camera position
float bike_straight = 1.0f;
float bike_side = 0.0f;
float bike_angle = 0.0f;
float handle_angle = 0.0f;
float x_comp;
float y_comp;
float lookz = 0.0;
float lkz = 0.0;
float pos_x = 0.0;
float pos_y = 0.0;
float x = 0.0, y = -10.0; // initially 5 units south of origin
float deltaMove = 0.0; // initially camera doesn't move
float deltaMovez = 0.0;

// Camera direction
float lx = 0.0, ly = 1.0, lz = 0.0; // camera points initially along y-axis
float angle = 0.0; // angle of rotation for the camera direction
float anglez = 0.0;
float deltaAngle = 0.0; // additional angle change when dragging
float deltaAnglez = 0.0;

// Mouse drag control
int isDragging = 0; // true when dragging
int xDragStart = 0; // records the x-coordinate when dragging starts
int yDragStart = 0;
int camera_flag = 2;

//----------------------------------------------------------------------
// Reshape callback
//
// Window size has been set/changed to w by h pixels. Set the camera
// perspective to 45 degree vertical field of view, a window aspect
// ratio of w/h, a near clipping plane at depth 1, and a far clipping
// plane at depth 100. The viewport is the entire window.
//
//----------------------------------------------------------------------
void changeSize(int w, int h) 
{
	float ratio =  ((float) w) / ((float) h); // window aspect ratio
	glMatrixMode(GL_PROJECTION); // projection matrix is active
	glLoadIdentity(); // reset the projection
	gluPerspective(45.0, ratio, 0.1, 100.0); // perspective transformation
	glMatrixMode(GL_MODELVIEW); // return to modelview mode
	glViewport(0, 0, w, h); // set viewport (drawing area) to entire window
}

//----------------------------------------------------------------------
// Draw one snowmen (at the origin)
//
// A snowman consists of a large body sphere and a smaller head sphere.
// The head sphere has two black eyes and an orange conical nose. To
// better create the impression they are sitting on the ground, we draw
// a fake shadow, consisting of a dark circle under each.
//
// We make extensive use of nested transformations. Everything is drawn
// relative to the origin. The snowman's eyes and nose are positioned
// relative to a head sphere centered at the origin. Then the head is
// translated into its final position. The body is drawn and translated
// into its final position.
//----------------------------------------------------------------------
void drawSnowman()
{
	// Draw body (a 20x20 spherical mesh of radius 0.75 at height 0.75)
	glColor3f(1.0, 1.0, 1.0); // set drawing color to white
	glPushMatrix();
		glTranslatef(0.0, 0.0, 0.75);
		glutSolidSphere(0.75, 20, 20);
	glPopMatrix();

	// Draw the head (a sphere of radius 0.25 at height 1.75)
	glPushMatrix();
		glTranslatef(0.0, 0.0, 1.75); // position head
		glutSolidSphere(0.25, 20, 20); // head sphere

		// Draw Eyes (two small black spheres)
		glColor3f(0.0, 0.0, 0.0); // eyes are black
		glPushMatrix();
			glTranslatef(0.0, -0.18, 0.10); // lift eyes to final position
			glPushMatrix();
				glTranslatef(-0.05, 0.0, 0.0);
				glutSolidSphere(0.05, 10, 10); // right eye
			glPopMatrix();
			glPushMatrix();
				glTranslatef(+0.05, 0.0, 0.0);
				glutSolidSphere(0.05, 10, 10); // left eye
			glPopMatrix();
		glPopMatrix();

		// Draw Nose (the nose is an orange cone)
		glColor3f(1.0, 0.5, 0.5); // nose is orange
		glPushMatrix();
			glRotatef(90.0, 1.0, 0.0, 0.0); // rotate to point along -y
			glutSolidCone(0.08, 0.5, 10, 2); // draw cone
		glPopMatrix();
	glPopMatrix();

	// Draw a faux shadow beneath snow man (dark green circle)
	glColor3f(0.0, 0.5, 0.0);
	glPushMatrix();
		glTranslatef(0.2, 0.2, 0.001);	// translate to just above ground
		glScalef(1.0, 1.0, 0.0); // scale sphere into a flat pancake
		glutSolidSphere(0.75, 20, 20); // shadow same size as body
	glPopMatrix();
}








class Terrain {
	private:
		int w; 
		int l; 
		float** hs; 
		Vec3f** normals;
		bool computedNormals; 
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;

			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}

			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}

			computedNormals = false;
		}

		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;

			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}

		int width() {
			return w;
		}

		int length() {
			return l;
		}


		void setHeight(int x, int y, float z) {
			hs[y][x] = z;
			computedNormals = false;
		}


		float getHeight(int x, int y) {
			return hs[y][x];
		}


		void computeNormals() {
			if (computedNormals) {
				return;
			}


			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}

			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);

					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}

					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}

					normals2[z][x] = sum;
				}
			}


			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];

					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}

					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}

			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;

			computedNormals = true;
		}

		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};





Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
Terrain* _terrain;

void cleanup() {
	delete _terrain;
}












void drawcube(float len)
{
	float half = len/2;

	glBegin(GL_POLYGON);
	glVertex3d(half, -half, half); 
	glVertex3d(half, half, half); 
	glVertex3d(-half, half, half); 
	glVertex3d(-half, -half, half); 
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(half, -half, -half); 
	glVertex3d(half, half, -half); 
	glVertex3d(-half, half, -half); 
	glVertex3d(-half, -half, -half); 
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(half, half, half); 
	glVertex3d(half, -half, half); 
	glVertex3d(half, -half, -half); 
	glVertex3d(half, half, -half); 
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(-half, -half, half); 
	glVertex3d(-half, half, half); 
	glVertex3d(-half, half, -half); 
	glVertex3d(-half, -half, -half); 
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(half, half, half); 
	glVertex3d(half, half, -half); 
	glVertex3d(-half, half, -half); 
	glVertex3d(-half, half, half); 
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3d(half, -half, half); 
	glVertex3d(half, -half, -half); 
	glVertex3d(-half, -half, -half); 
	glVertex3d(-half, -half, half); 
	glEnd();

}

//----------------------------------------------------------------------
// Update with each idle event
//
// This incrementally moves the camera and requests that the scene be
// redrawn.
//----------------------------------------------------------------------
void update(void) 
{
	if (deltaMove) { // update camera position
		x += deltaMove * lx * 0.1;
		y += deltaMove * ly * 0.1;
	}
	if (deltaMovez)
	{
		lookz += deltaMovez * 0.01;
		lookz += deltaMovez * 0.01;
	}
	glutPostRedisplay(); // redisplay everything
}

float xa,ya,xb,yb;
float angle_wheel;
double radius=2;

void drawbike()
{
	glPushMatrix();
	xa = 0.0,ya=0.0;
	glTranslatef(0.0,3.0,-1.0);
	glRotatef(-handle_angle*2,0.0f,0.0f,1.0f);
	glScalef(3.0f,1.0f,1.0f);
	for (angle_wheel=1.0f;angle_wheel<361.0f;angle_wheel+=20.0)
	{
		xb = xa+sin(angle_wheel)*radius;
		yb = ya+cos(angle_wheel)*radius;
		glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0,xb,yb);
		glEnd();
	}
	xa = 0.0,ya=0.0;
	for (angle_wheel=1.0f;angle_wheel<361.0f;angle_wheel+=0.2)
	{
		xb = xa+sin(angle_wheel)*radius;
		yb = ya+cos(angle_wheel)*radius;
		glBegin(GL_LINES);
		glLineWidth(1.0);
		glVertex3f(-0.1,xb,yb);
		glVertex3f(0.1,xb,yb);
		glEnd();
	}
	glPopMatrix();
	
	glBegin(GL_LINES);
	glLineWidth((GLfloat)200.0);
	glVertex3f(0.0,3.0,-1.0);
	glVertex3f(0.0,3.0,1.0);	
	glEnd();

	glPushMatrix();
	glTranslatef(0.0f,0.0f,1.0f);
	glScalef(3.0f,1.0f,1.0f);
	glBegin(GL_POLYGON);
	glVertex3d(-0.05,3.0,0.3);
	glVertex3d(-0.05,3.0,-0.3);
	glVertex3d(-0.05,-3.0,-0.3);
	glVertex3d(-0.05,-3.0,0.3);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(0.05,3.0,0.3);
	glVertex3d(0.05,3.0,-0.3);
	glVertex3d(0.05,-3.0,-0.3);
	glVertex3d(0.05,-3.0,0.3);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(0.05,3.0,0.3);
	glVertex3d(-0.05,3.0,0.3);
	glVertex3d(-0.05,-3.0,0.3);
	glVertex3d(0.05,-3.0,0.3);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(0.05,3.0,-0.3);
	glVertex3d(0.05,-3.0,-0.3);
	glVertex3d(-0.05,-3.0,-0.3);
	glVertex3d(-0.05,3.0,-0.3);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(0.05,3.0,0.3);
	glVertex3d(0.05,3.0,-0.3);
	glVertex3d(-0.05,3.0,-0.3);
	glVertex3d(-0.05,3.0,0.3);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(0.05,-3.0,0.3);
	glVertex3d(0.05,-3.0,-0.3);
	glVertex3d(-0.05,-3.0,-0.3);
	glVertex3d(-0.05,-3.0,0.3);
	glEnd();
	glPopMatrix();



	glPushMatrix();
	xa = 0.0,ya=0.0;
	glTranslatef(0.0,-3.0,-1.0);
	glScalef(3.0f,1.0f,1.0f);
	for (angle_wheel=1.0f;angle_wheel<361.0f;angle_wheel+=20.0)
	{
		xb = xa+sin(angle_wheel)*radius;
		yb = ya+cos(angle_wheel)*radius;
		glBegin(GL_LINES);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0,xb,yb);
		glEnd();
	}
	xa = 0.0,ya=0.0;
	for (angle_wheel=1.0f;angle_wheel<361.0f;angle_wheel+=0.2)
	{
		xb = xa+sin(angle_wheel)*radius;
		yb = ya+cos(angle_wheel)*radius;
		glBegin(GL_LINES);
		glLineWidth(1.0);
		glVertex3f(-0.1,xb,yb);
		glVertex3f(0.1,xb,yb);
		glEnd();
	}
	glPopMatrix();
}


//----------------------------------------------------------------------
// Draw the entire scene
//
// We first update the camera location based on its distance from the
// origin and its direction.
//----------------------------------------------------------------------
int cam_height=10;
int cam_dist = 15;
int cam_sens = 5;
void renderScene(void) 
{


	// Clear color and depth buffers
	glClearColor(0.0, 0.7, 1.0, 1.0); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();

	// Set the camera centered at (x,y,1) and looking along directional
	// vector (lx, ly, 0), with the z-axis pointing up

	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -10.0f);

	glRotatef(-_angle, 0.0f, 0.0f, 1.0f);

	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-6.0f, 0.0f, 0.0f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	float scale = 5.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef((-(float)(_terrain->width() - 1) / 2) , -(float)(_terrain->length() - 1) / 2, 0.0f);

	glColor3f(0.3f, 0.9f, 0.0f);
	for(int z = 0; z < _terrain->length() - 1; z++) {

		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, z,_terrain->getHeight(x, z));
			normal = _terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, z+1,_terrain->getHeight(x, z + 1));
		}
		glEnd();
	}
	glPopMatrix();


	gluLookAt(
			pos_x - cam_dist*lx, pos_y - cam_dist*ly, cam_height + lkz + lookz,
			lx + pos_x, ly + pos_y, 4.0 ,
			0.0,    0.0,    1.0);

	// Draw 36 snow men
	
//	glPushMatrix();	
//	glTranslatef(0.0,0.0,3.0);
//	glColor3f(0.0f,0.0f,0.0f);
//	glScalef(3.0f,1.0f,1.0f);
//	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(pos_x, pos_y, 3.0f);
	glRotatef(-bike_angle,0.0,0.0,1.0);
	glColor3f(1.0f,1.0f,1.0f);
	drawbike();
//	drawcube(2.0);
	glPopMatrix();
	
	
	glPushMatrix();
	glTranslatef(5.0f, 5.0f, 1.0f);
	glColor3f(0.0f,0.0f,1.0f);
	drawcube(2.0);
	glPopMatrix();

	glutSwapBuffers(); // Make it all visible
} 



//----------------------------------------------------------------------
// Userinput callbacks
//
// processNormalKeys: ESC, q, and Q cause program to exit
// pressSpecialKey: Up arrow = forward motion, down arrow = backwards
// releaseSpecialKey: Set incremental motion to zero
//----------------------------------------------------------------------
void processNormalKeys(unsigned char key, int xx, int yy)
{
	if( key == 'u' )
		lookz += 1;
	if( key == 'h' )
		lookz -= 1;
	if (key == ESC || key == 'q' || key == 'Q')
		exit(0);
	if( key == 'w' )
	{
		bike_angle += handle_angle;
		if(bike_angle >= 360)
			bike_angle -= 360;
		x_comp = sin(DEG2RAD(bike_angle));
		y_comp = cos(DEG2RAD(bike_angle));
		pos_x += x_comp;
		pos_y += y_comp;
	}
	if( key == 's' )
	{
		bike_angle -= handle_angle;
		if(bike_angle <= -360)
			bike_angle += 360;
		x_comp = sin(DEG2RAD(bike_angle));
		y_comp = cos(DEG2RAD(bike_angle));
		pos_x -= x_comp;
		pos_y -= y_comp;
	}
	if( key == 'a' )
		if(handle_angle >= -25)
			handle_angle -= 1;
	if( key == 'd' )
		if(handle_angle <= 25)
			handle_angle += 1;
	if( key == 'c' )
	{
		if(camera_flag == 2)
		{
			lkz -= 1;
			cam_sens = 1;
			camera_flag = 0;
			cam_height = 3;
			cam_dist = 0;
		}
		else if(camera_flag == 0)
		{
			lkz += 1;
			cam_sens = 2;
			camera_flag = 1;
			cam_height = 2;
			cam_dist = 10;
		}
		else if(camera_flag == 1)
		{
			cam_sens = 5;
			camera_flag = 2;
			cam_height = 10;
			cam_dist = 15;
		}
		std::cout << lookz << std::endl;
	}
} 

void pressSpecialKey(int key, int xx, int yy)
{
	switch (key) {
		case GLUT_KEY_UP : pos_x += lx; pos_y += ly;
		case GLUT_KEY_DOWN : deltaMove = -1.0; break;
		case GLUT_KEY_LEFT : deltaMovez = 1.0; break;
		case GLUT_KEY_RIGHT : deltaMovez = -1.0; break;
	}
} 

void releaseSpecialKey(int key, int x, int y) 
{
	switch (key) {
		case GLUT_KEY_UP : deltaMove = 0.0; break;
		case GLUT_KEY_DOWN : deltaMove = 0.0; break;
		case GLUT_KEY_LEFT : deltaMovez = 0.0; break;
		case GLUT_KEY_RIGHT : deltaMovez = 0.0; break;
	}
} 

//----------------------------------------------------------------------
// Process mouse drag events
// 
// This is called when dragging motion occurs. The variable
// angle stores the camera angle at the instance when dragging
// started, and deltaAngle is a additional angle based on the
// mouse movement since dragging started.
//----------------------------------------------------------------------
void mouseMove(int x, int y) 
{ 	
	if (isDragging) 
	{ 	// only when dragging
		// update the change in angle
		deltaAngle = (x - xDragStart) * 0.005;
		deltaAnglez = (y - yDragStart) * 0.005;

		// camera's direction is set to angle + deltaAngle
		lx = -sin(angle + deltaAngle);
		ly = cos(angle + deltaAngle);
		//lz = sin(deltaAnglez);
		lookz = -5*deltaAnglez;
	}
}

void mouseButton(int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) { // left mouse button pressed
			isDragging = 1; // start dragging
			xDragStart = x; // save x where button first pressed
			yDragStart = y;
		}
		else  { /* (state = GLUT_UP) */
			angle += deltaAngle; // update camera turning angle
			anglez += deltaAnglez;
			isDragging = 0; // no longer dragging
		}
	}
}



void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}


void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}


//----------------------------------------------------------------------
// Main program  - standard GLUT initializations and callbacks
//----------------------------------------------------------------------
int main(int argc, char **argv) 
{
	printf("\n\
-----------------------------------------------------------------------\n\
  OpenGL Sample Program:\n\
  - Drag mouse left-right to rotate camera\n\
  - Hold up-arrow/down-arrow to move camera forward/backward\n\
  - q or ESC to quit\n\
-----------------------------------------------------------------------\n");

	// general initializations
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 800);
	glutCreateWindow("OpenGL/GLUT Sampe Program");
	initRendering();

	_terrain = loadTerrain("heightmap.bmp", 20);

	// register callbacks
	glutReshapeFunc(changeSize); // window reshape callback
	glutDisplayFunc(renderScene); // (re)display callback
	glutIdleFunc(update); // incremental update 
//	glutIgnoreKeyRepeat(1); // ignore key repeat when holding key down
	glutMouseFunc(mouseButton); // process mouse button push/release
	glutMotionFunc(mouseMove); // process mouse dragging motion
	glutKeyboardFunc(processNormalKeys); // process standard key clicks
	glutSpecialFunc(pressSpecialKey); // process special key pressed
						// Warning: Nonstandard function! Delete if desired.
	glutSpecialUpFunc(releaseSpecialKey); // process special key release

	// OpenGL init
	glEnable(GL_DEPTH_TEST);

	// enter GLUT event processing cycle
	glutMainLoop();

	return 0; // this is just to keep the compiler happy
}
