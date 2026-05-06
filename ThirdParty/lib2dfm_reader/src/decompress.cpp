#include "internal.hpp"

uint8_t* decompressImpl(const uint8_t* original, int32_t compressedSize, int32_t destSize) {
    if (destSize <= 0) {
        return nullptr;
    }

    auto result = static_cast<uint8_t*>(calloc(static_cast<size_t>(destSize), 1));
    if (!result) return nullptr;

    int inputOffset = 0;
    int resultPointer = 0;

    while (inputOffset < compressedSize) {
        int current = static_cast<int>(original[inputOffset]);
        const int type = current >> 6;
        current = current & 0x3f;

        if (current == 0) {
            inputOffset += 1;
            current = static_cast<int>(original[inputOffset]);
            if (current == 0) {
                inputOffset += 1;
                current = static_cast<int>(original[inputOffset])
                        | (static_cast<int>(original[inputOffset + 1]) << 8);
                inputOffset += 2;
                int highBits = static_cast<int>(original[inputOffset]) << 16;
                current = current + highBits + 0x013f;
            } else {
                current += 0x3f;
            }
        }

        switch (type) {
        case 0: // current 个 0
            resultPointer += current;
            break;
        case 1: // 取接下来 current 个字节
            if (current > 0) {
                std::memcpy(result + resultPointer, original + inputOffset + 1, current);
                resultPointer += current;
                inputOffset += current;
            }
            break;
        case 2: // current 个重复下一字节
        {
            inputOffset += 1;
            const uint8_t nextByte = original[inputOffset];
            std::memset(result + resultPointer, nextByte, current);
            resultPointer += current;
        }
        break;
        case 3: // 回溯复制
        {
            inputOffset += 1;
            int backTraceCount = static_cast<int>(original[inputOffset]);
            if (backTraceCount == 0) {
                inputOffset += 1;
                backTraceCount = (static_cast<int>(original[inputOffset]) + 1) << 8;
            }
            auto backTraceStart = resultPointer - backTraceCount;
            // 不能用 memcpy，可能有重叠
            for (int i = 0; i < current; i++) {
                result[resultPointer + i] = result[backTraceStart + i];
            }
            resultPointer += current;
        }
        break;
        }

        inputOffset++;
    }

    return result;
}
