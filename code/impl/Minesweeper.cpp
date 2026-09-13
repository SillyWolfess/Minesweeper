#include <logs.hpp>
#include <manager/eventManager.hpp>
#include <Engine.hpp>
#include <math/Math.hpp>

#include "Minesweeper.hpp"
bool MINE_SWEEPER::Minesweeper::init() {
    _mineBar = "mineTopBar";
    LIA_TRY
        if (!getGuiManager()->loadWindow(_mineBar, "./data/minesweeper/gui/data/mineTopBar.xml")) {
            LIA_error_f("Failed to init window for {}", _mineBar);
            return false;
        }
        LIA::ButtonEvent startSimulation("minesweeper", "start_simulation", "");
        getEventManager()->handleEvent(startSimulation);
    LIA_CATCH_RETURN_FALSE
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
    if (!subscribe(LIA::ComponentEvent::BUTTON_ACTION)) {
        LIA_fatal("Failed to subscribe to button action event");
        return false;
    }
    if (!subscribe(LIA::ComponentEvent::OPTION_ACTION)) {
        LIA_fatal("Failed to subscribe to option change action");
        return false;
    }
    return true;
}

bool MINE_SWEEPER::Minesweeper::onGetGuiData(LIA::Event& event) {
    if (event.source.compare(_mineBar) != 0) {
        return false;
    }
    updateData(event.window);
    return true;
}

void MINE_SWEEPER::Minesweeper::updateData(std::string window) {
    LIA_TRY
        LIA::EventManager* eventManager = getEventManager();
        
        std::vector<std::string> resList;
        resList.push_back("1");
        resList.push_back("2");
        resList.push_back("3");
        
        LIA::UpdateGuiListEvent updateLevelsEvent(_mineBar, "level", resList, _iLevel);
        eventManager->handleEvent(updateLevelsEvent);
    LIA_CATCH_EMPTY
}

bool MINE_SWEEPER::Minesweeper::onButtonAction(LIA::Event& event) {
    if (event.window.compare(_mineBar) != 0) {
        return false;
    }
    if (event.action.compare("reset") == 0) {
        prepareLevel();
        return true;
    }
    return false;
}

bool MINE_SWEEPER::Minesweeper::onOptionChanged(LIA::Event& event) {
    if (event.source.compare(_mineBar) != 0) {
        return false;
    }
    if (event.action.compare("level") == 0) {
        _iLevel = event.argi;
        prepareLevel();
        return true;
    }
    return false;
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
        _iLevel = 0;
        if (!prepareLevel()) {
            return true;
        }
        LIA_CATCH(LIA::Engine::getInstance().fatal();)
    return true;
}

int MINE_SWEEPER::Minesweeper::getSizeByLevel() {
    switch (_iLevel) {
        case 0: return 9;
        case 1: return 16;
        case 2: return 20;
        default: return 9;
    }
}

int MINE_SWEEPER::Minesweeper::getMinesByLevel() {
    switch (_iLevel) {
    case 0: return 10;
    case 1: return 40;
    case 2: return 99;
    default: return 10;
    }
}

bool MINE_SWEEPER::Minesweeper::prepareLevel() {
    LIA::ObjectManager* objectManager = getObjectManager();
    LIA_trace("Creating objects");
    int offsetY = 45;
    float offsetX = 10;
    int dSize = getSizeByLevel();
    xSize = dSize;
    ySize = dSize;
    nMines = 0;
    nFlags = 0;
    nUncovered = 0;
    nFields = 0;
    LIA_trace("Cleaning old objects");
    for (Field& field : _fields) {
        int fieldId = objectManager->getByName(-1, field.id);
        if (fieldId == -1) {
            continue;
        }
        LIA::Object* oField = objectManager->get(fieldId);
        std::vector<std::string> toRemove;
        LIA_trace_f("Trying to remove {}", fieldId);
        toRemove.push_back(field.id);
        objectManager->remove(toRemove);
    }
    _fields.clear();
    float scale = 10;
    getGuiManager()->openWindow(_mineBar);
    LIA::Window* mineBarWindow = getGuiManager()->getWindow(_mineBar);
    LIA::Scale wScale = mineBarWindow->getScale();
    if (_minScalex < 0) {
        _minScalex = wScale.x;
    }
    wScale.x = _minScalex;
    if (offsetX + (xSize * 1.0f) * scale + (xSize * scale * 1.5f) + scale < wScale.x) {
        offsetX = offsetX + ((wScale.x - ((xSize * 1.0f) * scale + (xSize * scale * 1.5f) + scale)) * 0.5f);
    }
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
            field.x = (x * 1.0f) * scale + (x * scale * 1.5f) + offsetX + 2;
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
    int toPlant = getMinesByLevel();
    if (_fields.size() < toPlant * 2) {
        LIA_fatal_f("Number of mines to plant {} needs to be less than one half of available space {}", toPlant, _fields.size());
        return false;
    }
    while (toPlant > 0) {
        int mineId = LIA::Math::getRandomInt(0, _fields.size() - 1);
        if (!_fields.at(mineId).mine) {
            plantMine(mineId);
            toPlant--;
        }
    }
    LIA_debug_f("Planted {} mines on {} fields", nMines, nFields);
    int wWidth = 0;
    int wHeight = 0;
    for (Field& field : _fields) {
        int fieldId = objectManager->getByName(-1, field.id);
        if (fieldId == -1) {
            if (!objectManager->createObject(field.objectId, field.id)) {
                LIA_fatal_f("Failed to create {}", field.id);
                LIA::Engine::getInstance().fatal();
                return false;
            }
            fieldId = objectManager->getByName(-1, field.id);
            if (fieldId == -1) {
                LIA_fatal_f("Failed to get {}", field.id);
                LIA::Engine::getInstance().fatal();
                return false;
            }
        }
        LIA::Object* oField = objectManager->get(fieldId);
        oField->_position.x = field.x;
        oField->_position.y = field.y;
        oField->_scale.x = scale;
        oField->_scale.y = scale;
        if (field.x + scale > wWidth) {
            wWidth = field.x + scale;
        }
        if (field.y + scale > wHeight) {
            wHeight = field.y + scale;
        }
    }
    if (wScale.x > wWidth) {
        wWidth = wScale.x;
    }
    updateScoreUI();
    mineBarWindow->setScale(wWidth, wScale.y);
    getAppWindow()->resize(wWidth + 2, wHeight + 2);
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
                                std::string oId = field.id;
                                LIA::TimeCounter uncoverTime;
                                uncoverTime.setName(std::vformat("Uncovering around {}", std::make_format_args(oId)));
                                LIA_trace_f("Uncovering around {} start", oId);
                                uncoverTime.start();
                                uncoverAround(field, field.id);
                                uncoverTime.end();;
                                LIA_trace_f("Uncovering around {} end {}", oId, uncoverTime.getDelta());
                                LIA_debug_f("Uncovered {} / {} with {} mines", nUncovered, _fields.size(), nMines);
                                if (_fields.size() == nUncovered + nMines) {
                                    markAllMines();
                                }
                            }
                        }
                    } else if (mouseRClicked && field.hidden) {
                        // empty -> qm
                        // has flags to place ? qm -> flag : qm -> empty 
                        // flag -> empty
                        if (field.qm) {
                            if (nMines - nFlags > 0) {
                                // qm -> flag
                                std::string oldId = field.id;
                                field.objectId = "field_flag";
                                field.id = std::vformat("field_f[{}_{}]", std::make_format_args(field.iX, field.iY));
                                field.qm = false;
                                field.flag = true;
                                nFlags++;
                                switchFields(field, oldId);
                                updateScoreUI();
                            } else {
                                // qm -> empty
                                std::string oldId = field.id;
                                field.objectId = "field_empty";
                                field.id = std::vformat("field_e[{}_{}]", std::make_format_args(field.x, field.y));
                                field.qm = false;
                                switchFields(field, oldId);
                                
                            }
                        }
                        else if (field.flag) {
                            // flag -> emtpy
                            std::string oldId = field.id;
                            field.objectId = "field_empty";
                            field.id = std::vformat("field_e[{}_{}]", std::make_format_args(field.x, field.y));
                            field.flag = false;
                            nFlags--;
                            switchFields(field, oldId);
                            updateScoreUI();
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

void MINE_SWEEPER::Minesweeper::plantMine(int id) {
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
            LIA_trace_f("Mine counter {} [{} x {}] = {} from {}", nId, x, y, f.minesNear, id);
        }
    }
}

void MINE_SWEEPER::Minesweeper::plantMine(int mx, int my) {
    if (mx < 0 || my < 0) {
        return;
    }
    int id = computeId(mx, my);
    plantMine(id);
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
        field.objectId = "field_uncovered";
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
            /*
            if (x != field.iX && y != field.iY) {
                continue;
            }
            */
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

void MINE_SWEEPER::Minesweeper::updateScoreUI() {
    int nMinesToMark = nMines - nFlags;
    std::string hitsString = std::vformat("{}", std::make_format_args(nMinesToMark));
    LIA::UpdateGuiStringEvent scoreStringEvent(_mineBar, "nFlags", hitsString);
    getEventManager()->handleEvent(scoreStringEvent);
}