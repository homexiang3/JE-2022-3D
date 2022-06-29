#pragma once
#include "extra/bass.h"
#include <map>
#include <string>
#include <vector>


class Audio
{
public:
	std::vector<HSAMPLE> samples; //para nuestro manager game sounds
	HSAMPLE sample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad

	std::vector<HSAMPLE> Menu_samples; //menu souns
	Audio(); //importante poner sample a cero aqui
	~Audio(); //aqui deberiamos liberar el sample con BASS_SampleFree
	/*
	HCHANNEL play(float volume); //lanza el audio y retorna el channel donde suena

	static void Stop(HCHANNEL channel); //para parar un audio necesitamos su channel
	static Audio* Get(const char* filename); //manager de audios 
	static HCHANNEL* Play(const char* filename); //version estática para ir mas rapido
	*/
	//initial function se usa de prueba
	void PlayGameSound(int pos, int where);
	void LoadSample(const char* fileName, int vector);
	void ResetAudio();
	
};



