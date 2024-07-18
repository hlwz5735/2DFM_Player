//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_2DFMCOMMON_HPP
#define INC_2DFM_PLAYER_2DFMCOMMON_HPP

#include <cstdint>
#include <cstddef>
#include <string>

using std::byte;

namespace _2dfm {
    struct Script {
        char scriptName[32];
        std::uint16_t scriptIndex;
        byte gap;
        int flags;
    };

    struct ScriptItem {
        int type: 1;
        byte bytes[15];
    };

    struct SpriteFrameHeader {
        int unknownFlag1;
        int width;
        int height;
        int hasPrivatePalette;
        int size;
    };
    struct SpriteFrame {
        SpriteFrameHeader header;
        byte *content;
    };

    union ColorBgra {
        struct {
            byte blue;
            byte green;
            byte red;
            byte alpha;
        } channel;
        std::uint32_t value;

        std::string toString();
    };

    struct Palette {
        ColorBgra colors[256];
        int gap[8];
    };

    enum SoundType {
        WAVE = 1,
        MIDI,
        CDDA
    };

    struct SoundItemHeader {
        int unknown;
        char name[32];
        int size;
        SoundType soundType: 1;
        int track: 1;
    };
    struct Sound {
        SoundItemHeader header;
        byte *content;
    };
}

#endif //INC_2DFM_PLAYER_2DFMCOMMON_HPP
