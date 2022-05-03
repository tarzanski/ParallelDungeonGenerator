
// struct typedefs are entirely uniform
// one struct exists for the entire program
typedef uniform struct {
    uniform float x;
    uniform float y;
} point_t;

typedef uniform struct {
    uniform point_t center;
    uniform float height;
    uniform float width;
    uniform int8 status;
} rectangle_t;

// copy of isOverlapping from generate.cpp
static inline int isOverlapping(uniform rectangle_t rooms[], int i1, int i2, uniform int numRooms) {
    // need to check if outside range (ispc only)
    if (i1 >= numRooms || i2 >= numRooms)
        return 0;
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

// ispc version of function that finds if any rooms in room array are 
// overlapping
export uniform int anyOverlapping_ispc(uniform rectangle_t rooms[], uniform int numRooms) {
    uniform int overlap = 0;
    for (uniform int i = 0; i < numRooms; i++) {
        for (uniform int j = 0; j < numRooms; j += programCount) {
            if(isOverlapping(rooms, i, j + programIndex, numRooms)) {
                overlap = 1;
                return overlap;
            }
            if (overlap == 1)
                return overlap;
        }
    }
    return overlap;
}

export void seperate_ispc(uniform rectangle_t rooms[], uniform int numRooms) {

}