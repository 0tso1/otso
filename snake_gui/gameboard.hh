/* COMP.CS.110 - GUI-projekti 12.6 Matopeli
 * Tiedosto: gameboard.hh
 *
 * Olio matopelin pelialuetta varten. Toiminnot
 * madon siirtämiseksi, ruoan siirtämiseksi ja
 * voitosta/häviöstä ilmoittamiseen.
 *
 * Ohjelman muokkaaja:
 * - Nimi: Otso Tolonen
 * - Opiskelijanumero: removed
 * - Käyttäjätunnus: 0tso1
 * - S-posti: otso.tolonen@tuni.fi
 *
 */

#ifndef GAMEBOARD_HH
#define GAMEBOARD_HH

#include "point.hh"
#include <random>
#include <string>
#include <deque> // Better than vector, since we need erase front elements

// Constants for printing the gameboard
const std::string BODY = "~";
const std::string DEAD = "X";
const std::string FOOD = "*";
const std::string HEAD = "@";
const std::string TAIL = "-";
const std::string WALL = "#";
const std::string EMPTY = " ";

// Muutettu: Pelialueen oletusmitat ja oletusarvoinen siemenluku.
const int DEFAULT_WIDTH = 68;
const int DEFAULT_HEIGTH = 26;
const int DEFAULT_SEED = 1;

class GameBoard {
public:
    // Default constructor.
    // Calls the other constructor with default width, height, and seed.
    GameBoard();

    // Constructs a gameboard based the given width and height,
    // rng is used to generate random locations for the food.
    GameBoard(int width, int height, int seed_value);

    // Destructor
    ~GameBoard();

    // Checks if the game is over.
    // Returns false until the game has been either lost or won.
    bool gameOver() const;

    // Checks if the game has been lost.
    // Returns true if the snake is dead.
    bool gameLost() const;

    // Checks if the game has been won.
    // Returns true if the snake has filled the field.
    bool gameWon() const;

    // Moves the snake to the given direction, if possible.
    // Moving is not possible, if game is over, or if the snake would move
    // against a wall.
    // If moving is possible, calls the private method moveSnakeAndFood.
    // Returns true, if moving was possible.
    bool moveSnake(const std::string& direction);

    // Prints the gameboard and the snake in it.
    void print() const;

    // Lisätty: Paluttaa ruoan koordinaatit
    // eli food_:n.
    const Point& food_coordinates();

    // Lisätty: Palauttaa madon koordinaatit
    // eli snake_:n.
    const std::deque<Point>& snake_coordinates();

    // Lisätty: Resetoi madon seuraavaa peliä varten.
    void reset_snake();

    // Lisätty: Vaihtaa siemenluvun ja siirtää ruokaa, jotta
    // jo sijoitettu ruoka saadaan vaihdettua.
    void set_seed(const int s) noexcept {rng_.seed(s); moveFood();}



private:
    // Returns true if the given point is a part of the snake.
    bool isSnakePoint(const Point& point) const;

    // Returns the point of the snake's head.
    const Point& getHead() const;

    // Returns the point of the snake's tail.
    const Point& getTail() const;

    // Relocates food to a random, snakeless location.
    // Hides the food if the snake has completely filled the gameboard.
    void moveFood();

    // Moves the snake unless the new head would be the body of the snake.
    // If the new head is the neck of the snake, nothing happens.
    // If the new head is other body part of the snake, the snake dies.
    // Otherwise the snake moves, whereupon the method returns true.
    // If, in addition, the food got eaten a new one is placed somewhere,
    // and the snake grows a bit.
    bool moveSnakeAndFood(const Point& new_head);

    // Prints the top or bottom wall of the gameboard.
    void printHorizontalWall() const;

    // Tells if the snake is alive and able to move.
    bool dead_ = false;

    // Specifies the width and height of the gameboard.
    const int width_ = 0;
    const int height_ = 0;

    // Generates random numbers used to move the food item to random locations.
    std::default_random_engine rng_;

    // Points currently occupied by the snake, head being the last one.
    std::deque<Point> snake_;

    // The food item's position in the gameboard.
    Point food_;
};  // class GameBoard



#endif  // GAMEBOARD_HH
