#include "Audio.h"


void AudioClass::playAudio()
{
	jumpSB.loadFromFile("SFX/jump.wav");
	deathSB.loadFromFile("SFX/death.wav");
	coinSB.loadFromFile("SFX/coin.wav");
}
AudioClass::AudioClass() 
{

}