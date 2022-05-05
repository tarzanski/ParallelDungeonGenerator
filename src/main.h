
#include <SDL.h>

// generate constants
#define ISPC
#define MAX_ITERS 20000
#define VSTUDIO

// one extra pixel so all lines can be drawn
#define SCREEN_WIDTH 1280

#define SCREEN_HEIGHT 1024

// bit masks and bit assignments
#define BIT_INCLUDED  1 << 0

#define BIT_MAINROOM  1 << 1

#define BIT_NO_B_EDGE 1 << 2

#define BIT_NO_T_EDGE 1 << 3

#define BIT_NO_L_EDGE 1 << 4

#define BIT_NO_R_EDGE 1 << 5

class display {
    private:
        // SDL runtime variables
        bool running;
        SDL_Window *sdlwindow;
        SDL_Renderer *renderer;
        //SDL_Surface* gScreenSurface;
        SDL_Texture *gRoom;
        SDL_Texture *gSides;
        SDL_Texture *gGrey;
        SDL_Texture *gRed;
        SDL_Texture *gMain;
        SDL_Texture *gHall;
        SDL_Texture *gBlack;

        // gui display parameters
        int currRoomNumber;
        int pixPerUnit;
        int x_offset;
        int y_offset;
        int room_view; // 0 = all, 1 = main, 2 = included
        int show_hallways;
        int show_tree; // 0 = none, 1 = mst, 2 = dela
        int animate; // 0 = no animaion 1 = can animate
        int animate_on; // 0 = off, 1 = on
        int hall_type; // 0 = full, 1 = line

        // data pointers
        dungeon_t *dungeon_data;
        rectangle_t **room_data;
        double_edge_t *mst_dela_data;

    public:
        // constructor
        display(dungeon_t *dungeon, int animate_set, rectangle_t **room_hist, double_edge_t *mst_dela);
        int OnExecute(dungeon_t *dungeon, double_edge_t *mst_dela);
        bool OnInit();
        void OnEvent(SDL_Event* Event);
        void OnLoop();
        void OnRender(dungeon_t *dungeon, double_edge_t *mst_dela);
        void OnCleanup();

        // other functions
        void genBackground();
        void loadAssets();
        void renderRoom(rectangle_t room, SDL_Texture *roomTex, SDL_Texture *sideTex);
        void renderEdge(rectangle_t roomsrc, rectangle_t roomdest);
        void renderHallLine(int show_hallways);
        void renderHallFull(int show_hallways);
        int animation(int animate_sep_step);
};
