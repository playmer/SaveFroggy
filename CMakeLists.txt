project(SaveFroggy)
cmake_minimum_required(VERSION 3.16)

add_subdirectory(zlib)

include_directories(zlib)
include_directories(build/zlib)

set(ZLIB_LIBRARY zlib)
option(PNG_BUILD_ZLIB "Custom zlib Location, else find_package is used" ON)
add_subdirectory(lpng)

#target_link_libraries(png zlib)
#target_link_libraries(png-fix-itxt zlib)
#target_link_libraries(pngvalid zlib)
#target_link_libraries(pngfix zlib)

add_executable(SaveFroggy 
    "shell.c"
    "sqlite3.c"
    "sqlite3.h"
    "stb_image.h"
    "stb_image_write.h"
    "SaveFroggy.cpp"
)