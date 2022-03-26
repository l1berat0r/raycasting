#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>

#include "map.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#define NIL (0)

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define FOV 60.0

#define BLOCK_SIZE 30

struct game_img {
    char texture[1024];
    int height;
    int width;
    SDL_Surface* surface;
};

struct game_img game_resources[256];

SDL_Renderer* ren;
SDL_Window* win;
SDL_Surface* srfc;

//bool running = true;
char mode = 'V';

SDL_Surface* getSurface(char* texture) {
    for(int i=0; i<256; ++i) {
        if(strcmp(texture, game_resources[i].texture) == 0) {
            return game_resources[i].surface;
        }
    }
    return 0;
}

struct game_img* getResource(char* texture) {
    for(int i=0; i<256; ++i) {
        if(strcmp(texture, game_resources[i].texture) == 0) {
            return &(game_resources[i]);
        }
    }
    return 0;
}

bool addResource(char* texture, int width, int height) {
    int i=0;

    for(i=0; *(game_resources[i].texture) != 0; ++i);

    strcpy(game_resources[i].texture, texture);
    
    SDL_Surface* tmp_surface = IMG_Load(texture);
    SDL_Rect tmp_rect = {0, 0, tmp_surface->w, tmp_surface->h};
    SDL_Rect dst_rect = {0, 0, width, height};

    game_resources[i].surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    if(SDL_BlitScaled(tmp_surface, &tmp_rect, game_resources[i].surface, &dst_rect) < 0) {
        printf("SDL error: %s\n", SDL_GetError());
    }

    game_resources[i].width = width;
    game_resources[i].height = height;
    
    SDL_FreeSurface(tmp_surface);
    return game_resources[i].surface ? true : false;
}

bool loadResources() {
    bool success = true;

    memset(game_resources, 0, sizeof(struct game_img) *256 );

    addResource("resources/grass.jpg", 640, 480);
    addResource("resources/sky.jpg", 640, 480);
    addResource("resources/wall.jpeg", 640, 480);
    addResource("resources/w1.jpg", 640, 480);
    addResource("resources/w2.jpg", 640, 480);
    addResource("resources/w3.jpg", 640, 480);

    return success;
}

void drawLine(void *pixels, int x1, int x2, int y1, int y2, int color) {
    bool draw_x = abs(x2-x1) > abs(y2-y1);
    int dir = draw_x ? ((x2-x1) / abs(x2-x1)) : ((y2-y1) / abs(y2-y1));
    float ratio = draw_x ? (float)(y2-y1) / (float)(x2-x1) : (float)(x2-x1) / (float)(y2-y1);


    for(int i=0; i < (draw_x ? abs(x2-x1) : abs(y2-y1)); ++i) {        
        int _x = draw_x ? x1 + (i*dir) : x1 +(ratio * (i*dir));
        int _y = draw_x ? y1 + (ratio * (i*dir)) : y1 + (i*dir);
        //printf("%d %d \n", _x, _y);
        ((Uint32*)pixels)[_y * WINDOW_WIDTH + _x] = color;
    }
}

void drawSegment(void* pixels, float i, float h, struct game_obj* obj, float o_pos) {
    struct game_img* tex = getResource(obj->texture);
    int j=0;
    //for(int j=0; j<=WINDOW_WIDTH / 60 && ((i+30.0)*(WINDOW_WIDTH / 60.0) + j) < WINDOW_WIDTH;++j) {
        int h_start = (WINDOW_HEIGHT / 2) - h;
        int h_stop = (WINDOW_HEIGHT / 2) + h;

        int d_h = 0;

        if(h_start < 0) {
            d_h = h_start;
            h_start = 0;        
        }
        if(h_stop > WINDOW_HEIGHT) h_stop = WINDOW_HEIGHT;

        for(int x=h_start;x<h_stop;++x) {
            float p_x =((tex->height / (2.0 * h)) * (x - h_start - d_h));
            int i_p_x = (int)p_x;
            int c1, c2;

            c1 = ((Uint32*)tex->surface->pixels)[(int)(i_p_x * tex->width + ((int)(o_pos * 40) % tex->width))];
            
            if(i_p_x + 1 > tex->height) {
                c2 = c1;
            } else {
                c2 = ((Uint32*)tex->surface->pixels)[(int)((i_p_x + 1) * tex->width + ((int)(o_pos * 40) % tex->width))];
            }

            int color = c1 * (p_x - i_p_x) + c1 * (1 - (p_x - i_p_x));

            ((Uint32*)pixels)[(int)(x * WINDOW_WIDTH + ((i+30.0)*(WINDOW_WIDTH / 60.0) + j))] = color;
        }
    //}
}

bool drawScene(void *pixels, char mode) {
    SDL_Surface *sky, *grass;
    
    switch(mode) {
        case 'V':
            sky = getSurface("resources/sky.jpg");
            grass = getSurface("resources/grass.jpg");

            for(int i=0; i<WINDOW_HEIGHT / 2; ++i) {
                for(int j=0; j<WINDOW_WIDTH; ++j) {
                    // ((Uint32*)pixels)[i*WINDOW_WIDTH + j] = ((Uint32*)sky->pixels)[i*WINDOW_WIDTH + j];
                    // ((Uint32*)pixels)[(i + (WINDOW_HEIGHT / 2))*WINDOW_WIDTH + j] = ((Uint32*)grass->pixels)[i*WINDOW_WIDTH + j];
                    ((Uint32*)pixels)[i*WINDOW_WIDTH + j] = 0x888888;
                    ((Uint32*)pixels)[(i + (WINDOW_HEIGHT / 2))*WINDOW_WIDTH + j] = 0x444444;
                }
            }

            for(float i=-30; i<=30;i = i + (FOV / WINDOW_WIDTH)) {
            //for(int i=0; i<1;++i) {
                struct game_obj* o;
                float o_pos;
                float l = getNearestObj(player.a+i, &o, &o_pos);
                //printf("%f \n", l);
                if(l<=0) continue;

                l = l * cos(i * PI / 180.0);
                float h = ( (WINDOW_HEIGHT / 2) / (l*0.1));
                //if(h > (WINDOW_HEIGHT / 2)) h = (WINDOW_HEIGHT / 2);

                drawSegment(pixels, i, h, o, o_pos);

                // for(int j=0; j<=WINDOW_WIDTH / 60 && ((i+30.0)*(WINDOW_WIDTH / 60.0) + j) < WINDOW_WIDTH;++j) {
                //     //printf("%d %d %f \n", i, j, (i+30.0)*(WINDOW_WIDTH / 60.0) + j);
                //     drawLine(pixels, 
                //         (i+30.0)*(WINDOW_WIDTH / 60.0) + j, 
                //         (i+30.0)*(WINDOW_WIDTH / 60.0) + j, 
                //         (WINDOW_HEIGHT / 2) - h,
                //         (WINDOW_HEIGHT / 2) + h,
                //         0x444488);
                // }
            }

            break;
        case 'M':
            for(int i=0; i<WINDOW_HEIGHT; ++i) {
                for(int j=0; j<WINDOW_WIDTH; ++j) {
                    ((Uint32*)pixels)[i*WINDOW_WIDTH + j] = 0;
                }
            }

            for(int i=0;game_objs[i].exists;++i) {                
                drawLine(pixels, game_objs[i].x[0], game_objs[i].x[1], game_objs[i].y[0], game_objs[i].y[1], 0x888888);
            }

            ((Uint32*)pixels)[player.y * WINDOW_WIDTH + player.x] = 0xFF1144;

            //for(int i=-180; i<=180;++i) {
            for(float i=-30; i<=30;i = i + (FOV / WINDOW_WIDTH)) {
            //for(int i=0; i<1;++i) {
                struct game_obj* o;
                float o_pos;
                float l = getNearestObj(player.a+i, &o, &o_pos);
                //printf("%f \n", l);
                if(l<=0) l = 60;

                float lx = player.x + l * cos((player.a+i) * PI / 180);
                float ly = player.y + l * sin((player.a+i) * PI / 180);

                if(lx<0) lx = 0;
                if(lx > WINDOW_WIDTH) lx = WINDOW_WIDTH;
                if(ly<0) ly = 0;
                if(ly > WINDOW_HEIGHT) ly = WINDOW_HEIGHT;

                drawLine(pixels, 
                    player.x, 
                    lx , 
                    player.y, 
                    ly, 
                    0x11FF88);
            }

            break;
    }
    return true;
}

bool mainLoop() {
    SDL_Event e;
    bool running = true;
   

    while( SDL_PollEvent( &e ) != 0 ) {
            
        if( e.type == SDL_QUIT ) {
            running = false;
        } else if( e.type == SDL_KEYDOWN ) {
            switch( e.key.keysym.sym )
            {
                case SDLK_UP:
                    player.x += 10 * cos(player.a * PI / 180);
                    player.y += 10 * sin(player.a * PI / 180);
                break;

                case SDLK_DOWN:
                    player.x -= 10 * cos(player.a * PI / 180);
                    player.y -= 10 * sin(player.a * PI / 180);
                break;

                case SDLK_LEFT:
                    player.a = (player.a - 5) % 360;
                    //printf("%d\n", player.a);
                break;

                case SDLK_RIGHT:
                    player.a = (player.a + 5) % 360;
                    //printf("%d\n", player.a);
                break;

                case SDLK_q:
                    running = false;
                break;

                case SDLK_v:
                    printf("V click\n");
                    if(mode=='M') mode = 'V';
                    else if(mode=='V') mode = 'M';
                break;

                default:
                //OTHER
                break;
            }
        }
    }

    //Uint32* pixels = srfc->pixels;
    //for(int i=0; i<640*480;++i) {
        //pixels[i] = (Uint32)(rand()*0xFFFFFF);
    //}

    if (SDL_MUSTLOCK(srfc)) SDL_LockSurface(srfc);

    drawScene(srfc->pixels, mode);
    //printf("update sufrace\n");
    //Update the surface
    //SDL_UpdateWindowSurface( win );

    if (SDL_MUSTLOCK(srfc)) SDL_UnlockSurface(srfc);

    SDL_Texture *screenTexture = SDL_CreateTextureFromSurface(ren, srfc);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, screenTexture, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_DestroyTexture(screenTexture);

    return running;
}

int main(int argc, char** argv) {    
    time_t tt;
    bool running = true;
    srand(time(&tt));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(640, 480, 0, &win, &ren);

    //SDL_SetWindowTitle(win, ":)");

    srfc = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);

    //srfc = SDL_GetWindowSurface( win );

    game_objs = malloc(sizeof(struct game_obj) * 512);
    loadResources();
    generateMap();

    //Fill the surface white
    //SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xff, 0x00, 0x33 ) );

    //while(running) {
        
    //}
    
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(mainLoop, 0, true);
    #else
        while(running) {running = mainLoop();}
    #endif

    SDL_Delay(1000);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}