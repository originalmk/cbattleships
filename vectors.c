//
// Created by maciej on 04.12.2021.
//
#include <stdlib.h>
#include <string.h>
#include "vectors.h"

void initPointVec(PointVec* newPointVec) {
    newPointVec->capacity = 1;
    newPointVec->length = 0;
    newPointVec->ptr = (Point*) malloc(sizeof(Point));
}

void pointVecEnlargeIfNeeded(PointVec* vec) {
    // Vector is full
    if(vec->length == vec->capacity) {
        int newCapacity = vec->capacity * 2;
        Point* newPtr = (Point*) malloc(sizeof(Point) * newCapacity);
        memcpy(newPtr, vec->ptr, vec->length * sizeof(Point));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void pointVecPushBack(PointVec* vec, Point p) {
    pointVecEnlargeIfNeeded(vec);
    vec->ptr[vec->length++] = p;
}

void pointVecShrinkIfNeeded(PointVec* vec) {
    // Vector is size of 2x its length
    if(vec->capacity == 2 * vec->length) {
        int newCapacity = vec->capacity/2;
        Point* newPtr = (Point*) malloc(newCapacity * sizeof(Point));
        memcpy(newPtr, vec->ptr, vec->length * sizeof(Point));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void pointVecPopBack(PointVec* vec) {
    vec->length--;
    pointVecShrinkIfNeeded(vec);
}

void pointVecReset(PointVec* vec) {
    free(vec->ptr);
    initPointVec(vec);
}

// ======================================================================

void initShipVec(ShipVec* newShipVec) {
    newShipVec->capacity = 1;
    newShipVec->length = 0;
    newShipVec->ptr = (Ship**) malloc(sizeof(Ship*));
}

void shipVecEnlargeIfNeeded(ShipVec* vec) {
    // Vector is full
    if(vec->length == vec->capacity) {
        int newCapacity = vec->capacity * 2;
        Ship** newPtr = (Ship**) malloc(sizeof(Ship*) * newCapacity);
        memcpy(newPtr, vec->ptr, vec->length * sizeof(Ship*));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void shipVecPushBack(ShipVec* vec, Ship* s) {
    shipVecEnlargeIfNeeded(vec);
    vec->ptr[vec->length++] = s;
}

void shipVecShrinkIfNeeded(ShipVec* vec) {
    // Vector is size of 2x its length
    if(vec->capacity == 2 * vec->length) {
        int newCapacity = vec->capacity/2;
        Ship** newPtr = (Ship**) malloc(newCapacity * sizeof(Ship*));
        memcpy(newPtr, vec->ptr, vec->length * sizeof(Ship*));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void shipVecPopBack(ShipVec* vec) {
    vec->length--;
    shipVecShrinkIfNeeded(vec);
}

void shipVecReset(ShipVec* vec) {
    free(vec->ptr);
    initShipVec(vec);
}

void initShipElementVec(ShipElementVec* newShipElementVec) {
    newShipElementVec->capacity = 1;
    newShipElementVec->length = 0;
    newShipElementVec->ptr = (ShipElement*) malloc(sizeof(ShipElement));
}

void shipElementVecEnlargeIfNeeded(ShipElementVec* vec) {
    // Vector is full
    if(vec->length == vec->capacity) {
        int newCapacity = vec->capacity * 2;
        ShipElement* newPtr = (ShipElement*) malloc(sizeof(ShipElement) * newCapacity);
        memcpy(newPtr, vec->ptr, vec->length * sizeof(ShipElement));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void shipElementVecPushBack(ShipElementVec* vec, ShipElement s) {
    shipElementVecEnlargeIfNeeded(vec);
    vec->ptr[vec->length++] = s;
}

void shipElementVecShrinkIfNeeded(ShipElementVec* vec) {
    // Vector is size of 2x its length
    if(vec->capacity == 2 * vec->length) {
        int newCapacity = vec->capacity/2;
        ShipElement* newPtr = (ShipElement*) malloc(newCapacity * sizeof(ShipElement));
        memcpy(newPtr, vec->ptr, vec->length * sizeof(ShipElement));
        free(vec->ptr);
        vec->ptr = newPtr;
        vec->capacity = newCapacity;
    }
}

void shipElementVecPopBack(ShipElementVec* vec) {
    vec->length--;
    shipElementVecShrinkIfNeeded(vec);
}

void shipElementVecReset(ShipElementVec* vec) {
    free(vec->ptr);
    initShipElementVec(vec);
}