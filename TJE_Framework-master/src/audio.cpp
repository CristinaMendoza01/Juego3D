#include "audio.h"
#include "includes.h"


Audio* Audio::instance = NULL;

Audio::Audio() {
	sample = 0;
}

Audio::~Audio()
{
	BASS_SampleFree(sample);
}


HSAMPLE Audio::loadAudio(const char* fileName, DWORD flags) {
	//El handler para un sample
	HSAMPLE hSample;
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, flags);
	if (hSample == 0)
	{
		std::cout << "ERROR loading " << fileName << std::endl;
	}
	std::cout << " + AUDIO load " << fileName << std::endl;
	return hSample;
}

void Audio::PlayAudio(HSAMPLE hSample) {
	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
}

void Audio::StopAudio(HSAMPLE hSample) {

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelStop(hSampleChannel);
}