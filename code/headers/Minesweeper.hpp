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
            virtual bool onButtonAction(LIA::Event&);
            virtual bool onOptionChanged(LIA::Event&);
            virtual bool onGetGuiData(LIA::Event&);
        private:
            int xSize = 9;
            int ySize = 9;
            int nMines = 0;
            int nFlags = 0;
            int nUncovered = 0;
            int nFields = 0;
            int _iLevel = 0;
            float _minScalex = -1;
            std::string _mineBar;
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
            void plantMine(int);
            void uncoverAll();
            void markAllMines();
            void uncoverAround(Field&, std::string);
            void switchFields(Field, std::string);
            void updateScoreUI();
            bool prepareLevel();
            int getSizeByLevel();
            int getMinesByLevel();
            void updateData(std::string);
    };
}
#endif