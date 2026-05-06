/**
 * lib2dfm_reader - 2DFM 游戏文件格式解析库
 *
 * 纯解析库，零外部依赖（仅依赖 C/C++ 标准库）。
 * 公共接口为 extern "C"，可作为动态库跨语言调用。
 *
 * I/O 通过回调抽象，调用方可自由对接 FILE*、AAssetManager、内存缓冲等。
 */
#ifndef LIB2DFM_READER_H
#define LIB2DFM_READER_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
  #ifdef LIB2DFM_READER_BUILD
    #define DFM_API __declspec(dllexport)
  #else
    #define DFM_API __declspec(dllimport)
  #endif
#else
  #define DFM_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * I/O 回调抽象
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * 流式 I/O 回调接口。调用方实现这些函数指针以提供数据源。
 * ctx 为调用方自定义的上下文（如 FILE*、AAsset* 等）。
 */
typedef struct Kgt2dfmStream {
    /** 读取 size 字节到 buf，返回实际读取字节数 */
    size_t (*read)(void* ctx, void* buf, size_t size);
    /** 定位。whence: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END。成功返回 0 */
    int    (*seek)(void* ctx, long offset, int whence);
    /** 返回当前流位置 */
    long   (*tell)(void* ctx);
    /** 调用方上下文 */
    void*  ctx;
} Kgt2dfmStream;

/* ═══════════════════════════════════════════════════════════════════════════
 * 基础 POD 数据类型 (与文件中的二进制布局一致)
 * ═══════════════════════════════════════════════════════════════════════════ */

#pragma pack(push, 1)

/** 脚本项 (16 字节，直接映射文件格式) */
typedef struct Kgt2dfmScriptItem {
    uint8_t type;
    uint8_t bytes[15];
} Kgt2dfmScriptItem;

/** 图片头 (20 字节) */
typedef struct Kgt2dfmPictureHeader {
    int32_t unknownFlag1;
    int32_t width;
    int32_t height;
    int32_t hasPrivatePalette;
    int32_t size;
} Kgt2dfmPictureHeader;

/** BGRA 颜色 (4 字节) */
typedef union Kgt2dfmColorBgra {
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } channel;
    uint32_t value;
} Kgt2dfmColorBgra;

/** 声音头信息 (42 字节) */
typedef struct Kgt2dfmSoundHeader {
    int32_t unknown;
    char    name[32];
    int32_t size;
    uint8_t soundType;
    uint8_t track;
} Kgt2dfmSoundHeader;

/** 脚本信息 (解析后) */
typedef struct Kgt2dfmScript {
    char     name[32];      /* GBK 编码的脚本名 */
    int32_t  flags;
    int32_t  startIdx;
    int32_t  endIdx;
} Kgt2dfmScript;

/** 受击反应 */
typedef struct Kgt2dfmReaction {
    char    name[32];       /* GBK 编码 */
    int32_t isHurtAction;
} Kgt2dfmReaction;

/** 投掷反应 */
typedef struct Kgt2dfmThrowReaction {
    char name[32];          /* GBK 编码 */
} Kgt2dfmThrowReaction;

/** 硬直时间配置 */
typedef struct Kgt2dfmRecoverTimeConfig {
    uint8_t gap;
    uint8_t attackRecoverTime;
    uint8_t defenceRecoverTime;
    uint8_t clashRecoverTime;
} Kgt2dfmRecoverTimeConfig;

/** 影片配置 */
typedef struct Kgt2dfmGameDemoConfig {
    uint8_t titleDemoId;
    uint8_t storyModeCharSelectDemoId;
    uint8_t oneVsOneModeCharSelectDemoId;
    uint8_t teamModeCharSelectDemoId;
    uint8_t continueDemoId;
    uint8_t openingDemoId;
    uint8_t unknownTag1;
    uint8_t unknownTag2;
} Kgt2dfmGameDemoConfig;

/** 游戏基础配置 (位域) */
typedef struct Kgt2dfmProjectBaseConfig {
    uint8_t encryptGame;
    uint8_t allowClash;
    uint8_t enableStoryMode;
    uint8_t enable1V1Mode;
    uint8_t enableTeamMode;
    uint8_t showHpAfterHpBar;
    uint8_t pressToStart;
} Kgt2dfmProjectBaseConfig;

/** 角色选择画面配置 */
typedef struct Kgt2dfmCharSelectConfig {
    int16_t selectBoxStartX;
    int16_t selectBoxStartY;
    int16_t iconWidth;
    int16_t iconHeight;
    int16_t columnNum;
    int16_t rowNum;
    int16_t player1PortraitX;
    int16_t player1PortraitY;
    int16_t player1PortraitTeamOffsetX;
    int16_t player1PortraitTeamOffsetY;
    int16_t player2PortraitX;
    int16_t player2PortraitY;
    int16_t player2PortraitTeamOffsetX;
    int16_t player2PortraitTeamOffsetY;
} Kgt2dfmCharSelectConfig;

/** DEMO 配置 */
typedef struct Kgt2dfmDemoConfig {
    int16_t bgmSoundId;
    uint8_t pressToSkip;
    int16_t unknownGap;
    int32_t totalTime;
} Kgt2dfmDemoConfig;

/** 场景配置 */
typedef struct Kgt2dfmStageConfig {
    int32_t bgmSoundId;
} Kgt2dfmStageConfig;

#pragma pack(pop)

/* ═══════════════════════════════════════════════════════════════════════════
 * 图片数据 (变长)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct Kgt2dfmPicture {
    Kgt2dfmPictureHeader header;
    uint8_t* content;       /* 像素数据 (长度由 header 决定) */
    size_t   contentSize;
} Kgt2dfmPicture;

/* ═══════════════════════════════════════════════════════════════════════════
 * 声音数据 (变长)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct Kgt2dfmSound {
    Kgt2dfmSoundHeader header;
    uint8_t* content;       /* 音频数据 */
    size_t   contentSize;
} Kgt2dfmSound;

/* ═══════════════════════════════════════════════════════════════════════════
 * 公共资源部分 (所有文件类型共有)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct Kgt2dfmCommonResource {
    /* 脚本 */
    Kgt2dfmScript*      scripts;
    int32_t             scriptCount;
    /* 脚本项 */
    Kgt2dfmScriptItem*  scriptItems;
    int32_t             scriptItemCount;
    /* 图片 */
    Kgt2dfmPicture*     pictures;
    int32_t             pictureCount;
    /* 共享调色盘 (8 个，每个 256 色) */
    Kgt2dfmColorBgra    sharedPalettes[8][256];
    /* 声音 */
    Kgt2dfmSound*       sounds;
    int32_t             soundCount;
} Kgt2dfmCommonResource;

/* ═══════════════════════════════════════════════════════════════════════════
 * 顶层解析结果 (opaque → 通过 accessor 访问)
 * ═══════════════════════════════════════════════════════════════════════════ */

/** KGT 游戏主文件解析结果 */
typedef struct Kgt2dfmGame {
    char projectName[256];  /* GBK */
    Kgt2dfmCommonResource   common;
    /* 角色名称 (最多50) */
    char                    playerNames[50][256];
    /* 场景名称 (最多50) */
    char                    stageNames[50][256];
    /* DEMO 名称 (最多100) */
    char                    demoNames[100][256];
    /* 受击反应 (最多200) */
    Kgt2dfmReaction         reactions[200];
    /* 投掷反应 (最多200) */
    Kgt2dfmThrowReaction    throwReactions[200];
    /* 配置 */
    Kgt2dfmRecoverTimeConfig recoverTimeConfig;
    Kgt2dfmGameDemoConfig    demoConfig;
    Kgt2dfmProjectBaseConfig projectBaseConfig;
    Kgt2dfmCharSelectConfig  charSelectConfig;
    /* 角色可选择性 (最多50) */
    uint8_t                  playerSelectableInfos[50];
} Kgt2dfmGame;

/** Player 文件解析结果 */
typedef struct Kgt2dfmPlayer {
    char playerName[256];   /* GBK */
    Kgt2dfmCommonResource common;
} Kgt2dfmPlayer;

/** Demo 文件解析结果 */
typedef struct Kgt2dfmDemo {
    char demoName[256];     /* GBK */
    Kgt2dfmCommonResource common;
    Kgt2dfmDemoConfig     config;
} Kgt2dfmDemo;

/** Stage 文件解析结果 */
typedef struct Kgt2dfmStage {
    char stageName[256];    /* GBK */
    Kgt2dfmCommonResource common;
    Kgt2dfmStageConfig    config;
} Kgt2dfmStage;

/* ═══════════════════════════════════════════════════════════════════════════
 * 解析 API
 * ═══════════════════════════════════════════════════════════════════════════ */

/** 解析 KGT 游戏主文件。失败返回 NULL */
DFM_API Kgt2dfmGame*   kgt_read_game(Kgt2dfmStream* stream);

/** 解析 Player 文件。失败返回 NULL */
DFM_API Kgt2dfmPlayer* kgt_read_player(Kgt2dfmStream* stream);

/** 解析 Demo 文件。失败返回 NULL */
DFM_API Kgt2dfmDemo*   kgt_read_demo(Kgt2dfmStream* stream);

/** 解析 Stage 文件。失败返回 NULL */
DFM_API Kgt2dfmStage*  kgt_read_stage(Kgt2dfmStream* stream);

/* ═══════════════════════════════════════════════════════════════════════════
 * 释放 API
 * ═══════════════════════════════════════════════════════════════════════════ */

DFM_API void kgt_free_game(Kgt2dfmGame* game);
DFM_API void kgt_free_player(Kgt2dfmPlayer* player);
DFM_API void kgt_free_demo(Kgt2dfmDemo* demo);
DFM_API void kgt_free_stage(Kgt2dfmStage* stage);

/* ═══════════════════════════════════════════════════════════════════════════
 * 工具 API
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * 解压 2DFM 自定义压缩格式
 * @param compressed    压缩数据
 * @param compressedSize 压缩数据长度
 * @param destSize      解压后预期长度
 * @return 解压后的缓冲区 (调用方需用 kgt_free_buffer 释放)，失败返回 NULL
 */
DFM_API uint8_t* kgt_decompress(const uint8_t* compressed, int32_t compressedSize, int32_t destSize);

/** 释放由本库分配的缓冲区 */
DFM_API void kgt_free_buffer(void* ptr);

/* ═══════════════════════════════════════════════════════════════════════════
 * FILE* 便捷适配器
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * 用 FILE* 初始化一个 Kgt2dfmStream。
 * 调用方负责 fopen/fclose，本函数仅填充回调。
 */
DFM_API Kgt2dfmStream kgt_stream_from_file(void* file_ptr);

#ifdef __cplusplus
}
#endif

#endif /* LIB2DFM_READER_H */
