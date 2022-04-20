
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
} rectangle_t;

typedef struct {
    point_t start;
    point_t middle;
    point_t end;
} hallway_t;

typedef struct {
    int numRooms;
    int numMainRooms;
    int numHallways;
    rectangle_t *rooms;
    int *mainRoomIndices;
    hallway_t *hallways;
} dungeon_t;

typedef struct {
    edge_t* dela;
    edge_t* mst;
    int dela_edges;
    int mst_edges;
} double_edge_t;

point_t getRandomPointInCircle(float radius);
int isOverlapping(rectangle_t *rooms, int i1, int i2);
int anyOverlapping(rectangle_t *rooms, int numRooms);

/* Initializes rooms, numRooms, mainRoomIndices, and numMainRooms */
void generate(dungeon_t *dungeon, int numRooms, int radius);

/* Separates room centers */
void separateRooms(dungeon_t *dungeon);

/* Initializes hallways and numHallways */
double_edge_t *constructHallways(dungeon_t *dungeon);
