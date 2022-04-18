
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
    float *neighbors;  // -1 for unconnected, otherwise weight
} rectangle_t;

point_t getRandomPointInCircle(float radius);
void separateRooms(rectangle_t *rooms, int numRooms);
int isOverlapping(rectangle_t *rooms, int numRooms, int room_index);
rectangle_t *generate(int numMainRooms, int radius);
