# FindSDL2.cmake
# 在macOS上寻找Homebrew安装的SDL2库
find_path(SDL2_INCLUDE_DIR SDL2/SDL.h
        HINTS
        /usr/local/include/SDL2
        /opt/homebrew/include/SDL2
)

find_library(SDL2_LIBRARY
        NAMES SDL2
        HINTS
        /usr/local/lib
        /opt/homebrew/lib
)

set(SDL2_LIBRARIES ${SDL2_LIBRARY})
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)
