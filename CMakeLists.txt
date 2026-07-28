cmake_minimum_required(VERSION 3.10)

project(Buyld C)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2 SDL2main)
find_package(SDL2_ttf REQUIRED)
 
file(GLOB SOURCE_FILES "src/*.c") # for compiling adn lniking all c files 

add_executable(Buyld ${SOURCE_FILES})

target_link_libraries(Buyld PRIVATE 
    SDL2::SDL2main 
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
    m #for the math.h
)