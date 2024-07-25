#pragma once
#include "2dfmCommon.hpp"
#include "../math/Vector.hpp"

namespace _2dfm {
    enum DemoScriptTypes {
        NORMAL = 0,
        BACKGROUND = 1,
        SYSTEM_ICON = 3
    };

    enum DemoScriptItemTypes {
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

    struct Unknown {
        byte type;
        byte bytes[15];
    };
}
