#include <logs.hpp>
#include <manager/eventManager.hpp>
#include <Engine.hpp>

#include "Minesweeper.hpp"
bool MINE_SWEEPER::Minesweeper::init() {
    return true;
}

bool MINE_SWEEPER::Minesweeper::registerHandlers() {
    if (!subscribe(LIA::ComponentEvent::GET_DATA_GUI)) {
        LIA_fatal("Failed to subscribe to gui update");
        return false;
    }
    if (!subscribe(LIA::ComponentEvent::INIT_GUI_WINDOW)) {
        LIA_fatal("Failed to subscribe to gui window creation");
        return false;
    }
    if (!subscribe(LIA::ComponentEvent::LOAD)) {
        LIA_fatal("Failed to subscribe to load event");
        return false;
    }
    if (!subscribe(LIA::ComponentEvent::TICK)) {
        LIA_fatal("Failed to subscribe to tick event");
        return false;
    }
    return true;
}

bool MINE_SWEEPER::Minesweeper::onLoad(LIA::Event& event) {
    if (event.source.compare("game") != 0) {
        return false;
    }
    LIA::ObjectManager* objectManager = getObjectManager();

    LIA_TRY
        LIA_trace("registering data paths");
        if (!objectManager->registerPathsFromFile("./data/minesweeper/simulation/data.xml")) {
            LIA_fatal("Failed to register paths from file");
            LIA::Engine::getInstance().fatal();
            return true;
        }

        if (!objectManager->registerTemplatePathsFromFile("./data/minesweeper/objects/data.xml")) {
            LIA_fatal("Failed to register template paths from file");
            LIA::Engine::getInstance().fatal();
            return true;
        }

        if (!objectManager->getModelManager()->registerPathsFromFile("./data/minesweeper/models/data.xml")) {
            LIA_fatal("Failed to register model paths from file");
            LIA::Engine::getInstance().fatal();
            return true;
        }
        /***
         * Create what we need to
         */
        LIA_trace("Creating objects");
        int offsetY = 100;
        int offsetX = 100;
        nMines = 0;
        nUncovered = 0;
        nFields = 0;
        _fields.clear();
        float scale = 10;
        for (int y = 0; y < ySize; y++) {
            for (int x = 0; x < xSize; x++) {
                Field field;
                field.hidden = true;
                field.mine = false;
                field.qm = false;
                field.flag = false;
                field.iX = x;
                field.iY = y;
                field.minesNear = 0;
                field.objectId = "field_empty";
                field.id = std::vformat("field_e[{}_{}]", std::make_format_args(x, y));
                field.x = (x * 1.0f) * scale + (x * scale * 1.5f) + offsetX;
                field.y = (y * 1.0f) * scale + (y * scale * 1.5f) + offsetY;
                field.scale = scale;
                _fields.push_back(field);
                nFields++;
            }
        }
        int ii = 0;
        for (Field f : _fields) {
            int id = computeId(f.iX, f.iY);
            LIA_debug_f("at {}: Field[{}].xy = {} x {}", ii, id, f.iX, f.iY);
            ii++;
        }
        plantMine(1, 4);
        plantMine(1, 5);
        plantMine(4, 1);
        plantMine(5, 6);
        plantMine(6, 1);
        plantMine(6, 2);
        plantMine(6, 4);
        plantMine(7, 6);
        plantMine(7, 7);
        plantMine(8, 3);
        LIA_debug_f("Planted {} mines on {} fields", nMines, nFields);
        for (Field& field : _fields) {
            int fieldId = objectManager->getByName(-1, field.id);
            if (fieldId == -1) {
                if (!objectManager->createObject(field.objectId, field.id)) {
                    LIA_fatal_f("Failed to create {}", field.id);
                    LIA::Engine::getInstance().fatal();
                    return true;
                }
                fieldId = objectManager->getByName(-1, field.id);
                if (fieldId == -1) {
                    LIA_fatal_f("Failed to get {}", field.id);
                    LIA::Engine::getInstance().fatal();
                    return true;
                }
            }
            LIA::Object* oField = objectManager->get(fieldId);
            oField->_position.x = field.x;
            oField->_position.y = field.y;
            oField->_scale.x = scale;
            oField->_scale.y = scale;
        }
    LIA_CATCH(LIA::Engine::getInstance().fatal();)
    return true;
}

bool MINE_SWEEPER::Minesweeper::onTick(LIA::Event& event) {
    LIA_trace("onTick start");
    LIA_TRY
        LIA::Position mousePosition = getAppWindow()->getMousePos();
        bool mouseLClicked = getAppWindow()->wasMouseClicked() || getAppWindow()->isMousePressed();
        bool mouseRClicked = getAppWindow()->wasRMouseClicked();
        LIA::ObjectManager* objectManager = getObjectManager();

        if (mouseLClicked || mouseRClicked) {
            LIA::Scale scale;
            LIA_trace("mouse was clicked");
            for (Field& field: _fields) {
                LIA_trace_f(
                    "Checking click bounds {} x {} with {} = {} x {} and scale {} x {}",
                    mousePosition.x, mousePosition.y, field.id,
                    field.x, field.y,
                    field.scale, field.scale
                );
                LIA::Position pos;
                pos.x = field.x;
                pos.y = field.y;
                scale.x = field.scale;
                scale.y = field.scale;
                if (LIA::isInRangeCentered2D(mousePosition, pos, scale)) {
                    if (mouseLClicked) {
                        LIA_trace_f("Clicked left mouse inside {}", field.id);
                        if (field.hidden && !field.flag) {
                            if (field.mine) {
                                std::string oldId = field.id;
                                field.objectId = "field_lost";
                                field.id = std::vformat("field_l[{}_{}]", std::make_format_args(field.iX, field.iY));
                                field.hidden = false;
                                switchFields(field, oldId);
                                uncoverAll();
                            }
                            else {
                                uncoverAround(field, field.id);
                                LIA_debug_f("Uncovered {} / {} with {} mines", nUncovered, _fields.size(), nMines);
                                if (_fields.size() == nUncovered + nMines) {
                                    markAllMines();
                                }
                            }
                        }
                    } else if (mouseRClicked && field.hidden) {
                        // empty -> qm
                        // qm -> flag
                        // flag -> empty
                        if (field.qm) {
                            // qm -> flag
                            std::string oldId = field.id;
                            field.objectId = "field_flag";
                            field.id = std::vformat("field_f[{}_{}]", std::make_format_args(field.iX, field.iY));
                            field.qm = false;
                            field.flag = true;
                            switchFields(field, oldId);
                        }
                        else if (field.flag) {
                            // flag -> emtpy
                            std::string oldId = field.id;
                            field.objectId = "field_empty";
                            field.id = std::vformat("field_e[{}_{}]", std::make_format_args(field.x, field.y));
                            field.flag = false;
                            switchFields(field, oldId);
                        }
                        else if (!field.qm && !field.flag) {
                            //empty -> qm
                            std::string oldId = field.id;
                            field.objectId = "field_qm";
                            field.id = std::vformat("field_q[{}_{}]", std::make_format_args(field.iX, field.iY));
                            field.qm = true;
                            switchFields(field, oldId);
                        }
                        else {
                            LIA_warn("Unknown state. Should never happen");
                        }
                    }
                    break;
                }
            }
        }
    LIA_CATCH_EMPTY
    LIA_trace("onTick end");
    return true;
}

int MINE_SWEEPER::Minesweeper::computeId(int x, int y) {
    return (y * ySize) + x;
}

void MINE_SWEEPER::Minesweeper::plantMine(int mx, int my) {
    if (mx < 0 || my < 0) {
        return;
    }
    int id = computeId(mx, my);
    if (id > _fields.size() - 1) {
        return;
    }
    Field& field = _fields.at(id);
    if (field.mine) {
        return;
    }
    field.mine = true;
    nMines++;
    for (int y = field.iY - 1; y < field.iY + 2; y++) {
        for (int x = field.iX - 1; x < field.iX + 2; x++) {
            if (x < 0 || y < 0) {
                continue;
            }
            if (x == field.iX && y == field.iY) {
                continue;
            }
            if (x > xSize - 1 || y > ySize - 1) {
                continue;
            }
            int nId = computeId(x, y);
            if (nId > _fields.size() - 1) {
                continue;
            }
            Field& f = _fields.at(nId);
            f.minesNear = f.minesNear + 1;
            LIA_trace_f("Mine counter {} [{} x {}] = {} from {} x {}", nId, x, y, f.minesNear, mx, my);
        }
    }
}

void MINE_SWEEPER::Minesweeper::markAllMines() {
    for (int y = 0; y < ySize; y++) {
        for (int x = 0; x < xSize; x++) {
            int id = computeId(x, y);
            if (id < 0 || id > _fields.size() - 1) {
                continue;
            }
            Field& field = _fields.at(id);
            if (!field.hidden || !field.mine) {
                continue;
            }
            std::string oldId = field.id;
            field.objectId = "field_flag";
            field.id = std::vformat("field_fw[{}_{}]", std::make_format_args(field.iX, field.iY));
            field.flag = true;
            switchFields(field, oldId);
        }
    }
}

void MINE_SWEEPER::Minesweeper::uncoverAll() {
    for (int y = 0; y < ySize; y++) {
        for (int x = 0; x < xSize; x++) { 
            int id = computeId(x, y);
            if (id < 0 || id > _fields.size() - 1) {
                continue;
            }
            Field& field = _fields.at(id);
            if (!field.hidden) {
                continue;
            }
            if (field.mine) {
                std::string oldId = field.id;
                field.objectId = "field_mine";
                field.id = std::vformat("field_l[{}_{}]", std::make_format_args(field.iX, field.iY));
                field.hidden = false;
                switchFields(field, oldId);
            }
            else {
                uncoverAround(field, field.id);
            }
        }
    }
}

void MINE_SWEEPER::Minesweeper::uncoverAround(Field& field, std::string oldId) {
    if (!field.hidden || field.mine) {
        return;
    }
    LIA_trace_f("Uncover around {}", field.id);
    field.hidden = false;
    nUncovered++;
    if (field.minesNear > 0) {
        field.objectId = "field_x";
        switch (field.minesNear) {
            case 1: 
                field.objectId = "field_1";
            break;
            case 2:
                field.objectId = "field_2";
                break;
            case 3:
                field.objectId = "field_3";
                break;
            case 4:
                field.objectId = "field_4";
                break;
            case 5:
                field.objectId = "field_5";
                break;
            case 6:
                field.objectId = "field_6";
                break;
            case 7:
                field.objectId = "field_7";
                break;
            case 8:
                field.objectId = "field_8";
                break;
            default:
                field.objectId = "field_x";
            break;

        }
        field.id = std::vformat("field_m[{}_{}]", std::make_format_args(field.iX, field.iY));
    } else {
        field.objectId = "field";
        field.id = std::vformat("field[{}_{}]", std::make_format_args(field.iX, field.iY));
    }
    switchFields(field, oldId);
    if (field.minesNear > 0) {
        return;
    }
    for (int y = field.iY - 1; y < field.iY + 2; y++) {
        if (y < 0 || y > ySize - 1) {
            continue;
        }
        for (int x = field.iX - 1; x < field.iX + 2; x++) {
            if (x < 0 || x > xSize - 1) {
                continue;
            }
            if (x == field.iX && y == field.iY) {
                continue;
            }
            if (x == field.iX || y == field.iY) {
                int id = computeId(x, y);
                if (id > _fields.size() - 1) {
                    continue;
                }
                LIA_trace_f("Id = {} at {} x {}", id, x, y);
                Field& f = _fields.at(id);
                uncoverAround(f, f.id);
            }
        }
    }
}

void MINE_SWEEPER::Minesweeper::switchFields(Field field, std::string oldId) {
    LIA_TRY
        LIA_trace_f("Switching field {} to {} ({})", oldId, field.objectId, field.id);

        LIA::ObjectManager* objectManager = getObjectManager();
        int fieldId = objectManager->getByName(-1, oldId);
        LIA::Object* oField = objectManager->get(fieldId);
        if (!objectManager->createObject(field.objectId, field.id)) {
            LIA_fatal_f("Failed to create {}", field.id);
            LIA::Engine::getInstance().fatal();
            return;
        }
        int field2Id = -1;
        field2Id = objectManager->getByName(field2Id, field.id);
        LIA::Object* field2 = objectManager->get(field2Id);
        LIA::copy(field2->_position, oField->_position);
        LIA::copy(field2->_scale, oField->_scale);

        std::vector<std::string> toRemove;
        LIA_trace_f("Trying to remove {}", oldId);
        toRemove.push_back(oldId);
        objectManager->remove(toRemove);
   LIA_CATCH(LIA::Engine::getInstance().fatal();)
}