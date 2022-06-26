#include "audio.h"
#include "includes.h"

Audio::Audio() {
	sample = 0;
}


HSAMPLE Audio::loadAudio(const char* fileName) {
	//El handler para un sample
	HSAMPLE hSample;
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, BASS_SAMPLE_LOOP);
	if (hSample == 0)
	{
		std::cout << "ERROR loading " << fileName << std::endl;
	}
	std::cout << " + AUDIO load " << fileName << std::endl;
	return hSample;
}

void Audio::PlayAudio(HSAMPLE hSample) {
	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//HSAMPLE hSample = loadAudio(fileName);

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
}