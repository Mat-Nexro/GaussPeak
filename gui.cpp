#include <gtk/gtk.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <process.h>

using namespace std;

GtkWidget *etykieta;

int main(int argc, char *argv[])
{
    GtkWidget *okno;
    string name = argv[0];

    gtk_init(&argc, &argv);

    etykieta = gtk_label_new("Tekst");

    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Otwórz...", GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL)), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), "Wybierz widmo");

    char *nazwa_pliku;
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        nazwa_pliku = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        gtk_label_set_text(GTK_LABEL(etykieta), nazwa_pliku);
    }

    //cout<<nazwa_pliku<<endl;
    for(int i=0; i<7; i++)
        name.pop_back();

    name += "files\\files.txt";

    fstream zapis;
    zapis.open(name, ios::app | ios::out | ios::in);

    if(nazwa_pliku)
        zapis << nazwa_pliku << "\n";
    zapis.close();

    gtk_widget_destroy (dialog);

    //gtk_main();

    return 0;
}


