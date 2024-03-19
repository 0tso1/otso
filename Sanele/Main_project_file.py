#   .----------------. .----------------. .-----------------..----------------. .----------------. .----------------.
#  | .--------------. | .--------------. | .--------------. | .--------------. | .--------------. | .--------------. |
#  | |    _______   | | |      __      | | | ____  _____  | | |  _________   | | |   _____      | | |  _________   | |
#  | |   /  ___  |  | | |     /  \     | | ||_   \|_   _| | | | |_   ___  |  | | |  |_   _|     | | | |_   ___  |  | |
#  | |  |  (__ \_|  | | |    / /\ \    | | |  |   \ | |   | | |   | |_  \_|  | | |    | |       | | |   | |_  \_|  | |
#  | |   '.___`-.   | | |   / ____ \   | | |  | |\ \| |   | | |   |  _|  _   | | |    | |   _   | | |   |  _|  _   | |
#  | |  |`\____) |  | | | _/ /    \ \_ | | | _| |_\   |_  | | |  _| |___/ |  | | |   _| |__/ |  | | |  _| |___/ |  | |
#  | |  |_______.'  | | ||____|  |____|| | ||_____|\____| | | | |_________|  | | |  |________|  | | | |_________|  | |
#  | |              | | |              | | |              | | |              | | |              | | |              | |
#  | '--------------' | '--------------' | '--------------' | '--------------' | '--------------' | '--------------' |
#   '----------------' '----------------' '----------------' '----------------' '----------------' '----------------'

"""
----------------------------------------------------------------------------------------------------------------------
COMP.CS.100 Ohjelmointi 1 - SANELE
Tekijä: Otso Tolonen
S-posti: otso.tolonen@tuni.fi
Opiskelijanumero:removed

----------------------------------------------------------------------------------------------------------------------

Tervetuloa SANELE:n pariin!
 
Kuten innokkaimmat varmaan jo alun messageboxista lukivat, kyseessä suomenkielinen WORDLE-klooni.
Idea ei itsessään ole maatajärisyttävän uniikki, mutta loppujen lopuksi toteutuksesta tuli varsin
mielekäs. Pelissä on myös kaikki WORDLE:stä löytyvät ominaisuudet liittyen kirjainten väritykseen
(lisää aiheesta myöhemmin!). 

    Pelin mekaniikka kertauksena:

    Pelin tarkoituksena (kuten insipaartiona olleen Wordlenkin) on arvata oikein 
    satunnaisesti valikoitunut viisikirjaiminen sana. Kaikkien arvausten on oltava 
    kelvollisia Suomen kielen sanoja. Paina ENTER-painiketta lähettääksesi 
    sana. Jokaisen arvauksen jälkeen laattojen väri kertoo kuinka lähelle arvauksesi 
    osui:

    Virheä: oikea kirjain oikeassa paikkaa
    
    Keltainen: oikea kirjain väärässä paikkaa

    Harmaa: kirjain ei ole sanassa

Peli toiminnallisuus perustuu <Sanakirja_luoja.py> tiedoston skriptin luomiin <Sanakirja.txt> ja
<Sanakirja1.txt> tekstitiedostoihin, joista <Sanakirja1.txt> pitää sisällään verrattain yleisiä
sanoja "arvattaviksi" sanoiksi ja <Sanakirja.txt> kaikki mahdolliset hyväksytyt arvaukset. 
<Sanakirja_luoja.py> käyttää osoitteesta <https://joukahainen.puimula.org/> löytyvää sanasto
XML-tiedostoa lähteenään.

Pelin rautalanka-mallin toimintamalli on varsin yksinkertainen:

1) <self.word_picker> valitsee satunnaisen sanan <Sanakirja1.txt> filusta. 

2) Käyttäjä yrittää arvata tätä sanaa antamalla syötteen
    <self.__input> Entry-widgetillä. 
    
3)  <self.checker> tarkistaa syötteen kunnollisuuden ja luo stringin,
        josta kirjainten värit käyvät ilmi. 
    
4) <self.letter_colorer> esittää ja väritää for-loopin avulla kirjaimet
    näytölle.

    
----------------------------------------------------------------------------------------------------------------------
"""
import random
from tkinter import *
from tkinter import messagebox


class UI:

    def __init__(self):

        self.__rand_word = None  # str, satunnaisesti valikoitunut sana
        self.__current_row = 0  # int, rivi jossa mennään
        self.__tmp_label_list = []  # list, kaikki luodut Labelit

        # Seuraava osuus luo pääikkunan ja muotoilee sen sopivaksi.
        self.__mainwindow = Tk()
        self.__mainwindow.title("SANELE")
        self.__mainwindow.geometry("630x720")
        logo = PhotoImage(file="logo.png")
        self.__mainwindow.iconphoto(False, logo)

        if self.start_screen():  # Tosi, jos käyttäjä aloittaa pelin
            self.word_giver()  # Valitsee satunnaisen sanan

            # Seuraava osuus liittyy pääikkunan sisällön (fraamit ja gridit jne.) muotoiluun.
            self.__lower_frame = Frame(self.__mainwindow)
            self.__lower_frame.pack(side=BOTTOM)

            self.__lower_frame.columnconfigure(0, weight=1)
            self.__lower_frame.columnconfigure(1, weight=1)
            self.__lower_frame.columnconfigure(2, weight=1)
            self.__lower_frame.columnconfigure(3, weight=1)
            self.__lower_frame.columnconfigure(4, weight=1)

            self.__upper_frame = Frame(self.__mainwindow)
            self.__upper_frame.pack()

            self.__upper_frame.columnconfigure(0, weight=1)
            self.__upper_frame.columnconfigure(1, weight=1)
            self.__upper_frame.columnconfigure(2, weight=1)
            self.__upper_frame.columnconfigure(3, weight=1)
            self.__upper_frame.columnconfigure(4, weight=1)

            self.__upper_frame.rowconfigure(0, weight=1)
            self.__upper_frame.rowconfigure(1, weight=1)
            self.__upper_frame.rowconfigure(2, weight=1)
            self.__upper_frame.rowconfigure(3, weight=1)
            self.__upper_frame.rowconfigure(4, weight=1)

            self.start_bg_grid()  # Luo taustalle ruudukon

            # Seuraavaksi kysytään käyttäjältä syötettä.
            self.__input = Entry(self.__lower_frame, width=20, relief="flat", font="Arial 16 normal", fg="#8a8a8a")
            self.__input.grid(column=2, row=0, sticky=NSEW, padx=5, pady=5)

            # Syöttää annetun stringin syötekenttään malliksi.
            self.__input.insert(0, "Kirjoita tähän...")

            # Teksti häviää ja fontti muuttuu mustaksi kun syötekenttään klikataan.
            self.__input.bind("<FocusIn>", self.temporary_text)

            # Napin sijaan ENTER-näppäin on sidottu <self.inputter> funktioon bind-metodilla.
            self.__input.bind("<Return>", self.inputter_initializer)

    def inputter_initializer(self, event):
        """
        Käynnistää <self.inputter> funktion, kun ENTER-painiketta klikataan.

        :param event: "<Return>", ENTER-näppäimen klikkaus
        """

        self.inputter()

    def inputter(self):

        """
        Tarkistaa syötteen kunnollisuuden ja nostaa messageboxin, jos syöte on virheellinen.
        """
        try:
            input_word = str(self.__input.get()).lower()
        except ValueError:
            input_word = ""

        count_num = ""  # Str, jonne mahdolliset numerot ja muut merkit tallennetaan
        for letter in input_word:
            if letter.isdigit() or letter in "!@#$%^&*()-+?_=,<>/":
                count_num += letter

        if len(input_word) == 5 and len(count_num) == 0:
            self.checker(input_word)  # Jos kaikki kunnossa sana lähetetään eteepäin checkkeriin
        else:
            ans = messagebox.askyesno("Virhe ohjelmoinnissa...", "Syötteesi oli kummallinen. Lupaatko skarpata?")
            if not ans:
                messagebox.showinfo("Virhe ohjelmoinnissa...", ":(")
                self.__input.delete(0, END)

    def checker(self, word):

        """
        Muodostaa stringin johon syötteenä anetun sanan kirjainten värit kirjataan.

        Pääperiaate labelien värityksessä on, että oikea kirjain oikessa kohdassa sanaa on aina vihreä
        ja sanaan kuulumaton kirjain aina harmaa. Keltainen väri taas kuvastaa sitä, että kirjain on
        sanassa, mutta se on väärässä paikassa. Laattojen väritys antaa kutenkin myös enemmän informaatiota.

            1) Jos käyttäjä saa kirjaimen oikein (virheä väri) ja sanassa ei sama kirjain toistu useammin
                niin muut syötteenä annetun sanan vastaavat kirjaimet tulisi olla harmaita.

                ESIM:  Jos oikea sana on >POIKA< niin käyttäjän syötteestä <RIIMI> tulisi I-kirjaimista
                        kolmantena kirjaimena oleva I värittyä vihreäksi ja muut I:t harmaiksi, sillä
                        >POIKA< ei sisällä enempää kirjaimia I, kuin käyttäjä on jo arvannut oikein.

            2) Jos käyttäjän syöte pitää sisällään satunnaisesta sanasta löytyvän kirjaimen monikerran, niin
                vain oikea määrä kirjaimia (vasemmalta oikealle laskien) saa keltaisen taustan kuvastamaan
                oikeaa kirjainten lukumäärää.

                ESIM: Jos oikea sana on >LINKO< niin käyttäjän syötteestä <TOIMI> tulisi
                      sen I-kirjaimista keltaisella värittyä vain toisena kirjaimena oleva I
                      sillä >LINKO< ei sisällä kahta I:tä.

        Tämä toiminnallisuus saavutetaan tarkistamalla syöte kahteen kertaan, ensin vasemmalta oikealle ja
        sitten oikealta vasemmalle. Lopuksi näitä muodostuneita  <letter_colors_a> ja <letter_colors_b>
        verrataan ja mahdolliset eriäväisyydet merkataan harmaiksi.

        :param word: str, syötteenä annettu sana
        """

        filename = "Sanakirja.txt"
        file = open(filename)

        final_letter_colors = ""  # Lopullinen väristring
        letter_colors_a = ""  # Väliaikainen värirsting (A)
        letter_colors_b = ""  # Väliaikainen värirsting (A)
        green_str = ""  # String, jossa vihreän labelin saavat kirjaimet (A)
        green_str2 = ""  # String, jossa vihreän labelin saavat kirjaimet (B)
        yellow_str = ""  # String, jossa keltaisen labelin saavat kirjaimet (A)

        if str(word) in file.read():
            for x in range(5):                      # A-OSUUS
                if word[x] == self.__rand_word[x]:  # Määrittää saako kirjain virheän labelin.
                    letter_colors_a += "g"
                    green_str += word[x]
                else:
                    if word[x] in self.__rand_word:  # Keltainen tai harmaa label

                        if green_str.count(word[x]) == self.__rand_word.count(word[x]) and \
                                green_str.count(
                                    word[x]) > 0:  # Jos vihreällä merkattu jo kaikki sanan kysytyt kirjaimet
                            letter_colors_a += "r"  # = Harmaa (Huomioi, että tämä merkataan silti yellow_str)
                            yellow_str += word[x]
                        elif yellow_str.count(word[x]) == self.__rand_word.count(
                                word[x]):  # Jos keltaisella merkattu sanan
                            # kaikki kysytyt kirjaimet = Harmaa
                            letter_colors_a += "r"
                        else:
                            letter_colors_a += "y"  # Muuten keltainen
                            yellow_str += word[x]
                    else:
                        letter_colors_a += "r"  # Jos ei kirjain sanassa = Harmaa

            for y in range(4, -1, -1):  # B-OSUUS. Yellow_str toiminnallisuutta lukuunottamatta sama toisesta suunnasta.

                if word[y] == self.__rand_word[y]:
                    letter_colors_b += "g"
                    green_str2 += word[y]
                else:
                    if word[y] in self.__rand_word:
                        if green_str2.count(word[y]) == self.__rand_word.count(word[y]) and \
                                green_str2.count(word[y]) > 0:
                            letter_colors_b += "r"
                        else:
                            letter_colors_b += "y"
                    else:
                        letter_colors_b += "r"

            letter_colors_b = letter_colors_b[::-1]  # Käännetään muodostunut string slicen avulla.

            for z in range(5):  # Tutkitaan väristringien eroavaisuudet.
                if letter_colors_a[z] == letter_colors_b[z]:
                    final_letter_colors += letter_colors_b[z]
                else:
                    final_letter_colors += "r"
        else:
            ans = messagebox.askyesno("Virhe ohjelmoinnissa...", "Sanasi oli kummallinen. Lupaatko skarpata?")
            if not ans:
                messagebox.showinfo("Virhe ohjelmoinnissa...", ":(")

            final_letter_colors = "Error"

        self.letter_colorer(final_letter_colors, word)  # Laitetaan kirjain-värittäjä töihin
        file.close()

    def letter_colorer(self, letter_colors, valid_input_word):

        """
        Piirtää näytölle kirjaimet labeleita käyttäen ja värittää näiden abelien taustat
        <letter_colors> stringin osoittamien värien mukaisesti.

        :param letter_colors: str, kirjainten labelien värit kertova string
        :param valid_input_word: str, tarkastettu sana

        """

        if letter_colors != "Error":

            for value in range(5):

                label_color = letter_colors[value]

                if label_color == "g":  # "g" >> green
                    bg_color = "#34eb64"
                elif label_color == "y":  # "y" >> yellow
                    bg_color = "#e8eb34"
                else:
                    bg_color = "#e3e3e3"  # "r" >> (w)rong = gray

                letter_one = valid_input_word[value].capitalize()

                letter = Label(self.__upper_frame,
                               text=letter_one,
                               width="6",
                               height="3",
                               bg=bg_color,
                               font="Arial 20 bold",
                               fg="#262626")
                self.__tmp_label_list.append(letter)  # Lisää labelin listaan
                letter.grid(column=value, sticky=NSEW, row=self.__current_row, padx=2, pady=2)
                letter.config()

            self.__current_row += 1  # Siirtyy seuraavalle riville

        self.__input.delete(0, END)

        self.win_checker(letter_colors)  # Tarkastaa loppuiko peli

    def word_giver(self):
        """
        Arpoo satunnaisen sanan <Sanakirja1.txt> tiedostosta.
        """
        filename = "Sanakirja1.txt"
        file = open(filename)
        file_content = file.readlines()

        rand_int = random.randrange(0, 229)  # 229 on sanojen (rivien) lukumäärä tiedostossa
        rand_word = file_content[rand_int - 1]
        self.__rand_word = rand_word

        file.close()

    def temporary_text(self, event):
        """
        Poistaa tekstin <self.input> Entry-kentästä kun siihen klikataan.

        :param event: "<FocusIn>, kentän klikkaus
        """

        self.__input.delete(0, "end")
        self.__input.configure(fg="#262626")

    def resetter(self):
        """
        Resetoi labelit ja mahdollistaa uuden pelin.
        """

        for label in self.__tmp_label_list:  # Tuhoaa labelit listan avulla
            label.destroy()

        self.word_giver()  # Uusi sana
        self.__current_row = 0  # Resetoi rivilaskurin

    def win_checker(self, ltr_colors):

        """
        Tarkistaa loppuiko peli, eli onko Label-rivejä 6 tai onko sana arvattu oikein.

        :param ltr_colors: str, tulostettavat värit stringinä
        """
        cur_row = self.__current_row

        # Ensin tarkistetaan, mikäli kaikki kirjaimet oikein.
        if ltr_colors == "ggggg":
            ans1 = messagebox.askyesno("Peli loppui", "Mahtavaa! Sana " + self.__rand_word + " on oikein! "
                                                                    "Haluaisitko pelata uudestaan?")
            if ans1:
                self.resetter()
            else:
                self.quitter()

        # Sitten tarkistetaan, jos ollaan pelattu viimeinen rivi.
        elif cur_row == 6:
            ans = messagebox.askyesno("Peli loppui", "Voi harmi! Oikea sana oli " + self.__rand_word + ". "
                                                            "Haluaisitko kuitenkin pelata uudestaan?")
            if ans:
                self.resetter()
            else:
                self.quitter()

    def start(self):
        """
        Käynnistää pää-loopin (mainloop).
        """
        self.__mainwindow.mainloop()

    def quitter(self):
        """
        Sammuttaa pää-loopin (mainloop).
        """
        self.__mainwindow.destroy()

    def start_screen(self):
        """
        Luo ohjelman käynnistyksessä näytölle laatikon, joka pitää sisällään ohjeet peliin.

        HUOM! Messagebox ilmeisesti muotoilee tekstin automaattisesti, eikä sen kokoa voi
              muuttaa, minkä vuoksi rivivaihdot tehty manuaalisesti välilyönneillä.
        """

        text = "Tervetuloa SANELE-pelin pariin!" \
               "                                                        " \
               "                                                                                                          " \
               "Pelin tarkoituksena (kuten insipaartiona olleen Wordlenkin) on arvata oikein " \
               "satunnaisesti valikoitunut viisikirjaiminen sana. Kaikkien arvausten on olta" \
               "va kelvollisia Suomen kielen sanoja. Paina ENTER-painiketta lähettääksesi " \
               "sanan. Jokaisen arvauksen jälkeen laattojen väri kertoo kuinka lähelle " \
               "arvauksesi osui: " \
               "                                                                        " \
               "                                                                                                          " \
               "Virheä: oikea kirjain oikeassa paikkaa" \
               "                                         " \
               "Keltainen: oikea kirjain väärässä paikaa" \
               "                                       " \
               "Harmaa: kirjain ei ole sanassa"

        ans = messagebox.askyesno("Lämpimästi tervetuloa!", f"{text}")
        if not ans:
            self.quitter()  # Lopettaa ohjelman, jos klikataan "Ei"
        return ans

    def start_bg_grid(self):

        """
        Luo taustalle valkoisen ruudukon (Labeleita), joiden päälle myöhemmin kirjainlaatat tulevat.
        """

        for value_1 in range(6):
            for value in range(5):
                bg_grid = Label(self.__upper_frame,
                                width="6",
                                height="3",
                                bg="#ffffff",
                                fg="#ffffff",
                                font="Arial 20 bold",
                                text=" ")

                bg_grid.grid(column=value, sticky=NSEW, row=value_1, padx=2, pady=2)


def main():
    ui = UI()
    ui.start()


if __name__ == "__main__":
    main()
