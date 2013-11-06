#ifdef __APPLE__
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif

#include <iostream>

#include <time.h>
#include <sys/time.h>

#include <math.h>
#define PI 3.14159265

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

struct timeval point, current, tfrozen, thawed, instant;
double pTime = 0;
double cameraRadius = 170;
double Ex = 0;
double Ey = 50;
double Ez = cameraRadius;
double Mx = 0;
double Mz = 1;
bool friction = true;
bool paused = false;
bool frozen = false;
float tspacing = 0.02;
float pauseOffset = 0;
float p = 0;
int cameraRotation = 0;
renderstate rstate = fountain;
camerastate cstate = out;

//This function will return the difference in seconds with millisecond precision between points in time
double preciseDiff(struct timeval & t1, struct timeval & t2)
{
    long sec = t1.tv_sec - t2.tv_sec;
    long usec = t1.tv_usec - t2.tv_usec;

    return((((sec) * 1000 + usec/1000.0) + 0.5)/1000);
}

//Since the all the trig functions take radians, but it is far easier to think in terms of angles,
//this function will return the radian of any angle
float toRadians(float angle)
{
    return (angle*PI/180);
}

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
    bool getDeath()
    {
        if(marked && fade<=0)
            return true;
        else
            return false;
    }
    void setTimeOffset(float _offset)
    {
        timeOffset += _offset;
    }
    
    void applyForces()
    {
        if(friction && (rstate==fountain || rstate==cannon))
            f = 0.75;
        else if(friction && rstate==rain)
            f = 0.25;
        else
            f = 1;
        
        d  = preciseDiff(current, previous) - timeOffset;
        
        rotationAngle+=rotationSpeed*d;
        
        yspeed+=(gravity*d);
        
        x+=(xspeed*d);
        y+=(yspeed*d);
        z+=(zspeed*d);
        
        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yspeed = -yspeed*f;
            y = radius;
        }
        timeOffset = 0;
        previous = current;
        lived += d;
        checkLifeTime();
        if(marked && fade>=0)
        {
            fade = 1-(lived-lifespan);
            if(fade<0)
                fade = 0;
        }
    }
    
    void draw()
    {
        if(rstate==fountain || rstate == cannon)
        {
            colour[0] = red;
            colour[1] = green;
            colour[2] = blue;
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
        glRotatef(rotationAngle, rotationMultipliers[0], rotationMultipliers[1], rotationMultipliers[2]);
        glutSolidSphere(radius, 16, 16);
        glPopMatrix();
        
    }
    Particle()
    {
        previous = current;
        timeOffset = 0;
        d = 0;
        radius = 0.25;
        gravity = -9.8;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        gravity = gravity*mass;
        
        rotationAngle = (((float)rand()/RAND_MAX)*(25-1))+1;
        rotationSpeed = (((float)rand()/RAND_MAX)*(5-1))+1;
        rotationMultipliers[0] = ((float)rand()/RAND_MAX);
        rotationMultipliers[1] = ((float)rand()/RAND_MAX);
        rotationMultipliers[2] = ((float)rand()/RAND_MAX);
        red = ((float)rand()/RAND_MAX);
        green = ((float)rand()/RAND_MAX);
        blue = ((float)rand()/RAND_MAX);
        
        if(rstate == fountain)
        {
            x = 0;
            y = radius;
            z = 0;
            xspeed = (((float)rand()/RAND_MAX)*(5+5))-5;
            yspeed = (((float)rand()/RAND_MAX)*(140-120)+120);
            zspeed = (((float)rand()/RAND_MAX)*(5+5))-5;
            lifespan = 12;
        }
        else if(rstate == rain)
        {
            x = (((float)rand()/RAND_MAX)*(50+50))-50;
            y = 120;
            z = (((float)rand()/RAND_MAX)*(50+50))-50;
            xspeed = 0;
            yspeed = 0;
            zspeed = 0;
            lifespan = 5;
        }
        else if(rstate == cannon)
        {
            float cannonAngle = toRadians(c.getAngle());
            x = c.getX()+(c.getLength()*(cos(cannonAngle)))-(c.getRadius()*sin(cannonAngle));
            y = c.getY()+(c.getLength()*(sin(cannonAngle)))+(c.getRadius()*cos(cannonAngle));
            z = 0;
            xspeed = cos(cannonAngle)*80;
            yspeed = sin(cannonAngle)*80;
            zspeed = (((float)rand()/RAND_MAX)*(10))-5;
            lifespan = 12;
        }
        marked = false;
        fade = 1;
        lived = 0;
    }
    
private:
    void checkLifeTime()
    {
        if(lived > lifespan && !marked)
        {
            marked = true;
        }
    }
    float x, y, z, radius, mass;
    float gravity, xspeed, yspeed, zspeed, rotationAngle, rotationMultipliers[3], rotationSpeed;
    float fade, colour[4], red, green, blue, lifespan, lived, d, f, timeOffset;
    timeval previous;
    bool marked;
};

vector<Particle> parts;

void display()
{
    if(paused && !frozen)
    {
        gettimeofday(&tfrozen, NULL);
        frozen = true;
    }
    else if(!paused && frozen)
    {
        gettimeofday(&thawed, NULL);
        pauseOffset = preciseDiff(thawed, tfrozen);
        for(int x = 0; x<parts.size(); x++)
        {
            parts[x].setTimeOffset(pauseOffset);
        }
        frozen = false;
    }
    
    gettimeofday(&current, NULL);
    
    pTime = preciseDiff(current, point) - pauseOffset;
    
    if(pTime > tspacing && !paused)
    {
        parts.push_back(Particle());
        gettimeofday(&point, NULL);
    }
    
    if(!paused)
    {
        for(int x = 0; x<parts.size(); x++)
        {
            parts[x].applyForces();
        }
    }
    float l = -1;
    for(int x = 0; x<parts.size(); x++)
    {
        
        if(parts[x].getDeath())
        {
            l = x;
        }
        else if(l != -1)
        {
            if(p<=l)
                cstate = out;
            else
                p-=l;
            for(int z = 0; z<l; z++)
            {
                parts.erase(parts.begin());
            }
            break;
        }
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    if(cstate==out || parts.size()==p)
    {
        Ex = Mx*cameraRadius;
        Ez = Mz*cameraRadius;
        gluLookAt(Ex, Ey, Ez, 0, 30, 0, 0, 1, 0);
    }
    else if(cstate==pfollow)
    {
        float px = parts[p].getX();
        float py = parts[p].getY();
        float pz = parts[p].getZ();
        gluLookAt(px+(Mx*5), py+5, pz+(Mz*5), px, py, pz, 0, 1, 0);
    }
    
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(0.2, 0.2, 0.2);
    glVertex3f(50, 0, 50);
    glVertex3f(50, 0, -50);
    glVertex3f(-50, 0, -50);
    glVertex3f(-50, 0, 50);
    glEnd();
    glPopMatrix();
    
    
    for(int x = 0; x<parts.size(); x++)
    {
        parts[x].draw();
    }
    if(rstate == cannon)
    {
        c.draw();
    }

    pauseOffset = 0;
    
    glFlush();
    glutSwapBuffers();
    gettimeofday(&instant, NULL);
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
    else if(key == 'w')
    {
        cstate = pfollow;
        p = parts.size();
    }
    else if(key == 'e')
    {
        cstate = out;
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
        cameraRotation++;
        Mz = (cos(toRadians(10*cameraRotation)));
        Mx = (sin(toRadians(10*cameraRotation)));
    }
    else if(key == GLUT_KEY_LEFT)
    {
        cameraRotation--;
        Mz = (cos(toRadians(10*cameraRotation)));
        Mx = (sin(toRadians(10*cameraRotation)));
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

    gettimeofday(&current, NULL);
    gettimeofday(&point, NULL);
    gettimeofday(&instant, NULL);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
