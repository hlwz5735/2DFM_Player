#pragma once
#include "2dfmCommon.hpp"
#include "../math/Vector.hpp"

namespace _2dfm {
    enum class DemoScriptTypes {
        NORMAL = 0,
        BACKGROUND = 1,
        SYSTEM_ICON = 3
    };

    enum class DemoScriptItemTypes {
        START = 0,
        PIC = 12,
        MOVE = 1,
        SOUND = 3,
        OBJECT = 4,
        COLOR = 35,
        VARIABLE = 31,
        RANDOM = 32,
        JUMP = 10,
        CALL = 11,
        LOOP = 9,
        AFTERIMAGE = 37,
        END = 5
    };

    struct DemoStart {
        int32_t type;
        int32_t unused[3];
    };

    enum class ColorSetType {
        NORMAL = 0,
        TRANSPARENCY,
        ADD_BLEND,
        MINUS_BLEND,
        ALPHA_BLEND,
    };
#pragma pack(push, 1)
    struct ShowPic {
        byte type; // 0
        uint16_t keepTime; // 1
        uint16_t idxAndFlip; // 3
        int16_t offsetX; // 5
        int16_t offsetY; // 7
        byte fixDir; // 9

        int getPicIdx() const { return idxAndFlip & 0x3fff; }
        bool isFlipX() const { return (idxAndFlip & 0x4000) != 0; }
        bool isFlipY() const { return (idxAndFlip & 0x8000) != 0;  }
        Vector2 getOffset() const { return Vector2(offsetX, offsetY); }
    };

    struct PlaySound {
        byte type;
        byte unknown;
        uint16_t soundIdx;
    };

    struct ColorSet {
        byte type;
        byte colorSetType;
        byte red;
        byte green;
        byte blue;
        byte alpha;
    };

    struct MoveCmd {
        byte type;
        int16_t accelX;
        int16_t moveX;
        int16_t moveY;
        int16_t accelY;
        byte flags;

        /// 是否为相加模式
        bool isAdd() const {
            return flags & 0x1;
        }
        bool isIgnoreMoveX() const {
            return flags & 0x2;
        }
        bool isIgnoreMoveY() const {
            return flags & 0x4;
        }
        bool isIgnoreAccelX() const {
            return flags & 0x8;
        }
        bool isIgnoreAccelY() const {
            return flags & 0x16;
        }
    };
#pragma pack(pop)

    struct Unknown {
        byte type;
        byte bytes[15];
    };
}
