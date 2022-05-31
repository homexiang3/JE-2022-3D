#include "audio.h"
#include "utils.h"

Audio::Audio() {

}

Audio::~Audio() {

}

/* TO DO MANAGER
HCHANNEL Audio::play(float volume) {

} 

void Audio::Stop(HCHANNEL channel) {

};
Audio* Audio::Get(const char* filename) {

}
HCHANNEL* Audio::Play(const char* filename) {

}
*/
HSAMPLE Audio::loadSample(const char* filename) {

	//El handler para un sample
	HSAMPLE hSample;

	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, filename, 0, 0, 3, 0);
	if (hSample == 0)
	{
		std::cout << "ERROR load " << filename << std::endl;
	}

	std::cout << "+ AUDIO load " << filename << std::endl;
	return hSample;
}

void Audio::playGameSound(const char* filename) {
	

	HSAMPLE hSample = loadSample(filename);
	//El handler para un canal
	HCHANNEL hSampleChannel;
	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);


	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
}