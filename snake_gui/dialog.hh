/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: dialog.hh
 *
 * Avaa toisen ikkunan, josta käyttäjä voi muuttaa
 * pelin asetuksia, aloittaa/ lopettaa ohelman
 * ja tarkastella menneiden pelien tuloksia
 * lederboradista.
 *
 * Ohjelman kirjoittaja:
 * - Nimi: Otso Tolonen
 * - Opiskelijanumero: removed
 * - Käyttäjätunnus: 0tso1
 * - S-posti: otso.tolonen@tuni.fi
 *
 */



#ifndef DIALOG_HH
#define DIALOG_HH


#include <QDialog>
#include <QDebug>
#include <vector>
#include <map>
#include <fstream>
#include <QString>

// Vakioita asetuksia varten.
const int SEED_MIN = -10000000;
const int SEED_MAX = 10000000;
const QString DEFAULT_NAME = "Pelaaja";

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

public slots:
    void receive_user_info(int user_score, bool game_won);

private slots:
    void ok_button_pressed();
    void exit_button_pressed();

private:
    Ui::Dialog *ui;

    // Hakee tulokset tiedostosta.
    std::multimap<int, std::string> get_results();

    // Kirjoittaa tulokset multimapista tiedostoon.
    void write_results(std::multimap<int, std::string> &results);

    // Tulostaa tulokset multimapista text_boxiin.
    void print_results(std::multimap<int, std::string> &results);

    // Halkaisee stringin delim-charin perusteella. Paluttaa osaset
    // vektorissa.
    std::vector<std::string> split(const std::string &str, char delim = ';');

signals:

    // Lähettää käyttäjän syöttämät infot.
    void send_user_info(int seed);


};

#endif // DIALOG_HH
