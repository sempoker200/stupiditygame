#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"

#define PLAYER_RADIUS 20
#define SCRW 1366
#define SCRH 768

#define dapush(xs, x) do {                                           \
    if (xs.count >= xs.capacity){                                    \
        if (xs.capacity == 0) xs.capacity = 256;                     \
        else xs.capacity *= 2;                                       \
        xs.items = realloc(xs.items, xs.capacity*sizeof(*xs.items)); \
    }                                                                \
    xs.items[xs.count++] = x;                                        \
} while (0)


// REGION: OBJECT STRUCTURES
typedef struct {
    bool held;
    int rnd;
    int xpos;
    int ypos;
    int width;
    int height;
    Color bodyCol;
    Color textCol;
    char* text;
    int fontsize;
    bool enabled;
    void (*func)(void);
} Button;
typedef struct {
    Color textCol;
    int xpos;
    int ypos;
    int fontsize;
    char *text;
    bool enabled;
} Label;
typedef struct {
    Color col;
    int x;
    int y;
    int w;
    int h;
    int mass;
    bool enabled;
} MovableBox;
typedef struct {
    Color col;
    int x;
    int y;
    int w;
    int h;
    bool enabled;
} Box;
typedef struct {
    size_t count;
    size_t capacity;
    void **items;
} DA_VoidP;
typedef struct {
    size_t count;
    size_t capacity;
    Box *items;
} DA_Box;

// REGION: OBJECTS
Button play = (Button){};
Button pref = (Button){};
Button cprf = (Button){};
Button quit = (Button){};
Button fntb = (Button){};
Button bcgb = (Button){};
Label fontl = (Label){};
Label bckgl = (Label){};
Box box1 = (Box){};
Box box2 = (Box){};
Box box3 = (Box){};
Box box4 = (Box){};
Box box5 = (Box){};
Box box6 = (Box){};
Box box7 = (Box){};
MovableBox box8 = (MovableBox){};
MovableBox box9 = (MovableBox){};
MovableBox box10 = (MovableBox){};
MovableBox box11 = (MovableBox){};


// REGION: USER PREFERENCES
bool background = true;
bool font = false;

// REGION: GAME VARIABLES
float dt = 1.f/60.f;
DA_Box activeColliders;
DA_VoidP activeGameObjects;
Font fnt;
const char* title = "stumpid";
bool game = false;
Texture2D green = {0};


// REGION: PLAYER-RELATED VARIABLES
Vector2 playerPos = {0};
Vector2 camPos = {0};
Vector2 dir = {0};
float speed = 0.f;
float dashScale = 0.f;
MovableBox *pushing = NULL;


// REGION: MATH FUNCTIONS
bool valueInRange(int x, int lb, int ub){
    return x >= lb && x <= ub;
}
Vector2 worldspace(Vector2 x){
    return Vector2Add(Vector2Subtract(x, camPos), (Vector2){SCRW/2, SCRH/2});
}
bool raycast2D(Vector2 origin, Vector2 direction, int length){
    for (int i = 0; i < activeGameObjects.count; i++){
        Box gobj = *(Box*)activeGameObjects.items[i];
        Vector2 centerPos = {gobj.x+gobj.w/2, gobj.y+gobj.h/2};
        if (CheckCollisionCircleLine(centerPos, (gobj.w+gobj.h)/4, origin, Vector2Add(origin, Vector2Scale(direction, length)))) return true;
    }
    return false;
}


// REGION: BUTTON LOGIC
void optionsbtn(){
    play.enabled = false;
    pref.enabled = false;
    quit.enabled = false;
    cprf.enabled = true;
    fntb.enabled = true;
    bcgb.enabled = true;
    fontl.enabled = true;
    bckgl.enabled = true;
}
void switchbg(){
    background = !background;
    bcgb.text = background ? "V" : "";
}
void switchfont(){
    font = !font;
    fnt = font ? LoadFont("consola.ttf") : GetFontDefault();
    fntb.text = font ? "V" : "";
}
void clsoptbtn(){
    play.enabled = true;
    pref.enabled = true;
    quit.enabled = true;
    cprf.enabled = false;
    fntb.enabled = false;
    bcgb.enabled = false;
    fontl.enabled = false;
    bckgl.enabled = false;
}
void exitbtn(){
    CloseWindow();
}
void playbtn(){
    game = true;
    green = LoadTexture("./green.png");

    // OBJECT CREATION
    box1 = (Box) {
        GRAY,     40,   40,   100, 100,    true
    };
    box2 = (Box) {
        BLUE,     160,  160,  200, 200,    true
    };
    box3 = (Box) {
        RAYWHITE, 600,  160,  10,  200,    true
    };
    box4 = (Box) {
        RAYWHITE, 700,  160,  10,  200,    true
    };
    box5 = (Box) {
        RAYWHITE, 800,  160,  10,  200,    true
    };
    box6 = (Box) {
        RAYWHITE, 900,  160,  10,  200,    true
    };
    box7 = (Box) {
        RAYWHITE, 1000, 160,  10,  200,    true
    };
    box8 = (MovableBox) {
        MAROON,   -100, -100, 50,  50,  1, true
    };
    box9 = (MovableBox) {
        MAROON,  -160, -100, 75,  75, 2, true
    };
    box10 = (MovableBox) {
        MAROON,    -160, 0,    50,  50,  1, true
    };
    box11 = (MovableBox) {
        MAROON,    -160, -200, 50,  50,  1, true
    };
}


// REGION: OBJECT SIMULATION
void button(Button* b){
    if (!b->enabled) return;
    Color cb = b->bodyCol;
    if (valueInRange(GetMouseX(), b->xpos, b->xpos+b->width) && valueInRange(GetMouseY(), b->ypos, b->ypos+b->height)){
        cb = ColorBrightness(cb, 0.2f);
        if (IsMouseButtonDown(0)){
            cb = ColorBrightness(cb, -.2f);
            b->held = true;
        }
        else if (IsMouseButtonUp(0)){
            if (b->held) {
                b->func();
                b->held = false;
            }
        }
    }
    int rnd = b->rnd;
    int minside = ((b->width < b->height) ? b->width : b->height)/2;
    rnd = (rnd < minside) ? rnd : minside;

    if (rnd > 0){
        DrawCircle(b->xpos+rnd, b->ypos+rnd, rnd, cb);
        DrawCircle(b->xpos+b->width-rnd, b->ypos+b->height-rnd, rnd, cb);
        DrawCircle(b->xpos+rnd, b->ypos+b->height-rnd, rnd, cb);
        DrawCircle(b->xpos+b->width-rnd, b->ypos+rnd, rnd, cb);
        DrawRectangle(b->xpos, b->ypos+rnd, b->width, b->height-2*rnd, cb);
        DrawRectangle(b->xpos+rnd, b->ypos, b->width-2*rnd, b->height, cb);
    }
    else DrawRectangle(b->xpos, b->ypos, b->width, b->height, cb);
    DrawTextEx(fnt, b->text, (Vector2){b->width/2-MeasureTextEx(fnt, b->text, b->fontsize, 2).x/2+b->xpos, b->height/2-(MeasureTextEx(fnt, b->text, b->fontsize, 2).y)/2+b->ypos}, b->fontsize, 2, b->textCol);
}
void label(Label *l){
    if (!l->enabled) return;
    DrawTextEx(fnt, l->text, (Vector2){l->xpos, l->ypos}, l->fontsize, 2, l->textCol);
}
void movableBox(MovableBox *b){
    if (!b->enabled) return;
    dapush(activeGameObjects, b);
    if (CheckCollisionCircleRec(Vector2Add(playerPos, Vector2Scale(dir, (speed+dashScale))), PLAYER_RADIUS, (Rectangle){b->x, b->y, b->w, b->h})){
        bool collidesx, collidesy;
        for (int i = 0; i < activeColliders.count; i++) {
            Box col = activeColliders.items[i];
            if (!collidesx)
                collidesx = CheckCollisionRecs((Rectangle){col.x, col.y, col.w, col.h}, (Rectangle){b->x+dir.x*(speed+dashScale), b->y, b->w+dir.x*(speed+dashScale), b->h});
            if (!collidesy)
                collidesy = CheckCollisionRecs((Rectangle){col.x, col.y, col.w, col.h}, (Rectangle){b->x, b->y+dir.y*(speed+dashScale), b->w, b->h+dir.y*(speed+dashScale)});
        }
        b->x += dir.x*(speed+dashScale)*(int)!collidesx;
        b->y += dir.y*(speed+dashScale)*(int)!collidesy;
        playerPos = Vector2Subtract(playerPos, Vector2Scale(dir, (speed+dashScale)));
        pushing = b;
    }
    else{
        pushing = NULL;
    }
    DrawRectangleV(worldspace((Vector2){b->x-2, b->y-2}), (Vector2){b->w+4, b->h+4}, (Color){0, 0, 0, 120});
    DrawRectangleV(worldspace((Vector2){b->x, b->y}), (Vector2){b->w, b->h}, b->col);
}
void box(Box *b){
    if (!b->enabled) return;
    dapush(activeColliders, *b);
    dapush(activeGameObjects, b);
    DrawRectangleV(worldspace((Vector2){b->x-2, b->y-2}), (Vector2){b->w+4, b->h+4}, (Color){0, 0, 0, 120});
    DrawRectangleV(worldspace((Vector2){b->x, b->y}), (Vector2){b->w, b->h}, b->col);
}


// REGION: INPUT FUNCTIONS
bool moveUp(){
    return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
}
bool moveLeft(){
    return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
}
bool noMoveVert(){
    return (IsKeyUp(KEY_W) || IsKeyUp(KEY_UP)) && (IsKeyUp(KEY_S) || IsKeyUp(KEY_DOWN));
}
bool moveDown(){
    return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
}
bool moveRight(){
    return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
}
bool noMoveHoriz(){
    return ( IsKeyUp(KEY_D) || IsKeyUp(KEY_RIGHT) ) && ( IsKeyUp(KEY_A) || IsKeyUp(KEY_LEFT) );
}

// REGION: PLAYER LOGIC
void dash(){
    if (IsKeyPressed(KEY_SPACE)){
        dashScale = 1.6f/dt/.8f;
        while (raycast2D(playerPos, dir, dashScale)){
            if (dashScale > 1)
                dashScale /= 2;
            else
                break;
        }
    }
    dashScale *= 0.8f;
}
bool checkCollision(Vector2 futureOffset){
    for (int i = 0; i < activeColliders.count; i++) {
        Box col = activeColliders.items[i];
        Vector2 fpos = Vector2Add(playerPos, futureOffset);
        bool fl = CheckCollisionCircleRec(fpos, PLAYER_RADIUS, (Rectangle){col.x, col.y, col.w, col.h});
        if (fl) return true;
    }
    return false;
}

int main() {
    play = (Button){
        false, 10, SCRW/2-100, SCRH/3+75,  200, 50, LIGHTGRAY,    BLACK, "Play", 24, true, &playbtn
    };
    pref = (Button){
        false, 10, SCRW/2-100, SCRH/3+130, 200, 50, GRAY,         BLACK, "Preferences", 24, true, &optionsbtn
    };
    quit = (Button){
        false, 10, SCRW/2-100, SCRH/3+185, 200, 50, DARKGRAY,     BLACK, "Exit", 24, true, &exitbtn
    };
    cprf = (Button){
        false, 10, SCRW/2-100, SCRH/3+185, 200, 50, GRAY,         BLACK, "Close", 24, false, &clsoptbtn
    };
    fntb = (Button){
        false, 10, SCRW/2+70, SCRH/3+75,  40, 40, DARKGRAY,      BLACK, " ", 24, false, &switchfont
    };
    bcgb = (Button){
        false, 10, SCRW/2+70, SCRH/3+130,  40, 40, DARKGRAY,      BLACK, "V", 24, false, &switchbg
    };
    fontl = (Label){
        WHITE, SCRW/2-110, SCRH/3+85, 24, "Font: ", false
    };
    bckgl = (Label){
        WHITE, SCRW/2-110, SCRH/3+140, 24, "Background: ", false
    };
    InitWindow(SCRW, SCRH, title);
    fnt = font ? LoadFont("consola.ttf") : GetFontDefault();
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        if (!game) { // MAIN MENU
            BeginDrawing();
             ClearBackground(BLACK);
             DrawTextEx(fnt, title, (Vector2){SCRW/2-MeasureTextEx(fnt, title, 48, 2).x/2, SCRH/3}, 48, 2, WHITE);
             button(&play);
             button(&pref);
             button(&cprf);
             button(&quit);
             button(&fntb);
             button(&bcgb);
             label(&fontl);
             label(&bckgl);
            EndDrawing();
        } else {     // GAME
            // PRE-FRAME
            activeColliders = (DA_Box){0};
            activeGameObjects = (DA_VoidP){0};
            dt = GetFrameTime();
            speed = (IsKeyDown(KEY_LEFT_SHIFT) ? 600.f*dt : 300.f*dt)/(pushing == NULL ? 1 : pushing->mass);
            // DRAWING
            BeginDrawing();
             background ? DrawTextureEx(green, worldspace((Vector2){-6400, -6400}), 0, 50, WHITE) : ClearBackground(BLACK);
             DrawCircleV(worldspace(playerPos), PLAYER_RADIUS+2, (Color){0, 0, 0, 120});
             DrawCircleV(worldspace(playerPos), PLAYER_RADIUS, RED);
             DrawCircleV(worldspace(Vector2Add(Vector2Scale(dir, PLAYER_RADIUS-5), playerPos)), 10, (Color){128, 0, 0, 255});
             box(&box1);
             box(&box2);
             box(&box3);
             box(&box4);
             box(&box5);
             box(&box6);
             box(&box7);
             movableBox(&box8);
             movableBox(&box9);
             movableBox(&box10);
             movableBox(&box11);
             // for (int i = 0; i < activeGameObjects.count; i++){
             //     Box gobj = *(Box*)activeGameObjects.items[i];
             //     Vector2 centerPos = {gobj.x+gobj.w/2, gobj.y+gobj.h/2};
             //     DrawCircleV(worldspace(centerPos), (gobj.w+gobj.h)/4, WHITE);
             // }
             // box9.enabled = false;
             // DrawFPS(10, 10);
            EndDrawing();
            // MOVEMENT
            Vector2 fdir = dir;
            if (moveUp()){
                fdir.y = -1.f;
                if (checkCollision(Vector2Scale(fdir, (speed+dashScale))))
                    fdir.y = 0.f;
            }
            else if (moveDown()){
                fdir.y = 1.f;
                if (checkCollision(Vector2Scale(fdir, (speed+dashScale))))
                    fdir.y = 0.f;
            }
            else if (noMoveVert()){
                fdir.y = 0.f;
            }
            if (moveLeft()){
                fdir.x = -1.f;
                if (checkCollision(Vector2Scale(fdir, (speed+dashScale))))
                    fdir.x = 0.f;
            }
            else if (moveRight()){
                fdir.x = 1.f;
                if (checkCollision(Vector2Scale(fdir, (speed+dashScale))))
                    fdir.x = 0.f;
            }
            else if (noMoveHoriz()){
                fdir.x = 0.f;
            }
            fdir = Vector2Normalize(fdir);
            dir = fdir;
            playerPos = Vector2Add(Vector2Scale(dir, (speed+dashScale)), playerPos);
            dash();
            camPos = Vector2Lerp(camPos, playerPos, .3f);
        }
    }
    CloseWindow();
    return 0;
}