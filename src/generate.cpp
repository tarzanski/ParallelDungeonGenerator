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

#define MAX_ITERS 1000
#define P_EXTRA 0.10

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
    p.x = round(radius * r * cos(t));
    p.y = round(radius * r * sin(t));
    return p;
}

// Move the centers of the rooms away from each other
// stackoverflow.com/questions/70806500/separation-steering-algorithm-for-separationg-set-of-rectangles/
int separateRooms(rectangle_t *rooms, int numRooms) {
    int num_iters = 0;
    while (anyOverlapping(rooms, numRooms)) {
        if (num_iters >= MAX_ITERS)
            return 1;
        for (int i = 0; i < numRooms; i++) {
            for (int j = 0; j < numRooms; j++) {
                if (i == j)
                    continue;
                if (isOverlapping(rooms, i, j)) {
                    float step_x = rooms[j].center.x - rooms[i].center.x;
                    float step_y = rooms[j].center.y - rooms[i].center.y;
                    float dist = sqrt(pow(step_x, 2) + pow(step_y, 2));
                    if (round(dist) == 0)
                        dist = 0.001f;
                    step_x /= dist;
                    step_y /= dist;
                    step_x = round(step_x);
                    step_y = round(step_y);
                    rooms[i].center.x -= step_x;
                    rooms[i].center.y -= step_y;
                    rooms[j].center.x += step_x;
                    rooms[j].center.y += step_y;
                    // I suppose its possible for the centers to be exactly equal.
                    // Not handled here.
                }
            }
        }
        num_iters += 1;
    }
    printf("Converged: num iters for convergence: %d\n", num_iters);
    return 0;
}

int isOverlapping(rectangle_t *rooms, int i1, int i2) {
    if (i1 == i2)
        return 0;
    float left1 = rooms[i1].center.x - (rooms[i1].width / 2);
    float right1 = rooms[i1].center.x + (rooms[i1].width / 2);
    float bottom1 = rooms[i1].center.y + (rooms[i1].height / 2);
    float top1 = rooms[i1].center.y - (rooms[i1].height / 2);

    float left2 = rooms[i2].center.x - (rooms[i2].width / 2);
    float right2 = rooms[i2].center.x + (rooms[i2].width / 2);
    float bottom2 = rooms[i2].center.y + (rooms[i2].height / 2);
    float top2 = rooms[i2].center.y - (rooms[i2].height / 2);
    if (right1 < left2 || right2 < left1)
        return 0;
    if (bottom1 < top2 || bottom2 < top1)
        return 0;
    return 1;
}

// Check if a rectangle is overlapping any others
int anyOverlapping(rectangle_t *rooms, int numRooms) {
    for (int i = 0; i < numRooms; i++) {
        for (int j = 0; j < numRooms; j++) {
            if (isOverlapping(rooms, i, j))
                return 1;
        }
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
edge_t *findMinimumSpanningTree(edge_t *allEdges, int numMainRooms, int numVertices, int numEdges, float pExtras, int *numAddedEdges_p) {
    if (pExtras < 0.0f || pExtras > 1.0f)
        return NULL;

    // Note: size of allEdges is numEdges * 2
    std::sort(allEdges, allEdges + numEdges, edgeLT);

    int numAddedEdges = 0;  // Total number of edges to use in the dungeon
    int numSpanningEdges = 0;  // Number of edges that form the MST
    edge_t *mst = (edge_t *)calloc(numEdges, sizeof(edge_t));
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
            // Chance of adding an extra edge
            float roll = (double)rand() / (double)RAND_MAX;
            if (roll < pExtras) {
                mst[numAddedEdges] = {src, dest, allEdges[i].dist};
                numAddedEdges += 1;
            }
            continue;
        }
        mst[numAddedEdges] = {src, dest, allEdges[i].dist};
        numAddedEdges += 1;
        numSpanningEdges += 1;
        parentMap[parentDest] = parentSrc;
    }
    *numAddedEdges_p = numAddedEdges;
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

    srand(100);
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
        while (rooms[i].width < min_width) {
            rooms[i].width = round(width_distribution(generator));
        }
        while (rooms[i].height < min_height) {
            rooms[i].height = round(height_distribution(generator));
        }
        if (rooms[i].width > 1.25 * mean_width && rooms[i].height > 1.25 * mean_height) {
            main_rooms[main_index] = rooms[i];
            mainIndexToIndex[main_index] = i;
            main_index += 1;
        }
    }


    // separation steering
    if (separateRooms(rooms, numRooms))
        printf("Separation reached max iters");

    printf("There are %d main rooms\n", main_index);

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

    // Directed edges
    edge_t *allEdges = (edge_t *)calloc(numTriangleVertices * 2, sizeof(edge_t));
    for (int i = 0; i < (numTriangleVertices * 2); i++)
        allEdges[i].dist = std::numeric_limits<float>::infinity();

    int triangleCounter = 0;
    int vertices[3];
    printf("numTriangleVertices mod 3: %d\n", (numTriangleVertices % 3));

    // Set up the edges I guess?
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
            allEdges[edge_index++] = {vertices[0], vertices[1], dist_0_1};
            allEdges[edge_index++] = {vertices[1], vertices[0], dist_0_1};
            allEdges[edge_index++] = {vertices[0], vertices[2], dist_0_2};
            allEdges[edge_index++] = {vertices[2], vertices[0], dist_0_2};
            allEdges[edge_index++] = {vertices[1], vertices[2], dist_1_2};
            allEdges[edge_index++] = {vertices[2], vertices[1], dist_1_2};
            triangleCounter = 0;
        }
    }

    // Find MST + a few extra edges
    int numAddedEdges = 0;
    edge_t *mst = findMinimumSpanningTree(allEdges, main_index, numRooms, edge_index, P_EXTRA, &numAddedEdges);
    for (int i = 0; i < numAddedEdges; i++) {
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
