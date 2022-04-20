

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>

#include "generate.h"
#include "main.h"
#include <SDL.h>


int main(int argc, char** argv) {

    // implement argument parsing

    // initialize SDL interface if using a GUI

    // set up any timing before calling algorithm functions

    // parse arguments if using a gui

    int roomNum = 50;

    dungeon_t d;
    dungeon_t *dungeon = &d;

    generate(dungeon, roomNum, 25);
    separateRooms(dungeon);
    constructHallways(dungeon);

    display disp;

    for (int i = 0; i < dungeon->numHallways; i++) {
        printf("***********Hallway #%d\n",i);
        printf("start pos  x:%f y%f\n",dungeon->hallways[i].start.x, dungeon->hallways[i].start.y);
        printf("middle pos x:%f y%f\n",dungeon->hallways[i].middle.x, dungeon->hallways[i].middle.y);
        printf("end pos    x:%f y%f\n",dungeon->hallways[i].end.x, dungeon->hallways[i].end.y);
    }

    printf("*****STARTING GUI*****\n");

    int ecode = disp.OnExecute(dungeon);

    printf("***** CLOSING GUI*****\n");

    free(dungeon->rooms);
    free(dungeon->mainRoomIndices);
    free(dungeon->hallways);

    return ecode;
}


/*
 * Class constructor
 */
display::display() {
    running = true;
    currRoomNumber = 0;
    pixPerUnit = 5;
    x_offset = 0;
    y_offset = 0;
    only_main = false;
    show_hallways = 0;
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

    gScreenSurface = SDL_GetWindowSurface(sdlwindow);

    return true;
}

SDL_Surface* loadSurface(std::string path, SDL_Surface* gScreenSurface) {
    SDL_Surface* optimizedSurface = NULL;

    SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s :: Error %s\n",path.c_str(), SDL_GetError());
    }

    // converting surface to screen format
    optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
    if (optimizedSurface == NULL) {
        printf("Unable to optimize image %s\n", path.c_str());
    }

    SDL_FreeSurface(loadedSurface);
    return optimizedSurface;
}

void display::loadAssets() {
    gRoom = loadSurface("assets/room_proto_2.bmp", gScreenSurface);

    gSides = loadSurface("assets/turq_square.bmp", gScreenSurface);

    gGrey = loadSurface("assets/grey_square.bmp", gScreenSurface);

    gRed = loadSurface("assets/red_square.bmp", gScreenSurface);
}

/*
 * Execution Loop
 */
int display::OnExecute(dungeon_t *dungeon) {
    if (OnInit() == false)
        return -1;

    loadAssets();

    genBackround();

    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }

        OnRender(dungeon);

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
            y_offset += 1;
        if (currentKeyStates[SDL_SCANCODE_DOWN])
            y_offset -= 1;
        if (currentKeyStates[SDL_SCANCODE_LEFT])
            x_offset += 1;
        if (currentKeyStates[SDL_SCANCODE_RIGHT])
            x_offset -= 1;
        if (currentKeyStates[SDL_SCANCODE_SPACE])
            only_main = !only_main;
        if (currentKeyStates[SDL_SCANCODE_1] && show_hallways != 0)
            show_hallways -= 1;
        if (currentKeyStates[SDL_SCANCODE_2])
            show_hallways += 1;
            
    }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    //currRoomNumber++;
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

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }
    // left from origin
    for (int x = originx - pixPerUnit; x > 0; x -= pixPerUnit) {
        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }


    // going down from origin inclusive
    for (int y = originy; y < SCREEN_HEIGHT; y += pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }

    // going down from origin inclusive
    for (int y = originy - pixPerUnit; y > 0; y -= pixPerUnit) {
        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }
}

/*
 * Render fuction
 */
void display::OnRender(dungeon_t *dungeon) {
    rectangle_t *rooms = dungeon->rooms;
    int roomNum = dungeon->numRooms;
    hallway_t *hallways = dungeon->hallways;
    int hallwayNum = dungeon->numHallways;
    int *mainRoomIndices = dungeon->mainRoomIndices;
    int mainRoomNum = dungeon->numMainRooms;
    
    // clearing old frame
    SDL_FillRect(gScreenSurface, NULL, 0x000000);

    // first generate background grid
    genBackround();

    if (only_main) roomNum = mainRoomNum;

    for (int room_inc = 0; room_inc < roomNum; room_inc++) {

        if (only_main)
            renderRoom(rooms[mainRoomIndices[room_inc]]);
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
        // printf("******************\n");
        // printf("hallway %d x: %d\n",i,hallrect.x);
        // printf("hallway %d y: %d\n",i,hallrect.y);
        // printf("hallway %d w: %d\n",i,hallrect.w);
        // printf("hallway %d h: %d\n",i,hallrect.h);

        SDL_BlitScaled(gRed, NULL, gScreenSurface, &hallrect);

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

        SDL_BlitScaled(gRed, NULL, gScreenSurface, &hallrect);
    }
    SDL_UpdateWindowSurface(sdlwindow);
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

    roomRect.x = (units_x * pixPerUnit) - (((roomRect.w/2) / pixPerUnit) * pixPerUnit);
    roomRect.y = (units_y * pixPerUnit) - (((roomRect.h/2) / pixPerUnit) * pixPerUnit);

    // final alignment to move origin to middle of the window
    roomRect.x += SCREEN_WIDTH / 2;
    roomRect.y += SCREEN_HEIGHT / 2;

    SDL_BlitScaled(gRoom, NULL, gScreenSurface, &roomRect);

    /********* add sides to room *********/

    SDL_Rect sideRect;

    // left side
    sideRect.x = roomRect.x;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // top side
    sideRect.h = 1;
    sideRect.w = roomRect.w;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // bottom side
    sideRect.y = roomRect.y + roomRect.h - 1;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    // right side
    sideRect.x = roomRect.x + roomRect.w - 1;
    sideRect.y = roomRect.y;
    sideRect.h = roomRect.h;
    sideRect.w = 1;

    SDL_BlitScaled(gSides, NULL, gScreenSurface, &sideRect);

    SDL_Rect dotRect;

    // drawing red dot for center of room
    dotRect.h = 3;
    dotRect.w = 3;

    dotRect.x = (((int)(room.center.x) + x_offset) * pixPerUnit) + (SCREEN_WIDTH / 2) - dotRect.w/2;
    dotRect.y = (((int)(room.center.y) + y_offset) * pixPerUnit) + (SCREEN_HEIGHT / 2) - dotRect.h/2;

    SDL_BlitScaled(gRed, NULL, gScreenSurface, &dotRect);
}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_FreeSurface(gRoom);

    SDL_FreeSurface(gGrey);

    SDL_FreeSurface(gRed);

    SDL_FreeSurface(gSides);

    SDL_DestroyWindow(sdlwindow);

    SDL_Quit();
}
