//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_AUDIOSYSTEM_HPP
#define INC_2DFM_PLAYER_AUDIOSYSTEM_HPP

#include <vector>

class Game;
class SoundClip;

class AudioSystem {
public:
    AudioSystem(Game *game);

    bool initialize();
    void cleanUp();

    void addClip(SoundClip *clip);
    void removeClip(SoundClip *clip);

    void playClip(SoundClip *clip, bool loop = false);
    void stopAll();
private:
    Game *game = nullptr;
    std::vector<SoundClip *> clipPool;
};


#endif //INC_2DFM_PLAYER_AUDIOSYSTEM_HPP
