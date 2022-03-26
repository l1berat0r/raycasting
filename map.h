#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265359

struct game_obj {
    bool exists;
    int x[2];
    int y[2];
    char texture[1024];
};

struct game_player {
    int x;
    int y;
    int a;
};

extern struct game_obj *game_objs;
extern struct game_player player;
bool generateMap(void);
float getNearestObj(float, struct game_obj**, float*);