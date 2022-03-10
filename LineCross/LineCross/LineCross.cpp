#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  800 // Width of window
#define WINDOW_HEIGHT 500 // Height of window

#define TIMER_PERIOD  1000 // Period for the timer.
#define TIMER_ON         0 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

#define START 0 // State to draw starting circle
#define END 1 // State to draw end circle
#define LINE 2 // State to draw lines

typedef struct {
    float x, y; // variables for coordinates
} point_t;

point_t mouse; //Structure to track mouse

int mode = START; // Program starts in drawing start circle mode

typedef struct {
    int xpos, ypos; // variables for coordinates
}pts_t;

pts_t ps[40]; // Structure to track lines drawn in "draw lines" mode


/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

int startx, starty, endx, endy; // Variables for the coordinates of Start and End circles

bool cl = 0; // Variable for trackig the color of the lines

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r) // Drawing Circle
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r) 
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font) 
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

void vprint(int x, int y, void* font, const char* string, ...) // For writing on the screen
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}


void vprint2(int x, int y, float size, const char* string, ...) { 
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

void drawLine(float x1, float y1, float x2, float y2) { // for drawing lines
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void showLineSegment(int ax, int bx, int ay, int by) { // for drawing line segments
    glColor4f(0, 0, 0, 0.5);
    glLineWidth(4);
    glBegin(GL_LINES);
    glVertex2f(ax, ay);
    glVertex2f(bx, by);
    glEnd();
    glLineWidth(1);
}

void drawlinedynamic(int ax, int bx, int ay, int by, bool a) { // for drawing lines in "draw lines" mode

    if (a == 0)
        glColor4f(0, 1, 0, 0.5);
    else
        glColor4f(1, 0, 0, 0.5);

    glLineWidth(4);
    glBegin(GL_LINES);
    glVertex2f(ax, ay);
    glVertex2f(bx, by);
    glEnd();
    glLineWidth(1);

}


float findSlope(int x1, int y1, int x2, int y2)
{
    float slope = 0;

    if (x1 < x2)
        slope = (y2 - y1) / (x2 - x1);
    else if (x1 > x2)
        slope = (y2 - y1) / (x1 - x2);

    return(slope);
}

int findIntercept(int x1, int y1, int x2, int y2)
{
    float m;
    int n;

    m = findSlope(x1, y1, x2, y2);
    n = y1 - m * x1;

    return(n);
}

bool checkIntersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    bool flag = 0;
    float m1, m2, i;
    int n1, n2;

    m1 = findSlope(x1, y1, x2, y2);
    m2 = findSlope(x3, y3, x4, y4);

    n1 = findIntercept(x1, y1, x2, y2);
    n2 = findIntercept(x3, y3, x4, y4);

    if (x1 < x2)
    {
        for (i = x1; i < x2; i += 0.25)
        {
            float attempt = (m1 * i + n1) - (m2 * i + n2);
            if (fabs(attempt) < 3.0)
                flag = 1;
        }
    }

    else {
        for (i = x2; i < x1; i += 0.25)
        {
            float attempt = (m1 * i + n1) - (m2 * i + n2);
            if (fabs(attempt) < 3.0)
                flag = 1;
        }
    }

    return(flag);
}

int k = 0; // Variable to track which coordinate line we are drawing (0 = blue line, 1 = red line) 
void drawCoordinateSystem() { // Drawing the coordinate system

    glColor3f(0, 0, 1); // Blue lines

    for (int i = -winWidth / 2 / 50 * 50; i <= winWidth / 2; i += 50) { // Vertical lines
        if (k % 2 == 0) {
            glColor3f(0, 0, 1);
            drawLine(i, winHeight / 2, i, -winHeight / 2);
        } // Some lines are blue
        else
        {
            glColor3f(1, 0.5, 0.5);
            drawLine(i, winHeight / 2, i, -winHeight / 2);
        }
        k++; // Other lines are red
    }

    for (int i = -winHeight / 2 / 50 * 50; i <= winHeight / 2; i += 50) { // Horizontal lines
        if (k % 2 == 0) {
            glColor3f(0, 0, 1);
            drawLine(winWidth / 2, i, -winWidth / 2, i);
        } // Some lines are blue
        else
        {
            glColor3f(1, 0.5, 0.5);
            drawLine(winWidth / 2, i, -winWidth / 2, i);
        }
        k++; // Other lines are red
    }
    k = 0; // Return to original value
}

void showCursorPosition() { //show mouse coordinate at top right region
    glColor3f(1, 1, 1); // White text
    vprint(winWidth / 2 - 100, winHeight / 2 - 30, GLUT_BITMAP_8_BY_13, "(%.0f %.0f)", mouse.x, mouse.y);
}

void drawstart(int x, int y) // Drawing the starting circle
{
    glColor3f(0, 1, 0); // Green circle
    circle(x, y, 40);
    glColor3f(0, 0, 0); // Black text
    vprint(x - 5, y - 2, GLUT_BITMAP_9_BY_15, "S"); // Circle has letter S on it
}

void drawend(int x, int y) // Drawing the end circle
{
    glColor3f(0, 1, 0); // Green circle
    circle(x, y, 40);
    glColor3f(0, 0, 0); // Black text
    vprint(x - 5, y - 2, GLUT_BITMAP_8_BY_13, "E"); // Circle has letter E on it
}

int sn = 0, en = 0, ln = 0;
void onClick(int button, int stat, int x, int y) // for click event
{
    if (button == GLUT_LEFT_BUTTON && stat == GLUT_UP) {

        switch (mode)
        {
        case START:
            startx = x - winWidth / 2;
            starty = winHeight / 2 - y;
            sn++; // Draw starting circle at the location of the click
            break;
        case END:
            endx = x - winWidth / 2;
            endy = winHeight / 2 - y;
            en++; // Draw end circle at the location of the click
            break;
        case LINE:
            if (ln < 40)
            {
                ln++;
                ps[ln - 1].xpos = x - winWidth / 2;
                ps[ln - 1].ypos = winHeight / 2 - y;
                // Draw points to mark beginning and end of lines
            } break;
        }
    }
    glutPostRedisplay();
}


void display() {

    glClearColor(1, 1, 1, 1); // White screen
    glClear(GL_COLOR_BUFFER_BIT);


    drawCoordinateSystem(); //draw x and y axis

    glColor3f(0, 0, 0);
    glRectf(-winWidth / 2, -winHeight / 2, winWidth / 2, -winHeight / 2.5); // Bottom black bar
    glRectf(-winWidth / 2, winHeight / 2, winWidth / 2, winHeight / 2.5); // Top black bar
    showCursorPosition(); // Show mouse coordinate at top right region
    vprint(-winWidth / 2 + 30, winHeight / 2 - 30, GLUT_BITMAP_8_BY_13, "Kayra KAVLAK 2021");
    glColor3f(1, 1, 1);
    vprint(-winWidth / 2 + 30, -winHeight / 2 + 20, GLUT_BITMAP_8_BY_13, "F1: SET START POSITION");
    vprint(-winWidth / 2 + 230, -winHeight / 2 + 20, GLUT_BITMAP_8_BY_13, "F2: SET END POSITION");
    vprint(-winWidth / 2 + 420, -winHeight / 2 + 20, GLUT_BITMAP_8_BY_13, "F3: DRAW LINES");

    switch (mode) { // Drawing yellow circles on text
    case START:
        glColor4f(1, 1, 0, 0.5);
        circle(-winWidth / 2 + 40, -winHeight / 2 + 23, 15); // Yellow circle on "F1"

        break;
    case END:
        glColor4f(1, 1, 0, 0.5);
        circle(-winWidth / 2 + 240, -winHeight / 2 + 23, 15); //Yellow circle on "F2"

        break;
    case LINE:
        glColor4f(1, 1, 0, 0.5);
        circle(-winWidth / 2 + 430, -winHeight / 2 + 23, 15); //Yellow circle on "F3"
        break;
    }

    if (sn != 0)
        drawstart(startx, starty); // Draw starting circle if click event has happened
    if (en != 0) {
        drawend(endx, endy); // Draw end circle if click event has happened
        if (sn != 0)
            showLineSegment(startx, endx, starty, endy);
        // Draw line segments if both start and end circle is drawn
    }
    if (ln != 0) // Draw line points if click event has happened
    {
        for (int k = 0; k < ln; k++)
        {
            glColor3f(0, 0, 0); // Black points
            circle(ps[k].xpos, ps[k].ypos, 4); // Record the point coordinates into array

            if (k % 2 == 1) // Only draw line between points if a pair of points is drawn
            {
                cl = checkIntersect(startx, starty, endx, endy, ps[k - 1].xpos, ps[k - 1].ypos, ps[k].xpos, ps[k].ypos);
                drawlinedynamic(ps[k].xpos, ps[k - 1].xpos, ps[k].ypos, ps[k - 1].ypos, cl);
            }
        }

    }

    glutSwapBuffers();
}





void onKeyDown(unsigned char key, int x, int y) 
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y) 
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onSpecialKeyDown(int key, int x, int y) 
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = true; break;
    case GLUT_KEY_DOWN: down = true; break;
    case GLUT_KEY_LEFT: left = true; break;
    case GLUT_KEY_RIGHT: right = true; break;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onSpecialKeyUp(int key, int x, int y) // For F1, F2, F3 and F4 controls
{
    
    switch (key) {

    case GLUT_KEY_F1:
        mode = START;
        break; // F1 key for drawing starting circles mode

    case GLUT_KEY_F2:
        mode = END;
        break; // F2 key for drawing end circles mode

    case GLUT_KEY_F3:
        mode = LINE;
        break; // F3 key for drawing lines mode

    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}


void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) { // I didn't use this
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}


void onMove(int x, int y) { // Track the movement of the mouse
    // Write your codes here.
    mouse.x = x - winWidth / 2;
    mouse.y = winHeight / 2 - y;
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

#if TIMER_ON == 1 // I didn't use this
void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay(); // display()

}
#endif

void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("LineCross");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1 // I didn't use this
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}