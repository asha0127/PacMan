/**
 * Pac-Man Game - Main Entry Point
 * A simple Pac-Man clone using SplashKit graphics library
 */

#include "game.h"
#include <iostream>

/**
 * Main function - Entry point of the program
 */
int main()
{
    Game game;

    if (!game.initialize())
    {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }

    game.run();
    return 0;
}