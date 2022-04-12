/*
 * Sequential version of the Parallel Dungeon Generation algorithm
 *
 * Algorithm taken from:
 * https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm
 */

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>

#include "generate.h"

// Get random point in a circle of a certain radius
point_t getRandomPointInCircle(float radius) {
    float t = 2 * M_PI * ((double)rand() / (double)RAND_MAX);
    float u = ((double)rand() / (double)RAND_MAX) + ((double)rand() / (double)RAND_MAX);
    float r = 0;
    if (u > 1){
        r = 2 - u;
    }
    else {
        r = u;
    }
    point_t p;
    p.x = radius * r * cos(t);
    p.y = radius * r * sin(t);
    return p;
}

/* 
 * Top function of sequential algorithm, called by main in main.cpp
 */
rectangle_t *generate(int numMainRooms, int radius) {
    int totalRooms = numMainRooms * 10;
    int mean_width = 10.0;
    int stddev_width = 10.0;

    int mean_height = 10.0;
    int stddev_height = 10.0;

    std::default_random_engine generator;
    std::normal_distribution<double> width_distribution(mean_width, stddev_width);
    std::normal_distribution<double> height_distribution(mean_height, stddev_height);
    
    // create data structures
    rectangle_t *rooms = (rectangle_t *)malloc(sizeof(rectangle_t) * totalRooms);

    // generate list of rooms, add each to 1-d list
    for (int i = 0; i < totalRooms; i++) {
        point_t center = getRandomPointInCircle(radius);
        rooms[i].center = center;
        rooms[i].width = width_distribution(generator);
        rooms[i].height = height_distribution(generator);
        printf("center: %f, %f\n", center.x, center.y);
        printf("width: %f, height: %f\n", rooms[i].width, rooms[i].height);
    }

    //free(rooms);
    return rooms;
}