#ifndef MINE_SWEEPER_HPP
#define MINE_SWEEPER_HPP
#include <component/BaseGame.hpp>
#include <string>

namespace MINE_SWEEPER {
    class Minesweeper : public LIA::BaseGame {
        protected:
            bool init();
            virtual bool registerHandlers();
            virtual bool onLoad(LIA::Event&);
            virtual bool onTick(LIA::Event&);
        private:
            int xSize = 9;
            int ySize = 9;
            int nMines = 0;
            int nUncovered = 0;
            int nFields = 0;
            struct Field {
                bool mine;
                bool hidden;
                bool qm;
                bool flag;
                int iX;
                int iY;
                int minesNear;
                float x;
                float y;
                float scale;
                std::string objectId;
                std::string id;
            };
            std::vector<Field> _fields;
            int computeId(int, int);
            void plantMine(int, int);
            void uncoverAll();
            void markAllMines();
            void uncoverAround(Field&, std::string);
            void switchFields(Field, std::string);
    };
}
#endif