#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vectors.h"

#define LINE_MAX_SIZE 100
#define MAX_CMD_ELEMENTS 10
#define MAX_SHIPS 10
#define true 1
#define false 0

#define PLAYERS_COUNT 2
#define TYPES_COUNT 4
#define CARRIERS 0
#define BATTLESHIPS 1
#define CRUISERS 2
#define DESTROYERS 3

/* ===========================
 * Program CBattleShips
 * Author: Maciej Krzyżanowski
 * Index: 188872
 * ===========================*/

/* ============
 * Enumerations
 * ============*/

/* =================
 * Types definitions
 * =================*/

typedef struct {
    Point start;
    Point end;
} Rectangle;

typedef struct {
    char* commandGroup;
    char* commandName;
    char** commandArgs;
    int argsCount;
} Command;

typedef struct {
    int typesCounts[TYPES_COUNT];
    Ship ships[TYPES_COUNT][MAX_SHIPS];
    int hasShoot;
    Rectangle initArea;
    int isAI;
} Player;

typedef struct {
    Player** players;
    int nextPlayerIndex;
    char groupName[LINE_MAX_SIZE - 2];
    int isInsideGroup;
    int shouldEnd;
    int planeSizeX;
    int planeSizeY;
    PointVec* reefs;
    int extendedShips;
    unsigned int randomSeed;
    int wasSeedGiven;
} Game;

/* ================
 * Global constants
 * ===============*/
const int shipsSizes[4] = {5, 4, 3, 2};
const enum Direction directions[4] = {N, W, S, E};

/* ==================================
 * Command handling related functions
 * ==================================*/
int readLine(char* line);
int splitStringIntoWords(char* strToSplit, char* wordsOut[]);
int isLineGroup(const char*);
int handleGroup(char*, Game*);
int updateNextPlayer(char*, const char*, Game*);
void formCommand(Command*, char*, char*);
int handleCommand(Command*, Game*);
void printErrorFromLine(char*, char*);
void printError(Command*, char*);
void getLineFromCmd(Command* cmd, char* line);

/* ============
 * Constructors
 *= ===========*/
Ship createNewShip(int, int);
Player* createNewPlayer();
Game* initGame();

/* =================
 * Utility functions
 * =================*/
int areAllShipsPlaced(Player**);
int getCurrentPlayer(Command*);
int getClassIndex(char*);
void getShipElementsOfPlayer(Player*, ShipElementVec*);
void getAllShipElements(ShipElementVec*, Player**);
int getPlayerRemainingCount(Player*);
Rectangle getRectOccupiedBy(Ship);
int isShipOnReef(Ship ship, Game* game);
int isTooCloseToOtherShip(Ship*, Game*);
void freeAllSpyPlanes(Game* game);
char getCharOfPlayerIndex(int index);
int getIndexOfPlayerChar(char playerChar);
void freeGame(Game*);

/* ================
 * Command handlers
 * ================*/
int placeShip(Command*, Game*);
int shoot(Command*, Game*);
int setFleet(Command* cmd, Player**);
void updateTypesCounts(Player*, const int[]);
int setNextPlayer(Command*, Game*);
int statePrint(Command *cmd, Game *game);
int setBoardSize(Command*, Game*);
int setInitPos(Command*, Player**);
int addReef(Command*, Game*);
int shipCommand(Command*, Game*);
int moveShip(Command*, Game*);
int shootExtended(Command*, Game*);
int playerPrint(Command*, Game*);
int placeSpy(Command*, Game*);
int saveGame(Game*);
int setAIPlayer(Command*, Game*);
int setSrand(Command*, Game*);

void handleAI(Game*);

/* ===================================================================================================================*/
int main() {
    Game* game = initGame();
    char* line = (char*) malloc(sizeof(char) * LINE_MAX_SIZE);

    long chars = readLine(line);
    while(chars != EOF && (!game->shouldEnd)) {
        if(!handleGroup(line, game)) {
            Command* cmd = (Command*) malloc(sizeof(Command));
            formCommand(cmd, game->groupName, line);

            int anyErrors = handleCommand(cmd, game);
            if(anyErrors) break;

            free(cmd->commandArgs - 1);
            free(cmd);
        }

        chars = readLine(line);
    }

    Player* nextPlayer = game->players[game->nextPlayerIndex];
    if(chars == EOF && !game->isInsideGroup && nextPlayer->isAI) {
        // AI should be executed here
        handleAI(game);
    }

    freeGame(game);
    free(line);
    return 0;
}
/* ===================================================================================================================*/

void freeGame(Game* game) {
    freeAllSpyPlanes(game);
    free(game->players[0]);
    free(game->players[1]);
    free(game->players);
    free(game->reefs->ptr);
    free(game->reefs);
    free(game);
}

int splitStringIntoWords(char* strToSplit, char** wordsOut) {
    unsigned long strLength = strlen(strToSplit);

    // Remove spaces at the start and at the end
    while(strToSplit[0] == ' ') strToSplit++, strLength--;
    while(strToSplit[strLength - 1] == ' ') strToSplit[strLength - 1] = '\0', strLength--;

    strToSplit[strLength] = ' ';
    strToSplit[strLength + 1] = '\0';
    int wordsCount = 0;
    int currentLength = 0;
    for(int i = 0; strToSplit[i] != '\0'; i++) {
        if(strToSplit[i] == ' ') {
            strToSplit[i] = '\0';
            wordsOut[wordsCount] = strToSplit + (i - currentLength);
            currentLength = 0;
            wordsCount++;
        } else {
            currentLength++;
        }
    }

    return wordsCount;
}

// Return 0 or -1 if there was nothing to read
int readLine(char* line) {
    int readChars = 0;
    int readChar = getchar();

    while(readChar != '\n' && readChar != EOF) {
        line[readChars++] = (char) readChar;
        readChar = getchar();
    }

    line[readChars] = '\0';

    if(readChar == EOF) return -1;
    return 0;
}

void getLineFromCmd(Command* cmd, char* line) {
    unsigned long offset = 0;
    unsigned long cmdNameLen = strlen(cmd->commandName);
    for(unsigned int i = 0; i < cmdNameLen; i++) {
        line[i] = cmd->commandName[i];
        offset++;
    }
    line[offset++] = ' ';
    for(int argN = 0; argN < cmd->argsCount; argN++) {
        unsigned long argLen = strlen(cmd->commandArgs[argN]);
        strcpy(line + offset, cmd->commandArgs[argN]);
        offset += argLen;
        line[offset++] = ' ';
    }
    line[--offset] = '\0';
}

void printErrorFromLine(char* line, char* reason) {
    unsigned long lineLen = strlen(line);
    if(line[lineLen - 1] == ']') {
        line[lineLen] = ' ';
        line[lineLen + 1] = '\0';
    }
    printf("INVALID OPERATION \"%s\": %s\n", line, reason);
}

void printError(Command* cmd, char* reason) {
    char l[100];
    getLineFromCmd(cmd, l);
    printf("INVALID OPERATION \"%s\": %s\n", l, reason);
}

int isLineGroup(const char* str) {
    return str[0] == '[';
}

void getGroupNameFromLine(const char* line, char* readName) {
    int j;
    for (j = 1; line[j] != ']'; j++) {
        readName[j - 1] = line[j];
    }
    readName[j-1] = '\0';
}

void clearShipMovesAndShotsFor(Player* player) {
    for(int classI = 0; classI < TYPES_COUNT; classI++) {
        for(int shipI = 0; shipI < player->typesCounts[classI]; shipI++) {
            player->ships[classI][shipI].timesMoved = 0;
            player->ships[classI][shipI].shotThisTurn = 0;
        }
    }
}

// true if command handling should stop <==> line is a group statement
int handleGroup(char* line, Game* game) {
    char newGroupName[LINE_MAX_SIZE - 2];
    int isGroup = isLineGroup(line);
    if(isGroup) {
        getGroupNameFromLine(line, newGroupName);
        if(game->isInsideGroup && strcmp(game->groupName, newGroupName) == 0) {
            game->isInsideGroup = false;
            if(strncmp(newGroupName, "player", 6) == 0) {
                // PLAYER HAS ENDED HIS TURN! CHECK FOR VICTORY!
                int playerIndex = newGroupName[6] == 'A' ? 0 : 1;
                Player* currentPlayer = game->players[playerIndex];
                clearShipMovesAndShotsFor(currentPlayer);

                int remainingCount = getPlayerRemainingCount(game->players[game->nextPlayerIndex]);
                if(remainingCount == 0 && areAllShipsPlaced(game->players)) {
                    printf("%c won\n", newGroupName[6]);
                    game->shouldEnd = 1;
                }
            }
        } else if(game->isInsideGroup) {
            printErrorFromLine(line, "THE OTHER PLAYER EXPECTED");
            game->shouldEnd = 1;
        } else {
            if(strncmp(newGroupName, "player", 6) == 0) {
                if(!updateNextPlayer(line, newGroupName, game)) return isGroup;
            }
            strcpy(game->groupName, newGroupName);
            game->isInsideGroup = true;
        }
    }

    return isGroup;
}

int updateNextPlayer(char* line, const char* newGroupName, Game* game) {
    char playerX = newGroupName[6];
    if(playerX == 'A') {
        if(game->nextPlayerIndex != 0) {
            printErrorFromLine(line, "THE OTHER PLAYER EXPECTED");
            game->shouldEnd = 1;
            return 0;
        } else {
            game->nextPlayerIndex = 1;
        }
    } else {
        if (game->nextPlayerIndex != 1) {
            printErrorFromLine(line, "THE OTHER PLAYER EXPECTED");
            game->shouldEnd = 1;
            return 0;
        } else {
            game->nextPlayerIndex = 0;
        }
    }

    // Player updated, check if new player is A.I.
    // If so then execute A.I. function
//    Player* currentPlayer = game->players[getIndexOfPlayerChar(playerX)];
//    if(currentPlayer->isAI) {
//        handleAI(game, currentPlayer);
//    } DIDN'T WORK

    return 1;
}

void formCommand(Command* readBuffer, char* groupName, char* line) {
    char** commandElements = (char**) malloc(MAX_CMD_ELEMENTS * sizeof(char*));
    int argsCount = splitStringIntoWords(line, commandElements) - 1;
    char* commandName = commandElements[0];
    char** commandArgs = commandElements + 1;
    Command cmd;
    cmd.commandGroup = groupName;
    cmd.commandName = commandName;
    cmd.commandArgs = commandArgs;
    cmd.argsCount = argsCount;
    *readBuffer = cmd;
}

int handleCommand(Command* commandToHandle, Game* game) {
    if(!game->isInsideGroup) return 0;
    if(strcmp(commandToHandle->commandGroup, "state") == 0) {
        if(strcmp(commandToHandle->commandName, "PRINT") == 0) {
            return statePrint(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "SET_FLEET") == 0) {
            return setFleet(commandToHandle, game->players);
        } else if(strcmp(commandToHandle->commandName, "NEXT_PLAYER") == 0) {
            return setNextPlayer(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "BOARD_SIZE") == 0) {
            return setBoardSize(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "INIT_POSITION") == 0) {
            return setInitPos(commandToHandle, game->players);
        } else if(strcmp(commandToHandle->commandName, "REEF") == 0) {
           return addReef(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "SHIP") == 0) {
            return shipCommand(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "EXTENDED_SHIPS") == 0) {
            game->extendedShips = 1;
        } else if(strcmp(commandToHandle->commandName, "SAVE") == 0) {
            saveGame(game);
        } else if(strcmp(commandToHandle->commandName, "SET_AI_PLAYER") == 0) {
            return setAIPlayer(commandToHandle, game);
        }
    } else {
        if(strcmp(commandToHandle->commandName, "PLACE_SHIP") == 0) {
            return placeShip(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "SHOOT") == 0) {
            if(game->extendedShips) {
                return shootExtended(commandToHandle, game);
            } else {
                return shoot(commandToHandle, game);
            }
        } else if(strcmp(commandToHandle->commandName, "MOVE") == 0) {
            return moveShip(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "PRINT") == 0) {
            return playerPrint(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "SPY") == 0) {
            return placeSpy(commandToHandle, game);
        } else if(strcmp(commandToHandle->commandName, "SRAND") == 0) {
            return setSrand(commandToHandle, game);
        }
    }

    return 0;
}

int areAllShipsPlaced(Player** players) {
    int allPlaced = 1;
    for(int playerN = 0; playerN <= 1; playerN++) {
        for(int shipClassI = 0; shipClassI < TYPES_COUNT; shipClassI++) {
            for(int shipIndex = 0; shipIndex < players[playerN]->typesCounts[shipClassI]; shipIndex++) {
                if(!players[playerN]->ships[shipClassI][shipIndex].isPlaced) {
                    allPlaced = 0;
                    break;
                }
            }
        }
    }
    return allPlaced;
}

int getCurrentPlayer(Command* cmd) {
    if(strcmp(cmd->commandGroup, "playerA") == 0) {
        return 0;
    } else if(strcmp(cmd->commandGroup, "playerB") == 0) {
        return 1;
    } else {
        return -1;
    }
}

Ship createNewShip(int size, int ID) {
    Ship s;
    s.headPos.y = -1;
    s.headPos.x = -1;
    s.direction = N;
    s.shots = 0;
    s.isPlaced = 0;
    s.size = size;
    s.timesMoved = 0;
    s.shotThisTurn = 0;
    s.ID = ID;
    initPointVec(&s.spyPlanes);
    return s;
}

Player* createNewPlayer() {
    Player* p = (Player*) malloc(sizeof(Player));
    p->typesCounts[0] = 1;
    p->typesCounts[1] = 2;
    p->typesCounts[2] = 3;
    p->typesCounts[3] = 4;
    for(int i = 0; i < TYPES_COUNT; i++) {
        for(int j = 0; j < p->typesCounts[i]; j++) {
            p->ships[i][j] = createNewShip(shipsSizes[i], j);
        }
    }
    p->hasShoot = 0;
    p->isAI = 0;
    return p;
}

Game* initGame() {
    Game* newGame = (Game*) malloc(sizeof(Game));
    newGame->isInsideGroup = 0;
    newGame->nextPlayerIndex = 0;
    newGame->shouldEnd = 0;
    newGame->players = (Player**) malloc(sizeof(Player**) * 2);

    Rectangle aDefaultInitArea;
    aDefaultInitArea.start.x = 0;
    aDefaultInitArea.end.x = 9;
    aDefaultInitArea.start.y = 0;
    aDefaultInitArea.end.y = 9;

    Rectangle bDefaultInitArea;
    bDefaultInitArea.start.x = 0;
    bDefaultInitArea.end.x = 9;
    bDefaultInitArea.start.y = 11;
    bDefaultInitArea.end.y = 20;

    Player *a = createNewPlayer();
    a->initArea = aDefaultInitArea;
    newGame->players[0] = a;

    Player *b = createNewPlayer();
    b->initArea = bDefaultInitArea;
    newGame->players[1] = b;

    newGame->planeSizeX = 10;
    newGame->planeSizeY = 21;

    newGame->reefs = (PointVec*) malloc(sizeof(PointVec));
    initPointVec(newGame->reefs);

    newGame->extendedShips = 0;
    newGame->randomSeed = 0;
    newGame->wasSeedGiven = false;

    return newGame;
}

void updateTypesCounts(Player* dest, const int newTypesCounts[TYPES_COUNT]) {
    for(int i = 0; i < TYPES_COUNT; i++) {
        dest->typesCounts[i] = newTypesCounts[i];
    }
    for(int i = 0; i < TYPES_COUNT; i++) {
        for(int j = 0; j < dest->typesCounts[i]; j++) {
            dest->ships[i][j] = createNewShip(shipsSizes[i], j);
        }
    }
}

int setFleet(Command* cmd, Player** players) {
    char* playerX = cmd->commandArgs[0];
    int playerIndex;
    if(strcmp(playerX, "A") == 0) {
        playerIndex = 0;
    } else {
        playerIndex = 1;
    }

    int newTypesCounts[4];
    for(int i = 0; i < 4; i++) {
        newTypesCounts[i] = atoi(cmd->commandArgs[i+1]);
    }

    updateTypesCounts(players[playerIndex], newTypesCounts);
    return 0;
}

int getClassIndex(char* className) {
    int cIndex = 0;
    if(strcmp(className, "CAR") == 0) cIndex = CARRIERS;
    if(strcmp(className, "BAT") == 0) cIndex = BATTLESHIPS;
    if(strcmp(className, "CRU") == 0) cIndex = CRUISERS;
    if(strcmp(className, "DES") == 0) cIndex = DESTROYERS;
    return cIndex;
}

void getShipElementsOfPlayer(Player* p, ShipElementVec* shipElements) {
    for(int shipType = 0; shipType < TYPES_COUNT; shipType++) {
        int shipTypeCount = p->typesCounts[shipType];
        for(int shipIndex = 0; shipIndex < shipTypeCount; shipIndex++) {
            int shipSize = shipsSizes[shipType];
            Ship ship = p->ships[shipType][shipIndex];
            if(!ship.isPlaced) continue;

            int modY = 0, modX = 0;
            switch(ship.direction) {
                case N:
                {
                    modY = 1;
                    break;
                }
                case S:
                {
                    modY = -1;
                    break;
                }
                case E:
                {
                    modX = -1;
                    break;
                }
                case W:
                {
                    modX = 1;
                    break;
                }
            }

            int nowX = ship.headPos.x;
            int nowY = ship.headPos.y;

            int nth = 0;

            for(int s = 0; s < shipSize; s++) {
                ShipElement el;
                el.pos.x = nowX;
                el.pos.y = nowY;
                el.ship = &p->ships[shipType][shipIndex];
                el.nth = nth++;

                shipElementVecPushBack(shipElements, el);

                nowX += modX;
                nowY += modY;
            }
        }
    }
}

void getAllShipElements(ShipElementVec* shipElements, Player** players) {
    for(int playerN = 0; playerN <= 1; playerN++) {
        getShipElementsOfPlayer(players[playerN], shipElements);
    }
}

int getPlayerRemainingCount(Player* player) {
    ShipElementVec* elements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(elements);
    getShipElementsOfPlayer(player, elements);
    // Check if nth bit of ship's shot bitmask is 0, if so then count it
    int remainingCount = 0;
    for(int i = 0; i < elements->length; i++) {
        if(!(elements->ptr[i].ship->shots & (1 << elements->ptr[i].nth))) {
            remainingCount++;
        }
    }
    free(elements->ptr);
    free(elements);
    return remainingCount;
}

int placeShip(Command* cmd, Game* game) {
    int y = atoi(cmd->commandArgs[0]);
    int x = atoi(cmd->commandArgs[1]);
    enum Direction D = (unsigned char) cmd->commandArgs[2][0];
    int i = atoi(cmd->commandArgs[3]);
    char* C = cmd->commandArgs[4];
    int cIndex = getClassIndex(C);

    if(i >= game->players[getCurrentPlayer(cmd)]->typesCounts[cIndex]) {
        printError(cmd, "ALL SHIPS OF THE CLASS ALREADY SET");
        return 1;
    }

    if(game->players[getCurrentPlayer(cmd)]->ships[cIndex][i].isPlaced) {
        printError(cmd, "SHIP ALREADY PRESENT");
        return 1;
    }

    // VALIDATION
    int backY = y;
    int backX = x;
    int shipSize = shipsSizes[cIndex];
    switch(D) {
        case 'N': {
            backY += shipSize - 1;
            break;
        }
        case 'S': {
            backY -= shipSize - 1;
            break;
        }
        case 'E': {
            backX -= shipSize - 1;
            break;
        }
        case 'W': {
            backX += shipSize - 1;
            break;
        }
        default:
            break;
    }

    int currentPlayerIndex = getCurrentPlayer(cmd);
    int wellPlaced;

    Player* currentPlayer = game->players[currentPlayerIndex];
    currentPlayer->ships[cIndex][i].headPos.x = x;
    currentPlayer->ships[cIndex][i].headPos.y = y;
    currentPlayer->ships[cIndex][i].direction = D;

    Rectangle initArea = currentPlayer->initArea;
    wellPlaced = (y >= initArea.start.y) && (y <= initArea.end.y)
            && (x >= initArea.start.x) && (x <= initArea.end.x)
            && (backY >= initArea.start.y) && (backY <= initArea.end.y)
            && (backX >= initArea.start.x) && (backX <= initArea.end.x);

    int isOnReef = isShipOnReef(currentPlayer->ships[cIndex][i], game);
    int isTooCloseToOther = isTooCloseToOtherShip(&currentPlayer->ships[cIndex][i], game);

    if(!wellPlaced) {
        printError(cmd, "NOT IN STARTING POSITION");
        return 1;
    } else if(isOnReef) {
        printError(cmd, "PLACING SHIP ON REEF");
        return 1;
    } else if(isTooCloseToOther) {
        printError(cmd, "PLACING SHIP TOO CLOSE TO OTHER SHIP");
        return 1;
    }

    currentPlayer->ships[cIndex][i].isPlaced = 1;

    return 0;
}

int shipCommand(Command* cmd, Game* game) {
    char playerX = cmd->commandArgs[0][0];
    Player* player = game->players[playerX == 'A' ? 0 : 1];
    int y = atoi(cmd->commandArgs[1]);
    int x = atoi(cmd->commandArgs[2]);
    enum Direction D = (unsigned char) cmd->commandArgs[3][0];
    int i = atoi(cmd->commandArgs[4]);
    char* C = cmd->commandArgs[5];
    int cIndex = getClassIndex(C);
    char* bitmask = cmd->commandArgs[6];

    if(i >= player->typesCounts[cIndex]) {
        printError(cmd, "ALL SHIPS OF THE CLASS ALREADY SET");
        return 1;
    }

    Point headPos;
    headPos.y = y;
    headPos.x = x;
    player->ships[cIndex][i].headPos = headPos;

    int isAlreadyPlaced = player->ships[cIndex][i].isPlaced;
    int isOnReef = isShipOnReef(player->ships[cIndex][i], game);
    int isTooCloseToOther = isTooCloseToOtherShip(&player->ships[cIndex][i], game);

    if(isOnReef) {
        printError(cmd, "PLACING SHIP ON REEF");
        return 1;
    } else if(isTooCloseToOther) {
        printError(cmd, "PLACING SHIP TOO CLOSE TO OTHER SHIP");
        return 1;
    } else if(isAlreadyPlaced) {
        printError(cmd, "SHIP ALREADY PRESENT");
        return 1;
    }

    player->ships[cIndex][i].isPlaced = true;
    player->ships[cIndex][i].direction = D;

    int bitmaskLen = shipsSizes[cIndex];
    for(int b = 0; b < bitmaskLen; b++) {
        player->ships[cIndex][i].shots |= ((bitmask[b] == '0' ? 1 : 0) << b);
    }

    return 0;
}

int shoot(Command* cmd, Game *game) {
    if(!game->extendedShips && game->players[getCurrentPlayer(cmd)]->hasShoot) {
        printError(cmd, "NO DOUBLE SHOOTING!");
        return 1;
    } else if(!areAllShipsPlaced(game->players)) {
        printError(cmd, "NOT ALL SHIPS PLACED");
        return 1;
    }

    int y, x;
    if(game->extendedShips) {
        y = atoi(cmd->commandArgs[2]);
        x = atoi(cmd->commandArgs[3]);
    } else {
        y = atoi(cmd->commandArgs[0]);
        x = atoi(cmd->commandArgs[1]);
    }

    if((x < 0 || x >= game->planeSizeX) || (y < 0 || y >= game->planeSizeY)) {
        printError(cmd, "FIELD DOES NOT EXIST");
        return 1;
    }

    /*
     * 1. Iterate over ship fields
     * - some method which will return array of ship fields placed
     * Ship field being:
     * - x position
     * - y position
     * - [0|1] if shot (it needs to be pointer to real ship bitmask part)
     * 2. If x == x, y == y then change shot to 1
     *
     */
    ShipElementVec* elements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(elements);
    getAllShipElements(elements, game->players);

    for(int i = 0; i < elements->length; i++) {
        int elX = elements->ptr[i].pos.x;
        int elY = elements->ptr[i].pos.y;
        int nth = elements->ptr[i].nth;

        if(x == elX && y == elY) {
            elements->ptr[i].ship->shots |= (1 << nth);
            break;
        }
    }

    free(elements->ptr);
    free(elements);

    if(!game->extendedShips) {
        game->players[getCurrentPlayer(cmd)]->hasShoot = true;
        game->players[!getCurrentPlayer(cmd)]->hasShoot = false;
    }

    return 0;
}

int setNextPlayer(Command* cmd, Game* game) {
    char* nextPlayerX = cmd->commandArgs[0];
    if(nextPlayerX[0] == 'A') {
        game->nextPlayerIndex = 0;
    } else {
        game->nextPlayerIndex = 1;
    }

    return 0;
}

char** alloc2DArray(int y, int x) {
    char** arr = (char**) malloc(sizeof(char**) * y);

    for(int i = 0; i < y; i++) {
        arr[i] = (char*) malloc(sizeof(char*) * x);
    }

    return arr;
}

void clear2DArray(int y, int x, char** arr, char symbol) {
    for(int i = 0; i < y; i++) {
        for(int j = 0; j < x; j++) {
            arr[i][j] = symbol;
        }
    }
}

void free2DArray(int y, char** arr) {
    for(int i = 0; i < y; i++)
        free(arr[i]);
    free(arr);
}

int setBoardSize(Command* cmd, Game* game) {
    int y = atoi(cmd->commandArgs[0]);
    int x = atoi(cmd->commandArgs[1]);
    game->planeSizeY = y;
    game->planeSizeX = x;
    return 0;
}

int setInitPos(Command* cmd, Player** players) {
    char playerX = cmd->commandArgs[0][0];
    int playerIndex = playerX == 'A' ? 0 : 1;
    Player* playerToModify = players[playerIndex];
    int startX, endX, startY, endY;
    startY = atoi(cmd->commandArgs[1]);
    startX = atoi(cmd->commandArgs[2]);
    endY = atoi(cmd->commandArgs[3]);
    endX = atoi(cmd->commandArgs[4]);
    playerToModify->initArea.start.y = startY;
    playerToModify->initArea.start.x = startX;
    playerToModify->initArea.end.y = endY;
    playerToModify->initArea.end.x = endX;
    return 0;
}

int addReef(Command* cmd, Game* game) {
    int y = atoi(cmd->commandArgs[0]);
    int x = atoi(cmd->commandArgs[1]);

    int isWellPlaced = (x >= 0 && x <= game->planeSizeX - 1)
            && (y >= 0 && y < game->planeSizeY - 1);

    if(!isWellPlaced) {
        printError(cmd, "REEF IS NOT PLACED ON BOARD");
        return 1;
    }

    Point reef;
    reef.x = x;
    reef.y = y;
    pointVecPushBack(game->reefs, reef);
    return 0;
}

Rectangle getRectOccupiedBy(Ship ship) {
    int y = ship.headPos.y;
    int x = ship.headPos.x;
    int backY = y;
    int backX = x;
    enum Direction D = ship.direction;
    int shipSize = ship.size;
    Rectangle rect;
    switch(D) {
        case 'N': {
            backY += shipSize - 1;
            rect.start.x = x;
            rect.start.y = y;
            rect.end.x = backX;
            rect.end.y = backY;
            break;
        }
        case 'S': {
            backY -= shipSize - 1;
            rect.start.x = backX;
            rect.start.y = backY;
            rect.end.x = x;
            rect.end.y = y;
            break;
        }
        case 'E': {
            backX -= shipSize - 1;
            rect.start.x = backX;
            rect.start.y = backY;
            rect.end.x = x;
            rect.end.y = y;
            break;
        }
        case 'W': {
            backX += shipSize - 1;
            rect.start.x = x;
            rect.start.y = y;
            rect.end.x = backX;
            rect.end.y = backY;
            break;
        }
        default:
            break;
    }
    return rect;
}

int isPointInsideRect(Rectangle* rect, Point* point) {
    return (point->x >= rect->start.x) && (point->x <= rect->end.x)
    && (point->y >= rect->start.y) && (point->y <= rect->end.y);
}

int isShipOnReef(Ship ship, Game* game) {
    for(int reefI = 0; reefI < game->reefs->length; reefI++) {
        Point reef = game->reefs->ptr[reefI];
        Rectangle rect = getRectOccupiedBy(ship);
        if(isPointInsideRect(&rect, &reef)) {
            return 1;
        }
    }
    return 0;
}

int isTooCloseToOtherShip(Ship* ship, Game* game) {
    Rectangle rect = getRectOccupiedBy(*ship);
    rect.start.x--;
    rect.start.y--;
    rect.end.x++;
    rect.end.y++;
    ShipElementVec* elements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(elements);
    getAllShipElements(elements, game->players);
    for(int i = 0; i < elements->length; i++) {
        Point p = elements->ptr[i].pos;
        int isPInside = isPointInsideRect(&rect, &p);
        if(isPInside) {
            free(elements->ptr);
            free(elements);
            return 1;
        }
    }
    free(elements->ptr);
    free(elements);
    return 0;
}

void getShipDirMods(Ship* ship, int* modY, int* modX) {
    *modX = 0, *modY = 0;
    switch(ship->direction) {
        case N:
        {
            *modY = 1;
            break;
        }
        case S:
        {
            *modY = -1;
            break;
        }
        case E:
        {
            *modX = -1;
            break;
        }
        case W:
        {
            *modX = 1;
            break;
        }
    }
}

int isShotAt(Ship* ship, int distFromHead) {
    char shotBitmap = ship->shots;
    return (shotBitmap & (1 << distFromHead));
}

enum Direction rotateDirToRight(enum Direction prior) {
    enum Direction newDir;
    switch(prior) {
        case 'N':
            newDir = 'E';
            break;
        case 'S':
            newDir = 'W';
            break;
        case 'E':
            newDir = 'S';
            break;
        case 'W':
            newDir = 'N';
            break;
        default:
            break;
    }
    return newDir;
}

enum Direction rotateDirToLeft(enum Direction prior) {
    enum Direction newDir = N;
    switch(prior) {
        case 'N':
            newDir = 'W';
            break;
        case 'S':
            newDir = 'E';
            break;
        case 'E':
            newDir = 'N';
            break;
        case 'W':
            newDir = 'S';
            break;
        default:
            break;
    }
    return newDir;
}

int moveShip(Command* cmd, Game* game) {
    /* Validation:
     * 1. the ship has not destroyed engine (SHIP CANNOT MOVE), DONE
     * 2. the ship is not moving too many times(SHIP MOVED ALREADY), DONE
     * 3. the ship is not placed on reef(PLACING SHIP ON REEF), DONE
     * 4. the ship not moves out of board (SHIP WENT FROM BOARD) DONE
     * 5. the ship is not placed too close to other ships (PLACING SHIP TOO CLOSE TO OTHER SHIP).
     */
    int i = atoi(cmd->commandArgs[0]);
    int cIndex = getClassIndex(cmd->commandArgs[1]);
    char xDir = cmd->commandArgs[2][0];
    Player* currentPlayer = game->players[getCurrentPlayer(cmd)];

    // We will copy ship from player, try to move it and if validation succeeds then we will copy
    // validationShip x, y and direction to real player ship
    Ship validationShip = currentPlayer->ships[cIndex][i];

    // Validation which doesn't require to calculate new position

    // Check if engine is right, engine is a part of the ship at its back
    int cannotMove = game->extendedShips && isShotAt(&validationShip, validationShip.size - 1);
    if(cannotMove) {
        printError(cmd, "SHIP CANNOT MOVE");
        return 1;
    }

    int maxMoves = cIndex == CARRIERS ? 2 : 3;
    int hasShipUsedItsMoves = validationShip.timesMoved == maxMoves;
    if(hasShipUsedItsMoves) {
        printError(cmd, "SHIP MOVED ALREADY");
        return 1;
    }

    int modY, modX;
    getShipDirMods(&validationShip, &modY, &modX);

    switch(xDir) {
        case 'F':
            validationShip.headPos.x -= modX;
            validationShip.headPos.y -= modY;
            break;
        case 'L':
            validationShip.headPos.x -= modX;
            validationShip.headPos.y -= modY;
            validationShip.headPos.x -= (validationShip.size - 1) * modY;
            validationShip.headPos.y += (validationShip.size - 1) * modX;
            break;
        case 'R':
            validationShip.headPos.x -= modX;
            validationShip.headPos.y -= modY;
            validationShip.headPos.x += (validationShip.size - 1) * modY;
            validationShip.headPos.y += (validationShip.size - 1) * modX;
            break;
        default:
            break;
    }

    // Change direction of validationShip
    enum Direction currentDir = validationShip.direction;
    if(xDir == 'L') {
        validationShip.direction = rotateDirToLeft(currentDir);
    } else if(xDir == 'R') {
        validationShip.direction = rotateDirToRight(currentDir);
    }

    // Rest of validation which should be done after movement calculation
    int isOnReef = isShipOnReef(validationShip, game);
    if(isOnReef) {
        printError(cmd, "PLACING SHIP ON REEF");
        return 1;
    }

    Rectangle shipRect = getRectOccupiedBy(validationShip);
    int isInsideBoard = (shipRect.start.x >= 0 && shipRect.end.x <= (game->planeSizeX - 1))
            && (shipRect.start.y >= 0 && shipRect.end.y <= (game->planeSizeY - 1));
    if(!isInsideBoard) {
        printError(cmd, "SHIP WENT FROM BOARD");
        return 1;
    }

    // Disable isPlaced of ship so placement validation will work correctly
    currentPlayer->ships[cIndex][i].isPlaced = 0;

    int isTooCloseToOthers = isTooCloseToOtherShip(&validationShip, game);
    if(isTooCloseToOthers) {
        printError(cmd, "PLACING SHIP TOO CLOSE TO OTHER SHIP");
        return 1;
    }

    // Reenable isPlaced after check
    currentPlayer->ships[cIndex][i].isPlaced = 1;

    // Finally if all validations succeeded change position of real ship
    currentPlayer->ships[cIndex][i].headPos.x = validationShip.headPos.x;
    currentPlayer->ships[cIndex][i].headPos.y = validationShip.headPos.y;

    // Update moves count
    currentPlayer->ships[cIndex][i].timesMoved++;

    // Update ship's direction
    currentPlayer->ships[cIndex][i].direction = validationShip.direction;

    return 0;
}

int shootExtended(Command* cmd, Game* game) {
    /* Validation
     * 1. the ship has not destroyed cannon (SHIP CANNOT SHOOT),
     * 2. the ship is not shooting too many shoots (TOO MANY SHOOTS),
     * 3. the ship is shooting in the cannons range(SHOOTING TOO FAR).
     */
    int i = atoi(cmd->commandArgs[0]);
    int cIndex = getClassIndex(cmd->commandArgs[1]);
    int y = atoi(cmd->commandArgs[2]);
    int x = atoi(cmd->commandArgs[3]);

    Ship* shootingShip = &game->players[getCurrentPlayer(cmd)]->ships[cIndex][i];

    int isCannonDestroyed = isShotAt(shootingShip, 1);
    if(isCannonDestroyed) {
        printError(cmd, "SHIP CANNOT SHOOT");
        return 1;
    }

    int usedAllShots = shootingShip->shotThisTurn == shootingShip->size;
    if(usedAllShots) {
        printError(cmd, "TOO MANY SHOOTS");
        return 1;
    }

    int cannonX, cannonY;
    int modX, modY;
    getShipDirMods(shootingShip, &modY, &modX);
    cannonX = shootingShip->headPos.x + modX;
    cannonY = shootingShip->headPos.y + modY;

    int isNearEnough = cIndex == CARRIERS
            || (((y - cannonY)*(y - cannonY) + (x - cannonX)*(x - cannonX)) <= (shootingShip->size*shootingShip->size));
    if(!isNearEnough) {
        printError(cmd, "SHOOTING TOO FAR");
        return 1;
    }

    shoot(cmd, game);
    shootingShip->shotThisTurn++;

    return 0;
}

void printGameToArr(Command* cmd, Game *game, char** gamePlane) {
    char type = cmd->commandArgs[0][0];
    clear2DArray(game->planeSizeY, game->planeSizeX, gamePlane, ' ');

    ShipElementVec* shipElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(shipElements);
    getAllShipElements(shipElements, game->players);

    for(int elementIndex = 0; elementIndex < shipElements->length; elementIndex++) {
        ShipElement* element = &shipElements->ptr[elementIndex];
        int y = element->pos.y;
        int x = element->pos.x;
        int isBroken = shipElements->ptr[elementIndex].ship->shots & (1 << shipElements->ptr[elementIndex].nth);

        char displayChar = '+';

        if(type == '1') {
            if(element->nth == 0) { // Radar
                displayChar = '@';
            } else if(element->nth == element->ship->size-1) { // Engine
                displayChar = '%';
            } else if(element->nth == 1) { // Cannon
                displayChar = '!';
            }
        }

        if(isBroken) displayChar = 'x';

        gamePlane[y][x] = displayChar;
    }

    free(shipElements->ptr);
    free(shipElements);

    // Add reefs to plane
    for(int reefI = 0; reefI < game->reefs->length; reefI++) {
        Point reef = game->reefs->ptr[reefI];
        gamePlane[reef.y][reef.x] = '#';
    }
}

void printArr(char** arr, int sizeY, int sizeX) {
    for(int y = 0; y < sizeY; y++) {
        for (int x = 0; x < sizeX; x++) {
            printf("%c", arr[y][x]);
        }
        printf("\n");
    }
}

int getLengthOfNumber(int n) {
    int length = 0;
    do {
        n /= 10;
        length++;
    } while (n != 0);
    return length;
}

int intPow(int a, int n) {
    if(n == 0) return 1;
    int res = a;
    for(int i = 0; i < n; i++) {
        res *= n;
    }
    return res;
}

int getIDigitOfNumber(int n, int digitI) {
    int res = (n/intPow(10, digitI))%10;
    return res;
}

void printArrWithNumbers(char** arr, int sizeY, int sizeX) {
    int widthNumMaxLen = getLengthOfNumber(sizeX - 1);
    int heightNumMaxLen = getLengthOfNumber(sizeY - 1);

    for(int lineI = 0; lineI < widthNumMaxLen; lineI++) {
        for(int h = 0; h < heightNumMaxLen; h++) {
            printf(" ");
        }

        for(int x = 0; x < sizeX; x++) {
            int numLen = getLengthOfNumber(x);
            int howManyLeading = widthNumMaxLen - numLen;
            if(howManyLeading-1 >= lineI) {
                printf("0");
            } else {
                printf("%d", getIDigitOfNumber(x, numLen - 1 - lineI + howManyLeading));
            }
        }

        printf("\n");
    }

    for(int lineI = 0; lineI < sizeY; lineI++) {
        printf( "%0*d", heightNumMaxLen, lineI);
        for(int x = 0; x < sizeX; x++) {
            printf("%c", arr[lineI][x]);
        }
        printf("\n");
    }
//    printf( "%0*d", 3, i);
}

int statePrint(Command* cmd, Game *game) {
    char type = cmd->commandArgs[0][0];
    char** gamePlane = alloc2DArray(game->planeSizeY, game->planeSizeX);
    printGameToArr(cmd, game, gamePlane);

    if(type == '0') {
        printArr(gamePlane, game->planeSizeY, game->planeSizeX);
    } else if(type == '1') {
        printArrWithNumbers(gamePlane, game->planeSizeY, game->planeSizeX);
    }

    printf("PARTS REMAINING:: A : %d B : %d\n",
           getPlayerRemainingCount(game->players[0]),
           getPlayerRemainingCount(game->players[1]));
    return 0;
}

Point pointOf(int y, int x) {
    Point newP;
    newP.y = y;
    newP.x = x;
    return newP;
}

int arePointsInRange(Point a, Point b, int range) {
    return ((a.x - b.x)*(a.x - b.x)) + ((a.y - b.y)*(a.y - b.y)) <= range*range;
}

int canShipSee(Ship s, Point p) {
    int radarX = s.headPos.x;
    int radarY = s.headPos.y;
    if(arePointsInRange(pointOf(radarY, radarX), p, s.size)) {
        return true;
    }
    return false;
}

int canPlayerSee(int playerIndex, Point p, Game* game) {
    Player* player = game->players[playerIndex];
    for(int classI = 0; classI < TYPES_COUNT; classI++) {
        for(int shipI = 0; shipI < player->typesCounts[classI]; shipI++) {
            Ship s = player->ships[classI][shipI];
            if(canShipSee(s, p)) return true;
        }
    }
    return false;
}

void playerPrintToArr(Command* cmd, Game* game, char** gamePlane) {
    printGameToArr(cmd, game, gamePlane);

    char** fogOfWar = alloc2DArray(game->planeSizeY, game->planeSizeX);
    clear2DArray(game->planeSizeY, game->planeSizeX, fogOfWar, '?');

    // PRINT ALL PRINTING PLAYER'S SHIPS TO PLANE
    // CREATE ARRAY FULL OF FOG SYMBOLS
    // ITERATE OVER PRINTING PLAYERS SHIPS and clear some of the fog of war based on radars radiuses
    ShipElementVec* elements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(elements);
    Player* currentPlayer = game->players[getCurrentPlayer(cmd)];
    getShipElementsOfPlayer(currentPlayer, elements);

    for(int i = 0; i < elements->length; i++) {
        int elX = elements->ptr[i].pos.x;
        int elY = elements->ptr[i].pos.y;
        fogOfWar[elY][elX] = ' ';
    }

    free(elements->ptr);
    free(elements);

    for(int classI = 0; classI < TYPES_COUNT; classI++) {
        for(int shipI = 0; shipI < currentPlayer->typesCounts[classI]; shipI++) {
            if(!currentPlayer->ships[classI][shipI].isPlaced) continue;
            int headX = currentPlayer->ships[classI][shipI].headPos.x;
            int headY = currentPlayer->ships[classI][shipI].headPos.y;
            for(int y = 0; y < game->planeSizeY; y++) {
                for(int x = 0; x < game->planeSizeX; x++) {
                    Ship* currentShip = &currentPlayer->ships[classI][shipI];

                    int radarRange;

                    if(isShotAt(currentShip, 0)) {
                        radarRange = 1;
                    } else {
                        radarRange = (currentShip->size)*(currentShip->size);
                    }

                    int isInRadarRange = (((headX - x)*(headX - x)) + ((headY - y)*(headY - y)))
                                         <= radarRange;
                    if(isInRadarRange) {
                        fogOfWar[y][x] = ' ';
                    }

                    for(int spyI = 0; spyI < currentShip->spyPlanes.length; spyI++) {
                        Point* spyPlane = &currentShip->spyPlanes.ptr[spyI];
                        int startX = spyPlane->x - 1;
                        int startY = spyPlane-> y - 1;
                        int endX = spyPlane->x + 1;
                        int endY = spyPlane->y + 1;

                        int isInSpyPlaneRange = (x >= startX) && (x <= endX) && (y >= startY) && (y <= endY);

                        if(isInSpyPlaneRange) {
                            fogOfWar[y][x] = ' ';
                        }
                    }
                }
            }
        }
    }

    for(int y = 0; y < game->planeSizeY; y++) {
        for(int x = 0; x < game->planeSizeX; x++) {
            if(fogOfWar[y][x] == '?' && gamePlane[y][x] != '#') {
                gamePlane[y][x] = '?';
            }
        }
    }

    free2DArray(game->planeSizeY, fogOfWar);
}

int playerPrint(Command* cmd, Game* game) {
    char type = cmd->commandArgs[0][0];
    char** gamePlane = alloc2DArray(game->planeSizeY, game->planeSizeX);
    playerPrintToArr(cmd, game, gamePlane);
    if(type == '0') {
        printArr(gamePlane, game->planeSizeY, game->planeSizeX);
    } else if(type == '1') {
        printArrWithNumbers(gamePlane, game->planeSizeY, game->planeSizeX);
    }
    free2DArray(game->planeSizeY, gamePlane);
    return 0;
}

void freeAllSpyPlanes(Game* game) {
    for(int playerI = 0; playerI < 2; playerI++) {
        for(int classI = 0; classI < TYPES_COUNT; classI++) {
            for(int shipI = 0; shipI < game->players[playerI]->typesCounts[classI]; shipI++) {
                free(game->players[playerI]->ships[classI][shipI].spyPlanes.ptr);
            }
        }
    }
}

int placeSpy(Command* cmd, Game* game) {
    int i = atoi(cmd->commandArgs[0]);
    int y = atoi(cmd->commandArgs[1]);
    int x = atoi(cmd->commandArgs[2]);

    Player* currentPlayer = game->players[getCurrentPlayer(cmd)];
    Ship* carrier = &currentPlayer->ships[CARRIERS][i];

    int isCarrierPlaced = carrier->isPlaced;
    if(!isCarrierPlaced) {
        printError(cmd, "CARRIER IS NOT PLACED");
        return 1;
    }

    int isCannonDestroyed = isShotAt(carrier, 1);
    if(isCannonDestroyed) {
        printError(cmd, "CANNOT SEND PLANE");
        return 1;
    }

    if(carrier->spyPlanes.length == shipsSizes[CARRIERS]) {
        printError(cmd, "ALL PLANES SENT");
        return 1;
    }

    Point p;
    p.x = x;
    p.y = y;

    pointVecPushBack(&carrier->spyPlanes, p);
    carrier->shotThisTurn++;

    return 0;
}

char getCharOfPlayerIndex(int index) {
    return index == 0 ? 'A' : 'B';
}

int getIndexOfPlayerChar(char playerChar) {
    return playerChar == 'A' ? 0 : 1;
}

char* getClassNameFromIndex(int classI) {
    switch(classI) {
        case 0:
            return "CAR";
        case 1:
            return "BAT";
        case 2:
            return "CRU";
        case 3:
            return "DES";
        default:
            break;
    }
    return "";
}

// Only for saving purposes!
void getBitmaskStringFromChar(char* str, char bitmask, int size) {
    for(int i = 0; i < size; i++) {
        char bitmaskChar = (bitmask & (1 << i)) ? '0' : '1';
        str[i] = bitmaskChar;
    }
    str[size] = '\0';
}

int saveGame(Game* game) {
    printf("[state]\n");

    // Information about board size
    printf("BOARD_SIZE %d %d\n", game->planeSizeY, game->planeSizeX);

    // Information about next player
    Player* nowPlayer = game->players[(!game->nextPlayerIndex)];

    char nextPlayerChar;
    if(nowPlayer->isAI) {
        nextPlayerChar = getCharOfPlayerIndex(!game->nextPlayerIndex);
    } else {
        nextPlayerChar = getCharOfPlayerIndex(game->nextPlayerIndex);
    }

    printf("NEXT_PLAYER %c\n", nextPlayerChar);

    // Information about players
    for(int playerI = 0; playerI <= 1; playerI++) {
        Player* currentPlayer = game->players[playerI];
        char playerChar = getCharOfPlayerIndex(playerI);

        printf("INIT_POSITION %c %d %d %d %d\n",
               playerChar,
               currentPlayer->initArea.start.y,
               currentPlayer->initArea.start.x,
               currentPlayer->initArea.end.y,
               currentPlayer->initArea.end.x
        );

        printf("SET_FLEET %c %d %d %d %d\n",
               playerChar,
               currentPlayer->typesCounts[CARRIERS],
               currentPlayer->typesCounts[BATTLESHIPS],
               currentPlayer->typesCounts[CRUISERS],
               currentPlayer->typesCounts[DESTROYERS]
        );

        for(int classI = 0; classI < TYPES_COUNT; classI++) {
            for(int shipI = 0; shipI < currentPlayer->typesCounts[classI]; shipI++) {
                Ship* currentShip = &currentPlayer->ships[classI][shipI];
                if(!currentShip->isPlaced) continue;
                char bitmaskStr[8];
                getBitmaskStringFromChar(bitmaskStr, currentShip->shots, currentShip->size);
                printf("SHIP %c %d %d %c %d %s %s\n",
                    playerChar,
                    currentShip->headPos.y,
                    currentShip->headPos.x,
                    currentShip->direction,
                    shipI,
                    getClassNameFromIndex(classI),
                    bitmaskStr
                );
            }
        }
    }

    for(int reefI = 0; reefI < game->reefs->length; reefI++) {
        Point reef = game->reefs->ptr[reefI];
        printf("REEF %d %d\n", reef.y, reef.x);
    }

    if(game->extendedShips) {
        printf("EXTENDED_SHIPS\n");
    }

    for(int playerI = 0; playerI < 2; playerI++) {
        if(game->players[playerI]->isAI) {
            printf("SET_AI_PLAYER %c\n", getCharOfPlayerIndex(playerI));
        }
    }

    // Information about seed increased by 1
    if(game->wasSeedGiven) {
        printf("SRAND %u\n", game->randomSeed+1);
    }

    printf("[state]\n");
    return 0;
}

int setAIPlayer(Command* cmd, Game* game) {
    char playerChar = cmd->commandArgs[0][0];
    int playerIndex = getIndexOfPlayerChar(playerChar);
    Player* toSet = game->players[playerIndex];
    toSet->isAI = 1;
    return 0;
}

int setSrand(Command* cmd, Game* game) {
    int x = atoi(cmd->commandArgs[0]);
    game->randomSeed = x;
    game->wasSeedGiven = true;
    return 0;
}

int isShipRightPlaced(Game* game, Player* player, Ship* ship) {
    Rectangle sR = getRectOccupiedBy(*ship);
    Rectangle initArea = player->initArea;
    int wellPlaced = (sR.start.y >= initArea.start.y) && (sR.start.y <= initArea.end.y)
                     && (sR.start.x >= initArea.start.x) && (sR.start.x <= initArea.end.x)
                     && (sR.end.y >= initArea.start.y) && (sR.end.y <= initArea.end.y)
                     && (sR.end.x >= initArea.start.x) && (sR.end.x <= initArea.end.x);
    int isOnReef = isShipOnReef(*ship, game);
    int isTooCloseToOther = isTooCloseToOtherShip(ship, game);

    return wellPlaced && (!isOnReef) && (!isTooCloseToOther);
}

void getAllUnplacedShips(Player* player, ShipVec* dest) {
    for(int classI = 0; classI < TYPES_COUNT; classI++) {
        for(int shipI = 0; shipI < player->typesCounts[classI]; shipI++) {
            Ship* ship = &player->ships[classI][shipI];
            if(!ship->isPlaced) {
                shipVecPushBack(dest, ship);
            }
        }
    }
}

void copyShip(Ship* dest, Ship* source) {
    dest->headPos = source->headPos;
    dest->size = source->size;
    dest->isPlaced = source->isPlaced;
    dest->direction = source->direction;
    dest->shots = source->shots;
    dest->shotThisTurn = source->shotThisTurn;
    dest->timesMoved = source->timesMoved;
    initPointVec(&dest->spyPlanes);
    for(int spyI = 0; spyI < source->spyPlanes.length; spyI++) {
        pointVecPushBack(&dest->spyPlanes, source->spyPlanes.ptr[spyI]);
    }
    dest->ID = source->ID;
}

void copyPlayer(Player* dest, Player* source) {
    dest->initArea = source->initArea;
    dest->isAI = source->isAI;
    dest->hasShoot = source->hasShoot;
    updateTypesCounts(dest, source->typesCounts);
    for(int classI = 0; classI < TYPES_COUNT; classI++) {
        for(int shipI = 0; shipI < source->typesCounts[classI]; shipI++) {
            copyShip(&dest->ships[classI][shipI], &source->ships[classI][shipI]);
        }
    }
}

// Copies only key aspects of game
void copyGame(Game* dest, Game* source) {
    dest->planeSizeX = source->planeSizeX;
    dest->planeSizeY = source->planeSizeY;
    dest->nextPlayerIndex = source->nextPlayerIndex;
    dest->extendedShips = source->extendedShips;
    dest->shouldEnd = source->shouldEnd;
    dest->isInsideGroup = source->isInsideGroup;
    dest->randomSeed = source->randomSeed;
    memcpy(dest->groupName, source->groupName, strlen(source->groupName) * sizeof(char));

    dest->players = (Player**) malloc(PLAYERS_COUNT * sizeof(Player*));
    for(int playerI = 0; playerI < PLAYERS_COUNT; playerI++) {
        dest->players[playerI] = (Player*) malloc(sizeof(Player));
        copyPlayer(dest->players[playerI], source->players[playerI]);
    }

    PointVec* newReefs = malloc(sizeof(PointVec));
    initPointVec(newReefs);
    for(int reefI = 0; reefI < source->reefs->length; reefI++) {
        pointVecPushBack(newReefs, source->reefs->ptr[reefI]);
    }
    dest->reefs = newReefs;
}

char* getClassNameBySize(int size) {
    switch(size) {
        case 2:
            return "DES";
        case 3:
            return "CRU";
        case 4:
            return "BAT";
        case 5:
            return "CAR";
        default:
            break;
    }
    return "";
}

void aiPlaceShips(Player* aiPlayerCp, Game* copyOfGame) {
    // Randomly choose: Direction and one of unplaced Ships
    // Randomly choose x, y of ship
    // Validate x, y
    // If validation failed then go back to choosing x and y
    ShipVec* allUnplacedShips = (ShipVec*) malloc(sizeof(ShipVec));
    initShipVec(allUnplacedShips);
    getAllUnplacedShips(aiPlayerCp, allUnplacedShips);

    while(allUnplacedShips->length != 0) {
        int randShipI = rand() % allUnplacedShips->length;
        Ship* shipToPlace = allUnplacedShips->ptr[randShipI];
        enum Direction D = directions[rand() % TYPES_COUNT];

        int x, y;
        do {
            x = (rand() % (aiPlayerCp->initArea.end.x - aiPlayerCp->initArea.start.x))
                + aiPlayerCp->initArea.start.x;
            y = (rand() % (aiPlayerCp->initArea.end.y - aiPlayerCp->initArea.start.y))
                + aiPlayerCp->initArea.start.y;
            shipToPlace->headPos.x = x;
            shipToPlace->headPos.y = y;

            randShipI = rand() % allUnplacedShips->length;
            D = directions[rand() % TYPES_COUNT];
            shipToPlace->direction = D;

        } while(!isShipRightPlaced(copyOfGame, aiPlayerCp, shipToPlace));

        shipToPlace->isPlaced = true;

        printf("PLACE_SHIP %d %d %c %d %s\n",
               y,
               x,
               D,
               shipToPlace->ID,
               getClassNameBySize(shipToPlace->size)
        );

        free(allUnplacedShips->ptr);
        initShipVec(allUnplacedShips);
        getAllUnplacedShips(aiPlayerCp, allUnplacedShips);
    }

    free(allUnplacedShips->ptr);
    free(allUnplacedShips);
}

void getEnemyElementsToShoot(int enemyIndex, Ship s, Game* game, ShipElementVec* elements) {
    ShipElementVec* shipElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(shipElements);

    getShipElementsOfPlayer(game->players[enemyIndex], shipElements);

    for(int elI = 0; elI < shipElements->length; elI++) {
        if(canShipSee(s, shipElements->ptr[elI].pos)) {
            if(!isShotAt(shipElements->ptr[elI].ship, shipElements->ptr[elI].nth)) {
                int modX, modY;
                getShipDirMods(&s, &modY, &modX);
                Point cannonPos = s.headPos;
                cannonPos.y += modY;
                cannonPos.x += modX;
                if(arePointsInRange(cannonPos, shipElements->ptr[elI].pos, s.size)) {
                    shipElementVecPushBack(elements, shipElements->ptr[elI]);
                }
            }
        }
    }

    free(shipElements->ptr);
    free(shipElements);
}

void getEnemyShipElementsSeenBy(int playerIndex, Game* game, ShipElementVec* elements) {
    ShipElementVec* enemyShipElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(enemyShipElements);

    getShipElementsOfPlayer(game->players[playerIndex], enemyShipElements);

    // Iterate over enemy ship elements and add only those which are in radars line of sight
    for(int elI = 0; elI < enemyShipElements->length; elI++) {
        if(canPlayerSee(playerIndex, enemyShipElements->ptr[elI].pos, game)) {
            shipElementVecPushBack(elements, enemyShipElements->ptr[elI]);
        }
    }

    free(enemyShipElements->ptr);
    free(enemyShipElements);
}

void getShipElementsSeenBy(int playerIndex, Game* game, ShipElementVec* elements) {
    ShipElementVec* enemyShipElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
    initShipElementVec(enemyShipElements);

    getShipElementsOfPlayer(game->players[game->nextPlayerIndex], enemyShipElements);

    // All ships of a player is seen by the player
    getShipElementsOfPlayer(game->players[playerIndex], elements);

    // Iterate over enemy ship elements and add only those which are in radars line of sight
    getEnemyShipElementsSeenBy(playerIndex, game, elements);

    free(enemyShipElements->ptr);
    free(enemyShipElements);
}

void aiShoot(int playerIndex, Game* game) {
    if(!areAllShipsPlaced(game->players)) return;

    Player* aiPlayer = game->players[playerIndex];

    if(game->extendedShips) {
        for(int classI = 0; classI < TYPES_COUNT; classI++) {
            for(int shipI = 0; shipI < aiPlayer->typesCounts[classI]; shipI++) {
                Ship s = aiPlayer->ships[classI][shipI];
                if(!s.isPlaced || isShotAt(&s, 1)) continue;

                ShipElementVec* seenEnemyElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
                initShipElementVec(seenEnemyElements);

                int shotsRemaining = s.size;

                for(int i = 0; i < shotsRemaining; i++) {
                    shipElementVecReset(seenEnemyElements);
                    getEnemyElementsToShoot(!playerIndex, s, game, seenEnemyElements);

                    if(seenEnemyElements->length > 0) {
                        int randI = rand() % seenEnemyElements->length;

                        int modY, modX;
                        getShipDirMods(&s, &modY, &modX);
                        Point cannonPos = s.headPos;
                        cannonPos.y += modY;
                        cannonPos.x += modX;
                        ShipElement choosen = seenEnemyElements->ptr[randI];

                        if(!arePointsInRange(choosen.pos, cannonPos, s.size)) {
                            shotsRemaining++;
                            continue;
                        }

                        printf("SHOOT %d %s %d %d\n",
                               s.ID,
                               getClassNameBySize(s.size),
                               choosen.pos.y,
                               choosen.pos.x
                        );
                    } else {
                        int randX, randY;
                        int modY, modX;
                        getShipDirMods(&s, &modY, &modX);
                        Point cannonPos = s.headPos;
                        cannonPos.y += modY;
                        cannonPos.x += modX;
                        int shootingAtOwnShip;
                        int j = 0;
                        do {
                            shootingAtOwnShip = false;
                            j++;
                            randX = rand() % game->planeSizeX;
                            randY = rand() % game->planeSizeY;
                            ShipElementVec* ownElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
                            initShipElementVec(ownElements);
                            getShipElementsOfPlayer(aiPlayer, ownElements);

                            for(int elI = 0; elI < ownElements->length; elI++) {
                                ShipElement el = ownElements->ptr[elI];
                                if(el.pos.y == randY && el.pos.x == randX) {
                                    shootingAtOwnShip = true;
                                    break;
                                }
                            }

                            free(ownElements->ptr);
                            free(ownElements);
                        } while(!(arePointsInRange(cannonPos, pointOf(randY, randX), s.size)) || shootingAtOwnShip);

                        printf("SHOOT %d %s %d %d\n",
                               s.ID,
                               getClassNameBySize(s.size),
                               randY,
                               randX
                        );
                    }
                }

                free(seenEnemyElements->ptr);
                free(seenEnemyElements);
            }
        }
    } else {
        ShipElementVec* seenEnemyElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
        initShipElementVec(seenEnemyElements);
        getEnemyShipElementsSeenBy(playerIndex, game, seenEnemyElements);

        if(seenEnemyElements->length > 0) {
            int randEl = rand() % seenEnemyElements->length;
            ShipElement choosen = seenEnemyElements->ptr[randEl];

            printf("SHOOT %d %d\n", choosen.pos.y, choosen.pos.x);
        } else {
            int randY, randX;
            int shootingAtOwnShip = false;

            do {
                randY = rand() % game->planeSizeY;
                randX = rand() % game->planeSizeX;
                ShipElementVec* ownElements = (ShipElementVec*) malloc(sizeof(ShipElementVec));
                initShipElementVec(ownElements);
                getShipElementsOfPlayer(aiPlayer, ownElements);

                for(int elI = 0; elI < ownElements->length; elI++) {
                    ShipElement el = ownElements->ptr[elI];
                    if(el.pos.y == randY && el.pos.x == randX) {
                        shootingAtOwnShip = true;
                    }
                }

                free(ownElements->ptr);
                free(ownElements);
            } while(shootingAtOwnShip);

            printf("SHOOT %d %d\n", randY, randX);
        }

        free(seenEnemyElements->ptr);
        free(seenEnemyElements);
    }
}

void handleAI(Game* game) {
    Game* copyOfGame = (Game*) malloc(sizeof(Game));
    copyGame(copyOfGame, game);
    copyOfGame->nextPlayerIndex = !copyOfGame->nextPlayerIndex;

    int aiPlayerIndex = !copyOfGame->nextPlayerIndex;
    Player* aiPlayerCp = copyOfGame->players[aiPlayerIndex];

    srand(game->randomSeed);
    saveGame(copyOfGame);

    char playerX = getCharOfPlayerIndex(aiPlayerIndex);
    printf("[state]\nPRINT 0\n[state]\n");
    printf("[player%c]\n", playerX);

    aiPlaceShips(aiPlayerCp, copyOfGame);
    aiShoot(aiPlayerIndex, copyOfGame);
//    aiMove(aiPlayerIndex, copyOfGame);

    printf("[player%c]\n", playerX);
    printf("[state]\nPRINT 0\n[state]\n");
    game->shouldEnd = true;
    freeGame(copyOfGame);
}
