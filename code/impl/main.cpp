#include <Engine.hpp>
#include <component/BaseGame.hpp>
#include "Minesweeper.hpp"

int main() {
    LIA::Engine& engine = LIA::Engine::getInstance();
    MINE_SWEEPER::Minesweeper game;
    engine.setGame(&game);
    engine.run();
    return 0;
}