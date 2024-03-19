/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: mainwindow.hh
 *
 * Ohjelma toteuttaa graafisen käyttöliittymän matopeliin.
 *
 * Ohjelman ajaessa avautuu ikkuna (Dialog luokka), johon käyttäjä
 * voi syöttää haluamansa siemenluvun ja käyttäjätunnuksen. Nämä asetettuaan
 * pelaaja siirtyy varsinaiseen peliin Ok-napilla. Pelissä tulee kasvattaa matoa
 * syömällä satunnaisiin kohtiin ilmestyviä ruokia, samalla kuitenkin
 * välttäen osumasta seiniin tai omaan häntään. Matoa ohjataan WASD-näppäimin.
 * Pelaaja voi hetkenä minä hyvänsä pysäyttää pelin pause-painikkeella tai
 * resetoida pelin reset-painikkeella.
 *
 * Pelin päätyttyä avautuu taas ikkuna (Dialog-olio), joka sijoittaa äskettäin
 * saadun tuloksen leaderboardiin ja kertoo päättyikö peli voittoon vai häviöön
 * (resetoitu peli lasketaan häviöksi). Tästä ikkunasta pelaaja voi pelata uudestaan
 * tai päättää ohjelman poistu-painikkeella. Kuvailtu leaderboard tallentuu
 * tiedostoon results.txt, joten se säilyy pelikertojen välillä.
 *
 * Peli toimii GameBoard olion kautta, niin että vakio STEP määrittää yhden GameBoard
 * koordinaatin sivun mitan pikseleissä. Pelin mekaniikka perustuu ajastimeen, jonka
 * intervalein snake_parts palaset sijoitetaan pelialaudalle iteroimalle niille sijainnit
 * GameBoard oliosta. GameBoard ja Point luokkiin lisätyt metodit hakevat tarvittavat
 * koordinaatit tätä varten.
 *
 * Ominaisuudet:
 * - Taustamusiikki pelin pyöriessä (ei käytössä, jätetty kommeteiksi).
 *      - Toimii QMediaPlayerin avulla. Valittua 8s pituista tekijäin-
 *        oikeuksista vapaata kappaletta toistaan aina pelin pyöriessä.
 *
 * - Tulostaulu (Dialogin leaderboard).
 *      - Pelin alussa syötetty käyttäjänimi lähetetään mainwindowiin
 *        säilöön ja pelin päättyessä se lähetetään tuloksen kera
 *        takaisin dialogiin, jossa se tulostetaan ja kirjoitetaan
 *        tiedostoon.
 *
 * - Kuvien/ ikonien käyttäminen pelissä.
 *      - Mato koostuu tekemistäni png-kuvista, joiden QGraphicsItemeihin
 *        viittaavia pointereita säilytetään snake_parts vektorissa.
 *
 * - Pistelaskuri
 *      - Pelatessa pistetilanne päivittyy aina pelikentän yläreunan
 *        pistelaatikkoon. Lopulliset pisteet talleteaan tiedostoon
 *        ja esitetään leaderboardissa. Pisteiden määrä on madon osien
 *        määrä - 1, eli päätä ei lasketa mukaan.
 *
 * - Ajastin.
 *      - Kertoo pelin alusta kuluneen ajan muodossa minuutit:sekuntit.
 *        Ajastin pysähtyy pelin pysähtyessä.
 *
 * - Pause-nappula.
 *      - Mahdollistaa pelin tauottamisen. Kaikki toiminta lakkaa
 *        täksi ajaksi, madon pää ei enää käänny.
 *
 * - Erillinen ikkuna asetusten muuttamiseen ja tulosten esittämiseen.
 *      - Dialog-olio toimii pelin kakkosikkunana. Se avautuu pelin alussa
 *        ja lopussa esittäen tulokset.
 *
 * Ohjelman kirjoittaja:
 * - Nimi: Otso Tolonen
 * - Opiskelijanumero: removed
 * - Käyttäjätunnus: 0tso1
 * - S-posti: otso.tolonen@tuni.fi
 *
 */



#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "dialog.hh"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QPixmap>
#include <fstream>
#include <iostream>
#include <QGraphicsItem>
// #include QMediaPlayer (Osa taustamusiikki ominaisuutta)

// Määrittää liikken laajuuden ja GameBoard koordinaatin mitan pikselissä.
const int STEP = 10;
// Kuinka monta millisenkuntia per intervalli.
const int DURATION = 100;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent* event) override;

public slots:

    // Vastaanottaa dialogista tulevat tiedot.
    void receive_user_info_main(int seed);

private slots:
    // Kutsuu end_game()
    void end_game_slot();

    // Pause-napin toiminta.
    void pause_button_pressed();

    // Päälooppi, joka pyörittää peliä.
    void main_loop();

private:
    Ui::MainWindow *ui_;
    // Pinta grafiikkalle
    QGraphicsScene* scene_;

    // Madon graafinen osanen
    QGraphicsItem* rectangle;

    // Ruoka grafiikkana
    QGraphicsItem* food;

    // Jatkuvaa liikettä varten ajastin.
    QTimer timer_;

    // Kasvattaa matoa.
    void grow_snake();

    // Valmistelee madon ja aloittaa pelin.
    void start_game();

    // Kääntää häntää madon asennosta riippuen.
    void rotate_tail();

    // Lopettaa peli, resetoi madon ja ruoan.
    void end_game();

    // Liikuttaa madon graafisia osia.
    void snake_part_move();

    // Päivittää ajastimen.
    void update_time();

    // Kääntää madon pään.
    void rotate_head();

    // Aika millisekunteissa pelin alusta.
    int time_ = 0;

    // Madon osien pointerit vektorina.
    std::vector<QGraphicsItem*> snake_parts_ = {};

    // Liikesunnan määräävä komento (WASD).
    std::string command_;

    // Dialog olio (popup-ikkuna)
    Dialog* dialog;


signals:

    // Lähettää Dialogiin pelin tuloksista.
    void send_score_main(int score, bool game_won);

};



#endif // MAINWINDOW_HH
