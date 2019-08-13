#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <process.h>
#include <vector>
#include <gtk/gtk.h>
#include "WIDMO.h"
#include "Line.h"

using namespace std;
int ilosc_z = 20000; //ilosc iteracji dopasowania (ustawiana w config.txt)
int progress = 0.0;
int update = 0;

typedef struct _GAUSS
{
    double A = 0; // ustaiwnie poczatkowe A (przy mniejszej liczbie iteracji moze byc istotne)
    double mu = 0; // ustaiwnie poczatkowe e0 (malo wazne)
    double sigma = 120; // ustaiwnie poczatkowe sigma
    double V0 = 0; // ustaiwnie poczatkowe tla
    struct _GAUSS *pgn, *pgp; //wskaznik na nastepny i poprzedni element listy dwukierunkowej
};

double Gauss( double e, _GAUSS* pg, WIDMO* pw){
        int a = e - pg->mu;
    if(e < pg->mu + 3*pg->sigma && e > pg->mu - 3*pg->sigma) //obliczmy wartosci funkcji gaussa w obrebie 3 sigma
        return  pg->A*exp(-(a)*(a)/(pg->sigma*pg->sigma)) + pg->V0;
    else return 0; //dla pozostalych zwracamy 0
}

double ObliczHi( WIDMO* pw, _GAUSS* pg ){ //obliczanie chi2
    double suma=0.0;
    double *pd; //tworzenie wskaznika na wartosci widma
    double *pe; //tworzenie wskaznika na energie widma
    int ile=pw->max_n; //zapisanie do zmiennej ilosci punktow widma
    _GAUSS *pgb=pg; //tworzenie wskaznika na _GAUSS i przypisanie mu _GAUSS z argumentu
    pd=pw->val; //ustawienie stworzonego wskaznika na poczatek tablicy wartosci widma
    pe=pw->en;  //ustawienie stworzonego wskaznika na poczatek tablicy energii widma
    int i = 0; //iterator
    while(ile){
        ile--;
        if(pe[i] < pgb->mu + 3*pgb->sigma && pe[i] > pgb->mu - 3*pgb->sigma) //obliczanie chi2 w obrebie 3 sigma
            suma+=(pd[i]-Gauss(pe[i],pgb,pw)) * (pd[i]-Gauss(pe[i],pgb,pw));
        i++;
    };
    pgb=pgb->pgn; //przesuwamy wskaznik na _GAUSS na nastepny element
    return suma;
}

void progressbar(){
    glBegin(GL_LINE_STRIP);
        glColor3f(0.0, 1.0, 0.0);
        glVertex2f(200, 300);
        glVertex2f(200 + progress, 300);
    glEnd();

    glutSwapBuffers();
}

void dopasuj(WIDMO* pw, _GAUSS* pg){
    double Hiold = 0.0, Hicurr = 0.0; //chi2 przed zmiana parametru i po zmianie parametru
    double stepA = 1.0, stepSigma = 1.0, stepE0 = 1.0;
    int n = 0, n2 = 0, n3 = 0;
    _GAUSS *pgb=pg; // tworzymy i ustawiamy wskaznik _GAUSS na _GAUSS z argumentu

    for(int i=0; i<ilosc_z; i++){ // algorytm dopasowywania gaussa do danych
    Hiold=0,Hicurr=0;
    Hiold = ObliczHi(pw,pgb);
    pgb->mu+=stepE0;
    Hicurr = ObliczHi(pw,pgb);
    pgb->mu-=stepE0;

    if(Hiold>Hicurr){
        pgb->mu+=stepE0;
        stepE0=stepE0;
        n3 = 0;
    }
    if(Hiold<Hicurr){
        pgb->mu-=stepE0;
        stepE0=-stepE0;
        n3++;
    }

    if(n3>3) stepE0/=2; // jesli dopasowywany parametr zwieksza sie i zmniejsza to zmniejsza krok w celu dokladniejszego dopasowania

    Hiold=0,Hicurr=0;
    Hiold = ObliczHi(pw,pgb);
    pgb->A+=stepA;
    Hicurr = ObliczHi(pw,pgb);
    pgb->A-=stepA;

    if(Hiold>Hicurr){
        pgb->A+=stepA;
        stepA=stepA;
        n = 0;
    }
    if(Hiold<Hicurr){
        pgb->A-=stepA;
        stepA=-stepA;
        n++;
    }

    if(n>3) stepA/=2;

    /*Hiold=0,Hicurr=0;
    Hiold = ObliczHi(pw,pgb);
    pgb->sigma+=stepSigma;
    Hicurr = ObliczHi(pw,pgb);
    pgb->sigma-=stepSigma;

    if(Hiold>Hicurr){
        pgb->sigma+=stepSigma;
        stepSigma=stepSigma;
        n2 = 0;
    }
    if(Hiold<Hicurr){
        pgb->sigma-=stepSigma;
        stepSigma=-stepSigma;
        n2++;
    }

    if(n2>3) stepSigma/=2;*/

        //update++;

        //progress = progress / ilosc_z *400;
        /*if(update == 50){
        progress++;
        progressbar();
        update = 0;
        }*/

    }
    //progress = 0;
    pgb=pgb->pgn; //przesuwamy wskaznik na _GAUSS na nastepny element

    glutPostRedisplay(); // przerysowanie ekranu
}

//tworzenie listy dwukierunkowej i funkcji do dodawania elementow do listy
struct dlistVar
{
  _GAUSS *head;
  _GAUSS *tail;
  unsigned count;
};

void l_init(dlistVar &L)
{
  L.head  = L.tail  = NULL;
  L.count = 0;
}

void l_push_front(dlistVar &L, double a, double m, double s)
{
  _GAUSS *g;

  g = new _GAUSS;
  g->A = a;
  g->mu = m;
  g->sigma = s;

  g->pgp = NULL;
  g->pgn = L.head;
  L.head  = g;
  L.count++;
  if(g->pgn) g->pgn->pgp = g;
  else L.tail = g;
}

void l_push_back(dlistVar &L, double a, double m, double s)
{
  _GAUSS *g;

  g = new _GAUSS;
  g->A = a;
  g->mu = m;
  g->sigma = s;

  g->pgn = NULL;
  g->pgp = L.tail;
  L.tail  = g;
  L.count++;
  if(g->pgp) g->pgp->pgn = g;
  else L.head = g;
}

void l_printl(dlistVar & L)
{
  _GAUSS * g;

  cout << setw(3) << L.count << " : [";
  g = L.head;
  while(g)
  {
    cout << " " << g->mu;
    g = g->pgn;
  }
  cout << " ]\n\n";
}

void disp(void);
void gauss(void);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouse2(int x, int y);
void drawText(char *text, float x, float y, float z);
void RYSUJ_GAUSS(WIDMO *, dlistVar&); //rysowanie pojedynczej listy
void RYSUJ_GAUSS(WIDMO *, dlistVar*); //rysowanie tablicy list
void config(_GAUSS *); //wczytanie pliku z ustawieniami
void zapis(_GAUSS *, dlistVar&); //zapis parametrow dopasowanych gaussow do pliku txt (zapis pojedynczej listy)
void zapis(_GAUSS *, dlistVar*); //zapis parametrow dopasowanych gaussow do pliku txt (zapis tablicy list)
void fitAll(); //dopasowuje gaussy do wszystkich e0 z pliku e0.txt
void RYSUJ_MENU(int);
void RYSUJ_LINIE(double);
void zmien_widmo(int);
void kasuj_wiersz(string, size_t); //funkcja kasujaca wiersz z pliku
void nazwa_widma();
void wyswietl_pomoc();

static int win;
double xpos[10] = {-1}; //tablica pozycji linii
int liczba_linii = 0; //aktualna ilosc linii
int l = 0; //numer linii
int xprev, yprev, xnext, ynext;
int mouse_state,mouse_button,mouse_down,mouse_up,mouse_right;
char bufferX [33]; //buffer przechowujacy x polozenie myszy
char bufferY [33]; //buffer przechowujacy y polozenie myszy
char bufferW [33]; //buffer przechowujacy numer wyswietlanego widma
char bufferN [330]; //buffer przechowujacy nazwe aktualnie wyswietlanego widma
WIDMO *PW;
double posX = 0, posY = 0;
Line linie[10];
int xr, yr = 0;
dlistVar L;
dlistVar *L2; //wskaznik na obiekt typu lista
int numerw = 1; //numer aktualnie wyswietlanego widma
int w = glutGet(GLUT_SCREEN_WIDTH); //szerokosc okna
int h = glutGet(GLUT_SCREEN_HEIGHT); //wysokosc okna
char *name; //nazwa programu
bool showWPATH = false; //widocznosc sciezki pliku na ekranie
bool showHELP = false; //widocznosc pomocy
int HELP_PAGE = 1;

int main(int argc, char **argv){

	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE); //single buffering
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE); //double buffering
	glutInitWindowSize(800,600);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-800)/2,(glutGet(GLUT_SCREEN_HEIGHT)-600)/2);

	name = argv[0];

	win = glutCreateWindow("Gauss Peak");

	L2 = new dlistVar[100]; //alokacja rozmiaru tablicy list

	// funkcje petli glownej opengl
    glutMouseFunc(mouse); //funkcja do obslugi przyciskow myszy
    glutMotionFunc(mouse2); //funkcja do obslugi ruchu myszy
    glutKeyboardFunc(keyboard); //funkcja do obslugi klawiatury
	glutDisplayFunc(disp); //funkcja wyswietlania
	glutIdleFunc(gauss); //
    // funkcje petli glownej opengl

    PW = PW->czytaj_widmo(1); //wczytanie widma nr.1
    PW->WYSWIETL_WIDMO(PW); //wyswietlenie widma
    PW->czytaj_e0(PW); //wczytanie pliku e0

	glutMainLoop(); // petla glowna opengl

}

void gauss(void){
    glutPostRedisplay(); //przerysowanie ekranu
}

void disp(void){
        if(liczba_linii > 9) liczba_linii=0;

        w = glutGet(GLUT_WINDOW_WIDTH);
        h = glutGet(GLUT_WINDOW_HEIGHT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(PW->en_min, PW->en_max, PW->val_min, PW->val_max);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if(mouse_state == mouse_down && mouse_button == mouse_right && xr != 0){ //zaznaczanie obszaru przyblizenia
        glBegin(GL_LINE_LOOP);
            glColor3f(0.8, 0.8, 0.8);
            glVertex2f(xprev, glutGet(GLUT_WINDOW_HEIGHT) - yprev);
            glVertex2f(xprev, glutGet(GLUT_WINDOW_HEIGHT) - yr);
            glVertex2f(xr, glutGet(GLUT_WINDOW_HEIGHT) - yr);
            glVertex2f(xr, glutGet(GLUT_WINDOW_HEIGHT) - yprev);
        glEnd();
        }

        RYSUJ_LINIE(1.0); //rysowanie znacznikow

        PW->WYSWIETL_WIDMO(PW);

        //RYSUJ_GAUSS(PW, L);
        RYSUJ_GAUSS(PW, L2);

        RYSUJ_MENU(8); //rysowanie menu po prawej stronie

        wyswietl_pomoc();

        Sleep (1);

        glutSwapBuffers(); //double buffering
        //glFlush(); //single buffering
}

void keyboard(unsigned char key, int x, int y){ //obsluga klawiatury
    switch(key) {
        case 'q': // widok domyslny
            PW->en_max = glutGet(GLUT_WINDOW_WIDTH);
            PW->en_min = 0;
            PW->val_max = glutGet(GLUT_WINDOW_HEIGHT);
            PW->val_min = 0;
            break;
        case 32: //space case, nowa linia
            if(l>9)l=0;

            xpos[l] = PW->EN2PIXEL(PW->PIXEL2EN(x, PW),PW);
            linie[l] = Line(xpos[l],true);

            for(int i = 0; i<l; i++){
                if(linie[i].visible == true && linie[i].xpos < PW->EN2PIXEL(PW->PIXEL2EN(x+5, PW),PW) && linie[i].xpos > PW->EN2PIXEL(PW->PIXEL2EN(x-5, PW),PW)){
                    linie[i].visible = false;
                    }
                }

            l++;
            break;
        case 8: //backspace, usuwanie linii
            for(int i = 0; i<l; i++){
            if(linie[i].visible == true && linie[i].xpos < PW->EN2PIXEL(PW->PIXEL2EN(x+5, PW),PW) && linie[i].xpos > PW->EN2PIXEL(PW->PIXEL2EN(x-5, PW),PW)){
                linie[i].visible = false;
            }
            }
            glutPostRedisplay();
            break;
        case 'a': //left
            PW->en_max -= 1;
            PW->en_min -= 1;
            break;
        case 'd': //right
            PW->en_max += 1;
            PW->en_min += 1;
            break;
        case 'w': //up
            PW->val_max += 1;
            PW->val_min += 1;
            break;
        case 's': //down
            PW->val_max -= 1;
            PW->val_min -= 1;
            break;
        case '-': //zoom out
            PW->en_max += 1;
            PW->en_min -= 1;
            PW->val_max += 1;
            PW->val_min -= 1;
            break;
        case '=': //zoom in
            PW->en_max -= 1;
            PW->en_min += 1;
            PW->val_max -= 1;
            PW->val_min += 1;
            break;
        case 'r': // nastepne widmo
            zmien_widmo(1);
            break;
        case 'e': // porzednie widmo
            zmien_widmo(-1);
            break;
        case 'f': //fitting gauss
            fitAll();
            break;
        case 'z': //widocznosc sciezki widma
            if(showWPATH == false)
                showWPATH = true;
            else
                showWPATH = false;
            break;
        default:
            break;
}
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
    switch(button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_UP){
                string path;
                path=name;
                string filen = path.substr(path.find_last_of("\\") + 1);

                for(int i=0; i<filen.length(); i++)
                    path.pop_back();

                if (x < w && x > w - 115 && y > 30 && y < 60)
                    fitAll();
                if (x < w && x > w - 115 && y > 60 && y < 90){
                    path+="\\config.txt";
                    char result[path.size()+1];
                    strcpy(result, path.c_str());
                    //cout<<result;
                    ShellExecute( NULL, "edit", result, NULL, NULL, SW_RESTORE );
                }
                if (x < w && x > w - 115 && y > 90 && y < 120){
                    path+="\\e0.txt";
                    char result[path.size()+1];
                    strcpy(result, path.c_str());
                    //cout<<result;
                    ShellExecute( NULL, "edit", result, NULL, NULL, SW_RESTORE );
                }
                if (x < w && x > w - 57 && y > 120 && y < 150)
                    zmien_widmo(1);
                if (x < w - 57 && x > w - 115 && y > 120 && y < 150)
                    zmien_widmo(-1);
                if (x < w && x > w - 115 && y > 150 && y < 180){
                    path+="\\SFC.exe";
                    char result[path.size()+1];
                    strcpy(result, path.c_str());
                    //cout<<result;
                    ShellExecute( NULL, "open", result, NULL, NULL, SW_RESTORE );
                    //PW = PW->czytaj_widmo(PW->iloscw);
                    //PW->czytaj_e0(PW);
                }
                if (x < w && x > w - 115 && y > 180 && y < 210){
                    path+="\\files\\files.txt";
                    char result[path.size()+1];
                    strcpy(result, path.c_str());
                    kasuj_wiersz(result, PW->numerw);
                    if(PW->numerw == PW->iloscw)
                        zmien_widmo(-1);
                    else
                        zmien_widmo(0);
                }
                if (x < w && x > w - 115 && y > 210 && y < 240){
                    if(showHELP == false)
                        showHELP = true;
                    else
                        showHELP = false;
                }
                if (x < w - 115 && x > w - 207 && y > 210 && y < 240 && showHELP == true){
                    HELP_PAGE++;
                    if(HELP_PAGE>3) HELP_PAGE = 1;
                }
                if (x < w - 207 && x > w - 300 && y > 210 && y < 240 && showHELP == true){
                    HELP_PAGE--;
                    if(HELP_PAGE<1) HELP_PAGE = 3;
                }
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_DOWN){
                xprev = PW->EN2PIXEL(PW->PIXEL2EN(x, PW),PW);
                yprev = PW->VAL2PIXEL(PW->PIXEL2VAL(y, PW),PW);
            }
            if (state == GLUT_UP){
                xnext = PW->EN2PIXEL(PW->PIXEL2EN(x, PW),PW);
                ynext = PW->VAL2PIXEL(PW->PIXEL2VAL(y, PW),PW);
                xr = 0;
                yr = 0;

                if(xprev > xnext && yprev > ynext) // lewy gorny
                {
                    PW->en_max = xprev;
                    PW->en_min = xnext;
                    PW->val_max = glutGet(GLUT_WINDOW_HEIGHT) - ynext;
                    PW->val_min = glutGet(GLUT_WINDOW_HEIGHT) - yprev;
                }
                if(xprev > xnext && yprev < ynext) // lewy dolny
                {
                    PW->en_max = xprev;
                    PW->en_min = xnext;
                    PW->val_max = glutGet(GLUT_WINDOW_HEIGHT) - yprev;
                    PW->val_min = glutGet(GLUT_WINDOW_HEIGHT) - ynext;
                }
                if(xprev < xnext && yprev > ynext) // prawy gorny
                {
                    PW->en_max = xnext;
                    PW->en_min = xprev;
                    PW->val_max = glutGet(GLUT_WINDOW_HEIGHT) - ynext;
                    PW->val_min = glutGet(GLUT_WINDOW_HEIGHT) - yprev;
                }
                if(xprev < xnext && yprev < ynext) // prawy dolny
                {
                    PW->en_max = xnext;
                    PW->en_min = xprev;
                    PW->val_max = glutGet(GLUT_WINDOW_HEIGHT) - yprev;
                    PW->val_min = glutGet(GLUT_WINDOW_HEIGHT) - ynext;
                }
            }
            break;
        default:
            break;
    }
    mouse_down = GLUT_DOWN;
    mouse_up = GLUT_UP;
    mouse_state = state;
    mouse_button = button;
    mouse_right = GLUT_RIGHT_BUTTON;

    posX = PW->PIXEL2EN(x, PW);
    posY = PW->val_max_scr - PW->PIXEL2VAL(y, PW);
    if(posX<0) posX = 0;
    if(posY<0) posY = 0;
    if(PW->PIXEL2EN(x, PW)>PW->en_max_scr) posX = PW->en_max_scr;
    if(PW->val_max_scr - PW->PIXEL2VAL(y, PW)>PW->val_max_scr) posY = PW->val_max_scr;

    itoa(posX,bufferX,10);
    itoa(posY,bufferY,10);

    glutPostRedisplay();
}

void mouse2(int x, int y){
    if (mouse_button == GLUT_LEFT_BUTTON && mouse_state == GLUT_DOWN) {
                linie[l-1].xpos = PW->EN2PIXEL(PW->PIXEL2EN(x, PW),PW);
                glutPostRedisplay();
    }
        if (mouse_button == GLUT_RIGHT_BUTTON && mouse_state == GLUT_DOWN ){
            xr = PW->EN2PIXEL(PW->PIXEL2EN(x, PW),PW);
            yr = PW->VAL2PIXEL(PW->PIXEL2VAL(y, PW),PW);
            glutPostRedisplay();
        }

    posX = PW->PIXEL2EN(x, PW);
    posY = PW->val_max_scr - PW->PIXEL2VAL(y, PW);
    if(posX<0) posX = 0;
    if(posY<0) posY = 0;
    if(PW->PIXEL2EN(x, PW)>PW->en_max_scr) posX = PW->en_max_scr;
    if(PW->val_max_scr - PW->PIXEL2VAL(y, PW)>PW->val_max_scr) posY = PW->val_max_scr;

    itoa(posX,bufferX,10);
    itoa(posY,bufferY,10);

    glutPostRedisplay();
}

void drawText(char *text, float x, float y, float z){ //funkcja rysujaca napisy
	char *c;
	glRasterPos3f(x, y, z);

    for (c = text; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void RYSUJ_GAUSS(WIDMO *PW, dlistVar &L){
    _GAUSS *G;
    G = L.head; //ustawienie wskaznika na poczatku listy
    while(G){ // = while(G!=NULL)
    for(int i = 0; i<PW->max_n; i++)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(PW->en_min, PW->en_max, PW->val_min, PW->val_max);

        glLineWidth(1.0);

        glColor3f(1,0,0);
        if(i>0 && (Gauss(PW->en[i],G,PW)!=0 || Gauss(PW->en[i-1],G,PW)!=0)){
        glBegin(GL_LINE_STRIP);
            glVertex2f(PW->EN2PIXEL(PW->en[i-1],PW),PW->VAL2PIXEL(abs(Gauss(PW->en[i-1],G,PW)),PW));
            glVertex2f(PW->EN2PIXEL(PW->en[i],PW),PW->VAL2PIXEL(abs(Gauss(PW->en[i],G,PW)),PW));
        glEnd();
        }
    }
        G = G->pgn; //przesuniecie wskaznika na nastepny element w liscie
    }
}

void RYSUJ_GAUSS(WIDMO *PW, dlistVar *L){
    _GAUSS *G;
    G = L[PW->numerw].head; //ustawienie wskaznika na poczatku listy
    while(G){ // = while(G!=NULL)
    for(int i = 0; i<PW->max_n; i++)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(PW->en_min, PW->en_max, PW->val_min, PW->val_max);

        glLineWidth(1.0);

        glColor3f(1,0,0);
        if(i>0 && (Gauss(PW->en[i],G,PW)!=0 || Gauss(PW->en[i-1],G,PW)!=0)){
        glBegin(GL_LINE_STRIP);
            glVertex2f(PW->EN2PIXEL(PW->en[i-1],PW),PW->VAL2PIXEL(abs(Gauss(PW->en[i-1],G,PW)),PW));
            glVertex2f(PW->EN2PIXEL(PW->en[i],PW),PW->VAL2PIXEL(abs(Gauss(PW->en[i],G,PW)),PW));
        glEnd();
        }
    }
        G = G->pgn; //przesuniecie wskaznika na nastepny element w liscie
    }
}

void config(_GAUSS *G){
    string path;
    fstream file;
    string sigma;
    string iteracje;
    string V0;
    stringstream ss1, ss2, ss3;
    double s,i,v;
    char result[ MAX_PATH ];

    path = string( result, GetModuleFileName( NULL, result, MAX_PATH ) ); //zapis do zmiennej sciezki
    string filename = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filename.length(); i++)
        path.pop_back();

    path += "\\config.txt";

    file.open(path, ios::in);

    if(file.good()){
        getline(file, sigma);
        sigma.erase(sigma.begin(),sigma.end()-(sigma.length()-7));
        getline(file, iteracje);
        iteracje.erase(iteracje.begin(),iteracje.end()-(iteracje.length()-10));
        getline(file, V0);
        V0.erase(V0.begin(),V0.end()-(V0.length()-4));
    }
    else return;

    file.close();

    ss1 << sigma;
    ss1 >> s;

    ss2 << iteracje;
    ss2 >> i;

    ss3 << V0;
    ss3 >> v;

    G->sigma = s;
    ilosc_z = i;
    G->V0 = v;
}

void zapis(_GAUSS *G, dlistVar &L){
    string path;
    char result[ MAX_PATH ];
    string numer;
    stringstream ss;

    ss << PW->numerw;
    numer = ss.str();

    path = string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
    string filen = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filen.length(); i++)
        path.pop_back();

    nazwa_widma();
    string widmo(bufferN);

    string filename = widmo.substr(widmo.find_last_of("\\") + 1);
    size_t lastindex = filename.find_last_of(".");
    filename = filename.substr(0, lastindex);

    path += "\\gaussy\\";
    path += filename;
    path += "_g[";
    path += numer;
    path += "]";
    path += ".txt";

    ofstream zapis(path);

    G = L.head; //ustawienie wskaznika na poczatku listy
    zapis << "A\tE0\tsigma\tV0\n";
    while(G){
        zapis << G->A << "\t";
        zapis << G->mu << "\t";
        zapis << G->sigma << "\t";
        zapis << G->V0 << "\n";
        G = G->pgn; //przesuniecie wskaznika na nastepny element w liscie
    };

    zapis.close();
}

void zapis(_GAUSS *G, dlistVar *L){
    string path;
    char result[ MAX_PATH ];
    string numer;
    stringstream ss;
    stringstream ss2;

    ss << PW->numerw;
    numer = ss.str();

    path = string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
    string filen = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filen.length(); i++)
        path.pop_back();

    nazwa_widma();
    string widmo(bufferN);

    string filename = widmo.substr(widmo.find_last_of("\\") + 1);
    size_t lastindex = filename.find_last_of(".");
    filename = filename.substr(0, lastindex);

    path += "\\gaussy\\";
    path += filename;
    path += "_g[";
    path += numer;
    path += "]";
    path += ".txt";
    //cout<<path<<endl;

    ofstream zapis(path);

    G = L[PW->numerw].head; //ustawienie wskaznika na poczatku listy
    zapis << "A\tE0\tsigma\tV0\n";
    while(G){
        zapis << G->A << "\t";
        zapis << G->mu << "\t";
        zapis << G->sigma << "\t";
        zapis << G->V0 << "\n";
        G = G->pgn; //przesuniecie wskaznika na nastepny element w liscie
    };

    zapis.close();
}

void fitAll(){
    PW->czytaj_e0(PW);
    _GAUSS *g;
    l_init(L);
    l_init(L2[PW->numerw]);
    g = new _GAUSS;
    config(g);
    for(int i = 0; i<PW->iloscg; i++){
        g->mu = PW->e0[i];
        dopasuj(PW,g);
        l_push_back(L, g->A, g->mu, g->sigma);
        l_push_back(L2[PW->numerw], g->A, g->mu, g->sigma);
        //cout<<g->mu<<":"<<g->A<<endl;
        }
    zapis(g,L2);

}

void RYSUJ_MENU(int liczba_segmentow){
        glColor3f(0.0f, 0.0f, 0.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, w, 0, h);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glLineWidth(2);

        for(int i = 0; i<liczba_segmentow; i++){
        glBegin(GL_QUADS);
            glColor3f(0.9, 0.9, 0.9);
            glVertex2f(w, h - i*30);
            glVertex2f(w, h - (i*30+30));
            glVertex2f(w - 115, h - (i*30+30));
            glVertex2f(w - 115, h - i*30);
        glEnd();

        glBegin(GL_LINE_LOOP);
            glColor3f(0.7, 0.7, 0.7);
            glVertex2f(w-1, h - i*30 - 1);
            glVertex2f(w-1, h - (i*30+30));
            glVertex2f(w - 115, h - (i*30+30));
            glVertex2f(w - 115, h - i*30 - 1);
        glEnd();
        }

        glBegin(GL_LINE_STRIP);
            glColor3f(0.7, 0.7, 0.7);
            glVertex2f(w - 57, h - 120);
            glVertex2f(w - 57, h - 150);
        glEnd();

        glColor3f(0, 0, 0);

        itoa(PW->numerw,bufferW,10);
        drawText("Wykres numer:",w-112,h-12,0);
        drawText(bufferW,w-29,h-12,0);

        drawText("x:",w-112,h-24,0);
        drawText(bufferX,w-100,h-24,0);
        drawText("y:",w-55,h-24,0);
        drawText(bufferY,w-43,h-24,0);

        drawText("DOPASUJ",w-85,h-49,0);
        drawText("USTAWIENIA",w-92,h-79,0);
        drawText("USTAW E0",w-87,h-109,0);
        drawText("<",w-90,h-139,0);
        drawText(">",w-31,h-139,0);
        drawText("DODAJ WIDMO",w-102,h-169,0);
        drawText("USUN WIDMO",w-96,h-199,0);
        drawText("POMOC",w-80,h-229,0);

        nazwa_widma();

        if(showWPATH)
            drawText(bufferN,0,h-12,0);
}

void RYSUJ_LINIE(double grubosc){
    glLineWidth(grubosc);
        for(int a = 0; a<10; a++){
            if(linie[a].visible){
                if(a+1==l)
                glColor3f(0, 1, 0);
                else
                glColor3f(0, 0, 1);
            linie[a].drawLine(&linie[a]);
            }
        }
}

void zmien_widmo(int o_ile){
        l_init(L); //czyszczenie listy gaussow
        numerw += o_ile;
        if(numerw>PW->iloscw) numerw = 1;
        if(numerw<1) numerw = PW->iloscw;
        PW = PW->czytaj_widmo(numerw);
        PW->czytaj_e0(PW);
}

void kasuj_wiersz( const string sciezka, const size_t wiersz){
    vector < string > vec;
    ifstream in( sciezka.c_str() );
    string tmp;
    while( getline( in, tmp ) ) vec.push_back( tmp );

    in.close();
    ofstream out( sciezka.c_str() );
    for( size_t i = 0; i < vec.size(); ++i )
        if( i + 1 != wiersz ) out << vec[ i ] << endl;

    out.close();
}

void nazwa_widma(){
    string path;
    fstream file;
    path=name;
    string filen = path.substr(path.find_last_of("\\") + 1);

    for(int i=0; i<filen.length(); i++)
        path.pop_back();

    path+="\\files\\files.txt";
    char result[path.size()+1];
    strcpy(result, path.c_str());

    file.open(result, ios::in);

    for(int i = 0; i<PW->numerw; i++)
        getline(file, path);

    file.close();

    strcpy(bufferN, path.c_str());
}

void wyswietl_pomoc(){
    if(showHELP){
        glBegin(GL_QUADS);
            glColor3f(0.9, 0.9, 0.9);
            glVertex2f(w-116, h);
            glVertex2f(w-116, h-240);
            glVertex2f(w - 300, h - 240);
            glVertex2f(w - 300, h);
        glEnd();

        glBegin(GL_LINE_LOOP);
            glColor3f(0.7, 0.7, 0.7);
            glVertex2f(w-116, h - 1);
            glVertex2f(w-116, h-240);
            glVertex2f(w - 300, h - 240);
            glVertex2f(w - 300, h - 1);
        glEnd();

        glBegin(GL_LINE_STRIP);
            glColor3f(0.7, 0.7, 0.7);
            glVertex2f(w - 116, h - 210);
            glVertex2f(w - 300, h - 210);
        glEnd();

        glBegin(GL_LINE_STRIP);
            glColor3f(0.7, 0.7, 0.7);
            glVertex2f(w - 207, h - 210);
            glVertex2f(w - 207, h - 240);
        glEnd();

        glColor3f(0.0, 0.0, 0.0);
        drawText("<",w-257,h-229,0);
        drawText(">",w-165,h-229,0);

        if(HELP_PAGE == 1){
            glColor3f(0.0, 0.6, 0.0);
            drawText("KLAWISZE:",w-299,h-12,0);
            glColor3f(0.0, 0.0, 0.0);
            drawText("w,s,a,d - przesuwanie wykresu",w-299,h-26,0);
            drawText("widma (gora, dol, lewo, prawo);",w-299,h-38,0);

            drawText("q - przywrocenie domyslnego",w-299,h-52,0);
            drawText("widoku wykresu widma;",w-299,h-64,0);

            drawText("-,+ - zmniejszenie, powieksze-",w-299,h-78,0);
            drawText("nie wykresu widma;",w-299,h-90,0);

            drawText("e,r - wczytanie poprzedniego/",w-299,h-104,0);
            drawText("nastepnego wykresu widma;",w-299,h-116,0);

            drawText("f - dopasowanie gaussow zgo-",w-299,h-130,0);
            drawText("dnie z zadanymi parametrami;",w-299,h-142,0);

            drawText("z - pokazuje/ ukrywa sciezke",w-299,h-156,0);
            drawText("aktualnie wyswietlanego widma;",w-299,h-168,0);

            drawText("space/backspace - postawienie",w-299,h-182,0);
            drawText("lub zaznaczenie/usuniecie zna-",w-299,h-194,0);
            drawText("cznika",w-299,h-206,0);
        }

        if(HELP_PAGE == 2){
            glColor3f(0.0, 0.6, 0.0);
            drawText("PRZYCISKI:",w-299,h-12,0);
            glColor3f(0.0, 0.0, 0.0);
            drawText("DOPASUJ - dopasowanie gau-",w-299,h-26,0);
            drawText("ssow zgodnie z zadanymi para-",w-299,h-38,0);
            drawText("metrami;",w-299,h-50,0);

            drawText("USTAWIENIA - otwiera ustawie-",w-299,h-64,0);
            drawText("nia stalych dla programu para-",w-299,h-76,0);
            drawText("metrow;",w-299,h-88,0);

            drawText("USTAW E0 - otwiera plik z prze-",w-299,h-102,0);
            drawText("widywanymi wartosciami e0 do",w-299,h-114,0);
            drawText("ktorych dopasowywane sa gau-",w-299,h-126,0);
            drawText("ssy;",w-299,h-138,0);

            drawText("<,> - wczytanie poprzedniego",w-299,h-152,0);
            drawText("/nastepnego wykresu widma",w-299,h-164,0);

            drawText("DODAJ/USUN WIDMO - dodaje",w-299,h-178,0);
            drawText("/usuwa z pliku files.txt kolejna",w-299,h-190,0);
            drawText("/aktualna sciezke;",w-299,h-202,0);
        }

        if(HELP_PAGE == 3){
            glColor3f(0.0, 0.6, 0.0);
            drawText("PLIKI:",w-299,h-12,0);
            glColor3f(0.0, 0.0, 0.0);
            drawText("files.txt - plik z bezwzglednymi",w-299,h-26,0);
            drawText("sciezkami do plikow tekstowych",w-299,h-38,0);
            drawText("z ktorych rysowane sa widma;",w-299,h-50,0);

            drawText("config.txt - ustawienie stalych",w-299,h-64,0);
            drawText("dla programu parametrow;",w-299,h-78,0);

            drawText("e0.txt - plik z przewidywanymi",w-299,h-92,0);
            drawText("wartosciami e0 do ktorych do-",w-299,h-104,0);
            drawText("pasowywane sa gaussy;",w-299,h-116,0);

            drawText("nazwa pliku_g[numer widma].txt",w-299,h-130,0);
            drawText("- plik z dopasowanymi parame-",w-299,h-142,0);
            drawText("trami dla zadanych gaussow w",w-299,h-154,0);
            drawText("e0;",w-299,h-166,0);
        }
    }
}
