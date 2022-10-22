#include "Sound.h"

Sound::Sound(void)
{
	FMOD::System_Create(&system);// create an instance of the game engine
	system->init(32, FMOD_INIT_NORMAL, 0);// initialise the game engine with 32 channels (cantidad de sonido simultaneo que puede haber)
}

Sound::~Sound(void)
{
	for (int i = 0; i < NUM_SOUNDS; i++) sounds[i]->release();
	system->release();
}

bool Sound::Load()
{
	system->createStream("Sounds/Choose.wav", FMOD_HARDWARE, 0, &sounds[CHOOSE]);
	system->createSound("Sounds/Coin.wav", FMOD_HARDWARE, 0, &sounds[COIN]);
	system->createSound("Sounds/Heart.wav", FMOD_HARDWARE, 0, &sounds[HEART]);
	system->createSound("Sounds/PowerUp.wav", FMOD_HARDWARE, 0, &sounds[POWERUP]);
	system->createSound("Sounds/Hit.wav", FMOD_HARDWARE, 0, &sounds[HIT]);
	system->createSound("Sounds/GameOver.wav", FMOD_HARDWARE, 0, &sounds[GAMEOVER]);
	system->createSound("Sounds/ZurgLaughing.wav", FMOD_HARDWARE, 0, &sounds[ZURG]);
	system->createSound("Sounds/Cube.wav", FMOD_HARDWARE, 0, &sounds[CUBE]);
	return true;
}

void Sound::Play(int sound_id)
{
	system->playSound(FMOD_CHANNEL_FREE, sounds[sound_id], false, &effectsChannel);
}

void Sound::StopAll()
{
	effectsChannel->stop();
}

void Sound::Update()
{
	system->update();
}
