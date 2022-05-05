

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>
#include <chrono>
#include <string.h>

#include "generate.h"
#include "main.h"
#include <SDL.h>

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
#ifdef VSTUDIO // since can't set command arg with VSTUDIO
    int animate = 1;
    printf("Running with ANIMATE flag (-a), computation time WILL be slower.\n");
    rectangle_t **room_data = (rectangle_t **)malloc(sizeof(rectangle_t *) * MAX_ITERS);
#else
    int animate = 0;
    rectangle_t **room_data = NULL;
#endif
    // checking for -a animate flag
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i],"-a") == 0) {
            printf("Running with ANIMATE flag (-a), computation time WILL be slower.\n");
            animate = 1;
            room_data = (rectangle_t **)malloc(sizeof(rectangle_t *) * MAX_ITERS);
        }
    }

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
    double separate_time = 0;
    double hallway_time = 0;
    double inclusion_time = 0;
    double full_time = 0;

    dungeon_t d;
    dungeon_t *dungeon = &d;
    double_edge_t md;
    double_edge_t *mst_dela = &md;

    // insert timing functions here
    auto generate_start = Clock::now();
    generate(dungeon, roomNum, 25);
    generate_time += std::chrono::duration_cast<dsec>(Clock::now() - generate_start).count();
    printf("Generation Time: %lfs\n", generate_time);

    auto separate_start = Clock::now();
    separateRooms(dungeon, room_data, animate);
    separate_time += std::chrono::duration_cast<dsec>(Clock::now() - separate_start).count();
    printf("Seperation Time: %lfs\n", separate_time);
    
    auto hallway_start = Clock::now();
    mst_dela = constructHallways(dungeon);
    hallway_time += std::chrono::duration_cast<dsec>(Clock::now() - hallway_start).count();
    printf("Hallway Generation Time: %lfs\n", hallway_time);
    
    auto inclusion_start = Clock::now();
    getIncludedRooms(dungeon);
    inclusion_time += std::chrono::duration_cast<dsec>(Clock::now() - inclusion_start).count();
    printf("Inclusion Time: %lfs\n", inclusion_time);
    
    full_time += std::chrono::duration_cast<dsec>(Clock::now() - init_start).count();
    printf("Full Generation Time: %lfs\n", full_time);

    float quality = get_solution_quality(dungeon);
    printf("Dungeon solution quality (lower is better: %f\n", quality);
    
    display disp(dungeon, animate, room_data, mst_dela);

    printf("*****STARTING GUI*****\n");

    int ecode = disp.OnExecute(dungeon, mst_dela);

    printf("***** CLOSING GUI*****\n");

    if (animate == 1) {
        for (int i = 0; i < dungeon->numIters; i++) {
            free(room_data[i]);
        }
        free(room_data);
    }
    free(mst_dela->dela);
    free(mst_dela->mst);
    free(dungeon->rooms);
    free(dungeon->mainRoomIndices);
    free(dungeon->hallways);

    return ecode;
}

/*****************************************************************************
 *                            Class / MidRender Functions
 *****************************************************************************/

/*
 * Class constructor
 */
display::display(dungeon_t *dungeon, int animate_set, rectangle_t **room_hist,
                 double_edge_t *mst_dela) {
    running = true;
    currRoomNumber = dungeon->numRooms;
    room_data = room_hist;
    dungeon_data = dungeon;
    mst_dela_data = mst_dela;
    pixPerUnit = 5;
    x_offset = 0;
    y_offset = 0;
    room_view = 0;
    show_hallways = 0;
    animate = animate_set;
    show_tree = 0;
    animate_on = 0;
    hall_type = 0;
}

/*
 * Initialization function
 */
bool display::OnInit() {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Error with initing SDL\nSDL Error: %s\n", SDL_GetError());
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
    gMain = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\main_proto_1.bmp", renderer);
    gHall = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\hall_proto_1.bmp", renderer);
    gBlack = loadTexture("C:\\Users\\olekk\\OneDrive\\Desktop\\S2022\\15-418\\ParallelDungeonGenerator\\src\\assets\\black_square.bmp", renderer);
#else
#ifdef ISPC
    gRoom = loadTexture("../assets/room_proto_2.bmp", renderer);
    gSides = loadTexture("../assets/turq_square.bmp", renderer);
    gGrey = loadTexture("../assets/grey_square.bmp", renderer);
    gRed = loadTexture("../assets/red_square.bmp", renderer);
    gMain = loadTexture("../assets/main_proto_1.bmp", renderer);
    gHall = loadTexture("../assets/hall_proto_1.bmp", renderer);
    gBlack = loadTexture("../assets/black_square.bmp", renderer);
#else
    gRoom = loadTexture("assets/room_proto_2.bmp", renderer);
    gSides = loadTexture("assets/turq_square.bmp", renderer);
    gGrey = loadTexture("assets/grey_square.bmp", renderer);
    gRed = loadTexture("assets/red_square.bmp", renderer);
    gMain = loadTexture("assets/main_proto_1.bmp", renderer);
    gHall = loadTexture("assets/hall_proto_1.bmp", renderer);
    gBlack = loadTexture("assets/black_square.bmp", renderer);
#endif
#endif
}

/*
 * Execution Loop
 */
int display::OnExecute(dungeon_t *dungeon, double_edge_t *mst_dela) {
    if (OnInit() == false)
        return -1;

    loadAssets();

    SDL_Event Event;

    int animate_sep_step = 0;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }

        // stay in seperate render function for animation
        if (animate_on == 1) {
            if (animation(animate_sep_step) == 1) {
                animate_on = 0;
                animate_sep_step = 0;
            }
            animate_sep_step += 1;
            continue;
        }

        OnRender(dungeon, mst_dela);
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
        }
        if (event->wheel.y < 0 && pixPerUnit != 1) { // scroll down
            pixPerUnit -= 1;
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
        if (currentKeyStates[SDL_SCANCODE_A]) {
            if (animate)
                animate_on = 1;
        }
        if (currentKeyStates[SDL_SCANCODE_H]) {
            hall_type = (hall_type + 1) % 2;
        }
    }
}


/*
 * Generating the background image
 */
void display::genBackground() {
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

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }
    // left from origin
    for (int x = originx - pixPerUnit; x > 0; x -= pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }


    // going down from origin inclusive
    for (int y = originy; y < SCREEN_HEIGHT; y += pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_RenderCopy(renderer, gGrey, NULL, &moverect);
    }

    // going down from origin inclusive
    for (int y = originy - pixPerUnit; y > 0; y -= pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

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

    if (hall_type == 0)
        renderHallFull(show_hallways);

    // first generate background grid
    genBackground();

    int roomMax = currRoomNumber;

    if (room_view == 1) roomMax = mainRoomNum;

    for (int room_inc = 0; room_inc < roomMax; room_inc++) {

        if (room_view == 1)
            renderRoom(rooms[mainRoomIndices[room_inc]], gMain, gBlack);
        else if (room_view == 2) {
            if (rooms[room_inc].status & BIT_MAINROOM)
                renderRoom(rooms[room_inc], gMain, gBlack);
            else if (rooms[room_inc].status & BIT_INCLUDED)
                renderRoom(rooms[room_inc], gRoom, gSides);
        }
        else {
            if (rooms[room_inc].status & BIT_MAINROOM)
                renderRoom(rooms[room_inc], gMain, gBlack);
            else
                renderRoom(rooms[room_inc], gRoom, gSides);
        }
    }

    if (hall_type == 1)
        renderHallLine(show_hallways);

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
        renderEdge(roomsrc, roomdest);
        
    }
    SDL_RenderPresent(renderer);
}

void display::renderHallLine(int show_hallways) {
    SDL_Rect hallrect;
    for (int i = 0; i < show_hallways; i++) {
        // render line for start-->middle
        int hallsx = (int)dungeon_data->hallways[i].start.x;
        int hallsy = (int)dungeon_data->hallways[i].start.y;

        int hallmx = (int)dungeon_data->hallways[i].middle.x;
        int hallmy = (int)dungeon_data->hallways[i].middle.y;


        hallrect.x = ((std::min(hallsx, hallmx) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallsy, hallmy) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallmx - hallsx) * pixPerUnit, 1);
        hallrect.h = std::max(std::abs(hallmy - hallsy) * pixPerUnit, 1);

        if (hallrect.w == 1 && hallrect.h == 1) {
            hallrect.w = 0;
        }

        SDL_RenderCopy(renderer, gRed, NULL, &hallrect);

        // render line for middle-->end
        int hallex = (int)dungeon_data->hallways[i].end.x;
        int halley = (int)dungeon_data->hallways[i].end.y;

        hallrect.x = ((std::min(hallmx, hallex) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallmy, halley) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallex - hallmx) * pixPerUnit, 1);
        hallrect.h = std::max(std::abs(halley - hallmy) * pixPerUnit, 1);

        if (hallrect.w == 1 && hallrect.h == 1) {
            hallrect.w = 0;
        }

        SDL_RenderCopy(renderer, gRed, NULL, &hallrect);
    }
}

void display::renderHallFull(int show_hallways) {
    SDL_Rect hallrect;
    for (int i = 0; i < show_hallways; i++) {
        // render line for start-->middle
        int hallsx = (int)dungeon_data->hallways[i].start.x;
        int hallsy = (int)dungeon_data->hallways[i].start.y;

        int hallmx = (int)dungeon_data->hallways[i].middle.x;
        int hallmy = (int)dungeon_data->hallways[i].middle.y;


        hallrect.x = ((std::min(hallsx, hallmx) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallsy, hallmy) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallmx - hallsx) * pixPerUnit, pixPerUnit);
        hallrect.h = std::max(std::abs(hallmy - hallsy) * pixPerUnit, pixPerUnit);

        if (hallrect.w == pixPerUnit && hallrect.h == pixPerUnit) {
            hallrect.w = 0;
        }

        SDL_RenderCopy(renderer, gHall, NULL, &hallrect);

        // render line for middle-->end
        int hallex = (int)dungeon_data->hallways[i].end.x;
        int halley = (int)dungeon_data->hallways[i].end.y;

        hallrect.x = ((std::min(hallmx, hallex) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
        hallrect.y = ((std::min(hallmy, halley) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
        hallrect.w = std::max(std::abs(hallex - hallmx) * pixPerUnit, pixPerUnit);
        hallrect.h = std::max(std::abs(halley - hallmy) * pixPerUnit, pixPerUnit);

        if (hallrect.w == pixPerUnit && hallrect.h == pixPerUnit) {
            hallrect.w = 0;
        }

        SDL_RenderCopy(renderer, gHall, NULL, &hallrect);

        // full hallway bottom right intersection extension logic

        // start x < mid x, start y == mid y, mid y > end y
        // end x < mid x, end y == mid y, mid y > start y
        if ((hallsx < hallmx) && (hallsy == hallmy) && (hallmy > halley)) {
            // re render start to mid with + 1
            hallrect.x = ((std::min(hallsx, hallmx) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
            hallrect.y = ((std::min(hallsy, hallmy) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
            hallrect.w = std::max(std::abs(hallmx - hallsx) * pixPerUnit, pixPerUnit) + pixPerUnit;
            hallrect.h = std::max(std::abs(hallmy - hallsy) * pixPerUnit, pixPerUnit);

            if (hallrect.w == pixPerUnit && hallrect.h == pixPerUnit) {
                hallrect.w = 0;
            }

            SDL_RenderCopy(renderer, gHall, NULL, &hallrect);
        }
        if ((hallex < hallmx) && (halley == hallmy) && (hallmy > hallsy)) {
            // re render end to mid with + 1
            hallrect.x = ((std::min(hallmx, hallex) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
            hallrect.y = ((std::min(hallmy, halley) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);
            hallrect.w = std::max(std::abs(hallex - hallmx) * pixPerUnit, pixPerUnit) + pixPerUnit;
            hallrect.h = std::max(std::abs(halley - hallmy) * pixPerUnit, pixPerUnit);

            if (hallrect.w == pixPerUnit && hallrect.h == pixPerUnit) {
                hallrect.w = 0;
            }

            SDL_RenderCopy(renderer, gHall, NULL, &hallrect);
        }
    }
}

void display::renderEdge(rectangle_t roomsrc, rectangle_t roomdest) {
    int src_x = ((roomsrc.center.x + x_offset) * pixPerUnit) + SCREEN_WIDTH / 2;
    int src_y = ((roomsrc.center.y + y_offset) * pixPerUnit) + SCREEN_HEIGHT / 2;
    int dest_x = ((roomdest.center.x + x_offset) * pixPerUnit) + SCREEN_WIDTH / 2;
    int dest_y = ((roomdest.center.y + y_offset) * pixPerUnit) + SCREEN_HEIGHT / 2;

    SDL_RenderDrawLine(renderer, src_x, src_y, dest_x, dest_y);
}

void display::renderRoom(rectangle_t room, SDL_Texture *roomTex, SDL_Texture *sideTex) {
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

    SDL_RenderCopy(renderer, roomTex, NULL, &roomRect);

    /********* add sides to room *********/

    SDL_Rect sideRect;

    // left side
    sideRect.x = roomRect.x;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_RenderCopy(renderer, sideTex, NULL, &sideRect);

    // top side
    sideRect.h = 1;
    sideRect.w = roomRect.w;

    SDL_RenderCopy(renderer, sideTex, NULL, &sideRect);

    // bottom side
    sideRect.y = roomRect.y + roomRect.h - 1;

    SDL_RenderCopy(renderer, sideTex, NULL, &sideRect);

    // right side
    sideRect.x = roomRect.x + roomRect.w - 1;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_RenderCopy(renderer, sideTex, NULL, &sideRect);

    SDL_Rect dotRect;

    // drawing red dot for center of room
    dotRect.h = 2;
    dotRect.w = 2;

    dotRect.x = (((int)(room.center.x) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2);
    dotRect.y = (((int)(room.center.y) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2);

    SDL_RenderCopy(renderer, gRed, NULL, &dotRect);
}

void renderLoadBar(float prog, SDL_Texture *tex, SDL_Renderer *renderer) {
    int bar_end = (int)(prog * SCREEN_WIDTH);
    SDL_Rect barRect;
    barRect.x = 0;
    barRect.y = 0;
    barRect.h = 5;
    barRect.w = bar_end;
    SDL_RenderCopy(renderer, tex, NULL, &barRect);
}

/*
 * Animation function
 */
int display::animation(int animate_sep_step) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    genBackground();

    if (animate_sep_step < dungeon_data->numIters) {

        // total animation time should take max_sec ns (i believe it's ns)
        int max_sec = 10000; // 20 seconds, felt like a good balance
        int iter_delay = max_sec / dungeon_data->numIters;

        rectangle_t* iter_rooms = room_data[animate_sep_step];
        for (int curr_room = 0; curr_room < dungeon_data->numRooms; curr_room++) {
            renderRoom(iter_rooms[curr_room], gRoom, gSides);
        }
        // render the loading bar
        float prog = (float)animate_sep_step / (dungeon_data->numIters - 1);
        renderLoadBar(prog, gRed, renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(iter_delay);
        return 0;
    }
    SDL_Delay(1000);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    genBackground();
    // draw last iteration but main rooms red
    for (int curr_room = 0; curr_room < dungeon_data->numRooms; curr_room++) {
        rectangle_t room = dungeon_data->rooms[curr_room];
        if ((room.status & BIT_MAINROOM) != 0)
            renderRoom(room, gMain, gBlack);
        else
            renderRoom(room, gRoom, gSides);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    genBackground();
    // draw just main rooms
    for (int curr_room = 0; curr_room < dungeon_data->numMainRooms; curr_room++) {
        renderRoom(dungeon_data->rooms[dungeon_data->mainRoomIndices[curr_room]], gMain, gBlack);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    // then add in the dela graph
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    rectangle_t roomsrc;
    rectangle_t roomdest;
    for (int curr_edge = 0; curr_edge < mst_dela_data->dela_edges; curr_edge++) {    
        roomsrc = dungeon_data->rooms[mst_dela_data->dela[curr_edge].src];
        roomdest = dungeon_data->rooms[mst_dela_data->dela[curr_edge].dest];
        renderEdge(roomsrc, roomdest);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(750);
    // clear and do main rooms plus mst graph
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    genBackground();
    for (int curr_room = 0; curr_room < dungeon_data->numMainRooms; curr_room++) {
        renderRoom(dungeon_data->rooms[dungeon_data->mainRoomIndices[curr_room]], gMain, gBlack);
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    for (int curr_edge = 0; curr_edge < mst_dela_data->mst_edges; curr_edge++) {    
        roomsrc = dungeon_data->rooms[mst_dela_data->mst[curr_edge].src];
        roomdest = dungeon_data->rooms[mst_dela_data->mst[curr_edge].dest];
        renderEdge(roomsrc, roomdest);
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    // clear and draw the red line hallways with main rooms
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);
    genBackground();
    for (int curr_room = 0; curr_room < dungeon_data->numMainRooms; curr_room++) {
        renderRoom(dungeon_data->rooms[dungeon_data->mainRoomIndices[curr_room]], gMain, gBlack);
    }
    renderHallLine(dungeon_data->numHallways);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    // clear and draw main and included rooms below red lines
    SDL_RenderClear(renderer);
    genBackground();
    for (int curr_room = 0; curr_room < dungeon_data->numRooms; curr_room++) {
        rectangle_t room = dungeon_data->rooms[curr_room];
        if (room.status & (BIT_MAINROOM))
            renderRoom(room, gMain, gBlack);
        else if (room.status & (BIT_INCLUDED))
            renderRoom(room, gRoom, gSides);
    }
    renderHallLine(dungeon_data->numHallways);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    // clear and draw main and included rooms above full halls
    SDL_RenderClear(renderer);
    renderHallFull(dungeon_data->numHallways);
    genBackground();
    for (int curr_room = 0; curr_room < dungeon_data->numRooms; curr_room++) {
        rectangle_t room = dungeon_data->rooms[curr_room];
        if (room.status & (BIT_MAINROOM))
            renderRoom(room, gMain, gBlack);
        else if (room.status & (BIT_INCLUDED))
            renderRoom(room, gRoom, gSides);
    }
    // setting render vars to match end of animation
    room_view = 2;
    hall_type = 0;
    show_hallways = dungeon_data->numHallways;
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    return 1;
}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_DestroyTexture(gSides);
    SDL_DestroyTexture(gRoom);
    SDL_DestroyTexture(gGrey);
    SDL_DestroyTexture(gRed);
    SDL_DestroyTexture(gMain);
    SDL_DestroyTexture(gHall);
    SDL_DestroyTexture(gBlack);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdlwindow);
    SDL_Quit();
}
