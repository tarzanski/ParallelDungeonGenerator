
typedef struct {
    float x;
    float y;
} point_t;

typedef struct {
    point_t center;
    float height;
    float width;
} rectangle_t;

rectangle_t *generate(int numMainRooms, int radius);