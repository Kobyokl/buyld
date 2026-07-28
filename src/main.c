#include <stdio.h>
#include <stdint.h> 
#include <string.h>

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
     float ceiling_height;
     float floor_height; 
} sector_t; 

typedef struct level {
    sector_t *sectors; 
    wall_t *walls; 
} level_t; 

typedef struct player {
    vec2d_t position
    vec2d_t look_angle
    int curr_sector
} player_t; 

int s2i(const char *str); 

void parse_level_data(FILE *level_data, level_t *level, int sector_count){j
    
    if (level_data == NULL) {
        printf("Level data isn't found.");
        return; 
    }

    sector_t level_sectors[sector_count];
    sector_t level_sector;

    char buffer[100];

    while(buffer[0] != '$'){ fgets(buffer, sizeof(buffer), level_data); } // get to the sectors in the level data
    fgets(buffer, sizeof(buffer), level_data);

    printf("%s", buffer); 


    level_sector.id = (int) buffer[0] - '0'; 
    level_sector.num_walls = (int) buffer[2] - '0'; 
    level_sector.wall_index = (int) buffer[4] - '0'; 
    level_sector.ceiling_height = (float) buffer[6] - '0'; 
    level_sector.floor_height = (float) buffer[8] - '0'; 

    sector_count[0] = level_sector; 

    while(buffer[0] != '$'){ fgets(buffer, sizeof(buffer), level_data); } // get to the walls in the level data
    fgets(buffer, sizeof(buffer), level_data);

    wall_t walls[level_sector.num_walls]; // 5 == level_sector.num_walls

    for(size_t i = 0; i < level_sector.num_walls; i++){
        walls[i] = (wall_t) {
            .index = (int) buffer[0] - '0', 
            .start_pos = {
                .x = (int) buffer[2] - '0',
                .y = (int) buffer[4] - '0',
            },
            .end_pos = {
                .x = (int) buffer[6] - '0',
                .y = (int) buffer[8] - '0',
            },
            .portal = s2i(buffer + 10),
        };
        fgets(buffer, sizeof(buffer), level_data);
    }


    level->sectors = level_sectors;
    level->walls = walls;
}


int main(int argc, char *argv[]){

    FILE * level_data = fopen("../src/level.txt", "r");

    level_t LEVEL; 

    parse_level(level_data, &LEVEL, 3);
    
    
    printf("%d\n", level_sector.id);
    printf("%d\n", level_sector.num_walls);
    printf("%d\n", level_sector.wall_index);
    printf("%f\n", level_sector.ceiling_height);
    printf("%f\n", level_sector.floor_height);



    printf("%d\n", walls[0].index);
    printf("%d %d\n", walls[0].start_pos.x, walls[0].start_pos.y);
    printf("%d %d\n", walls[0].end_pos.x, walls[0].end_pos.y);
    printf("%d\n", walls[0].portal);

    printf("%d\n", walls[1].index);
    printf("%d %d\n", walls[1].start_pos.x, walls[1].start_pos.y);
    printf("%d %d\n", walls[1].end_pos.x, walls[1].end_pos.y);
    printf("%d\n", walls[1].portal);

    
    return 0; 
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


