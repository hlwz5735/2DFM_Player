#include "2dfmFileReader.hpp"
#include "2dfmCommon.hpp"
#include "../base/SoundClip.hpp"
#include <iconv.h>
#include <stdexcept>

std::string gb2312ToUtf8(const char* gb2312) {
    auto len = strlen(gb2312);
    if (len == 0) {
        return "";
    }
    std::vector<char> utf8(len * 4); // �����㹻��Ŀռ䣬��ֹ���
    char* inbuf = const_cast<char*>(gb2312);
    size_t inbytesleft = len;
    char* outbuf = &utf8[0];
    size_t outbytesleft = utf8.size();

    iconv_t cd = iconv_open("UTF-8", "GB2312");
    if (cd == (iconv_t) - 1) {
        throw std::runtime_error("iconv_open failed");
    }

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == static_cast<size_t>(-1)) {
        iconv_close(cd);
        throw std::runtime_error("iconv failed");
    }

    iconv_close(cd);
    utf8.resize(utf8.size() - outbytesleft); // ������С��ȥ��δʹ�õĿռ�
    return { utf8.begin(), utf8.end() };
}

/// ���ù�����Դ����
/// ע�⣺ֻ�ܳ������ã�����������ݣ�����ʱ�ᵼ���ڴ�й©
void setCommonResource(CommonResource *result, const _2dfm::CommonResourcePart &commonResource) {
    // ���ƽű���Ϣ
    result->scripts.reserve(commonResource.scriptCount);
    for (auto i = 0; i < commonResource.scriptCount; ++i) {
        result->scripts.emplace_back(new _2dfm::Script(commonResource.scripts[i]));
    }
    // ���ƽű�����Ϣ
    result->scriptItems.reserve(commonResource.scriptItemCount);
    for (auto i = 0; i < commonResource.scriptItemCount; ++i) {
        result->scriptItems.emplace_back(new _2dfm::ScriptItem(commonResource.scriptItems[i]));
    }

    // ��ȡ���������ɫ��
    result->sharedPalettes.reserve(8);
    for (auto sharedPalette: commonResource.sharedPalettes) {
        result->sharedPalettes.emplace_back(createSdlPalette(sharedPalette));
    }

    // ��ȡ��������֡����
    result->spriteFrames.reserve(commonResource.pictureCount);
    for (auto &picture : commonResource.pictures) {
        auto &sfi = result->spriteFrames.emplace_back();
        sfi.setFrom2dfmPicture(picture);
        sfi.setSharedPalettes(result->sharedPalettes.data());
    }

    // ����Ƭ��
    result->sounds.reserve(commonResource.soundCount);
    for (auto s : commonResource.sounds) {
        result->sounds.emplace_back(SoundClip::from2dfmSound(s));
    }
}

KgtGame *readKgtFile(const std::string& filepath) {
    _2dfm::KgtFileHeader header;
    // ���ļ�
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open kgt file failed");
    }
    // �����ļ�ͷ
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // ���빫����Դ����
    auto commonResource = readCommonResourcePart(&offset, file);

    // �������������Ϣ
    fseek(file, offset + 4, SEEK_SET);
    _2dfm::NameInfo *playerNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxPlayerNum));
    fread(playerNames, sizeof(_2dfm::NameInfo), _2dfm::maxPlayerNum, file);

    // �����ܻ���Ӧ��Ϣ
    _2dfm::ReactionItem *reactionItems = static_cast<_2dfm::ReactionItem *>(malloc(sizeof(_2dfm::ReactionItem) * _2dfm::maxReactionNum));
    fread(reactionItems, sizeof(_2dfm::ReactionItem), _2dfm::maxReactionNum, file);

    // ����Ӳֱʱ����Ϣ
    fseek(file, 4, SEEK_CUR);
    _2dfm::RecoverTimeConfig recoverTimeConfig;
    fread(&recoverTimeConfig, sizeof(_2dfm::RecoverTimeConfig), 1, file);

    // ���볡����Ϣ
    _2dfm::NameInfo *stageNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxStageNum));
    fread(stageNames, sizeof(_2dfm::NameInfo), _2dfm::maxStageNum, file);

    // ����ӰƬ��Ϣ
    _2dfm::NameInfo *demoNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxDemoNum));
    fread(demoNames, sizeof(_2dfm::NameInfo), _2dfm::maxDemoNum, file);

    // ����ӰƬ��Ϣ
    _2dfm::GameDemoConfig demoConfig;
    fread(&demoConfig, _2dfm::DEMO_CONFIG_SIZE, 1, file);

    // ������Ŀͨ������
    _2dfm::ProjectBaseConfig pbc;
    fread(&pbc, 4, 1, file);

    // ����Ͷ����Ӧ�б�
    _2dfm::ThrowReaction *throwReactions = static_cast<_2dfm::ThrowReaction *>(malloc(sizeof(_2dfm::ThrowReaction) * _2dfm::maxThrowReactionNum));
    fread(throwReactions, sizeof(_2dfm::ThrowReaction), _2dfm::maxThrowReactionNum, file);

    // ����λ������
    fseek(file, 264, SEEK_CUR);
    _2dfm::CharSelectConfig csc;
    fread(&csc, sizeof(_2dfm::CharSelectConfig), 1, file);

    // ƴװ����
    auto result = new KgtGame;
    result->projectName = header.name.name;
    setCommonResource(result, commonResource);

    result->playerNames.reserve(_2dfm::maxPlayerNum);
    for (auto playerName = playerNames; playerName != playerNames + _2dfm::maxPlayerNum; ++playerName) {
        result->playerNames.emplace_back(gb2312ToUtf8(playerName->name));
    }
    result->stageNames.reserve(_2dfm::maxStageNum);
    for (auto stageName = stageNames; stageName != stageNames + _2dfm::maxStageNum; ++stageName) {
        result->stageNames.emplace_back(gb2312ToUtf8(stageName->name));
    }
    result->demoNames.reserve(_2dfm::maxDemoNum);
    for (auto demoName = demoNames; demoName != demoNames + _2dfm::maxDemoNum; ++demoName) {
        result->demoNames.emplace_back(gb2312ToUtf8(demoName->name));
    }
    result->reactions.reserve(_2dfm::maxReactionNum);
    for (auto reactionItem = reactionItems; reactionItem != reactionItems + _2dfm::maxReactionNum; ++reactionItem) {
        Reaction r;
        r.name = gb2312ToUtf8(reactionItem->reactionName);
        r.isHurtAction = static_cast<bool>(reactionItem->isHurtAction);
        result->reactions.emplace_back(r);
    }
    result->throwReactions.reserve(_2dfm::maxThrowReactionNum);
    for (auto reaction = throwReactions; reaction != throwReactions + _2dfm::maxThrowReactionNum; ++reaction) {
        result->throwReactions.emplace_back(gb2312ToUtf8(reaction->name));
    }
    result->recoverTimeConfig = recoverTimeConfig;
    result->demoConfig = demoConfig;

    // ��Ϸ��������
    result->projectBaseConfig.encryptGame = pbc.value.encryptGame;
    result->projectBaseConfig.allowClash = pbc.value.allowClash;
    result->projectBaseConfig.enableStoryMode = pbc.value.enableStoryMode;
    result->projectBaseConfig.enable1V1Mode = pbc.value.enable1V1Mode;
    result->projectBaseConfig.enableTeamMode = pbc.value.enableTeamMode;
    result->projectBaseConfig.showHpAfterHpBar = pbc.value.showHpAfterHpBar;
    result->projectBaseConfig.pressToStart = pbc.value.pressToStart;

    // ��ɫѡ��������
    result->charSelectConfig.selectBoxStartPos = Vector2(csc.selectBoxStartX, csc.selectBoxStartY);
    result->charSelectConfig.playerAvatarIconSize = Vector2(csc.iconWidth, csc.iconHeight);
    result->charSelectConfig.rowCount = csc.rowNum;
    result->charSelectConfig.columnCount = csc.columnNum;
    result->charSelectConfig.player1PortraitPos = Vector2(csc.player1PortraitX, csc.player1PortraitY);
    result->charSelectConfig.player1PortraitOffset = Vector2(csc.player1PortraitTeamOffsetX, csc.player1PortraitTeamOffsetY);
    result->charSelectConfig.player2PortraitPos = Vector2(csc.player2PortraitX, csc.player2PortraitY);
    result->charSelectConfig.player2PortraitOffset = Vector2(csc.player2PortraitTeamOffsetX, csc.player2PortraitTeamOffsetY);

    // ��Դ����
    fclose(file);
    free(playerNames);
    free(stageNames);
    free(demoNames);
    free(reactionItems);
    free(throwReactions);
    freeCommonResourcePart(&commonResource);

    return result;
}

KgtDemo *readDemoFile(const std::string &filepath) {
    _2dfm::KgtFileHeader header;
    // ���ļ�
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open demo file failed");
    }
    // �����ļ�ͷ
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // ���빫����Դ����
    auto commonResource = readCommonResourcePart(&offset, file);
    fseek(file, offset + 4, SEEK_SET);

    // ����DEMO������Ϣ
    _2dfm::KgtDemoConfig config;
    fread(&config, sizeof(_2dfm::KgtDemoConfig), 1, file);

    auto result = new KgtDemo;
    // ��Դƴ�Ӳ���
    result->demoName = header.name.name;
    setCommonResource(result, commonResource);
    result->config = config;

    // ��Դ����
    fclose(file);
    freeCommonResourcePart(&commonResource);

    return result;
}

_2dfm::CommonResourcePart readCommonResourcePart(long *offset, FILE *file) {
    fseek(file, *offset, SEEK_SET);

    // ����ű����б�
    int intBuffer;
    fread(&intBuffer, sizeof(int), 1, file);
    _2dfm::CommonResourcePart crp;
    crp.scriptCount = intBuffer;
    crp.scripts = static_cast<_2dfm::Script *>(malloc(_2dfm::SCRIPT_SIZE * crp.scriptCount));
    if (!crp.scripts) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(crp.scripts, _2dfm::SCRIPT_SIZE, crp.scriptCount, file);

    // ����ű������б�
    fread(&intBuffer, sizeof(int), 1, file);
    crp.scriptItemCount = intBuffer;
    crp.scriptItems = static_cast<_2dfm::ScriptItem *>(malloc(_2dfm::SCRIPT_ITEM_SIZE * crp.scriptItemCount));
    if (!crp.scriptItems) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(crp.scriptItems, _2dfm::SCRIPT_ITEM_SIZE, crp.scriptItemCount, file);

    // ���뾫��֡��Ϣ
    fread(&intBuffer, sizeof(int), 1, file);
    crp.pictureCount = intBuffer;
    crp.pictures.reserve(crp.pictureCount);
    for (int i = 0; i < crp.pictureCount; ++i) {
        auto sf = static_cast<_2dfm::Picture *>(malloc(sizeof(_2dfm::Picture)));
        fread(&sf->header, _2dfm::PICTUR_HEADER_SIZE, 1, file);
        auto sz = get2dfmPictureSize(&sf->header);
        byte *pSpriteContent = static_cast<byte *>(malloc(sz));
        fread(pSpriteContent, sz, 1, file);
        sf->content = pSpriteContent;
        crp.pictures.emplace_back(sf);
    }

    // �����ɫ����Ϣ
    for (auto &sharedPalette : crp.sharedPalettes) {
        _2dfm::ColorBgra *pPalette = static_cast<_2dfm::ColorBgra *>(malloc(_2dfm::PALETTE_SIZE));
        fread(pPalette, _2dfm::PALETTE_SIZE, 1, file);
        sharedPalette = pPalette;
        fseek(file, sizeof(int) * 8, SEEK_CUR);
    }

    // ����������Ϣ
    fread(&intBuffer, sizeof(int), 1, file);
    crp.soundCount = intBuffer;
    crp.sounds.reserve(crp.soundCount);
    for (int i = 0; i < crp.soundCount; ++i) {
        auto s = static_cast<_2dfm::Sound *>(malloc(sizeof(_2dfm::Sound)));
        fread(&(s->header), _2dfm::SOUND_ITEM_HEADER_SIZE, 1, file);
        if (s->header.size > 0) {
            s->content = static_cast<byte *>(malloc(s->header.size));
            fread(s->content, s->header.size, 1, file);
        }
        crp.sounds.emplace_back(s);
    }

    *offset = ftell(file);
    return crp;
}

SDL_Palette *createSdlPalette(_2dfm::ColorBgra *originPalette) {
    auto palette = SDL_AllocPalette(256);

    for (int i = 0; i < 256; ++i) {
        auto &color = (palette->colors)[i];
        color.r = static_cast<Uint8>(originPalette[i].channel.red);
        color.g = static_cast<Uint8>(originPalette[i].channel.green);
        color.b = static_cast<Uint8>(originPalette[i].channel.blue);
        color.a = static_cast<Uint8>(static_cast<unsigned char>(originPalette[i].channel.alpha) == 0 ? 255 : 0);
        if (color.r == 0 && color.g == 0 && color.b == 0) {
            color.a = 0;
        }
    }

    return palette;
}

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh) {
    return sfh->width * sfh->height + (sfh->hasPrivatePalette ? 1024 : 0);
}

int get2dfmPictureSize(_2dfm::PictureHeader *sfh) {
    if (sfh->size == 0) {
        return get2dfmPictureRealSize(sfh);
    } else {
        return sfh->size;
    }
}
