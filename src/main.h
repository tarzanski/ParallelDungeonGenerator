
#include <SDL.h>

// one extra pixel so all lines can be drawn
#define SCREEN_WIDTH 1280

#define SCREEN_HEIGHT 1024

class display {
    private:
        // SDL runtime variables
        bool running;
        SDL_Window* sdlwindow;
        SDL_Renderer* renderer;
        //SDL_Surface* gScreenSurface;
        SDL_Texture* gRoom;
        SDL_Texture* gSides;
        SDL_Texture* gGrey;
        SDL_Texture* gRed;

        // gui display parameters
        int currRoomNumber;
        int pixPerUnit;
        int x_offset;
        int y_offset;
        int room_view; // 0 = all, 1 = main, 2 = included
        int show_hallways;
        int show_tree; // 0 = none, 1 = mst, 2 = dela

        // data pointers
        dungeon_t *dungeon_data;

    public:
        // constructor
        display(dungeon_t *dungeon);
        int OnExecute(dungeon_t *dungeon, double_edge_t *mst_dela);
        bool OnInit();
        void OnEvent(SDL_Event* Event);
        void OnLoop();
        void OnRender(dungeon_t *dungeon, double_edge_t *mst_dela);
        void OnCleanup();

        // other functions
        void genBackround();
        void loadAssets();
        void renderRoom(rectangle_t room);
};
