/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: mainwindow.cpp
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


#include "mainwindow.hh"
#include "gameboard.hh"
#include "ui_mainwindow.h"

// Suurentaa skaalausta hieman pelattavuuden vuoksi.
const double PRINT_MULTIPLIER = 1.2;

// Luodaan oletusarvoilla GameBoard olion field
GameBoard field(DEFAULT_WIDTH,DEFAULT_HEIGTH,DEFAULT_SEED);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow), command_("a")
{
    ui_->setupUi(this);
    this->setStyleSheet("background-color: lightGray;");

    // QGraphicsScene tarvitaan madon piirtämistä varten.
    scene_ = new QGraphicsScene(this);

    // GraphicsView sijoitetaan näihin koordinaatteihin.
    int left_margin = 60; // x koordinaatti
    int top_margin = 270; // y koordinaatti

    // GraphicsViewin leveys on DEFAULT_WIDTH*STEP lisättynä 2:lla,
    // koska rajat vievät yhden pikselin kummaltakin puolelta.
    // Vastaavasti GraphicsViewin korkeus on DEFAULT_HEIGTH*STEP lisättynä kahdella.
    ui_->graphicsView->setGeometry(left_margin, top_margin,
                                   DEFAULT_WIDTH*STEP + 2, DEFAULT_HEIGTH*STEP + 2);

    ui_->graphicsView->setScene(scene_);

    // Scene_:n leveys on DEFAULT_WIDTH*STEP vähennetään 1:llä ja
    // sen korkeus on DEFAULT_HEIGTH*STEP vähennettynä 1:llä.
    scene_->setSceneRect(0, 0, DEFAULT_WIDTH*STEP - 1, DEFAULT_HEIGTH*STEP - 1);

    // Taustavärin asettaminen mustaksi.
    scene_->setBackgroundBrush(QColor(60,60,60));


    // Non-singleshot -ajastin laukeaa valitun intervallin
    // DURATION (millisekuntia) välein, mikä pyörittää
    // koko peliä main_loopin avulla.
    timer_.setSingleShot(false);
    connect(&timer_, &QTimer::timeout, this, &MainWindow::main_loop);
    // Pyörittää ajastinta.
    connect(&timer_, &QTimer::timeout, this, &MainWindow::update_time);


    // Yhdistetään napit ja slotit.
    connect(ui_->pause_button,&QAbstractButton::clicked, this,&MainWindow::pause_button_pressed);
    connect(ui_->reset_button,&QAbstractButton::clicked, this,&MainWindow::end_game_slot);

    // Asetetaan labelit tyhjäksi.
    ui_->points_label->setText("0");
    ui_->time_label->setText("00:00");

    /*
    Taustamusiikki (toimii, ei käytössä):

    player = new QMediaPlayer;
    audioOutput = new QAudioOutput;
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(50);

    player->setLoops(QMediaPlayer::Infinite);       // Loputon musiikki.
    QUrl url = QUrl(QUrl::fromLocalFile("../snake_gui/Resources/game_music.mp3"));
    player->setSource(url);

    player->play();
    */

    // Luodaan popup-ikkuna, eli Dialog-olio
    dialog = new Dialog(this);
    dialog->setModal(true);

    // Yhdistetään slotit.
    connect(dialog,&Dialog::send_user_info, this, &MainWindow::receive_user_info_main);
    connect(this,&MainWindow::send_score_main, dialog, &Dialog::receive_user_info);

    // Avataan ikkuna.
    dialog->show();
}

// Purkaja.
MainWindow::~MainWindow()
{
    delete ui_;
}

// Käsittelee näppäimen painamiset ja asetttaa commandiksi
// päivitetyn liikekomennon. Varmistaa, ettei mato voi kääntyä
// välittömästi tulosuuntaan. Toimii ainoastaan pelin ollessa
// käynnissä.
void MainWindow::keyPressEvent(QKeyEvent* event) {

    if(ui_->pause_button->text() == "||"){

        // Alas.
        if(event->key() == Qt::Key_S) {
            if(command_ != "w"){
                command_ = "s";
            }
            return;
        }
        // Ylös.
        if(event->key() == Qt::Key_W) {
            if(command_ != "s"){
               command_ = "w";
            }
            return;
        }
        // Vasemmalle.
        if(event->key() == Qt::Key_A) {
            if(command_ != "d"){
                command_ = "a";
            }
            return;
        }
        // Oikealle.
        if(event->key() == Qt::Key_D) {
            if(command_ != "a"){
                command_ = "d";
            }
            return;
        }
    }
}

// Käsittelee popupista (dialogista) tulevan uuden siemenluvun arvon.
// Mikäli se ylittää makismi-seedin sulkee kummatkin kkunat,
// muussa tapauksessa asettaa alkutiedot ja kutsuu start_game().
void MainWindow::receive_user_info_main(int seed)
{
    if(seed == SEED_MAX+1){
        dialog->close();
        this->close();
    }

    else{
        field.set_seed(seed);
        start_game();
    }

}

// Slot kutsumaan pelin loppua ja resetointia.
void MainWindow::end_game_slot()
{
    end_game();
}

// Piirtää madon osat uusiin paikkoihin
// snake_coordinates avulla.
void MainWindow::snake_part_move()
{
    if(not field.gameOver()) {
        int num = 0;

        // Käy läpi käänteisessä suunnassa koordinaatit ja piirtää
        // niitä vastaavat palaset näkyviin.
        for (auto point = field.snake_coordinates().rbegin();
             point != field.snake_coordinates().rend(); ++point){

            snake_parts_.at(num)->setPos(point->return_x()*STEP,point->return_y()*STEP);

            // Palaset näkyviin kun ne on asetettu oikeaan paikkaan.
            snake_parts_.at(num)->setOpacity(1.0);
            num++;
        }
    }
}

// Slot pause-napin käsittelyyn. Vaihtaa napin tekstiä
// ja pysäyttää pelia ajavan ajastimen.
void MainWindow::pause_button_pressed()
{
    if(ui_->pause_button->text() == "||"){
        timer_.stop();
        // Osa taustamusiikki ominaisuutta:
        // player->stop();
        ui_->pause_button->setText("▶");
    }

    else{
        timer_.start();

        // Osa taustamusiikki ominaisuutta:
        // player->play();
        ui_->pause_button->setText("||");
    }

}


// Aloittaa pelin avaamalla ikkunan, kasvattmalla matoa
// ja lisäämällä ensimäisen ruoan.
void MainWindow::start_game()
{

    ui_->pause_button->setText("||");
    grow_snake();

    QPixmap food_image(":/Resources/food.png");
    food_image = food_image.scaled(STEP*PRINT_MULTIPLIER,STEP*PRINT_MULTIPLIER);

    food = scene_->addPixmap(food_image);
    food->setPos(field.food_coordinates().return_x()*STEP,
                 field.food_coordinates().return_y()*STEP);


    timer_.start(DURATION);

}

// Kääntää madon viimeistä osaa madon asennosta riippuen,
// jotta mato näyttäisi järkevältä. Tarkistaa onko
// häntä jo käännetty, jotta sitä ei turhaan käännetä.
void MainWindow::rotate_tail()
{

    if(field.snake_coordinates().size() > 1){

        // Viimeinen koordinaatti.
        auto tail_cord = field.snake_coordinates().at(0);

        // Toisiksi viimeinen koordinaatti.
        auto s_last_cord = field.snake_coordinates().at(1);


        auto tail_image = snake_parts_.back();
        qreal angle = tail_image->rotation();

        if(tail_cord.return_x() < s_last_cord.return_x() and
                angle != 90){
            tail_image->setRotation(90);
        }

        if(tail_cord.return_x() > s_last_cord.return_x() and
                angle != -90){
            tail_image->setRotation(-90);
        }

        if(tail_cord.return_y() > s_last_cord.return_y() and
                angle != 0){
            tail_image->setRotation(0);
        }

        if(tail_cord.return_y() < s_last_cord.return_y() and
                angle != -180){
            tail_image->setRotation(-180);
        }
    }
}


// Lopettaa pelin, eli avaa popupin ja poistaa
// edellisen pelin kaikki tiedot.
void MainWindow::end_game()
{
    ui_->pause_button->setText("▶");

    timer_.stop();
    time_ = 0;

    //player->stop(); (Osa taustamusiikki ominaisuutta)

    // Avaa dialog-ikkunan.
    dialog->show();

    // Lähettää tulokset dialogiin.
    if(snake_parts_.size() >= 1){
        emit send_score_main(snake_parts_.size()-1, field.gameWon());
    }

    // Poistaa QGraphicsItemit
    for(auto part:snake_parts_){
        delete part;
    }
    delete food;

    // Tyhjää ja resetoi madon osat.
    snake_parts_.clear();
    field.reset_snake();

}


// Päivittää ajastimen muododssa mm:ss. Lisää tarvittaessa
// nollan luvun eteen.
void MainWindow::update_time()
{
    time_++;
    int minutes = 0;

    if(time_ > 600){
         minutes = time_ / 600;
    }
    std::string minutes_s = std::to_string(minutes);

    if(minutes_s.size() == 1){
        minutes_s = "0" + minutes_s;
    }
    std::string seconds_s = std::to_string((time_/10) % 60);

    if(seconds_s.size() == 1){
        seconds_s = "0" + seconds_s;
    }

    QString time = QString::fromStdString(minutes_s + ":" + seconds_s);
    ui_->time_label->setText(time);

}

// Käntää madon pään suuntaa. Kutsutaan main_loopissa
// ajastimen tahtiin, jotta käyttäjä ei ehdi ajastimen
// intervallin aikana kääntää matoa itseensä kohti.
void MainWindow::rotate_head()
{
    if(command_ == "s"){
        snake_parts_.front()->setRotation(180);
        return;
    }

    if(command_ == "w"){
        snake_parts_.front()->setRotation(0);
        return;
    }

    if(command_ == "a"){
        snake_parts_.front()->setRotation(-90);
        return;
    }

    if(command_ == "d"){
        snake_parts_.front()->setRotation(90);
        return;
    }

}

// Pää-looppi, joka toteuttaa ajastimen tahtiin
// pelin pyörittämiseksi tarvittavat asiat.
void MainWindow::main_loop()
{

    if(not field.gameOver()){

        // Muuttaa GameBoardin koordinaatteja.
        field.moveSnake(command_);
        // Säätää liikesuunnan.
        rotate_head();

        // Asettaa ruoan.
        food->setPos(field.food_coordinates().return_x()*STEP,
                     field.food_coordinates().return_y()*STEP);

        // Kääntää häntää.
        rotate_tail();

        // Mikäli käärmeen koordinaatit ovat lisääntyneet
        // kasvattaa matoa.
        if(snake_parts_.size() != field.snake_coordinates().size()){
            grow_snake();
        }

        // Asettaa palaset uusille paikoille.
        snake_part_move();

        ui_->points_label->setText(QString::fromStdString(std::to_string(snake_parts_.size() - 1)));

    }

    else{
        // Lopettaa pelin.
        end_game();
    }

}


// Lisää piirrettävään käärmeeseen pituutta, eli lisää
// snake_parts vektoriin sopivan kuvan.
void MainWindow::grow_snake()
{
    int snake_lenght = snake_parts_.size();

    if(snake_lenght == 0){
        QPixmap snake_head_image(":/Resources/head.png");
        snake_head_image = snake_head_image.scaled
                (STEP*PRINT_MULTIPLIER,STEP*PRINT_MULTIPLIER);

        rectangle = scene_->addPixmap(snake_head_image);
    }

    else if(snake_lenght == 1){
        QPixmap snake_tail_image(":/Resources/tail.png");
        snake_tail_image = snake_tail_image.scaled
                (STEP*PRINT_MULTIPLIER,STEP*PRINT_MULTIPLIER);

        rectangle = scene_->addPixmap(snake_tail_image);
    }

    else{
        QPixmap snake_body_image(":/Resources/body.png");
        snake_body_image = snake_body_image.scaled
                (STEP*PRINT_MULTIPLIER,STEP*PRINT_MULTIPLIER);

        rectangle = scene_->addPixmap(snake_body_image);

        // Asettaa palasen läpinäkyväksi, jotta se
        // se voidaan siirtää oikeaan paikkaan ja vasta siellä
        // tehdä näkyväksi.
        rectangle->setOpacity(0.0);
        snake_parts_.insert(snake_parts_.end()-1,rectangle);

        return;

    }

    // Asettaa palasen läpinäkyväksi, jotta se
    // se voidaan siirtää oikeaan paikkaan ja vasta siellä
    // tehdä näkyväksi.
    rectangle->setOpacity(0.0);

    // Asettaa "origon" keskelle palasta
    rectangle->setTransformOriginPoint(rectangle->boundingRect().center());
    snake_parts_.push_back(rectangle);

}
