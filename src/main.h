
#include <SDL.h>

// one extra pixel so all lines can be drawn
#define SCREEN_WIDTH 1280

#define SCREEN_HEIGHT 1024

class display {
    private:
        // SDL runtime variables
        bool running;
        SDL_Window* sdlwindow;
        SDL_Surface* gScreenSurface;
        SDL_Surface* gRoom;
        SDL_Surface* gSides;
        SDL_Surface* gGrey;
        SDL_Surface* gRed;

        // gui display parameters
        int currRoomNumber;
        int pixPerUnit;
        int x_offset;
        int y_offset;
        bool only_main;
        int show_hallways;

        // data pointers
        dungeon_t *dungeon_data;

    public:
        // constructor
        display(dungeon_t *dungeon);
        int OnExecute(dungeon_t *dungeon);
        bool OnInit();
        void OnEvent(SDL_Event* Event);
        void OnLoop();
        void OnRender(dungeon_t *dungeon);
        void OnCleanup();

        // other functions
        void genBackround();
        void loadAssets();
        void renderRoom(rectangle_t room);
};