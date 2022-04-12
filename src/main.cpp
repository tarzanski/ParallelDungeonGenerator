

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <random>
#include "generate.h"
#include "main.h"
#include <SDL.h>

#define SCREEN_WIDTH 640

#define SCREEN_HEIGHT 480


int main(int argc, char** argv) {

    // implement argument parsing
    
    // initialize SDL interface if using a GUI

    // set up any timing before calling algorithm functions

    // parse arguments if using a gui

    int roomNum = 50;

    rectangle_t *rooms = generate(roomNum/10, 25);

    display disp;

    int ecode = disp.OnExecute(rooms, roomNum);

    free(rooms);

    return ecode;
}


/*
 * Class constructor
 */
display::display() {
    running = true;
    currRoomNumber = 0;
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
    SDL_Renderer* renderer;
    if ((renderer = SDL_CreateSoftwareRenderer(gScreenSurface)) == NULL) {
        printf("Error with creating software renderer");
        return false;
    }

    SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
    SDL_RenderClear(renderer);
    
    return true;
}

/*
 * Generating the background image
 */
void display::genBackround() {
    gRoom = SDL_LoadBMP("assets/room_proto_1.bmp");
    if (gRoom == NULL) {
        printf("Error loading bmp file\n");
    }

    gGrey = SDL_LoadBMP("assets/grey_square.bmp");
    if (gGrey == NULL) {
        printf("Error loading bmp file\n");
    }

    // SDL_BlitSurface(gRed, NULL, gScreenSurface, NULL);

    for (int x = 5; x < SCREEN_WIDTH; x += 10) {
        SDL_Rect moverect;

        moverect.x = x;
        moverect.y = 0;
        moverect.w = 1;
        moverect.h = SCREEN_HEIGHT;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }

    for (int y = 5; y < SCREEN_HEIGHT; y += 10) {
        SDL_Rect moverect;

        moverect.x = 0;
        moverect.y = y;
        moverect.w = SCREEN_WIDTH;
        moverect.h = 1;

        SDL_BlitScaled(gGrey, NULL, gScreenSurface, &moverect);
    }

    SDL_UpdateWindowSurface(sdlwindow);
}

/*
 * Execution Loop
 */
int display::OnExecute(rectangle_t *rooms, int roomNum) {
    if (OnInit() == false)
        return -1;

    genBackround();
    
    SDL_Event Event;

    while (running) {
        while (SDL_PollEvent(&Event)) {
            OnEvent(&Event);
        }
        if (currRoomNumber < roomNum) {

            OnRender(rooms, roomNum);
            
            // 2 seconds between rooms
            SDL_Delay(500);
            
            OnLoop();
        }
    } 

    OnCleanup();

    return 0;
}

/*
 * Event handler function
 */
void display::OnEvent(SDL_Event* Event) {
    if (Event->type == SDL_QUIT) {
        running = false;
    }
}

/*
 * Extra loop function that tutorial had
 */
void display::OnLoop() {
    currRoomNumber++;
}

/*
 * Render fuction
 */
void display::OnRender(rectangle_t *rooms, int roomNum) {

    // right now just displaying the first room

    // using 0 instead of curr_room_number

    SDL_Rect roomRect;

    roomRect.x = (int)rooms[currRoomNumber].center.x + (SCREEN_WIDTH / 2);
    roomRect.y = (int)rooms[currRoomNumber].center.y + (SCREEN_HEIGHT / 2);
    roomRect.h = (int)rooms[currRoomNumber].height;
    roomRect.w = (int)rooms[currRoomNumber].width;

    roomRect.h = (roomRect.h < 0) ? (roomRect.h * -1) : roomRect.h;
    roomRect.w = (roomRect.w < 0) ? (roomRect.w * -1) : roomRect.w;

    roomRect.h *= 5;
    roomRect.w *= 5;

    SDL_BlitScaled(gRoom, NULL, gScreenSurface, &roomRect);

    SDL_UpdateWindowSurface(sdlwindow);

}


/*
 * Closing function called before exit
 */
void display::OnCleanup() {
    SDL_FreeSurface(gRoom);

    SDL_FreeSurface(gGrey);

    SDL_DestroyWindow(sdlwindow);

    SDL_Quit();
}