/*
 * polyphonic-tunes.c
 *
 *  Created on: Oct 5, 2020
 *      Author: renanmoreira
 */


#include "polyphonic_tunes.h"
#include "polyphonic-tunes-tables.h"

#define SET(x,y) (x |=(1<<y))		        		//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       			// |
#define CHK(x,y) (x & (1<<y))           			// |
#define TOG(x,y) (x^=(1<<y))            			//-+

volatile uint16_t PCW[4] = {0, 0, 0, 0};			//-Wave phase accumolators
volatile uint16_t FTW[4] = {1000, 200, 300, 400};           //-Wave frequency tuning words
volatile uint8_t AMP[4] = {255, 255, 255, 255};           //-Wave amplitudes [0-255]
volatile uint16_t PITCH[4] = {500, 500, 500, 500};          //-Voice pitch
volatile int16_t MOD[4] = {20, 0, 64, 127};                         //-Voice envelope modulation [0-1023 512=no mod. <512 pitch down >512 pitch up]
volatile uint16_t* wavs[4];                                  //-Wave table selector [address of wave in memory]
volatile uint16_t* envs[4];                                  //-Envelopte selector [address of envelope in memory]
volatile uint16_t EPCW[4] = {0x8000, 0x8000, 0x8000, 0x8000}; //-Envelope phase accumolator
volatile uint16_t EFTW[4] = {10, 10, 10, 10};               //-Envelope speed tuning word
volatile uint8_t divider = 4;                             //-Sample rate decimator for envelope
volatile uint16_t  tim = 0;
volatile uint8_t tik = 0;
volatile uint8_t output_mode;

void (*output_func)(uint16_t) = NULL;

TIM_HandleTypeDef* tim_control;

TIM_HandleTypeDef* tim_audio_out = NULL;
uint8_t output_channel;

double timer_bus_freq;

//*********************************************************************************************
//  Audio driver interrupt
//*********************************************************************************************

void audio_synthesis() {
	//-------------------------------
	// Time division
	//-------------------------------
	divider++;
	if (divider > 3) {
		divider = 0;
		tik = 1;
	}

	//-------------------------------
	// Volume envelope generator
	//-------------------------------

	if ((EPCW[divider]&0xFF00) < 0x8000) {
		EPCW[divider]+=EFTW[divider];
		AMP[divider] = envs[divider][(EPCW[divider]&0xFF00)>>8];
	} else {
		AMP[divider] = 0;
	}

	//-------------------------------
	//  Synthesizer/audio mixer
	//-------------------------------
	uint32_t synthesized_output = 0; //half of an int16 max value

	for(int i = 0; i < 4; i++) {
		PCW[i]+=FTW[i];
		synthesized_output += ( wavs[i][(PCW[i]&0xFFC0)>>6]*AMP[i])>>8;
	}

	synthesized_output = (synthesized_output>>8);

//	synthesized_output = (synthesized_output<<2);

	output_func(synthesized_output);

	//************************************************
	//  Modulation engine
	//************************************************
	//  FTW[divider] = PITCH[divider] + (int)   (((PITCH[divider]/64)*(EPCW[divider]/64)) /128)*MOD[divider];
	FTW[divider] = PITCH[divider] + (uint16_t)   (((PITCH[divider]>>6)*(EPCW[divider]>>6))/128)*MOD[divider];
	tim++;
}




void timer_output_handler(uint32_t output) {
	switch (output_channel) {
		case 1:
			tim_audio_out->Instance->CCR1 = output;
			break;
		case 2:
			tim_audio_out->Instance->CCR2 = output;
			break;
		case 3:
			tim_audio_out->Instance->CCR3 = output;
			break;
		case 4:
			tim_audio_out->Instance->CCR4 = output;
			break;
		default:
			break;
	}
}


void setup_synth_engine(double timer_frequency, TIM_HandleTypeDef* ctrl_tim, TIM_HandleTypeDef* output_tim, uint8_t out_channel, void (*output_handler)(uint32_t))
{
	timer_bus_freq = timer_frequency;

	tim_control = ctrl_tim;

	tim_audio_out = output_tim;
	output_channel = out_channel;

	if (output_handler != NULL)
		output_func = output_handler;
	else
		output_func = timer_output_handler;

}



//*********************************************************************
//  Timing/sequencing functions
//*********************************************************************

unsigned char synthTick(void)
{
	if(tik)
	{
	  tik=0;
	  return 1;  //-True every 4 samples
	}
	return 0;
}

unsigned char voiceFree(uint8_t voice)
{
	if (!(((unsigned char*)&EPCW[voice])[1]&0x80))
	  return 0;
	return 1;
}


//*********************************************************************
//  Setup all voice parameters in MIDI range
//  voice[0-3],wave[0-6],pitch[0-127],envelope[0-4],length[0-127],mod[0-127:64=no mod]
//*********************************************************************

void setupVoice(uint8_t voice, uint8_t wave, uint8_t pitch, uint8_t env, uint8_t length, uint16_t mod)
{
	setWave(voice,wave);
	setPitch(voice,pitch);
	setEnvelope(voice,env);
	setLength(voice,length);
	setMod(voice,mod);
}

//*********************************************************************
//  Setup wave [0-6]
//*********************************************************************

void setWave(uint8_t voice, uint8_t wave)
{
	switch (wave)
	{
	case TRIANGLE:
	  wavs[voice] = TriangleTable;
	  break;
	case SQUARE:
	  wavs[voice] = SquareTable;
	  break;
	case SAW:
	  wavs[voice] = SawTable;
	  break;
	case RAMP:
	  wavs[voice] = RampTable;
	  break;
	case NOISE:
	  wavs[voice] = NoiseTable;
	  break;
	default:
	  wavs[voice] = SinTable;
	  break;
	}
}
//*********************************************************************
//  Setup Pitch [0-127]
//*********************************************************************

void setPitch(uint8_t voice, uint8_t MIDInote)
{
	PITCH[voice]=PITCHS[MIDInote];
}

//*********************************************************************
//  Setup Envelope [0-4]
//*********************************************************************

void setEnvelope(uint8_t voice, uint8_t env)
{
	switch (env)
	{
	case 1:
	  envs[voice] = Env0;
	  break;
	case 2:
	  envs[voice] = Env1;
	  break;
	case 3:
	  envs[voice] = Env2;
	  break;
	case 4:
	  envs[voice] = Env3;
	  break;
	default:
	  envs[voice] = Env0;
	  break;
	}
}

//*********************************************************************
//  Setup Length [0-128]
//*********************************************************************

void setLength(uint8_t voice, uint8_t length)
{
	EFTW[voice]=EFTWS[length];
}

//*********************************************************************
//  Setup mod
//*********************************************************************

void setMod(uint8_t voice, uint16_t mod)
{
//    MOD[voice]=(unsigned int)mod*8;//0-1023 512=no mod
	MOD[voice]=mod-64;//0-1023 512=no mod
}

//*********************************************************************
//  Midi trigger
//*********************************************************************

void mTrigger(uint8_t voice, uint8_t MIDInote)
{
	PITCH[voice]=PITCHS[MIDInote];
	EPCW[voice]=0;
	FTW[divider] = PITCH[voice] + (int)   (((PITCH[voice]>>6)*(EPCW[voice]>>6))/128)*MOD[voice];
}

//*********************************************************************
//  Set frequency direct
//*********************************************************************

void setFrequency(uint8_t voice, float f)
{
	PITCH[voice]=f/(timer_bus_freq/65535.0);
}

//*********************************************************************
//  Set time
//*********************************************************************

void setTime(uint8_t voice, float t)
{
	EFTW[voice]=(1.0/t)/(timer_bus_freq/(32767.5*10.0));//[s];
}

//*********************************************************************
//  Simple trigger
//*********************************************************************

void trigger(uint8_t voice)
{
	EPCW[voice]=0;
	FTW[voice]=PITCH[voice];
	//    FTW[voice]=PITCH[voice]+(PITCH[voice]*(EPCW[voice]/(32767.5*128.0  ))*((int)MOD[voice]-512));
}

//*********************************************************************
//  Suspend/resume synth
//*********************************************************************

void suspend()
{

	HAL_TIM_Base_Stop(tim_control);
	HAL_TIM_Base_Stop_IT(tim_control);
}
void resume()
{
	HAL_TIM_Base_Start(tim_control);
	HAL_TIM_Base_Start_IT(tim_control);                           //-Start audio interrupt
}



