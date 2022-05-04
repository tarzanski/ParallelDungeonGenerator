
typedef struct {
    float x;
    float y;
} point_t;

typedef struct {
    int src;
    int dest;
    float dist;
} edge_t;

typedef struct {
    point_t center;
    float height;
    float width;
    char status;
} rectangle_t;

typedef struct {
    point_t start;
    point_t middle;
    point_t end;
} hallway_t;

typedef struct {
    rectangle_t *rooms;
    hallway_t *hallways;
    int numRooms;
    int numMainRooms;
    int numHallways;
    int *mainRoomIndices;
    int numIters;
} dungeon_t;

typedef struct {
    edge_t* dela;
    edge_t* mst;
    int dela_edges;
    int mst_edges;
} double_edge_t;

// #ifdef __cplusplus
// extern "C" {
// #endif // __cplusplus
//     extern void anyOverlapping_ispc(rectangle_t* rooms, int numRooms);
// #ifdef __cplusplus
// }
// #endif // __cplusplus

point_t getRandomPointInCircle(float radius);
int isOverlapping(rectangle_t *rooms, int i1, int i2);
int anyOverlapping(rectangle_t *rooms, int numRooms);

/* Initializes rooms, numRooms, mainRoomIndices, and numMainRooms */
void generate(dungeon_t *dungeon, int numRooms, int radius);

/* Separates room centers */
void separateRooms(dungeon_t *dungeon, rectangle_t **room_data, int animate);

/* Initializes hallways and numHallways */
double_edge_t *constructHallways(dungeon_t *dungeon);

/* finds non-main rooms to include in final generation */
void getIncludedRooms(dungeon_t* dungeon);
