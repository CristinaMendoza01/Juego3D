#ifndef AUDIO_H
#define AUDIO_H

#include <map>
#include <bass.h>
#include <string>

class Audio
{
public:
	static Audio* instance;

	static std::map<std::string, Audio*> sLoadedAudios; //para nuestro manager
	HSAMPLE sample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad

	Audio(); //importante poner sample a cero aqui
	~Audio(); //aqui deberiamos liberar el sample con BASS_SampleFree

	//HCHANNEL play(float volume); //lanza el audio y retorna el channel donde suena

	HSAMPLE loadAudio(const char* fileName);
	void PlayAudio(const char* fileName);

	//static void Stop(HCHANNEL channel); //para parar un audio necesitamos su channel
	//static Audio* Get(const char* filename); //manager de audios 
	//static HCHANNEL* Play(const char* filename); //version estática para ir mas rapido
};

#endif
