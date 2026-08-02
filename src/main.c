#include <stdio.h>
#include <stdint.h> 
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define PI 3.1415
#define HFOV (0.77f * SCREEN_HEIGHT)
#define VFOV (0.2f * SCREEN_HEIGHT)
// use GCC's typeof shi in teh swap 

typedef struct vec2d { int x; int y; } vec2d_t; 
typedef struct vec3d { int x; int y; int z; } vec3d_t; 
typedef struct fvec2d { float x; float y; } fvec2d_t; 
typedef struct fvec3d { float x; float y; float z; } fvec3d_t; 

typedef enum roomsz {WALL1, WALL2, FLOOR, CEILING} room_parts; 


typedef struct state { // abstract state that can apply to multipe objects
    int last_sector; //this is lastknownsector from the build style documnets thingy....
    unsigned char id; // id of whatever entity.... player is p, anyting else is tbd... made it unsigned jus cuz idk maybe there'll be a purpose for this 
} state_t; 

typedef struct engine { 
    SDL_Window *window; 
    SDL_Renderer *renderer; 
} engine_t; 


typedef struct wall{
     int index;
     vec2d_t start_pos; 
     vec2d_t end_pos;
     int portal;
} wall_t; 

typedef struct sector {
     int id;
     int num_walls; 
     int wall_index;
     float ceiling_height;
     float floor_height; 
} sector_t; 

typedef struct level {
    sector_t *sectors; 
    wall_t *walls; 
} level_t; 

typedef struct player {
    fvec2d_t position;
    state_t state; // has the player's id as well as the player's last position
    float height; // how high are the player's eyes in the space
    float look_angle; // in radians
} player_t; 

void parse_level_data(FILE *level_data, level_t *level, int sector_count);
fvec2d_t intersection(int x1,int y1, int x2,int y2, int x3,int y3, int x4, int y4);

static inline void vline_draw(engine_t *engine, int x, int y1, int y2, room_parts part); 

int main(int argc, char *argv[]){
    engine_t engine; 
    FILE * level_data = fopen("../src/level.txt", "r");

    level_t LEVEL; 

    parse_level_data(level_data, &LEVEL, 3);

    

    player_t player = {
        .position = (fvec2d_t) {
            .x = 3.0f, 
            .y = 2.0f,
        },
        .look_angle = PI/2,
        .state = (state_t) {
            .id = 'p',
            .last_sector = 0, 
        }, 
        .height = 0.5f

    };


    int running = 1; 
    SDL_Event event;

    if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) ){
        printf("SDL_Init Error: %s \n", SDL_GetError());
        return 1;
    }
    if(TTF_Init() < 0 ){
        printf("TTF_Init Error%s \n", TTF_GetError());
        return 1; 
    }

    engine.window = SDL_CreateWindow(
        "Renderer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN 
    );

    engine.renderer = SDL_CreateRenderer(engine.window, -1, SDL_RENDERER_SOFTWARE); 

    
    
    uint64_t time = SDL_GetPerformanceCounter();
    double dt; 
    uint64_t old_time = time;

    float rotation_speed = PI/5; 

    while(running){
        while (SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT: 
                {
                    running = 0;
                    break; 
                }
            }
        }
        
        old_time = time; 
        time = SDL_GetPerformanceCounter(); 
        dt = (double) (time - old_time) / (double) SDL_GetPerformanceFrequency(); 
        

        SDL_SetRenderDrawColor(engine.renderer, 20, 20, 20, 255);
        SDL_RenderClear(engine.renderer);
       
        player.look_angle += rotation_speed * dt ; 
        /*
            put the render code here
        */
        
        // just code for rendering the first sector
        for(int i = 0; i < LEVEL.sectors[0].num_walls; i++){
            wall_t curr_wall = LEVEL.walls[ (LEVEL.sectors[0].wall_index + i) ]; 
            // change to player basis (think change of basis in linear algebra..) 
            fvec2d_t rv1 = { .x = curr_wall.start_pos.x - player.position.x, .y = curr_wall.start_pos.y - player.position.y };
            fvec2d_t rv2 = { .x = curr_wall.end_pos.x - player.position.x, .y = curr_wall.end_pos.y - player.position.y }; 
            // ^^^^ these are the positions of teh walls relatiive to teh player.. now we have to rotate them using js the 2d rotation matrix
            
            float cos_a = cosf(player.look_angle);
            float sin_a = sinf(player.look_angle);
            
            float old_x = rv1.x;
            float old_y = rv1.y;

            rv1.x = old_x * sin_a - old_y * cos_a;
            rv1.y = old_x * cos_a + old_y * sin_a;
            
            old_x = rv2.x;
            old_y = rv2.y;

            rv2.x = old_x * sin_a - old_y * cos_a;
            rv2.y = old_x * cos_a + old_y * sin_a;

            if (rv1.y <= 0 && rv2.y <= 0) continue; // the wall is cmpletely behind the camera then we dont have to render it

            if (rv1.y <= 0 || rv2.y <= 0){ // check if part of the wall is behind the player and if it is then we need to do clipping trick so our shi isnt fucked up
                const float near_y = 0.01f;

                if (rv1.y <= near_y && rv2.y <= near_y) continue;

                if (rv1.y <= near_y || rv2.y <= near_y){
                    float t = (near_y - rv1.y) / (rv2.y - rv1.y);

                    float intersection_x = rv1.x + t * (rv2.x - rv1.x);

                    if (rv1.y <= near_y){
                        rv1.x = intersection_x;
                        rv1.y = near_y;
                    }
                    else{
                        rv2.x = intersection_x;
                        rv2.y = near_y;
                    }
                }
            }


            // here's perspective transformation.. the first two variabels are how much we scale the x and y coordinates to map them onto screen space
            float xscale1 = HFOV / rv1.y, yscale1 = VFOV / rv1.y; 
            float xscale2 = HFOV / rv2.y, yscale2 = VFOV / rv2.y;  

            int x1 = SCREEN_WIDTH/2 - (int)(rv1.x * xscale1); 
            int x2 = SCREEN_WIDTH/2 - (int)(rv2.x * xscale2); 


            float ceil_height = LEVEL.sectors[0].ceiling_height - player.height;
            float floor_height = LEVEL.sectors[0].floor_height - player.height;

            int y1a = SCREEN_HEIGHT / 2 - (int)(ceil_height * yscale1);
            int y1b = SCREEN_HEIGHT / 2 - (int)(floor_height * yscale1);

            int y2a = SCREEN_HEIGHT / 2 - (int)(ceil_height * yscale2); 
            int y2b = SCREEN_HEIGHT / 2 - (int)(floor_height * yscale2); 
            


            if (x1 > x2){ // swapping porjected enpoitns for rihgt now ebcause the way the build engin renders shi is in a particular winding order that i accidently used the opposite of....
                int temp;

                temp = x1;
                x1 = x2;
                x2 = temp;

                temp = y1a;
                y1a = y2a;
                y2a = temp;

                temp = y1b;
                y1b = y2b;
                y2b = temp;
            }   

            if ( x1 == x2 || x2 < 0 || x1 >= SCREEN_WIDTH) continue;
            
            float wall_dx = fabsf(curr_wall.end_pos.x - curr_wall.start_pos.x); 
            float wall_dy = fabsf(curr_wall.end_pos.y - curr_wall.start_pos.y);

            int begin_x = x1 < 0 ? 0 : x1;
            int end_x = (x2 >= SCREEN_WIDTH) ? SCREEN_WIDTH - 1 : x2;

            for (int x = begin_x; x < end_x; x++){
                float percentage = (float) (x - x1) / (float) (x2 - x1); // how far along the wall we are 
                int ya = (int) (y1a + (percentage * (y2a - y1a))); 
                int yb = (int) (y1b + (percentage * (y2b - y1b))); 
                
                /* 
                    NOW DRAW THE TS ONTO THE SCREEEEENEN AYEEEEE 
                */
                
                vline_draw(&engine, x, 0, ya - 1, CEILING); // render the ceiling

                if(wall_dx <= wall_dy){// this is just for some basic shading purposes... the wall slopes upwards or the wall slopes downwards in space
                    vline_draw(&engine, x, ya, yb, WALL1); // the wall
                }else{ 
                    vline_draw(&engine, x, ya, yb, WALL2);
                }
                
                vline_draw(&engine, x, yb, SCREEN_HEIGHT - 1, FLOOR); // the floor
            }
        }

        printf("FPS : %f \n", (float) 1/dt); 


        SDL_SetRenderDrawColor(engine.renderer, 20, 20, 20, 255);


        SDL_RenderPresent(engine.renderer);

        old_time = SDL_GetPerformanceCounter(); 

    }
    SDL_DestroyWindow(engine.window);
    SDL_Quit();
    return 0;

}

void parse_level_data(FILE *level_data, level_t *level, int sector_count){
    
    if (level_data == NULL) {
        printf("Level data isn't found.");
        return; 
    }

    char buffer[100];

    
    level->sectors = malloc(sizeof(sector_t) * sector_count); 

    { // traverses the buffer to get to meaningful data
        while(fgets(buffer, sizeof(buffer), level_data)){if(buffer[0] == '$') break;} // get to the sectors in the level data
        fgets(buffer, sizeof(buffer), level_data);
    }

    sscanf(buffer, 
        "%d %d %d %f %f",
        &level->sectors[0].id, 
        &level->sectors[0].num_walls,
        &level->sectors[0].wall_index,
        &level->sectors[0].ceiling_height,
        &level->sectors[0].floor_height
    ); 
    
    level->walls = malloc(sizeof(wall_t) * level->sectors[0].num_walls); 

    { // traverses the buffer to get to meaningful data
        while(fgets(buffer, sizeof(buffer), level_data)){if(buffer[0] == '$') break;}  // get to the walls in the level data
        fgets(buffer, sizeof(buffer), level_data);
    }


    for(size_t i = 0; i < level->sectors[0].num_walls; i++){
        sscanf(
            buffer,
            "%d %d %d %d %d %d", 
            &level->walls[i].index, 
            &level->walls[i].start_pos.x, 
            &level->walls[i].start_pos.y, 
            &level->walls[i].end_pos.x, 
            &level->walls[i].end_pos.y, 
            &level->walls[i].portal
        );
        fgets(buffer, sizeof(buffer), level_data);
    }

    // the reason this is returning garbage value is because level_sectors and walls are addresses that have memory allocated to them on the stack... however once this 
    // function returns and the stack pointer increments back to normal, level_sectors and walls point to garbage values... sooo we need to
    // allocate them on the heap with malloc. 
}
fvec2d_t intersection(int x1,int y1, int x2,int y2, int x3,int y3, int x4, int y4){ 


    /* 
        returns the point of intersection of two lines (x1, y1) -> (x2, y2) AND (x3, y3) -> (x4, y4) 
        
        if they dont intersect then we will return a vector w/ NAN so yeaaa
    */
    float denom = ((x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)); 
    if (denom == 0.0) return (fvec2d_t) { .x = NAN, .y = NAN};  

    fvec2d_t retvec; 
    
    retvec.x = (((x1 * y2 - y1 * x2) * (x3 - x4)) - ((x1 - x2) * (x3 * y4 - y3 * x4))) / denom; 
    retvec.y = (((x1 * y2 - y1 * x2) * (y3 - y4)) - ((y1 - y2) * (x3 * y4 - y3 * x4))) / denom; 
    
    return retvec; 

}

static inline void vline_draw(engine_t *engine, int x, int y1, int y2, room_parts part){
    if (y2 < y1){
        int _tmp = y1;
        y1 = y2;
        y2 = _tmp;
    }

    SDL_Rect line = {
        .x = x,
        .y = y1, 
        .w = 1,
        .h = y2 - y1 + 1
    };

    if(part == WALL1){
        SDL_SetRenderDrawColor(engine->renderer, 15, 147, 8, 255); 
    }else if(part == WALL2){
        SDL_SetRenderDrawColor(engine->renderer, 21, 92, 17, 225);
    }else if(part == FLOOR){
        SDL_SetRenderDrawColor(engine->renderer, 10, 50, 60, 225);
    }else{
        SDL_SetRenderDrawColor(engine->renderer, 103, 106, 110, 225);
    }
    
    SDL_RenderFillRect(engine->renderer, &line);
}