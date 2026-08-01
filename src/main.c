#include <stdio.h>
#include <stdint.h> 
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PI 3.1415
#define HFOV (0.77f * SCREEN_HEIGHT)
#define VFOV (0.2f * SCREEN_HEIGHT)


typedef struct vec2d { int x; int y; } vec2d_t; 
typedef struct vec3d { int x; int y; int z; } vec3d_t; 
typedef struct fvec2d { float x; float y; } fvec2d_t; 
typedef struct fvec3d { float x; float y; float z; } fvec3d_t; 


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
    float look_angle; // in radians
} player_t; 

int s2i(const char *str); 
void parse_level_data(FILE *level_data, level_t *level, int sector_count);

vec2d_t intersection(x1,y1, x2,y2, x3,y3, x4,y4){ 
    /* 
        returns the point of intersection of two lines (x1, y1) -> (x2, y2) AND (x3, y3) -> (x4, y4) 
        
        if they dont intersect then we will return a vector w/ NAN so yeaaa
    */
    float denom = ((x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)); 
    if (denom == 0.0) return (fvec2d_t) = { .x = NAN, .y = NAN};  

    fvec2d_t retvec; 
    
    retvec.x = (((x1 * y2 - y1 * x2) * (x3 - x4)) - ((x1 - x2) * (x3 * y4 - y3 * x4))) / denom; 
    retvec.y = (((x1 * y2 - y1 * x2) * (y3 - y4)) - ((y1 - y2) * (x3 * y4 - y3 * x4))) / denom; 
    
    return retvec; 

}


int main(int argc, char *argv[]){

    FILE * level_data = fopen("../src/level.txt", "r");

    level_t LEVEL; 

    parse_level_data(level_data, &LEVEL, 3);

    player_t player = {
        .position = (fvec2d_t) {
            .x = 3.0, 
            .y = 2.0,
        },
        .look_angle = PI/2,
        .state = (state_t) {
            .id = 'p',
            .last_sector = 0, 
        }

    };

    // just code for LEVEL.sectors[0]
    for(int i = 0; i < LEVEL.sectors[0].num_walls; i++){
        
        // change to player basis (think change of basis in linear algebra..) 
        fvec2d_t rv1 = { .x = LEVEL.walls[i].start_pos.x - player.position.x, .y = LEVEL.walls[i].start_pos.y - player.position.y };
        fvec2d_t rv2 = { .x = LEVEL.walls[i].end_pos.x - player.position.x, .y = LEVEL.walls[i].end_pos.y - player.position.y }; 
        // ^^^^ these are the positions of teh walls relatiive to teh player.. now we have to rotate them using js the 2d rotation matrix
        int tempx = rv1.x; 
        rv1.x = (rv1.x * sinf(player.look_angle) - rv1.y * cosf(player.look_angle));
        rv1.y = (tempx * sinf(player.look_angle) + rv1.y * cosf(player.look_angle));
        
        tempx = rv2.x;
        rv2.x = (rv2.x * sinf(player.look_angle) - rv2.y * cosf(player.look_angle));
        rv2.y = (tempx * sinf(player.look_angle) + rv2.y * cosf(player.look_angle));

        if (rv1.y <= 0 && rv2.y <= 0) continue; // the wall is cmpletely behind the camera then we dont have to render it

        if (rv1.y <= 0 || rv2.y <= 0){ // check if part of the wall is behind the player and if it is then we need to do clipping trick so our shi isnt fucked up
            float neary = 1e-4;
            float d1 = rv1.y - neary; 
            float d2 = rv2.y - neary; 

            float t = d1 / (d1 - d2); 
            float ix = rv1.x + t * (rv2.x - rv1.x);
            float iy = rv1.y + t * (rv2.y - rv1.y);
        }


        // here's perspective transformation.. the first two variabels are how much we scale the x and y coordinates to map them onto screen space
        float xscale1 = HFOV / rv1.y, yscale1 = VFOV / rv1.y; 
        float xscale2 = HFOV / rv2.y, yscale2 = VFOV / rv2.y;  

        int x1 = W/2 - (int)(rv1.x * xscale1); 
        int x2 = W/2 - (int)(rv2.x * xscale2); 

        if (x1 >= x2 || x2 < 0 || x1 > SCREEN_WIDTH) continue ;


    }

}

int s2i(const char *str) {
int result = 0;
int sign = 1;
int i = 0;

// Handle optional sign
if (str[0] == '-') {
sign = -1;
i++;
} else if (str[0] == '+') {
i++;
}

// Process each digit
for (; str[i] != '\0'; i++) {
if (str[i] >= '0' && str[i] <= '9') {
result = result * 10 + (str[i] - '0');
} else {
// Stop at first non-digit character
break;
}
}

return sign * result;
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

