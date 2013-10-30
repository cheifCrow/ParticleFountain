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

#define camerastate unsigned int
#define out 0
#define pfollow 1

time_t begTime = 0;
struct timeval start, update;
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
int numP = 0;
float cspacing = 0.0001;
float tspacing = 0;
int rotation = 0;
renderstate rstate = fountain;
camerastate cstate = out;

using namespace std;

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

class Particle{
public:
    bool getDeath()
    {
        if(fade <= 0 && marked == true)
            return(true);
        else
            return(false);
    }
    void draw()
    {
        checkLifeTime();
        applyforces();
        if(marked == true && fade > 0)
        {
            gettimeofday(&current, NULL);
            d = preciseDiff(current, death);
            fade = 1-(1*d);
        }
        if(rstate==fountain)
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
        glutSolidSphere(radius,8,8);
        glPopMatrix();
    }
    Particle()
    {
        radius = .25;
        gravity = -9.8;
        lifespan = 12;
        mass = (((float)rand()/RAND_MAX)*(15-10))+10;
        yAccel = gravity*mass;

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
        }

        marked = false;
        fade = 1;

        gettimeofday(&spawn, NULL);
        update = spawn;

    }
private:
    void checkLifeTime()
    {
        gettimeofday(&current, NULL);
        t = preciseDiff(current, spawn);
        if( t > lifespan && marked == false)
        {
            marked = true;
            gettimeofday(&death, NULL);
        }
    }
    void applyforces()
    {
        printf("%i\n",rstate);
        if (friction && rstate == fountain)
            f = 0.75;
        else if (friction && rstate == rain)
            f = 0.25;
        else
            f = 1;

        gettimeofday(&current, NULL);
        t = preciseDiff(current, spawn);
        t2 = preciseDiff(current, update);

        yspeed = yo+(yAccel*t2);

        x = xi+(xo*t);
        y = yi+((yo*t2)+(((float)1/2)*yAccel*(t2*t2)));
        z = zi+(zo*t);

        if(y<=radius && yspeed<0 && (x<50 && x>-50) && (z<50 && z>-50))
        {
            yo = -yspeed*f;
            yi = y;
            gettimeofday(&update, NULL);
            printf("%f\n",yo);
            //printf("%f %f\n",yspeed,y);
        }
        //printf("%f %f %f\n",x,y,z);
        //printf("%f\n",(y));
    }
    float x, y, z, xi, yi, zi, radius, mass;
    float gravity, yAccel, f, xspeed, yspeed, zspeed, xo, yo, zo;
    float fade, lifespan;
    float colour[4];
    double t, t2, d;
    bool marked;
    struct timeval spawn, update, current, death;
};

//Particle test = Particle();

vector<Particle> parts;

void display()
{
    gettimeofday(&update, NULL);

    pTime = preciseDiff(update, start);

    //printf("%i\n",parts.size());
    if(pTime > tspacing)
    {
        parts.push_back(Particle());
        tspacing += cspacing;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(Ex, Ey, Ez, 0, 30, 0, 0, 1, 0);

    /*glPushMatrix();
    glRotatef(20*pTime, 0, 1, 0);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_FRONT);

        glColor3f(0, 0, 0);
        glutSolidCube(2.1);

        glCullFace(GL_BACK);

        glColor3f(1, 0, 0);
        glutSolidCube(2);
    glPopMatrix();*/

    glBegin(GL_QUADS);
    glColor3f(0, 0, 1);
    glVertex3f(50, 0, 50);
    glVertex3f(50, 0, -50);
    glVertex3f(-50, 0, -50);
    glVertex3f(-50, 0, 50);
    glEnd();

    //test.draw();
    for(int x = 0; x<parts.size(); x++)
    {
        if(parts[x].getDeath() == true)
        {
            parts.erase(parts.begin()+x);
            break;
        }
    }

    for(int x = 0; x<parts.size(); x++)
    {
        parts[x].draw();
    }
    glFlush();
    glutSwapBuffers();
    frames++;
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
        for(int x = 0; x<parts.size(); x++)
        {
            parts.erase(parts.end());;
        }
    }
    else if(key == 'x' && rstate != rain)
    {
        rstate = rain;
        for(int x = 0; x<parts.size(); x++)
        {
            parts.erase(parts.end());;
        }
    }
    else if(key == 'a')
    {
        friction = !friction;
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
