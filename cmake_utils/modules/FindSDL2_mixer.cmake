# FindSDL2_mixer.cmake
# 在macOS上寻找Homebrew安装的SDL2_mixer库
find_path(SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h
        HINTS
        /usr/local/include/SDL2
        /opt/homebrew/include/SDL2
)

find_library(SDL2_MIXER_LIBRARY
        NAMES SDL2_mixer
        HINTS
        /usr/local/lib
        /opt/homebrew/lib
)

set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

mark_as_advanced(SDL2_MIXER_INCLUDE_DIR SDL2_MIXER_LIBRARY)
