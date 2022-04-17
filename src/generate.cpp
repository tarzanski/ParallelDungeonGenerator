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
#include "Clarkson-Delaunay.h"

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

// Move the centers of the rooms away from each other
void separateRooms(rectangle_t *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        rectangle_t *room_p = &rooms[i];
        float x = room_p->center.x;
        float y = room_p->center.y;
        float dist = sqrt(pow(x, 2) + pow(y, 2));
        float step_x = x / dist;
        float step_y = y / dist;
        while (isOverlapping(rooms, numRooms, i)) {
            point_t new_center;
            new_center.x = room_p->center.x + step_x;
            new_center.y = room_p->center.y + step_y;
            room_p->center = new_center;
        }
    }
}

// Check if a rectangle is overlapping any others
int isOverlapping(rectangle_t *rooms, int numRooms, int room_index) {
    float r_x = rooms[room_index].center.x;
    float r_y = rooms[room_index].center.y;
    float r_width = rooms[room_index].width;
    float r_height = rooms[room_index].height;
    float r_left = r_x - (r_width / 2);
    float r_right = r_x + (r_width / 2);
    float r_bottom = r_y + (r_height / 2);
    float r_top = r_y - (r_height / 2);
    for (int i = 0; i < numRooms; i++) {
        if (i == room_index)
            continue;
        float left = rooms[i].center.x - (rooms[i].width / 2);
        float right = rooms[i].center.x + (rooms[i].width / 2);
        float bottom = rooms[i].center.y + (rooms[i].height / 2);
        float top = rooms[i].center.y - (rooms[i].height / 2);
        if (right < r_left || r_right < left)
            continue;
        if (bottom < r_top || r_bottom < top)
            continue;

        return 1;
    }
    return 0;
}

/*
 * Top function of sequential algorithm, called by main in main.cpp
 */
rectangle_t *generate(int numMainRooms, int radius) {
    int totalRooms = numMainRooms * 10;
    int mean_width = 10;
    float min_width = 3;
    int stddev_width = 10;

    int mean_height = 10;
    float min_height = 3;
    int stddev_height = 10;

    std::default_random_engine generator;
    std::normal_distribution<float> width_distribution(mean_width, stddev_width);
    std::normal_distribution<float> height_distribution(mean_height, stddev_height);

    // create data structures
    rectangle_t *rooms = (rectangle_t *)malloc(sizeof(rectangle_t) * totalRooms);
    rectangle_t *main_rooms = (rectangle_t *)malloc(sizeof(rectangle_t) * totalRooms);

    // generate list of rooms, add each to 1-d list
    int main_index = 0;
    for (int i = 0; i < totalRooms; i++) {
        point_t center = getRandomPointInCircle(radius);
        rooms[i].center = center;
        while (rooms[i].width < min_width) {
            rooms[i].width = std::max(min_width, width_distribution(generator));
        }
        while (rooms[i].height < min_height) {
            rooms[i].height = std::max(min_height, height_distribution(generator));
        }
        if (rooms[i].width > 1.25 * mean_width && rooms[i].height > 1.25) {
            main_rooms[main_index] = rooms[i];
            main_index += 1;
        }
    }

    printf("There are %d main rooms\n", main_index);

    // separation steering
    separateRooms(rooms, totalRooms);

    // Get center points of main rooms
    float *pointList = (float *)malloc(sizeof(float) * main_index * 2);
    for (int i = 0; i < main_index; i++) {
        pointList[i * 2] = main_rooms[i].center.x;
        pointList[i * 2 + 1] = main_rooms[i].center.y;
    }

    int numTriangleVertices;
    int *triangleIndexList = BuildTriangleIndexList(
            (void *)pointList,
            (float)RAND_MAX,
            main_index,
            2,
            0,
            &numTriangleVertices);

    /*
    for (int i = 0; i < totalRooms; i++) {
        point_t center = rooms[i].center;
        printf("center: %f, %f\n", center.x, center.y);
        printf("width: %f, height: %f\n", rooms[i].width, rooms[i].height);
        printf("is overlapping: %d\n", isOverlapping(rooms, totalRooms, i));
    }
    */

    for (int i = 0; i < numTriangleVertices; i++) {
        printf("%d\n", triangleIndexList[i]);
    }
    free(pointList);
    free(triangleIndexList);

    return rooms;
}
