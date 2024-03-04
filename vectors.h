//
// Created by maciej on 04.12.2021.
//

#ifndef CBATTLESHIPS_VECTORS_H
#define CBATTLESHIPS_VECTORS_H

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int capacity;
    int length;
    Point* ptr;
} PointVec;

enum Direction {
    N='N', W='W', S='S', E='E'
};

typedef struct {
    Point headPos;
    enum Direction direction;
    char shots;
    int isPlaced;
    int size;
    int timesMoved;
    int shotThisTurn;
    PointVec spyPlanes;
    int ID;
} Ship;

typedef struct {
    int capacity;
    int length;
    Ship** ptr;
} ShipVec;

typedef struct {
    Point pos;
    int nth;
    Ship* ship;
} ShipElement;

typedef struct {
    int capacity;
    int length;
    ShipElement* ptr;
} ShipElementVec;

void initPointVec(PointVec* newPointVec);
void pointVecEnlargeIfNeeded(PointVec* vec);
void pointVecPushBack(PointVec* vec, Point p);
void pointVecShrinkIfNeeded(PointVec* vec);
void pointVecPopBack(PointVec* vec);
void pointVecReset(PointVec* vec);

void initShipVec(ShipVec* newShipVec);
void shipVecEnlargeIfNeeded(ShipVec* vec);
void shipVecPushBack(ShipVec* vec, Ship* s);
void shipVecShrinkIfNeeded(ShipVec* vec);
void shipVecPopBack(ShipVec* vec);
void shipVecReset(ShipVec* vec);

void initShipElementVec(ShipElementVec* newShipElementVec);
void shipElementVecEnlargeIfNeeded(ShipElementVec* vec);
void shipElementVecPushBack(ShipElementVec* vec, ShipElement s);
void shipElementVecShrinkIfNeeded(ShipElementVec* vec);
void shipElementVecPopBack(ShipElementVec* vec);
void shipElementVecReset(ShipElementVec* vec);

#endif //CBATTLESHIPS_VECTORS_H
