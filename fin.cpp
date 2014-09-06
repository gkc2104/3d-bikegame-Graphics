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
#include<string.h>

// The following works for both linux and MacOS
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#include<GL/freeglut.h>
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)

// escape key (for exit)
#define ESC 27
#include "imageloader.h"
#include "vec3f.h"
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
int keyflag[256];
float height_forward;
float height_backward;
float elevation;
float pitch_angle;
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
float pedal_angle = 0.0;
float x = 0.0, y = -10.0; // initially 5 units south of origin
float deltaMove = 0.0; // initially camera doesn't move
float deltaMovez = 0.0;
float star_angle = 0;
float tilt_angle = 0;
float velocity = 0;
// Camera direction
float lx = 0.0, ly = 1.0, lz = 0.0; // camera points initially along y-axis
float angle = 0.0; // angle of rotation for the camera direction
float anglez = 0.0;
float deltaAngle = 0.0; // additional angle change when dragging
float deltaAnglez = 0.0;
float height_next;
float current_height;
int jump_flag = 0;
float zoom = 0;
// Mouse drag control
int isDragging = 0; // true when dragging
int xDragStart = 0; // records the x-coordinate when dragging starts
int yDragStart = 0;
int camera_flag = 2;
int rand_c[10][3],i;
int tree_pos[15][2];
int point_count = 0;
int timer=0;
int end_game=0;
float heli_x=0;
float heli_y=0;
float heli_look_x=0;
float heli_look_y=0;
float heli_look_z;

using namespace std;

class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
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

		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}

		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			if(z>=0 && x>=0)
				return hs[z][x];
			else
				return 0;
		}

		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}

			//Compute the rough version of the normals
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

			//Smooth out the normals
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

		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	int  span=2;
	Terrain* t = new Terrain(image->width*span, image->height*span);
	for(int y = 0; y < image->height*span; y++) {
		for(int x = 0; x < image->width*span; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * ((y%image->height) * image->width + x%image->width)];
			float h = height * ((color / 255.0f) - 0.5f);
			if(h<-8)
				t->setHeight(x,y,-0.5);
			else if(h<0)
				t->setHeight(x, y, 0);
			else
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








void drawstar(float h, float theta)
{
	glScalef(0.5f,0.5f,0.5f);
	glColor3f(1.0f,1.0f,0.0f);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+36))/2,h*sin(DEG2RAD(theta+36))/2);
	glVertex2f(h*cos(DEG2RAD(theta+72)),h*sin(DEG2RAD(theta+72)));
	glVertex2f(h*cos(DEG2RAD(theta+108))/2,h*sin(DEG2RAD(theta+108))/2);
	glEnd();
	glColor3f(0.5f,0.5f,1.0f);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+108))/2,h*sin(DEG2RAD(theta+108))/2);
	glVertex2f(h*cos(DEG2RAD(theta+144)),h*sin(DEG2RAD(theta+144)));
	glVertex2f(h*cos(DEG2RAD(theta+180))/2,h*sin(DEG2RAD(theta+180))/2);
	glEnd();
	glColor3f(1.0f,0.5f,0.0f);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+180))/2,h*sin(DEG2RAD(theta+180))/2);
	glVertex2f(h*cos(DEG2RAD(theta+216)),h*sin(DEG2RAD(theta+216)));
	glVertex2f(h*cos(DEG2RAD(theta+252))/2,h*sin(DEG2RAD(theta+252))/2);
	glEnd();
	glColor3f(1.0f,0.5f,0.5f);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+252))/2,h*sin(DEG2RAD(theta+252))/2);
	glVertex2f(h*cos(DEG2RAD(theta+288)),h*sin(DEG2RAD(theta+288)));
	glVertex2f(h*cos(DEG2RAD(theta+324))/2,h*sin(DEG2RAD(theta+324))/2);
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+324))/2,h*sin(DEG2RAD(theta+324))/2);
	glVertex2f(h*cos(DEG2RAD(theta+0)),h*sin(DEG2RAD(theta+0)));
	glVertex2f(h*cos(DEG2RAD(theta+36))/2,h*sin(DEG2RAD(theta+36))/2);
	glEnd();
	glColor3f(1,1,0);
	glBegin(GL_POLYGON);
	glVertex2f(h*cos(DEG2RAD(theta+36))/2,h*sin(DEG2RAD(theta+36))/2);
	glVertex2f(h*cos(DEG2RAD(theta+108))/2,h*sin(DEG2RAD(theta+108))/2);
	glVertex2f(h*cos(DEG2RAD(theta+180))/2,h*sin(DEG2RAD(theta+180))/2);
	glVertex2f(h*cos(DEG2RAD(theta+252))/2,h*sin(DEG2RAD(theta+252))/2);
	glVertex2f(h*cos(DEG2RAD(theta+324))/2,h*sin(DEG2RAD(theta+324))/2);
	glEnd();
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(h*cos(DEG2RAD(theta+0)),h*sin(DEG2RAD(theta+0)));
	glVertex2f(h*cos(DEG2RAD(theta+36))/2,h*sin(DEG2RAD(theta+36))/2);
	glVertex2f(h*cos(DEG2RAD(theta+72)),h*sin(DEG2RAD(theta+72)));
	glVertex2f(h*cos(DEG2RAD(theta+108))/2,h*sin(DEG2RAD(theta+108))/2);
	glVertex2f(h*cos(DEG2RAD(theta+144)),h*sin(DEG2RAD(theta+144)));
	glVertex2f(h*cos(DEG2RAD(theta+180))/2,h*sin(DEG2RAD(theta+180))/2);
	glVertex2f(h*cos(DEG2RAD(theta+216)),h*sin(DEG2RAD(theta+216)));
	glVertex2f(h*cos(DEG2RAD(theta+252))/2,h*sin(DEG2RAD(theta+252))/2);
	glVertex2f(h*cos(DEG2RAD(theta+288)),h*sin(DEG2RAD(theta+288)));
	glVertex2f(h*cos(DEG2RAD(theta+324))/2,h*sin(DEG2RAD(theta+324))/2);
	glEnd();
}



void fall()
{
	for(i=0;i<10;i++)
	{
		rand_c[i][2] = 1;
	}
	tilt_angle = 0;
	point_count = 0;
}








//----------------------------------------------------------------------
// Update with each idle event
//
// This incrementally moves the camera and requests that the scene be
// redrawn.
//----------------------------------------------------------------------
void update(void) 
{
	if(end_game == 0)
	{
		if (deltaMove) { // update camera position
			x += deltaMove * lx * 0.1;
			y += deltaMove * ly * 0.1;
		}
		if (deltaMovez)
		{
			lookz += deltaMovez * 0.1;
			lookz += deltaMovez * 0.1;
		}
		star_angle += 5;
		for(i=0;i<10;i++)
		{
			if((((pos_x - rand_c[i][0] + (_terrain->width()-1)/2)*(pos_x - rand_c[i][0] + (_terrain->width()-1)/2)) < 0.5) && (((pos_y - rand_c[i][1] + (_terrain->length()-1)/2)*(pos_y - rand_c[i][1] + (_terrain->length()-1)/2)) < 0.5) && rand_c[i][2] != 0)
			{
				rand_c[i][2] = 0;
				point_count ++;
			}
		}
		glutPostRedisplay(); // redisplay everything

		if(current_height < 0)
			jump_flag = 0;
		if(jump_flag == 1)
		{
			pos_x += velocity*(sin(DEG2RAD(bike_angle)));
			pos_y += velocity*(cos(DEG2RAD(bike_angle)));
			current_height -= 0.1;
		}
		if(jump_flag == 0)
		{
			if(((pos_x + 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_x + 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) >= -58) && ((pos_y + 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_y + 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) >= -58))
			{
				height_next = 1.0 + _terrain->getHeight((pos_x + velocity*(sin(DEG2RAD(bike_angle+handle_angle))) + (_terrain->width()-1)/2),(pos_y + 0.2*(cos(DEG2RAD(bike_angle+handle_angle))) + (_terrain->width()-1)/2));
				current_height = 1.0 +  _terrain->getHeight((pos_x + (_terrain->width()-1)/2),pos_y + (_terrain->width()-1)/2);
			}
			else
			{
				pos_x = 0;
				pos_y = 0;
				velocity = 0;
				jump_flag = 0;
			}
			if(tilt_angle != 90 || tilt_angle!= -90)
			{
				if(keyflag['w'] == 1)
				{
					if(((pos_x + 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_x + 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) >= -58) && ((pos_y + 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_y + 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) >= -58))
					{
						if(current_height - height_next >= 0.2 && velocity > 0.15)
							jump_flag = 1;
						if(jump_flag == 0)
						{
							if(velocity < 0.5)
								velocity += 0.05;
							bike_angle += (0.5)*handle_angle;
							if(bike_angle >= 360)
								bike_angle -= 360;
							x_comp = velocity*(sin(DEG2RAD(bike_angle)));
							y_comp = velocity*(cos(DEG2RAD(bike_angle)));
							pos_x += x_comp;
							pos_y += y_comp;
							if(handle_angle > 0)
								handle_angle -=0.05;
							if(handle_angle < 0)
								handle_angle += 0.05;
							if(tilt_angle > 0 )
								tilt_angle -= 0.1;
							if(tilt_angle < 0 )
								tilt_angle += 0.1;
							pedal_angle += 1;
							if(pedal_angle > 360)
								pedal_angle = 0;
						}
					}
					else
					{
						pos_x = 0;
						pos_y = 0;
						velocity = 0;
					}
				}
			}
			if(keyflag['w'] == 0)
				velocity = 0;
			if(keyflag['s'] == 1)
			{
				if(((pos_x - 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_x - 0.2*(sin(DEG2RAD(bike_angle+handle_angle)))) >= -58) && ((pos_y - 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) <= 58) && ((pos_y - 0.2*(cos(DEG2RAD(bike_angle+handle_angle)))) >= -58))
				{
					velocity = 0;
					bike_angle -= (0.5)*handle_angle;
					if(bike_angle <= -360)
						bike_angle += 360;
					x_comp = 0.1*(sin(DEG2RAD(bike_angle)));
					y_comp = 0.1*(cos(DEG2RAD(bike_angle)));
					pos_x -= x_comp;
					pos_y -= y_comp;
					if(handle_angle > 0)
						handle_angle -=0.05;
					if(handle_angle < 0)
						handle_angle += 0.05;

					pedal_angle -= 1;
					if(pedal_angle < -360)
						pedal_angle = 0;
				}
				else
				{
					pos_x = 0;
					pos_y = 0;
				}
			}
			if(keyflag['a'] == 1)
			{
				if(handle_angle >= -25)
					handle_angle -= 0.1;
			}
			if(keyflag['d'] == 1)
			{
				if(handle_angle <= 25)
					handle_angle += 0.1;
			}
			if(keyflag['q'] == 1)
			{
				if(tilt_angle >= -25)
					tilt_angle -= 1;
			}
			if(keyflag['e'] == 1)
			{
				if(tilt_angle <= 25)
					tilt_angle += 1;
			}
		}




		if(velocity > 0.17)
		{
			tilt_angle += -0.05*(handle_angle);
			if(tilt_angle > 90)
				tilt_angle = 90;
			if(tilt_angle < -90)
				tilt_angle = -90;

			if(handle_angle > 10)
			{
				if(tilt_angle < 20)
				{
					fall();
					tilt_angle = -90;
				}
			}
			else if(handle_angle < -10)
			{
				if( tilt_angle > -20)
				{
					fall();
					tilt_angle = 90;
				}
			}
		}



	}

}



void drawtree()
{
	glPushMatrix();
	glTranslatef(0.0,0.0,-0.5);
	glPushMatrix();
	glScalef(0.3,0.3,3.0);
	glColor3f(0.5,0.35,0.05);
	drawcube(1);
	glPopMatrix();
	glPushMatrix();
	glColor3f(0.3,1.0,0.3);
	glScalef(0.6,0.6,1.5);
	glTranslatef(0.0,0.0,-2);
	glutSolidSphere(2.0,50,50);
	glPopMatrix();
	glPopMatrix();
}




float xa,ya,xb,yb;
float angle_wheel;
double radius=2.1;

void drawbike()
{
	glPushMatrix();
	glColor3f(0.0,0.0,0.0);
	xa = 0.0,ya=0.0;
	glTranslatef(0.0,3.0,-1.0);
	glRotatef(-handle_angle*2,0.0f,0.0f,1.0f);
	glScalef(3.0f,1.0f,1.0f);
	glPushMatrix();
	glRotatef(90.0f,0.0,1.0,0.0);
	glScalef(0.5,0.5,0.07);
	glutSolidTorus(2.0,2.2,100,100);
	glPopMatrix();/*
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
			 }*/
	glPopMatrix();

	glLineWidth(20);
	glBegin(GL_LINES);
	glVertex3f(0.0,3.0,2.0);
	glVertex3f(0.0,3.0,-1.0);
	glEnd();

	glPushMatrix();
	glColor3f(1.0,1.0,1.0);
	glTranslatef(0.0,3.0,0.5);
	glScalef(0.2,0.2,1.0);
	drawcube(2);
	glPopMatrix();



	glPushMatrix();
	glTranslatef(0.0,2.0,4.0);
	glRotatef(-handle_angle*2,0.0,0.0,1.0);
	glScalef(2.0,0.2,0.2);
	drawcube(2);
	glPopMatrix();




	glPushMatrix();
	glTranslatef(0.0,0.0,0);
	glScalef(0.2,1,0.2);
	drawcube(2);
	glPopMatrix();





	glPushMatrix();
	glColor3f(0,0,1);
	glTranslatef(0,-0.45,3);
	glRotatef(-90,1,0,0);
	glScalef(0.5,0.5,0.5);
	drawcube(4);
	glPopMatrix();




	glPushMatrix();
	glColor3f(1,1,1);
	glRotatef(-20,1,0,0);
	glTranslatef(0,-2,8);
	glScalef(0.5,0.5,1);
	drawcube(2);
	glPopMatrix();



	glPushMatrix();
	glRotatef(handle_angle/10, 0,0,1);


	glPushMatrix();
	glColor3f(1,0,0);
	glRotatef(-20,1,0,0);
	glTranslatef(0,-2,5);
	glScalef(0.5,0.5,1);
	drawcube(4);
	glPopMatrix();


	glPushMatrix();
	glColor3f(1,1,1);
	glTranslatef(-1,1.5,5);
	glRotatef(-140,1,0,0);
	glScalef(0.2,0.2,1.7);
	drawcube(2);
	glPopMatrix();



	glPushMatrix();
	glTranslatef(1,1.5,5);
	glRotatef(-140,1,0,0);
	glScalef(0.2,0.2,1.7);
	drawcube(2);
	glPopMatrix();

	glPopMatrix();




	glPushMatrix();
	glRotatef(-pedal_angle,1,0,0);
	glScalef(0.2,0.2,1.0);
	glTranslatef(0.0,-0.1,1);
	drawcube(2);
	glPopMatrix();




	glPushMatrix();
	glRotatef(-pedal_angle,1,0,0);
	glScalef(0.2,0.2,1.0);
	glTranslatef(0.0,0.1,-1);
	drawcube(2);
	glPopMatrix();




	glPushMatrix();
	glColor3f(1.0,1.0,1.0);
	glTranslatef(0.0,2.5,3.0);
	glRotatef(30,1.0,0.0,0.0);
	glScalef(0.2,0.2,1.0);
	drawcube(2);
	glPopMatrix();





	//Draw Score
	if(end_game == 0)
	{
		glPushMatrix();
		char message[100];
		glColor3f(0.0f,0.0f,0.0f);
		glRasterPos3f(1, -1, 5);
		sprintf(message,"\nScore: %d",point_count);
		int len = (int) strlen(message);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)message[i]);
		}
		glPopMatrix();
	}



	if(end_game == 1)
	{	
		glPushMatrix();
		char message[100];
		glColor3f(0.0f,0.0f,0.0f);
		glRasterPos3f(1, -1, 5);
		sprintf(message,"\nGame Over!!");
		int len = (int) strlen(message);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)message[i]);
		}
		glPopMatrix();
	}

	if(end_game == 2)
	{	
		glPushMatrix();
		char message[100];
		glColor3f(0.0f,0.0f,0.0f);
		glRasterPos3f(1, -1, 5);
		sprintf(message,"\nComplete!!");
		int len = (int) strlen(message);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)message[i]);
		}
		glPopMatrix();
	}

	glBegin(GL_LINES);
	glLineWidth((GLfloat)200.0);
	glVertex3f(0.0,3.0,-1.0);
	glVertex3f(0.0,3.0,1.0);	
	glEnd();

	glPushMatrix();
	glColor3f(1.0f,1.0f,1.0f);
	glTranslatef(0.0f,0.0f,2.0f);
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
	glColor3f(1.0,1.0,1.0);
	glTranslatef(0.0,-3.0,0.5);
	glScalef(0.2,0.2,1.0);
	drawcube(2);
	glPopMatrix();


	glPushMatrix();
	glColor3f(0.0f,0.0f,0.0f);
	xa = 0.0,ya=0.0;
	glTranslatef(0.0,-3.0,-1.0);
	glScalef(3.0f,1.0f,1.0f);
	glPushMatrix();
	glRotatef(90.0f,0.0,1.0,0.0);
	glScalef(0.5,0.5,0.07);
	glutWireTorus(2.0,2.2,100,100);
	glPopMatrix();/*
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
			 }*/
	glPopMatrix();
}


//----------------------------------------------------------------------
// Draw the entire scene
//
// We first update the camera location based on its distance from the
// origin and its direction.
//----------------------------------------------------------------------
int cam_height=20;
int cam_dist = 40;
int cam_sens = 5;
void renderScene(void) 
{


	// Clear color and depth buffers
	glClearColor(0.0, 0.7, 1.0, 1.0); // sky color is light blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();



	float scale = 50.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	// Set the camera centered at (x,y,1) and looking along directional
	// vector (lx, ly, 0), with the z-axis pointing up
	//	glPushMatrix();
	//	glScalef(scale,scale,scale);
	//	glPopMatrix();




	// Draw 36 snow men

	//	glPushMatrix();	
	//	glTranslatef(0.0,0.0,3.0);
	//	glColor3f(0.0f,0.0f,0.0f);
	//	glScalef(3.0f,1.0f,1.0f);
	//	glPopMatrix();


	/*
	   glColor3f(0.0, 0.7, 0.0);
	   glBegin(GL_QUADS);
	   glVertex3f(-100.0, -100.0, 0.0);
	   glVertex3f(-100.0,  100.0, 0.0);
	   glVertex3f( 100.0,  100.0, 0.0);
	   glVertex3f( 100.0, -100.0, 0.0);
	   glEnd();	
	 */


	glPushMatrix();
	glTranslatef(5.0f, 5.0f, 1.0f);
	glColor3f(0.0f,0.0f,1.0f);
	drawcube(2.0);
	glPopMatrix();


	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {3.0f, 0.0f, 2.0f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	glScalef(scale, scale, scale);

	if( camera_flag == 2)
	{
		//		if(cam_height + lkz + lookz >= _terrain->getHeight((int)((pos_x - sin(DEG2RAD(bike_angle))*cam_dist+(_terrain->width()-1)/2)),(int)((pos_y - cos(DEG2RAD(bike_angle))*cam_dist+(_terrain->length()-1)/2))))
		//		{
		gluLookAt(
				pos_x - sin(DEG2RAD(bike_angle))*20 , pos_y - cos(DEG2RAD(bike_angle))*20 , 10 + lkz + lookz,
				pos_x, pos_y, 2+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				0.0,    0.0,    1.0);
		//		}
		//		else
		//		{
		//			gluLookAt(
		//				pos_x - sin(DEG2RAD(bike_angle))*cam_dist, pos_y - cos(DEG2RAD(bike_angle))*cam_dist, 0.2+_terrain->getHeight((int)((pos_x - sin(DEG2RAD(bike_angle))*cam_dist+(_terrain->width()-1)/2)),(int)((pos_y - cos(DEG2RAD(bike_angle))*cam_dist+(_terrain->length()-1)/2))),
		//				pos_x, pos_y, 2+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
		//				0.0,    0.0,    1.0);
		//
		//		}
	}
	else if( camera_flag == 0)
	{
		gluLookAt(
				pos_x - 0.3*sin(DEG2RAD(bike_angle)), pos_y - 0.3*cos(DEG2RAD(bike_angle)), 0.7+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				pos_x + sin(DEG2RAD(bike_angle)), pos_y + cos(DEG2RAD(bike_angle)), 0.65+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				0.0,0.0,1.0);
	}
	else if( camera_flag == 1)
	{
		//		if(2+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))) >= _terrain->getHeight((int)((pos_x - sin(DEG2RAD(bike_angle))*cam_dist+(_terrain->width()-1)/2)),(int)((pos_y - cos(DEG2RAD(bike_angle))*cam_dist+(_terrain->length()-1)/2))))
		//		{
		gluLookAt(
				pos_x - 3*sin(DEG2RAD(bike_angle)), pos_y - 3*cos(DEG2RAD(bike_angle)), 4+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				pos_x , pos_y , 3+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				0.0,0.0,1.0);
		//		}
		//		else
		//		{
		//			gluLookAt(
		//				pos_x - sin(DEG2RAD(bike_angle))*cam_dist, pos_y - cos(DEG2RAD(bike_angle))*cam_dist, 0.2+_terrain->getHeight((int)((pos_x - sin(DEG2RAD(bike_angle))*cam_dist+(_terrain->width()-1)/2)),(int)((pos_y - cos(DEG2RAD(bike_angle))*cam_dist+(_terrain->length()-1)/2))),
		//				pos_x, pos_y, 2+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
		//				0.0,    0.0,    1.0);
		//		}

	}
	else if ( camera_flag == 3)
	{
		gluLookAt(
				pos_x - 20*lx, pos_y - 20*ly, cam_height + lkz + lookz,
				pos_x, pos_y, 1+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				0.0, 0.0, 1.0);
	}


	else if ( camera_flag == 4)
	{
		gluLookAt(
				pos_x, pos_y, 20,
				pos_x, pos_y, 1+_terrain->getHeight((int)((pos_x+(_terrain->width()-1)/2)),(int)((pos_y+(_terrain->length()-1)/2))),
				sin(DEG2RAD(bike_angle)),cos(DEG2RAD(bike_angle)),0.0);
	}

	else if(camera_flag == 5)
	{
		gluLookAt(
				heli_x+60,heli_y+60,40,
				heli_x-60*heli_look_x,heli_y-60*heli_look_y,heli_look_z+10+zoom,
				0,0,1);
	}



	for(i=0;i<10;i++)
	{
		if(rand_c[i][2] == 1)
		{
			glPushMatrix();
			glColor3f(1.0,1.0,1.0);
			if(_terrain->getHeight(rand_c[i][0],rand_c[i][1]) > -6)
				glTranslatef(rand_c[i][0] - (_terrain->width()-1)/2, rand_c[i][1] - (_terrain->length()-1)/2, 1+_terrain->getHeight(rand_c[i][0],rand_c[i][1]));
			else
				glTranslatef(rand_c[i][0] - (_terrain->width()-1)/2, rand_c[i][1] - (_terrain->length()-1)/2, -5);
			glRotatef(90,1.0,0.0,0.0);
			drawstar(3,star_angle);
			glPopMatrix();
		}
	}




	for(i=0;i<15;i++)
	{
		if(_terrain->getHeight(tree_pos[i][0],tree_pos[i][1]) > -6)
		{
			glPushMatrix();
			glTranslatef(tree_pos[i][0] - (_terrain->width()-1)/2, tree_pos[i][1] - (_terrain->length()-1)/2, _terrain->getHeight(tree_pos[i][0],tree_pos[i][1]));
			glRotatef(180,1,0,0);
			drawtree();
			glPopMatrix();
		}
	}

	height_forward = _terrain->getHeight(pos_x + 1*sin(DEG2RAD(bike_angle)) + (float)(_terrain->width() - 1) / 2, pos_y + 1*cos(DEG2RAD(bike_angle)) + (float)(_terrain->length() - 1) / 2);
	height_backward = _terrain->getHeight(pos_x - 1*sin(DEG2RAD(bike_angle)) + (float)(_terrain->width() - 1) / 2, pos_y - 1*cos(DEG2RAD(bike_angle)) + (float)(_terrain->length() - 1) / 2);
	elevation = height_forward - height_backward;
	pitch_angle = atan(elevation);
	pitch_angle = RAD2DEG(pitch_angle);



	glPushMatrix();
	glTranslatef(pos_x, pos_y, current_height);
	glRotatef(-bike_angle,0.0,0.0,1.0);
	glRotatef(0.7*pitch_angle,1,0,0);
	glRotatef(tilt_angle,0,1,0);
	glScalef(0.3,0.3,0.3);
	drawbike();
	glPopMatrix();



	glTranslatef(-(float)(_terrain->width() - 1) / 2,-(float)(_terrain->length() - 1) / 2, 0.0f);
	//	glColor3f(0.3f, 0.9f, 0.0f);
	for(int z = 0; z < _terrain->length() - 1; z++) {

		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _terrain->width(); x++) {
			Vec3f normal = _terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[2], normal[1]);
			if(_terrain->getHeight(x,z) < 0)
			{
				glColor3f(0.0,0.0,1.0);
				//				glVertex3f(x, z,_terrain->getHeight(x, z));
				glVertex3f(x, z,0);
				normal = _terrain->getNormal(x, z + 1);
				glNormal3f(normal[0], normal[2], normal[1]);
				//				glVertex3f(x, z+1, _terrain->getHeight(x, z + 1));
				glVertex3f(x, z+1, 0);
			}
			else if(_terrain->getHeight(x,z) > 5)
			{
				glColor3f(1,1,1);
				glVertex3f(x, z,_terrain->getHeight(x, z));
				normal = _terrain->getNormal(x, z + 1);
				glNormal3f(normal[0], normal[2], normal[1]);
				glVertex3f(x, z+1, _terrain->getHeight(x, z + 1));
			}
			else
			{
				glColor3f(0.3,0.9,0.0);
				glVertex3f(x, z,_terrain->getHeight(x, z));
				normal = _terrain->getNormal(x, z + 1);
				glNormal3f(normal[0], normal[2], normal[1]);
				glVertex3f(x, z+1, _terrain->getHeight(x, z + 1));
			}
		}
		glEnd();
	}
	//	Vec3f norm = _terrain->getNormal(pos_x + (float)(_terrain->width() - 1) / 2, pos_y + (float)(_terrain->length() - 1) / 2);
	//	cout << norm[0] << ' ' << norm[2] << ' ' << norm[1] << endl;




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
	if (key == ESC)
		exit(0);
	if( key == 'w' )
		keyflag['w'] = 1;
	if( key == 's' )
		keyflag['s'] = 1;
	if( key == 'a' )
		keyflag['a'] = 1;
	if( key == 'd' )
		keyflag['d'] = 1;
	if( key == 'q' )
		keyflag['q'] = 1;
	if( key == 'e' )
		keyflag['e'] = 1;
	if( key == 'c' )
	{
		if(camera_flag == 2)
		{
			lkz -= 1;
			cam_sens = 1;
			camera_flag = 3;
			cam_height = 3;
			cam_dist = 0;
		}
		else if(camera_flag == 3)
			camera_flag = 4;
		else if(camera_flag == 4)
			camera_flag = 5;
		else if(camera_flag == 5)
			camera_flag = 0;
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
			cam_height = 20;
			cam_dist = 40;
		}
	}
}




void releasekey(unsigned char key, int x, int y)
{
	if(key == 'w')
		keyflag['w'] = 0;
	if(key == 'a')
		keyflag['a'] = 0;
	if(key == 's')
		keyflag['s'] = 0;
	if(key == 'd')
		keyflag['d'] = 0;
	if(key == 'q')
		keyflag['q'] = 0;
	if(key == 'e')
		keyflag['e'] = 0;
}




void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		zoom+=3;
	}
	else
	{
		zoom-=3;
	}
	return;
}





void pressSpecialKey(int key, int xx, int yy)
{
	switch (key) {
		case GLUT_KEY_UP : heli_y += 1; break;
		case GLUT_KEY_DOWN : heli_y -= 1; break;
		case GLUT_KEY_LEFT : heli_x += 1; break;
		case GLUT_KEY_RIGHT : heli_x -= 1; break;
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
		heli_look_x = -sin(angle + deltaAngle);
		heli_look_y = cos(angle + deltaAngle);
		heli_look_z = 20*sin(deltaAnglez);
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
	if ((button == 3) || (button == 4)) // It's a wheel event
	{
		if (button == 3)
		{
			zoom+=10;
			return; 
		}// Disregard redundant GLUT_UP events
		if(button == 4)
		{
			zoom-=10;
			return;
		}
	}
}


void timeup(int value)
{
	if(timer == 100 && point_count < 10)
		end_game = 1;
	else if(timer == 100 && point_count == 10)
		end_game = 2;
	timer++;
	if(timer>100)
		timer = 0;


	glutTimerFunc(1000,timeup,0);
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

	for(i=0;i<10;i++)
	{
		rand_c[i][0] = 10 + rand() % 100;
		rand_c[i][1] = 10 + rand() % 100;
		rand_c[i][2] = 1;
	}
	for(i=0;i<15;i++)
	{
		tree_pos[i][0] = 10 + rand() % 100;
		tree_pos[i][1] = 10 + rand() % 100;
	}
	for(i=0;i<256;i++)
		keyflag[i] = 0;
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
	glutKeyboardUpFunc(releasekey);
	glutSpecialFunc(pressSpecialKey); // process special key pressed

	glutMouseWheelFunc(mouseWheel);	
	// Warning: Nonstandard function! Delete if desired.
	glutTimerFunc(1000,timeup,0);
	glutSpecialUpFunc(releaseSpecialKey); // process special key release

	// OpenGL init
	glEnable(GL_DEPTH_TEST);

	// enter GLUT event processing cycle
	glutMainLoop();

	return 0; // this is just to keep the compiler happy
}
