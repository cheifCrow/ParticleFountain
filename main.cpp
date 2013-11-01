#ifdef __APPLE__
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif

#define PI 3.14159265

#include <iostream>

#include <time.h>
#include <sys/time.h>

#include <math.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>

#define renderstate unsigned int
#define fountain 0
#define cannon 1
#define rain 2
#define debug 5

#define camerastate unsigned int
#define out 0
#define pfollow 1

using namespace std;

time_t begTime = 0;
struct timeval start, update, tfrozen, thawed;
float frames = 0;
float sleeptime = 0;
double mod = 0;
long sec = 0;
long usec = 0;
double pTime = 0;
double camerahRadius = 170;
double Ex = 0;
double Ey = 50;
double Ez = camerahRadius;
bool fRun = true;
bool friction = true;
bool paused = false;
bool frozen = false;
int numP = 0;
float cspacing = 0.0001;
float tspacing = 0;
float pauseOffset = 0;
int rotation = 0;
float cangle = 45; //the angle of the cannon
float clength = 60; //the length of the cannon shaft
float cradius = 20; //the radius of the cannon barrel
vector<int> tokill;
renderstate rstate = fountain;
camerastate cstate = out;

double preciseDiff(struct timeval & t1, struct timeval & t2)
{
    long sec = t1.tv_sec - t2.tv_sec;
    long usec = t1.tv_usec - t2.tv_usec;

    return((((sec) * 1000 + usec/1000.0) + 0.5)/1000);
}

float toRadians(float angle)
{
    return (angle*PI/180);
}

class Cuboid{
public:
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    float getLength()
    {
        return length;
    }
    float getHeight()
    {
        return height;
    }
    float getWidth()
    {
        return width;
    }
    void draw()
    {
        glPushMatrix();
        glBegin(GL_QUADS);
        glColor3f(1, 0, 0);
        
        glVertex3f(x, y, z);
        glVertex3f(x+length, y, z);
        glVertex3f(x+length, y+height, z);
        glVertex3f(x, y+height, z);
        
        glVertex3f(x+length, y, z);
        glVertex3f(x+length, y, z-width);
        glVertex3f(x+length, y+height, z-width);
        glVertex3f(x+length, y+height, z);
        
        glVertex3f(x+length, y, z-width);
        glVertex3f(x, y, z-width);
        glVertex3f(x, y+height, z-width);
        glVertex3f(x+length, y+height, z-width);
        
        glVertex3f(x, y, z);
        glVertex3f(x, y+height, z);
        glVertex3f(x, y+height, z-width);
        glVertex3f(x, y, z-width);
        
        glVertex3f(x, y+height, z);
        glVertex3f(x+length, y+height, z);
        glVertex3f(x+length, y+height, z-width);
        glVertex3f(x, y+height, z-width);
        
        glVertex3f(x, y, z);
        glVertex3f(x+length, y, z);
        glVertex3f(x+length, y, z-width);
        glVertex3f(x, y, z-width);
        glEnd();
        glPopMatrix();
    }
    Cuboid(float _x, float _y, float _z, float _length, float _height, float _width)
    {
        x = _x;
        y = _y;
        z = _z;
        width = _width;
        height = _height;
        length = _length;
    }
private:
    float x, y, z, length, height, width;
};

Cuboid object = Cuboid(5, 0, -10, 20, 20, 20);

class Cannon{
public:

    void setAngle(float _angle)
    {
        angle = _angle;
        if(angle > 90)
            angle = 90;
        if(angle < 0)
            angle = 0;
    }
    float getAngle()
    {
        return angle;
    }
    float getRadius()
    {
        return radius;
    }
    float getX()
    {
        return x;
    }
    float getY()
    {
        return y;
    }
    float getZ()
    {
        return z;
    }
    float getLength()
    {
        return length;
    }
    void draw()
    {
        glPushMatrix();
        glTranslatef(x,y,z);
        glRotatef(angle, 0, 0, 1);
        glPushMatrix();
        
        glTranslatef(0, radius, 0);
        
        for(int x = 0; x<sections; x++)
        {
            glPushMatrix();
            glRotatef(rotation*x, 1, 0, 0);
            glTranslatef(0, height, 0);
            glColor3f(1, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(0, 0, z+width);
            glVertex3f(length, 0, z+width);
            glVertex3f(length, 0, z-width);
            glVertex3f(0, 0, z-width);
            
            glVertex3f(0, 0, z-width);
            glVertex3f(length, 0, z-width);
            glVertex3f(length, 0, z+width);
            glVertex3f(0, 0, z+width);
            glEnd();
            glPopMatrix();
        }
        glPushMatrix();
        glRotatef(rotation/2, 1, 0, 0);
        glBegin(GL_POLYGON);
        for(int x = 0; x<sections; x++)
        {
            glVertex3f(0, cos(toRadians(rotation*-x))*radius, sin(toRadians(rotation*-x))*radius);
        }
        glEnd();
        glPopMatrix();
        glPopMatrix();
        glPopMatrix();
    }
    Cannon(float _x, float _y, float _z, float _sections, float _length, float _radius, float _angle)
    {
        x = _x;
        y = _y;
        z = _z;
        sections = _sections;
        length = _length;
        radius = _radius;
        angle = _angle;
        rotation = 360/sections;
        width = sin(toRadians(rotation/2))*radius;
        height = cos(toRadians(rotation/2))*radius;
        if(sections < 3){
            sections = 3;
        }
    }
private:
    float x, y, z, sections, length, radius, angle, rotation, width, height;
};

Cannon c = Cannon(-50, 0, 0, 8, 30, 5, 45);

class Particle{
public:
    void setTOffset(float _t)
    {
        tOffset += _t;
        t2Offset += _t;
        t3Offset += _t;
        t4Offset += _t;
        if(marked && fade > 0)
            dOffset += _t;
    }
    bool getDeath()
    {
        if(fade <= 0 && marked == true)
            return(true);
        else
            return(false);
    }
    void draw()
    {
        if(!paused)
        {
            checkLifeTime();
            applyforces();
            if(marked == true && fade > 0)
            {
                gettimeofday(&current, NULL);
                d = preciseDiff(current, death) - dOffset;
                fade = 1-(1*d);
            }
        }
        //printf("%f\n",t);
        if(rstate==fountain || rstate == cannon)
        {
            colour[0] = 0;
            colour[1] = 1;
            colour[2] = 0;
            colour[3] = fade;
        }
        else if(rstate==rain)
        {
            colour[0] = 0;
            colour[1] = 0.5;
            colour[2] = 0.5;
            colour[3] = fade;
        }
        glPushMatrix();
        glColor4fv(colour);
        glTranslatef(x, y, z);
        glRotatef(rotation*t, rdirection[0], rdirection[1], rdirection[2]);
        glutSolidSphere(radius,8,8);
        glPopMatrix();
    }
    Particle()
    {
        radius = .25;
        gravity = -9.8;
        lifespan = 10;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        rotation = (((float)rand()/RAND_MAX)*(25-1))+1;
        rdirection[0] = ((float)rand()/RAND_MAX);
        rdirection[1] = ((float)rand()/RAND_MAX);
        rdirection[2] = ((float)rand()/RAND_MAX);
        yAccel = gravity*mass;
        tOffset = 0;
        t2Offset = 0;
        dOffset = 0;

        if(rstate == fountain)
        {
            xi = 0;
            yi = radius;
            zi = 0;
            xo = (((float)rand()/RAND_MAX)*(10))-5;
            yo = (((float)rand()/RAND_MAX)*(140-120))+120;
            zo = (((float)rand()/RAND_MAX)*(10))-5;
        }
        if(rstate == rain)
        {
            xi = (((float)rand()/RAND_MAX)*(50+50))-50;
            yi = 120;
            zi = (((float)rand()/RAND_MAX)*(50+50))-50;
            xo = (((float)rand()/RAND_MAX)*(10))-5;
            yo = 0;
            zo = (((float)rand()/RAND_MAX)*(10))-5;
            lifespan = 5;
        }
        if(rstate == cannon)
        {
            float cannonAngle = toRadians(c.getAngle());
            xi = (c.getX()+(cos(cannonAngle)*c.getLength()))-sin(cannonAngle)*c.getRadius();
            yi = (c.getY()+(sin(cannonAngle)*c.getLength()))+cos(cannonAngle)*c.getRadius();
            zi = 0;
            xo = cos(cannonAngle)*80;
            yo = sin(cannonAngle)*80;
            zo = (((float)rand()/RAND_MAX)*(10))-5;
        }
        marked = false;
        fade = 1;

        gettimeofday(&spawn, NULL);
        update = spawn;
        update2 = spawn;
        update3 = spawn;

    }
private:
    void checkLifeTime()
    {
        if( t > lifespan && marked == false)
        {
            marked = true;
            gettimeofday(&death, NULL);
        }
        
    }
    void applyforces()
    {
        if (friction && (rstate == fountain || rstate == cannon))
            f = 0.75;
        else if (friction && rstate == rain)
            f = 0.25;
        else
            f = 1;

        gettimeofday(&current, NULL);
        t = preciseDiff(current, spawn) - tOffset;
        t2 = preciseDiff(current, update) - t2Offset;
        t3 = preciseDiff(current, update2) - t3Offset;
        t4 = preciseDiff(current, update3) - t4Offset;
        
        
        yspeed = yo+(yAccel*t3);

        x = xi+(xo*t2);
        y = yi+(yo*t3+(((float)1/2)*yAccel*(t3*t3)));
        z = zi+(zo*t4);
        
        
        if(y<=radius && yspeed < 0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yo = -yspeed*f;
            yi = radius;
            t3Offset = 0;
            gettimeofday(&update2, NULL);

        }
        if(y<=(radius+object.getY()+object.getHeight()) && yspeed < 0 &&
           (x<object.getX()+object.getLength() && x>object.getX()) &&
           (z<object.getZ()) && (z>object.getZ()-object.getWidth()))
        {
            yo = -yspeed*f;
            yi = radius+object.getY()+object.getHeight();
            t3Offset = 0;
            gettimeofday(&update2, NULL);

        }
        if(z<=(object.getZ()+radius) && zspeed < 0 &&
           (x<object.getX()+object.getLength() && x>object.getX()) &&
           (y<object.getY()+object.getHeight() && y>object.getY()))
        {
            zo = -zo;
            zi = object.getZ()+radius;
            t4Offset = 0;
            gettimeofday(&update3, NULL);
        }
    }
    float x, y, z, xi, yi, zi, radius, mass;
    float gravity, yAccel, f, xspeed, yspeed, zspeed, xo, yo, zo, rotation;
    float fade, lifespan, tOffset, t2Offset, t3Offset, t4Offset, dOffset;
    float colour[4];
    float rdirection[3];
    double t, t2, t3, t4, d;
    bool marked;
    struct timeval spawn, update, update2, update3, current, death;
};

vector<Particle> parts;

void display()
{
    if(paused && !frozen)
    {
        gettimeofday(&tfrozen, NULL);
        frozen = true;
    }
    if(!paused && frozen)
    {
        gettimeofday(&thawed, NULL);
        pauseOffset = preciseDiff(thawed, tfrozen);
        for(int x = 0; x<parts.size(); x++)
        {
            parts[x].setTOffset(pauseOffset);
        }
        frozen = false;
    }
    gettimeofday(&update, NULL);

    pTime = preciseDiff(update, start) - pauseOffset;

    if(pTime > tspacing && !paused)
    {
        parts.push_back(Particle());
        tspacing += cspacing;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(Ex, Ey, Ez, 0, 30, 0, 0, 1, 0);

    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(0, 0, 1);
    glVertex3f(50, 0, 50);
    glVertex3f(50, 0, -50);
    glVertex3f(-50, 0, -50);
    glVertex3f(-50, 0, 50);
    glEnd();
    glPopMatrix();
    
    object.draw();
    if(rstate == cannon)
    {
        c.draw();
    }
    //test.draw();
    for(int x = 0; x<parts.size(); x++)
    {
        if(parts[x].getDeath() == true)
        {
            tokill.push_back(x);
            if(tokill.size() > 1)
                tokill[tokill.size()-1] -= tokill.size()-1;
        }
    }
    for(int x = 0; x<tokill.size(); x++)
    {
        parts.erase(parts.begin()+tokill[x]);
        if(x == tokill.size()-1)
        {
            tokill.clear();
        }
    }

    for(int x = 0; x<parts.size(); x++)
    {
        parts[x].draw();
    }
    
    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();

}

void reshape(int width, int height)
{
    float aspect = 1.0*width/height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0,0,width,height);
    gluPerspective(45,aspect, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);

}

void keyboard(unsigned char key, int x, int y)
{
    if(key == 'z' && rstate != fountain)
    {
        rstate = fountain;
        parts.clear();
    }
    else if(key == 'x' && rstate != rain)
    {
        rstate = rain;
        parts.clear();
    }
    else if(key == 'c' && rstate != cannon)
    {
        rstate = cannon;
        parts.clear();
    }
    else if(key == 'a')
    {
        friction = !friction;
    }
    else if(key == 'r')
    {
        parts.clear();
    }
    else if(key == ' ')
    {
        paused = !paused;
    }

    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    if(key == GLUT_KEY_RIGHT)
    {
        rotation++;
        Ez = (cos(toRadians(10*rotation)))*camerahRadius;
        Ex = (sin(toRadians(10*rotation)))*camerahRadius;
    }
    else if(key == GLUT_KEY_LEFT)
    {
        rotation--;
        Ez = (cos(toRadians(10*rotation)))*camerahRadius;
        Ex = (sin(toRadians(10*rotation)))*camerahRadius;
    }
    else if(key == GLUT_KEY_UP && rstate == cannon && !paused)
    {
        c.setAngle(c.getAngle()+5);
    }
    else if(key == GLUT_KEY_DOWN && rstate == cannon && !paused)
    {
        c.setAngle(c.getAngle()-5);
    }
    glutPostRedisplay();
}
int main(int argc, char *argv[])
{
    srand(clock());
    rand();
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(500, 50);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Particle Fountain");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 0.1, 1000);

    glClearColor(0, 0, 0, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(Ex, Ey, Ez, 0, 30, 0, 0, 1, 0);

    gettimeofday(&start, NULL);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
