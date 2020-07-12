
#ifndef  I_AUDIO_PLAYER_HPP_INCLUDED
#define  I_AUDIO_PLAYER_HPP_INCLUDED

class I_AudioPlayer
{
public:

    I_AudioPlayer() {}
    virtual ~I_AudioPlayer() noexcept {}

    virtual void Update() = 0;

private:

};

#endif   //  I_AUDIO_PLAYER_HPP_INCLUDED
