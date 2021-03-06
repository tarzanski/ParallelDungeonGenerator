

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>
#include <chrono>
#include <omp.h>

#include "generate.h"
#include "main.h"
#include <SDL.h>

// #define VSTUDIO

static int _argc;
static char **_argv;

// Get number of threads
int get_option_int(const char *option_name, int default_value) {
    for (int i = _argc - 2; i >= 0; i -= 2)
        if (strcmp(_argv[i], option_name) == 0)
            return atoi(_argv[i + 1]);
    return default_value;
}

// Rough "quality" metric for dungeons, area of bounding rectangle
float get_solution_quality(dungeon_t *dungeon) {
    rectangle_t *rooms = dungeon->rooms;
    int numRooms = dungeon->numRooms;
    float top = std::numeric_limits<float>::max();
    float bottom = std::numeric_limits<float>::min();
    float left = std::numeric_limits<float>::max();
    float right = std::numeric_limits<float>::min();
    for (int i = 0; i < numRooms; i++) {
        top = std::min(top, rooms[i].center.y - rooms[i].height / 2);
        bottom = std::max(bottom, rooms[i].center.y + rooms[i].height / 2);
        left = std::min(left, rooms[i].center.x - rooms[i].width / 2);
        right = std::max(right, rooms[i].center.y + rooms[i].width / 2);
    }
    return (bottom - top) * (right - left);
}

int main(int argc, char** argv) {
    _argc = argc - 1;
    _argv = argv + 1;
    int num_of_threads = get_option_int("-n", 1);
    omp_set_num_threads(num_of_threads);
    printf("Number of threads: %d\n", num_of_threads);

    // getting room generation number
    int roomNum = 500;
    printf("Enter Number of Rooms: ");
    scanf("%d", &roomNum);
    printf("Generating %d Rooms\n", roomNum);

    // set up any timing before calling algorithm functions
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;

    auto init_start = Clock::now();
    double generate_time = 0;
    double time_difference = 0;


    dungeon_t d;
    dungeon_t *dungeon = &d;
    double_edge_t md;
    double_edge_t *mst_dela = &md;

    // insert timing functions here

    generate(dungeon, roomNum, 25);
    generate_time = std::chrono::duration_cast<dsec>(Clock::now() - init_start).count();
    printf("Initial Room Generation Time: %lfs\n", generate_time);

    separateRooms(dungeon);
    time_difference = std::chrono::duration_cast<dsec>(Clock::now() - init_start).count() - generate_time;
    generate_time += time_difference;
    printf("Room Separation Time: %lfs\n", time_difference);

    mst_dela = constructHallways(dungeon);
    time_difference = std::chrono::duration_cast<dsec>(Clock::now() - init_start).count() - generate_time;
    generate_time += time_difference;
    printf("MST and Delaunay Time: %lfs\n", time_difference);

    getIncludedRooms(dungeon);
    time_difference = std::chrono::duration_cast<dsec>(Clock::now() - init_start).count() - generate_time;
    generate_time += time_difference;
    printf("Included Rooms Time: %lfs\n", time_difference);

    generate_time = std::chrono::duration_cast<dsec>(Clock::now() - init_start).count();
    printf("Total Dungeon Generation Time: %lfs\n", generate_time);

    float quality = get_solution_quality(dungeon);
    printf("Dungeon solution quality (lower is better: %f\n", quality);

    display disp(dungeon);

    printf("*****STARTING GUI*****\n");

    int ecode = disp.OnExecute(dungeon, mst_dela);

    printf("***** CLOSING GUI*****\n");

    
    free(dungeon->rooms);
    free(dungeon->mainRoomIndices);
    free(dungeon->hallways);

    return ecode;
    //return 0;
}

/*****************************************************************************
 *                            Prerender functions 
 *****************************************************************************/

// function to modify rooms when hallways touch the edge of a room
/* function should essentially knock off the side wall of the room touching 
   the hallway (could also be parallelized) */

// int checkEdge(rectangle_t* room, hallway_t* hallway) {

// }

// void fixRoomEdges(dungeon_t* dungeon) {
//     for (int roomInd = 0; roomInd < dungeon->numRooms; roomInd++) {
//         rectangle_t* room = &dungeon->rooms[roomInd];
//         if (!(room->status & BIT_INCLUDED))
//             continue;
//         // code looks familiar?
//         float topLeftx = dungeon->rooms[roomNum].center.x - dungeon->rooms[roomNum].width/2;
//         float topLefty = dungeon->rooms[roomNum].center.y - dungeon->rooms[roomNum].height/2;
//         float botRightx = dungeon->rooms[roomNum].center.x + dungeon->rooms[roomNum].width/2;
//         float botRighty = dungeon->rooms[roomNum].center.y + dungeon->rooms[roomNum].height/2;

//         for (int hallInd = 0; hallInd < dungeon->numHallways; hallInd++) {
//             hallway_t* hall = &dungeon->hallways[hallInd];

//             // check both hallway segments if adjacent to current room
//             if ()

//         }
//     }
// }



// function to deal with hallway intersections
// create list of all hallway intersections 
/* intersections should not exist when hallways are directly on top of eachother
   and travelling in the same direction (this will be hard) */



/*****************************************************************************
 *                            Class / MidRender Functions
 *****************************************************************************/

/*
 * Class constructor
 */
display::display(dungeon_t *dungeon) {
    running = true;
    currRoomNumber = 0;
    pixPerUnit = 5;
    x_offset = 0;
    y_offset = 0;
    room_view = 0;
    show_hallways = 0;
    dungeon_data = dungeon;
    show_tree = 0;
}

/*
 * Initialization function
 */
bool display::OnInit() {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Error with initing SDL\n");
        return false;
    }

    if((sdlwindow = SDL_CreateWindow("Dungeon Generator Visualizer",
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SDL_WINDOWPOS_UNDEFINED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT,
                                     SDL_WINDOW_OPENGL)) == NULL) {
        printf("Error with creating window\n");
        return false;
    }

    if ((renderer = SDL_CreateRenderer(sdlwindow, -1, 0)) == NULL) {
        printf("Error with creating software renderer: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,SDL_ALPHA_OPAQUE);
    SDL_RenderPresent(renderer);

    //gScreenSurface = SDL_GetWindowSurface(sdlwindow);

    return true;
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer) {
    SDL_Texture* tex = NULL;

    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s :: Error %s\n",path.c_str(), SDL_GetError());
    }

    // converting surface to texture for rendering
    tex = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (tex == NULL) {
        printf("Unable to create texture from image %s\n", path.c_str());
    }

    SDL_FreeSurface(loadedSurface);
    return tex;
}

void display::loadAssets() {
#ifdef VSTUDIO
    gRoom = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\room_proto_2.bmp", renderer);

    gSides = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\turq_square.bmp", renderer);

    gGrey = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\grey_square.bmp", renderer);

    gRed = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\red_square.bmp", renderer);
#endif
#ifndef VSTUDIO
    gRoom = loadTexture("assets/room_proto_2.bmp", renderer);

    gSides = loadTexture("assets/turq_square.bmp", renderer);

    gGrey = loadTexture("assets/grey_square.bmp", renderer);

    gRed = loadTexture("assets/red_square.bmp", renderer);
#endif
}

/*
 * Execution Loop
 */
int display::OnExecute(dungeon_t *dungeon, double_edge_t *mst_dela) {
    if (OnInit() == false)
        return -1;

    loadAssets();

    genBackround();

    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }

        OnRender(dungeon, mst_dela);

        OnLoop();
    }

    OnCleanup();

    return 0;
}

/*
 * Event handler function
 */
void display::OnEvent(SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        running = false;
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (event->wheel.y > 0) { // scroll up
            pixPerUnit += 1;
            //printf("Scrolling up by: %d ppu: %d\n", event->wheel.y, pixPerUnit);
        }
        if (event->wheel.y < 0 && pixPerUnit != 1) { // scroll down
            pixPerUnit -= 1;
            //printf("Scrolling down by: %d ppu: %d\n", event->wheel.y, pixPerUnit);
        }
    }
    if (event->type == SDL_KEYDOWN) {
        const uint8_t* currentKeyStates = SDL_GetKeyboardState(NULL);
        if (currentKeyStates[SDL_SCANCODE_UP])
            y_offset += std::max(1, 10 / pixPerUnit);
        if (currentKeyStates[SDL_SCANCODE_DOWN])
            y_offset -= std::max(1, 10 / pixPerUnit);
        if (currentKeyStates[SDL_SCANCODE_LEFT])
            x_offset += std::max(1, 10 / pixPerUnit);
        if (currentKeyStates[SDL_SCANCODE_RIGHT])
            x_offset -= std::max(1, 10 / pixPerUnit);
        if (currentKeyStates[SDL_SCANCODE_SPACE])
            room_view = (room_view + 1) % 3;
        if (currentKeyStates[SDL_SCANCODE_1] && currRoomNumber != 0)
            currRoomNumber -= 1;
        if (currentKeyStates[SDL_SCANCODE_2])
            currRoomNumber += 1;
        if (currentKeyStates[SDL_SCANCODE_3]) {
            if (show_hallways != dungeon_data->numHallways)
                show_hallways = dungeon_data->numHallways;
            else
                show_hallways = 0; 

        }
        if (currentKeyStates[SDL_SCANCODE_4]) {
            show_tree = (show_tree + 1) % 3;
        }
        if (currentKeyStates[SDL_SCANCODE_5]) {
            currRoomNumber = dungeon_data->numRooms;
        }
    }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    //SDL_Delay(125);
}

/*
 * Generating the background image
 */
void display::genBackround() {
    // going right from origin inclusive
    int originx = SCREEN_WIDTH / 2;
    int originy = SCREEN_HEIGHT / 2;

    SDL_Rect moverect;
    // right from origin inclusive
    for (int x = originx; x < SCREEN_WIDTH; x += pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        //SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }
    // left from origin
    for (int x = originx - pixPerUnit; x > 0; x -= pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        // SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }


    // going down from origin inclusive
    for (int y = originy; y < SCREEN_HEIGHT; y += pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        // SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }

    // going down from origin inclusive
    for (int y = originy - pixPerUnit; y > 0; y -= pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        // SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }
}

// gets either x or y room center given ppu and width/height
int getRoomCenter(int units, int ppu, int wh) {
    return (units * ppu) - (((wh/2) / ppu) * ppu);
}

/*
 * Render fuction
 */
void display::OnRender(dungeon_t *dungeon, double_edge_t *mst_dela) {
    // dungeon struct contents
    rectangle_t *rooms = dungeon->rooms;
    //int roomNum = dungeon->numRooms;
    hallway_t *hallways = dungeon->hallways;
    //int hallwayNum = dungeon->numHallways;
    int *mainRoomIndices = dungeon->mainRoomIndices;
    int mainRoomNum = dungeon->numMainRooms;
    
    // mst_dela struct contents
    edge_t *dela = mst_dela->dela;
    edge_t *mst = mst_dela->mst;
    int dela_edges = mst_dela->dela_edges;
    int mst_edges = mst_dela->mst_edges;

    // clearing old frame
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    //SDL_FillRect(gScreenSurface, NULL, 0x000000);

    // first generate background grid
    genBackround();

    int roomMax = currRoomNumber;

    if (room_view == 1) roomMax = mainRoomNum;

    for (int room_inc = 0; room_inc < roomMax; room_inc++) {

        if (room_view == 1)
            renderRoom(rooms[mainRoomIndices[room_inc]]);
        else if (room_view == 2) {
            if (rooms[room_inc].status && BIT_INCLUDED)
                renderRoom(rooms[room_inc]);
        }
        else
            renderRoom(rooms[room_inc]);

    }

    
    SDL_Rect hallrect;
    for (int i = 0; i < show_hallways; i++) {
        // render line for start-->middle
        int hallsx = (int)hallways[i].start.x;
        int hallsy = (int)hallways[i].start.y;

        int hallmx = (int)hallways[i].middle.x;
        int hallmy = (int)hallways[i].middle.y;


        hallrect.x = ((std::min(hallsx, hallmx) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallsy, hallmy) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallmx - hallsx) * pixPerUnit, 1);
        hallrect.h = std::max(std::abs(hallmy - hallsy) * pixPerUnit, 1);

        if (hallrect.w == 1 && hallrect.h == 1) {
            hallrect.w = 0;
        }

        //SDL_BlitScaled(gRed, NULL, gScreenSurface, &hallrect);
        SDL_RenderCopy(renderer, gRed, NULL, &hallrect);

        // render line for middle-->end
        int hallex = (int)hallways[i].end.x;
        int halley = (int)hallways[i].end.y;

        hallrect.x = ((std::min(hallmx, hallex) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallmy, halley) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallex - hallmx) * pixPerUnit, 1);
        hallrect.h = std::max(std::abs(halley - hallmy) * pixPerUnit, 1);

        if (hallrect.w == 1 && hallrect.h == 1) {
            hallrect.w = 0;
        }

        //SDL_BlitScaled(gRed, NULL, gScreenSurface, &hallrect);
        SDL_RenderCopy(renderer, gRed, NULL, &hallrect);
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);

    int tree_limit = 0;
    if (show_tree == 1) 
        tree_limit = mst_edges;
    if (show_tree == 2)
        tree_limit = dela_edges;

    for (int tree_inc = 0; tree_inc < tree_limit; tree_inc++) {
        rectangle_t roomsrc;
        rectangle_t roomdest;
        if (show_tree == 1) {
            roomsrc = rooms[mst[tree_inc].src];
            roomdest = rooms[mst[tree_inc].dest];
        }
        else { // don't care about 0 case
            roomsrc = rooms[dela[tree_inc].src];
            roomdest = rooms[dela[tree_inc].dest];
        }
        int src_x = ((roomsrc.center.x + x_offset) * pixPerUnit) + SCREEN_WIDTH / 2;
        int src_y = ((roomsrc.center.y + y_offset) * pixPerUnit) + SCREEN_HEIGHT / 2;
        int dest_x = ((roomdest.center.x + x_offset) * pixPerUnit) + SCREEN_WIDTH / 2;
        int dest_y = ((roomdest.center.y + y_offset) * pixPerUnit) + SCREEN_HEIGHT / 2;

        SDL_RenderDrawLine(renderer, src_x, src_y, dest_x, dest_y);
    }
    SDL_RenderPresent(renderer);
    //SDL_UpdateWindowSurface(sdlwindow);
}

void display::renderRoom(rectangle_t room) {
    SDL_Rect roomRect;

    float roomW = room.width;
    float roomH = room.height;

    // taking abs of width and height
    roomW = (roomW < 0) ? (roomW * -1) : roomW;
    roomH = (roomH < 0) ? (roomH * -1) : roomH;

    roomRect.h = (int)roomH * pixPerUnit;
    roomRect.w = (int)roomW * pixPerUnit;

    // calculating the room centers based on the number of pixels per unit
    int units_x = (int)room.center.x + x_offset;
    int units_y = (int)room.center.y + y_offset;

    roomRect.x = getRoomCenter(units_x, pixPerUnit, roomRect.w);
    roomRect.y = getRoomCenter(units_y, pixPerUnit, roomRect.h);

    // final alignment to move origin to middle of the window
    roomRect.x += SCREEN_WIDTH / 2;
    roomRect.y += SCREEN_HEIGHT / 2;

    //SDL_BlitScaled(gRoom, NULL, gScreenSurface, &roomRect);
    SDL_RenderCopy(renderer, gRoom, NULL, &roomRect);

    /********* add sides to room *********/

    SDL_Rect sideRect;

    // left side
    sideRect.x = roomRect.x;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    //SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);
    SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

    // top side
    sideRect.h = 1;
    sideRect.w = roomRect.w;

    //SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);
    SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

    // bottom side
    sideRect.y = roomRect.y + roomRect.h - 1;

    //SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);
    SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

    // right side
    sideRect.x = roomRect.x + roomRect.w - 1;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    //SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);
    SDL_RenderCopy(renderer, gSides, NULL, &sideRect);

    SDL_Rect dotRect;

    // drawing red dot for center of room
    dotRect.h = 3;
    dotRect.w = 3;

    dotRect.x = (((int)(room.center.x) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2) - dotRect.w/2;
    dotRect.y = (((int)(room.center.y) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2) - dotRect.h/2;

    //SDL_BlitScaled(gRed, NULL, gScreenSurface, &dotRect);
    SDL_RenderCopy(renderer, gRed, NULL, &dotRect);
}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_DestroyTexture(gSides);

    SDL_DestroyTexture(gRoom);

    SDL_DestroyTexture(gGrey);

    SDL_DestroyTexture(gRed);

    SDL_DestroyRenderer(renderer);

    SDL_DestroyWindow(sdlwindow);

    SDL_Quit();
}
