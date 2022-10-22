#pragma once

#pragma comment(lib, "fmodex_vc.lib" ) // fmod library

#include "fmod.hpp" //fmod c++ header
#include <stdio.h>
#include <stdlib.h>

//Sound array size
#define NUM_SOUNDS 8

//Sound identifiers
enum {
	CHOOSE,
	COIN,
	HEART,
	POWERUP,
	HIT,
	GAMEOVER,
	ZURG,
	CUBE
};

class Sound
{
public:
	Sound(void);
	virtual ~Sound(void);

	bool Load();
	void Play(int sound_id);
	void StopAll();
	void Update();

	FMOD::System* system; //handle to FMOD engine
	FMOD::Sound* sounds[NUM_SOUNDS]; //sound that will be loaded and played
	FMOD::Channel* effectsChannel;
	FMOD::Channel* bounceChannel;
	FMOD::DSP* dspSmoothStop;
};
