#include <stdio.h>
#include "map.h" 

struct game_obj *game_objs;
struct game_player player;

bool generateMap(void) {
    int i=0;

    memset(game_objs, 0, sizeof(struct game_obj) * 256);

    game_objs[i++] = (struct game_obj){true, 10,10, 10,130, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 50,50, 10,110, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 10,50, 10,10, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 50,90, 110,110, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 10,210, 130,130, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 210,210, 130,160, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 230,230, 130,160, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 230,290, 130,130, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 290,290, 10,130, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 90,290, 10,10, "resources/wall.jpeg"};
    game_objs[i++] = (struct game_obj){true, 90,90, 10,110, "resources/wall.jpeg"};

    game_objs[i++] = (struct game_obj){true, 95,110, 25,17, "resources/w1.jpg"};
    game_objs[i++] = (struct game_obj){true, 120,135, 129,129, "resources/w2.jpg"};
    game_objs[i++] = (struct game_obj){true, 22,37, 30,30, "resources/w3.jpg"};

    player = (struct game_player){250, 250, 235};
    return true;  
}

float vectorCross(float x1, float y1, float x2, float y2) {
    return (x1 * y2) - (x2 * y1);
}

float getNearestObj(float angle, struct game_obj** obj, float* o_pos) {
    float l = -1;
    float ux = cos(angle * PI / 180);
    float uy = sin(angle * PI / 180);

    for(int i=0;game_objs[i].exists;++i) {
        struct game_obj* o = &(game_objs[i]);

        float o_l = sqrt(pow(o->x[0] - o->x[1], 2) + pow(o->y[0] - o->y[1], 2));
        float o_ux = (o->x[0] - o->x[1]) / o_l;
        float o_uy = (o->y[0] - o->y[1]) / o_l;

        float l1 = (vectorCross(o->x[1], o->y[1], o_ux, o_uy) - vectorCross(player.x, player.y, o_ux, o_uy)) / vectorCross(ux, uy, o_ux, o_uy);
        float l2 = (vectorCross(player.x, player.y, ux, uy) - vectorCross(o->x[1], o->y[1], ux, uy)) / vectorCross(o_ux, o_uy, ux, uy);

        if(l2 > 0 && l2 < o_l && l1 > 0 && (l1 < l || l < 0)) {
            *obj = o;
            *o_pos = l2;
            l = l1;
        }
    }
    return l;
}