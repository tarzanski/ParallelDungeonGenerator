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
#include <chrono>
#include <cstdint>

#include "generate.h"
#include "main.h"
#ifdef ISPC
#include "ispc/objs/generate_ispc.h"
#endif
#include "Clarkson-Delaunay.h"

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
void separateRooms(dungeon_t *dungeon, rectangle_t **room_data, int animate) {
    // extra timing code for analysis
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    
    rectangle_t *rooms = dungeon->rooms;
    int num_iters = 0;

    auto start = Clock::now();
    double anyoverlap_time = 0; 

#ifdef ISPC
    using namespace ispc;
    while (anyOverlapping_ispc(
#ifdef VSTUDIO
        (ispc::$anon1*)
#else
        (ispc::$anon3*)
#endif
        rooms, dungeon->numRooms)) {
#else
    while (anyOverlapping(rooms, dungeon->numRooms)) {
#endif
        anyoverlap_time += std::chrono::duration_cast<dsec>(Clock::now() - start).count();
        //printf("anyOverlapping Time: %lfs\n", anyoverlap_time);
        
        if (num_iters >= MAX_ITERS) {
            printf("Did not converge in %d iterations\n", num_iters);
            return;
        }
#ifdef ISPC
        separate_ispc_withtasks(
#ifdef VSTUDIO // workaround to match typing with ISPC internals
            (ispc::$anon1*)
#else            
            (ispc::$anon3*)
#endif
            rooms, dungeon->numRooms);
#else
        for (int i = 0; i < dungeon->numRooms; i++) {
            for (int j = 0; j < dungeon->numRooms; j++) {
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

                    // I suppose its possible for the centers to be exactly equal.
                    if (step_x == 0.0f)
                        step_x = 1.0f;
                    if (step_y == 0.0f)
                        step_y = 1.0f;

                    rooms[i].center.x -= step_x;
                    rooms[i].center.y -= step_y;
                    rooms[j].center.x += step_x;
                    rooms[j].center.y += step_y;
                }
            }
        }
#endif

        // storing room data if animating seperation
        if (animate == 1) {
            rectangle_t *iter_room_data = (rectangle_t*)malloc(sizeof(rectangle_t) * dungeon->numRooms);
            memcpy(iter_room_data, dungeon->rooms, sizeof(rectangle_t) * dungeon->numRooms);
            room_data[num_iters] = iter_room_data;
        }

        num_iters += 1;
        start = Clock::now();
    }
    printf("Average anyOverlapping Time = %lfs\n",anyoverlap_time / num_iters);
    printf("Converged in %d iterations\n", num_iters);
    dungeon->numIters = num_iters;
    if (anyOverlapping(rooms, dungeon->numRooms) == 0)
        printf("No Overlap Verified by Sequential anyOverlapping\n");
    else
        printf("SEQUENTIAL anyOverlapping RETURNED 0\n");
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
        pExtras = 0.0f;

    std::sort(allEdges, allEdges + numEdges, edgeLT);

    int numAddedEdges = 0;  // Total number of edges to use in the dungeon
    int numSpanningEdges = 0;  // Number of edges that form the MST
    edge_t *mst = (edge_t *)calloc(numEdges, sizeof(edge_t));
    int *parentMap = (int *)malloc(sizeof(int) * numVertices);

    // Structure for checking if an edge has already been added
    int *adjMatrix = (int*)calloc(numEdges * numEdges, sizeof(int));

    // Initialize the union find thing
    for (int i = 0; i < numVertices; i++) {
        parentMap[i] = -1;
    }
    for (int i = 0; i < numEdges; i++) {
        int src = allEdges[i].src;
        int dest = allEdges[i].dest;
        if (adjMatrix[dest + numEdges * src] || adjMatrix[src + numEdges * dest])
            continue;
        int parentSrc = findSubset(src, parentMap);
        int parentDest = findSubset(dest, parentMap);
        if (parentSrc == parentDest) {
            // Chance of adding an extra edge
            float roll = (double)rand() / (double)RAND_MAX;
            if (roll < pExtras) {
                mst[numAddedEdges] = {src, dest, allEdges[i].dist};
                numAddedEdges += 1;
                adjMatrix[dest + numEdges * src] = 1;
                adjMatrix[src + numEdges * dest] = 1;
            }
            continue;
        }
        mst[numAddedEdges] = {src, dest, allEdges[i].dist};
        adjMatrix[dest + numEdges * src] = 1;
        adjMatrix[src + numEdges * dest] = 1;
        numAddedEdges += 1;
        numSpanningEdges += 1;
        parentMap[parentDest] = parentSrc;
    }
    *numAddedEdges_p = numAddedEdges;
    free(parentMap);
    free(adjMatrix);
    return mst;
}

/*
 * Top function of sequential algorithm, called by main in main.cpp
 */
void generate(dungeon_t *dungeon, int numRooms, int radius) {
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
    rectangle_t *mainRooms = (rectangle_t *)malloc(sizeof(rectangle_t) * numRooms);
    dungeon->rooms = rooms;
    dungeon->numRooms = numRooms;

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
            mainRooms[main_index] = rooms[i];
            mainIndexToIndex[main_index] = i;
            main_index += 1;
        }
    }
    printf("There are %d main rooms\n", main_index);
    dungeon->mainRoomIndices = mainIndexToIndex;
    dungeon->numMainRooms = main_index;
    free(mainRooms);
}

double_edge_t* constructHallways(dungeon_t *dungeon) {
    rectangle_t *rooms = dungeon->rooms;

    // Get center points of main rooms
    float *pointList = (float *)calloc(dungeon->numMainRooms * 2, sizeof(float));
    for (int i = 0; i < dungeon->numMainRooms; i++) {
        pointList[i * 2] = dungeon->rooms[dungeon->mainRoomIndices[i]].center.x;
        pointList[i * 2 + 1] = dungeon->rooms[dungeon->mainRoomIndices[i]].center.y;
    }

    int numTriangleVertices;

    // Call Delaunay function
    int *triangleIndexList = BuildTriangleIndexList(
            (void *)pointList,
            (float)RAND_MAX,
            dungeon->numMainRooms,
            2,
            0,
            &numTriangleVertices);

    // Construct directed edges
    edge_t *allEdges = (edge_t *)calloc(numTriangleVertices * 2, sizeof(edge_t));
    for (int i = 0; i < (numTriangleVertices * 2); i++)
        allEdges[i].dist = std::numeric_limits<float>::infinity();

    int triangleCounter = 0;
    int vertices[3];
    printf("numTriangleVertices mod 3: %d\n", (numTriangleVertices % 3));

    int edge_index = 0;
    for (int i = 0; i < numTriangleVertices; i++) {
        int vertex = triangleIndexList[i];
        vertices[triangleCounter] = dungeon->mainRoomIndices[vertex];
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
    edge_t *mst = findMinimumSpanningTree(allEdges, dungeon->numMainRooms, dungeon->numRooms, edge_index, P_EXTRA, &numAddedEdges);
    // for (int i = 0; i < numAddedEdges; i++) {
    //     printf("src: %d, dest: %d\n", mst[i].src, mst[i].dest);
    // }

    // Construct hallway points
    hallway_t *hallways = (hallway_t *)calloc(numAddedEdges, sizeof(hallway_t));
    for(int i = 0; i < numAddedEdges; i++) {
        rectangle_t src_room = rooms[mst[i].src];
        rectangle_t dest_room = rooms[mst[i].dest];

        float src_left = round(src_room.center.x - (src_room.width / 2));
        float src_right = round(src_room.center.x + (src_room.width / 2));
        float src_top = round(src_room.center.y - (src_room.height / 2));
        float src_bottom = round(src_room.center.y + (src_room.height / 2));

        float dest_left = round(dest_room.center.x - (dest_room.width / 2));
        float dest_right = round(dest_room.center.x + (dest_room.width / 2));
        float dest_top = round(dest_room.center.y - (dest_room.height / 2));
        float dest_bottom = round(dest_room.center.y + (dest_room.height / 2));

        // Some are going to be duplicates due to directed graph and due to triangle construction, need to remove
        float mid_x = ((src_room.center.x + dest_room.center.x) / 2);
        float mid_y = round((src_room.center.y + dest_room.center.y) / 2);

        if (src_left < mid_x && mid_x < src_right && dest_left < mid_x && mid_x < dest_right) {
            hallways[i].start = {mid_x, src_room.center.y};
            hallways[i].middle = {mid_x, dest_room.center.y};
            hallways[i].end = {mid_x, dest_room.center.y};
        }
        else if (src_top < mid_y && mid_y < src_bottom && dest_top < mid_y && mid_y < dest_bottom) {
            hallways[i].start = {src_room.center.x, mid_y};
            hallways[i].middle = {dest_room.center.x, mid_y};
            hallways[i].end = {dest_room.center.x, mid_y};
        }
        else {
            hallways[i].start = {src_room.center.x, src_room.center.y};
            hallways[i].middle = {src_room.center.x, dest_room.center.y};
            hallways[i].end = {dest_room.center.x, dest_room.center.y};
        }
    }

    dungeon->hallways = hallways;
    dungeon->numHallways = numAddedEdges;

    free(pointList);
    free(triangleIndexList);
    // free(allEdges);
    // free(mst);
    double_edge_t *mst_dela = (double_edge_t*)malloc(sizeof(double_edge_t));
    mst_dela->dela = allEdges;
    mst_dela->mst = mst;
    mst_dela->dela_edges = numTriangleVertices * 2;
    mst_dela->mst_edges = numAddedEdges;
    return mst_dela;
}

// helper function for finding if hallway is within bounds of a room
// takes in two hallway points, could be either start --> middle or middle --> end
int checkBounds(float topLeftx, float topLefty, float botRightx, float botRighty, 
                point_t* start, point_t* end) {
    if (start->x == end->x && (start->y != end->y)) {
        float highery = (start->y > end->y) ? start->y : end->y;
        float lowery = (start->y > end->y) ? end->y : start->y;
        if (start->x > topLeftx && start->x < botRightx && 
            ((topLefty > lowery && topLefty < highery) || (botRighty > lowery && botRighty < highery))) {
            // printf("    TLx: %f, BRx %f, Hx: %f\n", topLeftx, botRightx, start->x);
            // printf("found vertical ");
            return 1;
        }
    }
    if (start->y == end->y && (start->x != end->x)) {
        float higherx = (start->x > end->x) ? start->x : end->x;
        float lowerx = (start->x > end->x) ? end->x : start->x;
        if (start->y > topLefty && start->y < botRighty && 
            ((topLeftx > lowerx && topLeftx < higherx) || (botRightx > lowerx && botRightx < higherx))) {
            // printf("    TLy: %f, BRy %f, Hy: %f\n", topLefty, botRighty, start->y);
            // printf("    TLx: %f, BRx %f, Hxs: %f, Hxe: %f\n", topLeftx, botRightx, start->x, end->x);
            // printf("found horizontal ");
            return 1;
        }
    }
    return 0;
}

// function to find set of non-main rooms that overlap with hallways.
// MainRoomIndices array is ordered, can use that to avoid O(n) lookup
void getIncludedRooms(dungeon_t* dungeon) {
    //printf("***************************************************\n");
    
    int mainRoomIndex = 0;
    for (int roomNum = 0; roomNum < dungeon->numRooms; roomNum++) {
        // check if next main room
        // can be done since main room index array is ordered, O(1) vs O(n)
        if (roomNum == dungeon->mainRoomIndices[mainRoomIndex]) {
            mainRoomIndex++;
            // setting bit to be included
            dungeon->rooms[roomNum].status += BIT_INCLUDED;
            dungeon->rooms[roomNum].status += BIT_MAINROOM;
            continue;
        }
        //printf("checking halls for room %d\n", roomNum);

        float topLeftx = dungeon->rooms[roomNum].center.x - dungeon->rooms[roomNum].width/2;
        float topLefty = dungeon->rooms[roomNum].center.y - dungeon->rooms[roomNum].height/2;
        float botRightx = dungeon->rooms[roomNum].center.x + dungeon->rooms[roomNum].width/2;
        float botRighty = dungeon->rooms[roomNum].center.y + dungeon->rooms[roomNum].height/2;
        
        dungeon->rooms[roomNum].status = 0;

        // looping over all hallways
        for (int hallwayNum = 0; hallwayNum < dungeon->numHallways; hallwayNum++) {
            hallway_t* hall = &dungeon->hallways[hallwayNum];

            // check if room is within bounds of hallway, either of two edges
            // two conditions for each

            // start --> middle
            if (checkBounds(topLeftx, topLefty, botRightx, botRighty, &hall->start, &hall->middle)) {
                dungeon->rooms[roomNum].status += BIT_INCLUDED;
                //printf("%d\n",hallwayNum);
                break;
            }

            if (checkBounds(topLeftx, topLefty, botRightx, botRighty, &hall->middle, &hall->end)) {
                // included could already be set
                if (!(dungeon->rooms[roomNum].status & BIT_INCLUDED))
                    dungeon->rooms[roomNum].status += BIT_INCLUDED;
                //printf("%d\n",hallwayNum);
                break;
            }
        }

        // if (dungeon->rooms[roomNum].include == 0)
        //     printf("no intersections found for %d\n", roomNum);
        
    }
    // printf("***************************************************\n");
}
