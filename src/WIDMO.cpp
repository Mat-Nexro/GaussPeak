#include <windows.h>
#include <GL/glut.h>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include "WIDMO.h"

using namespace std;

string getexepath() //funkcja zwracajaca sciezke wykonywanego programu (pliku exe)
{
  char result[ MAX_PATH ];
  return string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
}

WIDMO::WIDMO()
{

}

void WIDMO::czytaj_e0(WIDMO *PW){

    string path;
    string unused;
    fstream file;

    path = getexepath();
    string filename = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filename.length(); i++)
        path.pop_back();

    path += "\\e0.txt";

    PW->iloscg = 0;
    file.open(path, ios::in);
    while(getline(file, unused))
        PW->iloscg++;

    file.close();

    PW->e0 = new double[iloscg];

    file.open(path, ios::in);
    for(int i = 0; i<PW->iloscg; i++){
        file >> PW->e0[i];
    }
    file.close();
}

WIDMO* WIDMO::czytaj_widmo(int numer)
{
    WIDMO *PW;
    PW = new WIDMO;

    PW->numerw = numer;

    string name;
    string path;
    string path2;
    fstream file;
    string *paths;

    PW->iloscw = 0;

    path = getexepath();
    string filename = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filename.length(); i++)
        path.pop_back();

    path += "\\files\\files.txt";

    file.open(path, ios::in);

    while(getline(file, path2))
        PW->iloscw++;

    file.close();
    paths = new string[PW->iloscw];

    file.open(path, ios::in);

        for(int i = 0; i<PW->iloscw; i++)
        {
            getline(file, path);
            paths[i] = path;
        }
        file.close();

    name = paths[PW->numerw - 1];

    PW->max_n = 0;
    ifstream in;
    in.open(name, ios::in);
    string unused;
    while ( getline(in, unused) )
        PW->max_n++;

    in.close();

    PW->en = new double[PW->max_n];
    PW->val = new double[PW->max_n];

    double a, b;
    int i = 0;
    in.open(name, ios::in);
    while (getline(in, unused))
    {
        in >> a >> b;
        PW->en[i]= a;
        PW->val[i]= b;

        if(i==0){ //ustawianie min i max energi i wartosci rysowanych na ekranie
            PW->en_max_scr = PW->en[i];
            PW->en_min_scr = PW->en[i];
            PW->val_max_scr = PW->val[i];
            PW->val_min_scr = PW->val[i];
        }

        if(i!=0 && PW->en[i] > PW->en_max_scr)
            PW->en_max_scr = PW->en[i];

        if(i!=0 && PW->en[i] < PW->en_min_scr)
            PW->en_min_scr = PW->en[i];

        if(i!=0 && PW->val[i] > PW->val_max_scr)
            PW->val_max_scr = PW->val[i];

        if(i!=0 && PW->val[i] < PW->val_min_scr)
            PW->val_min_scr = PW->val[i];

        i++;
    }

    in.close();

    return PW;
}

void WIDMO::numer_widma(int numer, WIDMO *PW)
{
    if(numer>PW->iloscw) return;
    else PW->numerw = numer;
}

void WIDMO::WYSWIETL_WIDMO(WIDMO *PW){

    PW->x = new double[PW->max_n];
    PW->y = new double[PW->max_n];

    double const col_max = glutGet(GLUT_WINDOW_WIDTH); //pobieranie aktualnej szerokosci okna
    double const row_max = glutGet(GLUT_WINDOW_HEIGHT); //pobieranie aktualnej wysokosci okna

    double skala_e = col_max/(PW->en_max_scr - PW->en_min_scr);
    double skala_v = row_max/(PW->val_max_scr - PW->val_min_scr);

    es = col_max/(PW->en_max/skala_e - PW->en_min/skala_e);
    vs = row_max/(PW->val_max/skala_v - PW->val_min/skala_v);

    for(int i = 0; i<PW->max_n; i++)
    {
        PW->x[i] = skala_e*(PW->en[i] - PW->en_min_scr);
        PW->y[i] = skala_v*(PW->val[i] - PW->val_min_scr);
    }

    for(int i = 0; i<PW->max_n; i++)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(PW->en_min, PW->en_max, PW->val_min, PW->val_max);

        glLineWidth(1.0);
        glColor3f(0,0,0);
        if(i>0){
        glBegin(GL_LINE_STRIP);
            glVertex2f(x[i-1],y[i-1]);
            glVertex2f(x[i],y[i]);
        glEnd();

        }
    }
}

double WIDMO::PIXEL2EN(double pixel, WIDMO *PW){
    double energy;
    energy = pixel / PW->es + (PW->en_min / (glutGet(GLUT_WINDOW_WIDTH)/(PW->en_max_scr - PW->en_min_scr)));
    return energy;
}

double WIDMO::EN2PIXEL(double energy, WIDMO *PW){
    double pixel;
    pixel = (energy ) * glutGet(GLUT_WINDOW_WIDTH) /(PW->en_max_scr - PW->en_min_scr);
    return pixel;
}

double WIDMO::PIXEL2VAL(double pixel, WIDMO *PW){
    double value;
    value = PW->val_max_scr + pixel / PW->vs - PW->val_max / (glutGet(GLUT_WINDOW_HEIGHT)/(PW->val_max_scr + PW->val_min_scr));
    return value;
}

double WIDMO::VAL2PIXEL(double value, WIDMO *PW){
    double pixel;
    pixel = (value ) * glutGet(GLUT_WINDOW_HEIGHT) /(PW->val_max_scr - PW->val_min_scr);
    return pixel;
}
