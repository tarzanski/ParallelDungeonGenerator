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
#include <limits>
#include <algorithm>

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
            /*
            for (int j = 0; j < numRooms; j++) {
                float step_x = rooms[i].center.x / 10;
                float step_y = rooms[i].center.y / 10;
                point_t new_center;
                new_center.x = rooms[i].center.x + step_x;
                new_center.y = rooms[i].center.y + step_y;
                rooms[i].center = new_center;
            }
            */
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

bool edgeLT(edge_t a, edge_t b) {
    return a.dist < b.dist;
}

int findSubset(int a, int *parentMap) {
    if (parentMap[a] == -1)
        return a;
    return findSubset(parentMap[a], parentMap);
}

// Return list of (unnecessarily directed) edges that form minimum spanning tree
edge_t *findMinimumSpanningTree(edge_t *allEdges, int numMainRooms, int numVertices, int numEdges) {
    // Note: size of allEdges is numEdges * 2
    std::sort(allEdges, allEdges + numEdges, edgeLT);

    int numSpanningEdges = 0;
    edge_t *mst = (edge_t *)calloc(numVertices - 1, sizeof(edge_t));
    int *parentMap = (int *)malloc(sizeof(int) * numVertices);
    // Initialize the union find thing
    for (int i = 0; i < numVertices; i++) {
        parentMap[i] = -1;
    }
    for (int i = 0; i < numEdges; i++) {
        // Check if done
        if (numSpanningEdges == numMainRooms - 1)
            break;

        int src = allEdges[i].src;
        int dest = allEdges[i].dest;
        int parentSrc = findSubset(src, parentMap);
        int parentDest = findSubset(dest, parentMap);
        if (parentSrc == parentDest) {
            continue;
        }
        mst[numSpanningEdges] = {src, dest, allEdges[i].dist};
        numSpanningEdges += 1;
        parentMap[parentDest] = parentSrc;
    }
    free(parentMap);
    return mst;
}

/*
 * Top function of sequential algorithm, called by main in main.cpp
 */
rectangle_t *generate(int numRooms, int radius) {
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
    rectangle_t *rooms = (rectangle_t *)calloc(numRooms, sizeof(rectangle_t));
    rectangle_t *main_rooms = (rectangle_t *)malloc(sizeof(rectangle_t) * numRooms);

    // generate list of rooms, add each to 1-d list
    int main_index = 0;
    int *mainIndexToIndex = (int *)calloc(numRooms, sizeof(int));
    for (int i = 0; i < numRooms; i++) {
        point_t center = getRandomPointInCircle(radius);
        rooms[i].center = center;
        rooms[i].neighbors = (float *)calloc(numRooms, sizeof(float));
        for (int j = 0; j < numRooms; j++)
            rooms[i].neighbors[j] = std::numeric_limits<float>::infinity();
        while (rooms[i].width < min_width) {
            rooms[i].width = width_distribution(generator);
        }
        while (rooms[i].height < min_height) {
            rooms[i].height = height_distribution(generator);
        }
        if (rooms[i].width > 1.25 * mean_width && rooms[i].height > 1.25 * mean_height) {
            main_rooms[main_index] = rooms[i];
            mainIndexToIndex[main_index] = i;
            main_index += 1;
        }
    }

    printf("There are %d main rooms\n", main_index);

    // separation steering
    separateRooms(rooms, numRooms);

    // Get center points of main rooms
    float *pointList = (float *)calloc(main_index * 2, sizeof(float));
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
    for (int i = 0; i < numRooms; i++) {
        point_t center = rooms[i].center;
        printf("center: %f, %f\n", center.x, center.y);
        printf("width: %f, height: %f\n", rooms[i].width, rooms[i].height);
        printf("is overlapping: %d\n", isOverlapping(rooms, numRooms, i));
    }
    */

    // Directed edges
    edge_t *allEdges = (edge_t *)calloc(numTriangleVertices * 2, sizeof(edge_t));
    for (int i = 0; i < (numTriangleVertices * 2); i++)
        allEdges[i].dist = std::numeric_limits<float>::infinity();

    int triangleCounter = 0;
    int vertices[3];
    printf("numTriangleVertices mod 3: %d\n", (numTriangleVertices % 3));

    // Set up the neighbors I guess?
    int edge_index = 0;
    for (int i = 0; i < numTriangleVertices; i++) {
        int vertex = triangleIndexList[i];
        vertices[triangleCounter] = mainIndexToIndex[vertex];
        triangleCounter += 1;
        if (triangleCounter == 3) {
            float dist_0_1 = sqrt(pow(rooms[vertices[0]].center.x - rooms[vertices[1]].center.x, 2)
                    + pow(rooms[vertices[0]].center.y - rooms[vertices[1]].center.y, 2));
            float dist_0_2 = sqrt(pow(rooms[vertices[0]].center.x - rooms[vertices[2]].center.x, 2)
                    + pow(rooms[vertices[0]].center.y - rooms[vertices[2]].center.y, 2));
            float dist_1_2 = sqrt(pow(rooms[vertices[1]].center.x - rooms[vertices[2]].center.x, 2)
                    + pow(rooms[vertices[1]].center.y - rooms[vertices[2]].center.y, 2));
            rooms[vertices[0]].neighbors[vertices[1]] = dist_0_1;
            rooms[vertices[1]].neighbors[vertices[0]] = dist_0_1;
            rooms[vertices[0]].neighbors[vertices[2]] = dist_0_2;
            rooms[vertices[2]].neighbors[vertices[0]] = dist_0_2;
            rooms[vertices[1]].neighbors[vertices[2]] = dist_1_2;
            rooms[vertices[2]].neighbors[vertices[1]] = dist_1_2;
            allEdges[edge_index++] = {vertices[0], vertices[1], dist_0_1};
            allEdges[edge_index++] = {vertices[1], vertices[0], dist_0_1};
            allEdges[edge_index++] = {vertices[0], vertices[2], dist_0_2};
            allEdges[edge_index++] = {vertices[2], vertices[0], dist_0_2};
            allEdges[edge_index++] = {vertices[1], vertices[2], dist_1_2};
            allEdges[edge_index++] = {vertices[2], vertices[1], dist_1_2};
            triangleCounter = 0;
        }
    }
    edge_t *mst = findMinimumSpanningTree(allEdges, main_index, numRooms, edge_index);
    for (int i = 0; i < main_index - 1; i++) {
        printf("src: %d, dest: %d\n", mst[i].src, mst[i].dest);
    }

    free(pointList);
    free(main_rooms);
    free(triangleIndexList);
    free(mainIndexToIndex);
    free(allEdges);
    free(mst);

    return rooms;
}
