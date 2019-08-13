#include <windows.h>
#include <GL/glut.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include "Line.h"

using namespace std;

Line::Line(){}

Line::Line(float x, bool vis)
{
    visible = vis;
    xpos = x;
}

void Line::drawLine(Line *line){
    double h = glutGet(GLUT_WINDOW_HEIGHT);

    glBegin(GL_LINES);
        glVertex2d(line->xpos,0);
        glVertex2d(line->xpos,h);
    glEnd();
}
