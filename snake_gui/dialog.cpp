/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: dialog.cpp
 *
 * Avaa toisen ikkunan, josta käyttäjä voi muuttaa
 * pelin asetuksia, aloittaa/ lopettaa ohjelman
 * ja tarkastella menneiden pelien tuloksia
 * lederboradista. Alhaalle vasemmalle tulee myös
 * pelin loputtua ilmoitus voitosta/ tappiosta.
 *
 * Ohjelman kirjoittaja:
 * - Nimi: Otso Tolonen
 * - Opiskelijanumero: removed
 * - Käyttäjätunnus: 0tso1
 * - S-posti: otso.tolonen@tuni.fi
 *
 */


#include "dialog.hh"
#include "ui_dialog.h"
#include "gameboard.hh"

#include <QDebug>
#include <iostream>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Asetaa logon näkyviin.
    QPixmap image_to_label(":/Resources/logo.png");
    ui->image_label->setPixmap(image_to_label);

    // Asetaa raja-arvot ja oletusarvot.
    ui->seed_spin_box->setValue(DEFAULT_SEED);
    ui->seed_spin_box->setMinimum(SEED_MIN);
    ui->seed_spin_box->setMaximum(SEED_MAX);
    ui->username_line_edit->setText(DEFAULT_NAME);
    ui->game_over_label->setText("");

    // Yhdistää napit ja slotit.
    connect(ui->ok_button, &QAbstractButton::clicked, this, &Dialog::ok_button_pressed);
    connect(ui->exit_button, &QAbstractButton::clicked,this,&Dialog::exit_button_pressed);


    // Hakee ja tulostaa pelitulokset.
    std::multimap<int, std::string> results = get_results();
    print_results(results);


}

// Purkaja.
Dialog::~Dialog()
{
    delete ui;
}

// Slot vastaanottamaan ja käsittelemään tiedot mainwindowista.
// Lisää tuloksen tiedostoon.
void Dialog::receive_user_info(int user_score, bool game_won)
{
    std::string username = ui->username_line_edit->text().toStdString();

    std::multimap<int, std::string> results = get_results();

    results.insert({user_score, username});

    write_results(results);
    print_results(results);


    if(game_won){
        ui->game_over_label->setText("Voitit :)");
    }

    else{
        ui->game_over_label->setText("Hävisit :(");
    }

}

// Slot ok-napin käsittelyyn. Lähettää
// pelaajan tiedot mainwindowiin.
void Dialog::ok_button_pressed()
{

    int seed = ui->seed_spin_box->value();
    std::string name = ui->username_line_edit->text().toStdString();

    emit send_user_info(seed);

    this->close();

}

// Lähettää makismi siemenenarvon ylittävän
// signaalin, joka käsitellään ohjelman
// sulkevana käskynä mainwindowissa lähetetyn
// pointerin avulla.
void Dialog::exit_button_pressed()
{
    emit send_user_info(SEED_MAX+1);

}

// Palauttaa multimapin tiedostusta results.txt
// luetuista pelituloksista kasatun multimapin.
std::multimap<int, std::string> Dialog::get_results()
{

    std::multimap<int, std::string> results;


    std::ifstream file;
    file.open("../snake_gui/Resources/results.txt");
    std::string row;

    while(getline(file, row)){
        std::vector<std::string> split_vector;
        split_vector = split(row, ':');

        if(split_vector.size() >= 2){
            results.insert({stoi(split_vector.at(0)), split_vector.at(1)});
        }

        split_vector.clear();
    }

    file.close();

    return results;
}

// Kirjoittaa multimapin sisällön tiedostoon results.txt.
void Dialog::write_results(std::multimap<int, std::string> &results)
{

    std::ofstream file;
    file.open("../snake_gui/Resources/results.txt", std::ios::trunc);

    for(auto const &result:results){
        file << result.first << ':' << result.second << std::endl;
    }

    file.close();


}

// Tulostaa multimapin sisällön textBrowser elementtiin.
void Dialog::print_results(std::multimap<int, std::string> &results)
{
    ui->text_box->clear();

    // Tulostaa pelitulokset aloittaen isoimmasta tuloksesta (avaimesta).
    for (auto result = results.rbegin(); result != results.rend(); result++) {
        std::string line = std::to_string(result->first) + " : " + result->second;
        ui->text_box->append(QString::fromStdString(line));
    }
}

// Stringin pilkkova funktio, palauttaa osaset vektorina.
// Jos delim-merkki on "':n välissä, se jätetään huomiotta.
std::vector<std::string> Dialog::split(const std::string &str, char delim)
{

    std::vector<std::string> result = {""};
    bool inside_quotation = false;

    for (char current_char : str) {

        if (current_char == '"') {
          inside_quotation = not inside_quotation;
        }

        else if (current_char == delim and not inside_quotation) {
          result.push_back("");
        }

        else {
          result.back().push_back(current_char);
        }
    }
    if (result.back() == "") {
        result.pop_back();
    }

    return result;


}



