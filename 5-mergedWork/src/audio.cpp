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
void Audio::PlayGameSound(int pos)
{
	//El handler para un sample
	HSAMPLE hSample = this->samples[pos];

	HCHANNEL hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
}

void Audio::LoadSample(const char* fileName)
{
	//El handler para un sample
	HSAMPLE hSample;

	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, 0);

	if (hSample == 0)
	{
		std::cout << "+Error load  " << fileName << std::endl;
	}
	std::cout << "+AUDIO load" << fileName << std::endl;
	this->samples.push_back(hSample);
}