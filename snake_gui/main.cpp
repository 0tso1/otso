/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: main.cpp
 *
 * Ajaa ohjelman.
 *
 * Ohjelman kirjoittaja:
 * - Nimi: Otso Tolonen
 * - Opiskelijanumero: removed
 * - Käyttäjätunnus: 0tso1
 * - S-posti: otso.tolonen@tuni.fi
 *
 */

#include "mainwindow.hh"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    return a.exec();
}
