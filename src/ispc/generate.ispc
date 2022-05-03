
#define BIT_OVERLAPPING 1 << 6

#define BIT_NO_MOVE 1 << 7

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
        int prog_i = i;
        for (uniform int j = 0; j < numRooms; j += programCount) {
            if(isOverlapping(rooms, prog_i, j + programIndex, numRooms)) {
                overlap = 1;
                return overlap;
            }
            if (overlap == 1)
                return overlap;
        }
    }
    return overlap;
}

// ispc version of inside loop of seperateRooms function
export void separate_ispc(uniform rectangle_t rooms[], uniform int numRooms) {
    for (uniform int i = 0; i < numRooms; i++) {
        int prog_i = i + programIndex;
        for (uniform int j = 0; j < numRooms; j += programCount) {
            int prog_j = j + programIndex;
            if (prog_i == prog_j)
                continue;
            
            if (isOverlapping(rooms, prog_i, prog_j, numRooms)) {
                //print("FOUND OVERLAP rooms i:% j:%\n",prog_i,prog_j);
                float step_x = rooms[prog_j].center.x - rooms[prog_i].center.x;
                float step_y = rooms[prog_j].center.y - rooms[prog_i].center.y;
                //print("step_x: %\n",step_x);
                //print("step_y: %\n",step_y);
                float dist = sqrt((step_x * step_x) + (step_y * step_y));
                //print("DIST: %\n",dist);
                if (round(dist) == 0)
                    dist = 0.001f;
                step_x /= dist;
                step_y /= dist; 
                step_x = round(step_x);
                step_y = round(step_y);

                if (step_x == 0.0f)
                    step_x = 1.0f;
                if (step_y == 0.0f)
                    step_y = 1.0f;
                //print("Moving room % by \n*** X:% \n*** Y:%\n",prog_i, 0-step_x, 0-step_y);
                //print("Moving room % by \n*** X:% \n*** Y:%\n",prog_j, step_x, step_y);
                rooms[prog_i].center.x -= step_x;
                rooms[prog_i].center.y -= step_y;
                rooms[prog_j].center.x += step_x;
                rooms[prog_j].center.y += step_y;
            }
        }
    }
}