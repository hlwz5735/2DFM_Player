//
// Created by 厉猛 on 2024-07-17.
//
#include <format>
#include "2dfmCommon.hpp"

using namespace _2dfm;

std::string ColorBgra::toString() {
    return std::format("0x{:2X}{:2X}{:2X}{:2X}",
        (int) channel.red,
        (int) channel.green,
        (int) channel.blue,
        (unsigned char) channel.alpha == 0 ? 0 : 255);
}

byte *_2dfm::decompress(byte *original, int compressedSize, int destSize) {
    if (destSize <= 0) {
        return nullptr;
    }
    const auto result = static_cast<byte *>(calloc(destSize, 1));
    try {
        int inputOffset = 0;
        int resultPointer = 0;
        while (inputOffset < compressedSize) {
            int current = static_cast<int>(original[inputOffset]);
            // type只取前2位，4种取值
            const int type = current >> 6;
            // current取后6位，最大值63
            current = current & 0x3f;
            if (current == 0) {
                inputOffset += 1;
                current = static_cast<int>(original[inputOffset]);
                // 如果后1字节还是0，向后再取2字节，配合第3字节组合得到新的current
                if (current == 0) {
                    inputOffset += 1;
                    // 后2字节对应的整数
                    current = *reinterpret_cast<unsigned short *>(original + inputOffset);
                    // current = (int) *(original + inputOffset + 1) << 8 | (int) *(original + inputOffset);
                    inputOffset += 2;

                    int highBits = static_cast<int>(original[inputOffset]) << 16;
                    current = current + highBits + 0x013f;
                } else {
                    current += 0x3f;
                }
            }

            switch (type) {
            case 0: // 接current个0
                //for (int i = 0; i < current; i++) {
                //    result[resultPointer + i] = static_cast<byte>(0);
                //}

                resultPointer += current;
                break;
            case 1: // 取接下来current个字节
                if (current > 0) {
                    for (int i = 0; i < current; i++) {
                        result[resultPointer + i] = original[inputOffset + 1 + i];
                    }

                    resultPointer += current;
                    inputOffset += current;
                }
                break;
            case 2: // 接current个下一字节
            {
                inputOffset += 1;
                const byte nextByte = original[inputOffset];
                for (int i = 0; i < current; i++) {
                    result[resultPointer + i] = nextByte;
                }

                resultPointer += current;
            }
            break;
            case 3: // 先取下一字节得到要向上重复的字节数，再重复此前对应count个已有字节
            {
                inputOffset += 1;
                int backTraceCount = static_cast<int>(original[inputOffset]);
                if (backTraceCount == 0) {
                    inputOffset += 1;
                    backTraceCount = (static_cast<int>(original[inputOffset]) + 1) << 8;
                }

                auto backTraceStart = resultPointer - backTraceCount;
                for (int i = 0; i < current; i++) {
                    result[resultPointer + i] = result[backTraceStart + i];
                }

                resultPointer += current;
            }
            break;
            }

            inputOffset++;
        }
    }
    catch (std::exception &e) {
        return nullptr;
    }
    return result;
}
