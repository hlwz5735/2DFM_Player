//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_2DFMCOMMON_HPP
#define INC_2DFM_PLAYER_2DFMCOMMON_HPP

#include <cstdint>
#include <string>

typedef unsigned char byte;

namespace _2dfm {
#pragma pack(push, 1)
    constexpr int SCRIPT_SIZE = 39;
    struct Script {
        char scriptName[32];
        std::uint16_t scriptIndex;
        byte gap;
        int32_t flags;
    };

    constexpr int SCRIPT_ITEM_SIZE = 16;
    struct ScriptItem {
        byte type;
        byte bytes[15];
    };

    constexpr int PICTUR_HEADER_SIZE = 20;
    struct PictureHeader {
        int unknownFlag1; // 0-3
        int width; // 4-7
        int height; // 8-11
        int hasPrivatePalette; // 12-15
        int size; // 16-19
    };

    struct Picture {
        PictureHeader header;
        byte *content;
    };

    constexpr int COLOR_SIZE = 4;
    constexpr int PALETTE_SIZE = COLOR_SIZE * 256;
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

    enum class SoundType {
        WAVE = 1,
        MIDI,
        CDDA
    };

    constexpr int SOUND_ITEM_HEADER_SIZE = 42;
    struct SoundItemHeader {
        int unknown; // 0-3
        char name[32]; // 4-35
        int size; // 36-39
        byte soundType; // 40
        byte track; // 41

        SoundType getSoundType() {
            return static_cast<SoundType>(soundType & 0b1111);
        }
        bool isLoop() {
            return soundType & 0b10000;
        }
    };
    struct Sound {
        SoundItemHeader header;
        byte *content;
    };
#pragma pack(pop)

    byte *decompress(byte *compressed, int compressedSize, int destSize);

    constexpr int maxPlayerNum = 50;
    constexpr int maxStageNum = 50;
    constexpr int maxDemoNum = 100;
    constexpr int maxReactionNum = 200;
    constexpr int maxThrowReactionNum = 200;
}

#endif //INC_2DFM_PLAYER_2DFMCOMMON_HPP
