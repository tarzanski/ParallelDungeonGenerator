
#include <SDL.h>

// one extra pixel so all lines can be drawn
#define SCREEN_WIDTH 640

#define SCREEN_HEIGHT 480

// #define PIX_PER_UNIT 5

class display {
    private:
        bool running;

        int currRoomNumber;

        int pixPerUnit;

        SDL_Window* sdlwindow;

        SDL_Surface* gScreenSurface;

        SDL_Surface* gRoom;

        SDL_Surface* gSides;

        SDL_Surface* gGrey;

        SDL_Surface* gRed;

    public:
        // constructor
        display();

        int OnExecute(rectangle_t* rooms, int roomNum);
 
        bool OnInit();
 
        void OnEvent(SDL_Event* Event);
 
        void OnLoop();
 
        void OnRender(rectangle_t *rooms, int roomNum);
 
        void OnCleanup();

        // other functions

        void genBackround();

        void loadAssets();
};