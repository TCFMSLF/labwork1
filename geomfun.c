#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <time.h>

#define W 800
#define H 600
#define MAX 32
#define ACCEL 0.6
#define FRICTION 0.95
#define MAX_SPD 8.0
#define DEFORM_SCALE 0.3
#define STIFFNESS 0.15
#define DAMPING 0.2
#define MAX_VERTS 8

typedef struct { double x, y, z; } V;

typedef enum {
    SHAPE_CUBE,
    SHAPE_TETRAHEDRON,
    SHAPE_OCTAHEDRON,
    SHAPE_PYRAMID,
    SHAPE_PRISM,
    NUM_SHAPES
} ShapeType;

static const V cube_verts[MAX_VERTS] = {
    {-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1},
    {-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1}
};
static const int cube_edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

static const V tet_verts[MAX_VERTS] = {
    {1,1,1},{1,-1,-1},{-1,1,-1},{-1,-1,1}
};
static const int tet_edges[6][2] = {
    {0,1},{0,2},{0,3},
    {1,2},{1,3},{2,3}
};

static const V oct_verts[MAX_VERTS] = {
    {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
};
static const int oct_edges[12][2] = {
    {0,2},{0,3},{0,4},{0,5},
    {1,2},{1,3},{1,4},{1,5},
    {2,4},{4,3},{3,5},{5,2}
};

static const V pyr_verts[MAX_VERTS] = {
    {-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1},
    {0,1.5,0}
};
static const int pyr_edges[8][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {0,4},{1,4},{2,4},{3,4}
};

static const V prism_verts[MAX_VERTS] = {
    {0,-1,1},{1,-1,-1},{-1,-1,-1},
    {0,1,1},{1,1,-1},{-1,1,-1}
};
static const int prism_edges[9][2] = {
    {0,1},{1,2},{2,0},
    {3,4},{4,5},{5,3},
    {0,3},{1,4},{2,5}
};

typedef struct {
    const V *verts;
    int nverts;
    const int (*edges)[2];
    int nedges;
} ShapeDef;

static const ShapeDef shapedefs[NUM_SHAPES] = {
    {cube_verts,  8, cube_edges,  12},
    {tet_verts,   4, tet_edges,   6},
    {oct_verts,   6, oct_edges,   12},
    {pyr_verts,   5, pyr_edges,   8},
    {prism_verts, 6, prism_edges, 9},
};

typedef struct {
    ShapeType type;
    double rx, ry;
    double sx, sy;
    double ox, oy;
    double vx, vy;
    double dx, dy;
    double vdx, vdy;
    double scale;
    Color color;
} Shape;

Shape shapes[MAX];
int nshapes = 0, cur = 0;

void rot(V *v, double rx, double ry) {
    double x=v->x, y=v->y, z=v->z;
    double cx=cos(rx), sx=sin(rx);
    double cy=cos(ry), sy=sin(ry);
    double y1 = y*cx - z*sx;
    double z1 = y*sx + z*cx;
    double x2 = x*cy + z1*sy;
    double z2 = -x*sy + z1*cy;
    v->x = x2; v->y = y1; v->z = z2;
}

Color rand_color() {
    double h = (double)rand()/RAND_MAX * 360;
    double s = 0.7 + (double)rand()/RAND_MAX * 0.3;
    double v = 0.8 + (double)rand()/RAND_MAX * 0.2;
    int i = (int)(h / 60) % 6;
    double f = h/60 - i;
    double p = v*(1-s);
    double q = v*(1-s*f);
    double t = v*(1-s*(1-f));
    double r, g, b;
    switch (i) {
        case 0: r=v;g=t;b=p;break;
        case 1: r=q;g=v;b=p;break;
        case 2: r=p;g=v;b=t;break;
        case 3: r=p;g=q;b=v;break;
        case 4: r=t;g=p;b=v;break;
        case 5: r=v;g=p;b=q;break;
    }
    return (Color){ (unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), 255 };
}

void add_shape() {
    if (nshapes >= MAX) return;
    Shape *s = &shapes[nshapes++];
    s->type = rand() % NUM_SHAPES;
    s->rx = (double)rand()/RAND_MAX * 6.28;
    s->ry = (double)rand()/RAND_MAX * 6.28;
    s->sx = ((double)rand()/RAND_MAX - 0.5) * 0.04;
    s->sy = ((double)rand()/RAND_MAX - 0.5) * 0.04;
    s->ox = ((double)rand()/RAND_MAX - 0.5) * 300;
    s->oy = ((double)rand()/RAND_MAX - 0.5) * 200;
    s->vx = 0; s->vy = 0;
    s->dx = s->dy = s->vdx = s->vdy = 0;
    s->scale = 0.5 + (double)rand()/RAND_MAX * 60;
    s->color = rand_color();
    cur = nshapes - 1;
}

int main() {
    srand(time(NULL));
    InitWindow(W, H, "Shapes (raylib)");
    SetTargetFPS(60);

    add_shape();

    while (!WindowShouldClose()) {
        // rot
        if (IsKeyPressed(KEY_LEFT))  shapes[cur].sy -= 0.002;
        if (IsKeyPressed(KEY_RIGHT)) shapes[cur].sy += 0.002;
        if (IsKeyPressed(KEY_UP))    shapes[cur].sx += 0.002;
        if (IsKeyPressed(KEY_DOWN))  shapes[cur].sx -= 0.002;
        if (IsKeyPressed(KEY_SPACE)) shapes[cur].sx = shapes[cur].sy = 0;

        // other 
        if (IsKeyPressed(KEY_N) && nshapes < MAX) add_shape();
        if (IsKeyPressed(KEY_LEFT_BRACKET))  cur = (cur - 1 + nshapes) % nshapes;
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) cur = (cur + 1) % nshapes;
        if (IsKeyPressed(KEY_ESCAPE)) break;

        // phys
        for (int i = 0; i < nshapes; i++) {
            Shape *s = &shapes[i];
            s->rx += s->sx;
            s->ry += s->sy;

            double px = s->vx, py = s->vy;

            if (i == cur) {
                if (IsKeyDown(KEY_W))      s->vy -= ACCEL;
                if (IsKeyDown(KEY_S))      s->vy += ACCEL;
                if (IsKeyDown(KEY_A))      s->vx -= ACCEL;
                if (IsKeyDown(KEY_D))      s->vx += ACCEL;
            }

            s->vx *= FRICTION;
            s->vy *= FRICTION;
            if (fabs(s->vx) < 0.01) s->vx = 0;
            if (fabs(s->vy) < 0.01) s->vy = 0;
            double spd = sqrt(s->vx*s->vx + s->vy*s->vy);
            if (spd > MAX_SPD) {
                s->vx = s->vx/spd * MAX_SPD;
                s->vy = s->vy/spd * MAX_SPD;
            }

            double ax = s->vx - px;
            double ay = s->vy - py;
            double tx = fmin(1, fmax(-1, ax * DEFORM_SCALE));
            double ty = fmin(1, fmax(-1, ay * DEFORM_SCALE));

            s->vdx += (tx - s->dx) * STIFFNESS - s->vdx * DAMPING;
            s->vdy += (ty - s->dy) * STIFFNESS - s->vdy * DAMPING;
            s->dx += s->vdx;
            s->dy += s->vdy;
            s->dx = fmin(1, fmax(-1, s->dx));
            s->dy = fmin(1, fmax(-1, s->dy));

            s->ox += s->vx;
            s->oy += s->vy;

            double margin = 100.0;
            if (s->ox > W/2 + margin) s->ox = -W/2 - margin;
            if (s->ox < -W/2 - margin) s->ox = W/2 + margin;
            if (s->oy > H/2 + margin) s->oy = -H/2 - margin;
            if (s->oy < -H/2 - margin) s->oy = H/2 + margin;
        }

      
        BeginDrawing();
        ClearBackground(BLACK);

        for (int ci = 0; ci < nshapes; ci++) {
            Shape *s = &shapes[ci];
            const ShapeDef *def = &shapedefs[s->type];

            V p[MAX_VERTS];
            for (int i = 0; i < def->nverts; i++) {
                p[i] = def->verts[i];
                rot(&p[i], s->rx, s->ry);
                double f = 4.0 / (4.0 + p[i].z);
                p[i].x = W/2 + s->ox + p[i].x * s->scale * f;
                p[i].y = H/2 + s->oy - p[i].y * s->scale * f;
            }

            double cx = W/2 + s->ox;
            double cy = H/2 + s->oy;
            for (int i = 0; i < def->nverts; i++) {
                double rx = p[i].x - cx;
                double ry = p[i].y - cy;
                p[i].x = cx + rx * (1 - s->dx);
                p[i].y = cy + ry * (1 - s->dy);
            }

            for (int i = 0; i < def->nedges; i++)
                DrawLine((int)p[def->edges[i][0]].x, (int)p[def->edges[i][0]].y,
                         (int)p[def->edges[i][1]].x, (int)p[def->edges[i][1]].y,
                         s->color);
        }

        
        int sx = W/2 + shapes[cur].ox;
        int sy = H/2 + shapes[cur].oy;
        DrawLine(sx-5, sy, sx+5, sy, WHITE);
        DrawLine(sx, sy-5, sx, sy+5, WHITE);

        char buf[96];
        snprintf(buf, sizeof(buf), "%d/%d  [N]ew  [][]sel  WASD move  arrows spin  Space stop", cur+1, nshapes);
        DrawText(buf, 10, 10, 14, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}