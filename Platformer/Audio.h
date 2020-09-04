#pragma once
#include <SFML/Audio.hpp>


class AudioClass
{
public:
	AudioClass();
sf::SoundBuffer jumpSB;
sf::SoundBuffer deathSB;
sf::SoundBuffer coinSB;
sf::Sound sound;
void playAudio();
};
