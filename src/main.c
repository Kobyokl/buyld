#include <stdio.h>
#include <stdint.h> 
    
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> 

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct vec2d { int x; int y; } vec2d_t; 
typedef struct vec3d { int x; int y; int z; } vec3d_t; 

typedef struct fvec2d { float x; float y; } fvec2d_t; 
typedef struct fvec3d { float x; float y; float z; } fvec3d_t; 



typedef struct state { // abstract state that can apply to multipe objects
    int last_sector; //this is lastknownsector from the build style documnets thingy....
    int id; // id of whatever entity.... player is 0, anyting else is tbd
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
     int ceiling_height;
     int floor_height; 
} sector_t; 

typedef struct level {
    sector_t *sectors; 
    wall_t *walls; 

} level_t; 



int main(int argc, char *argv[]){

    FILE * level_data = fopen("level.txt", "r"); 
    char buffer[8] 
    while(buffer[0] != "$"){
        fgets("%7s", buffer);
    }
    fgets("%*s", buffer);

    for(int i = 0; i < 5; i++){
        continue
    }

    //bro im finishing this tomorrow im tired as balls iclllll

    
    return 0; 
}

