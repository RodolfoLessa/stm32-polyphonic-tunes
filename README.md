# stm32-polyphonic-tunes


## How to write song: 

You may be wondering how one can write songs for the stm32 without any knowledge of Music! Its actually quite interesting and fun. First you need to know the value of the notes in Music. 

The API only can play 4 voices together.

### The stm32 Code:

#### Struct **music**

Formed by 5 parameters: octave, bpm, control, voices and nota ref. 
Voices and control has an index to 

##### The octave, the bpm and the ref note are fixed parameters of the music.

**octave**: octave reference. Usually the reference is the middle C (C4). 
*By default, notes are associated with the first octave: c0.*
**Formula: note = note + 12*octave**

**bpm**: beats per minute - the tempo of the song. 
**ref note**: the note ()

**voice**: music *string.*
An example: *music.voices[0] = "4g1,8p,8d1,4g1,8p,8d1"*

The control of the voice string is done through 5 parameters of the struct song_ctrl.

**note**: contains the FTW table index corresponding to the note.
**duration**: contains the time the note will be played based on the following equation.
**position**: the index of the string related to note and tempo.

**pause_ticks**: number of clock pulses which the note is not played.   }
**remaining_ticks**: number of clock pulses which the note is played.   } The sum of the two parameters is the total note time


**Voice and control has voice reference indicators. It is extremely important to use the same indices for the same voices.**
. example to reference the same voice:
music.voice[*0*]; music.control.note[*0*]; music.control.note[*0*];

#### Note request:
##### The song's notes are made after the second semicolon.

##### Time:
- The duration of each note is made before the letter
. ex: 4f is a quarter note
- Put a period . for dotted notes
. ex: 2.f is a dotted half note
- The duration can be determined by this formula.
 **Formula: 4/(note value)= duration. So an eight note would be 4/(1/2) = 8.**

##### Note:
- There are only 7 possible letters. a,b,c,d,e,f,g
- A rest uses the letter p
- To name a letter a flat put an underscore _ right after the letter
. ex: b_
- To name a letter a sharp put a hashtag # right after the letter
. ex: c#
- To raise a note up an octave, put the number of octaves to be raised after the Sharp/Flats/Letter
. ex: b1
. ex2: c#1
- To drop a note down an Octave, put a minus - sign and then the number of octaves to be dropped after the Sharp/Flats/Letter
. ex: b-1
. ex: c#-2

#### Creating a Song:

music song;
song.octave = 4. // reference in middle c (c4)
song.bpm = 100; 
song.ref_note = 4 // quarter_note
song.voice = "8c,8d,8e,8f,8g,8a,8b,8c1"; // c-scale

## How to read a note
To read the note you just have to know the control variables, and note_uptade gives it. Before using the note update, it is important to initialize the position field with null values.

**uint8_t note_update(music *song, uint8_t instrument)**
instrument represent the voice of the score. Usually in classical music, they have several instruments in which each represents a voice.

The function will uptade duration, position, note and will return 1 when the string have more notes to read. If not, return 0. The function return it to other the programmer know when the music it is over.

example: uint8_t violine_I; note_update(&song, violine_I);

song.control.note[violine_I] will be the number that represent c4 in the FTW table. song.contol.duration[violine_I] will be the time -> 4(nota_ref)/8(time);

## How to produce sound

*The library polyphonic_tunes make it! 

First you have to understand how the code produce play diferent notes at the same time.
Basicly it sums diferrent sin waves at the same time.
Read the article: https://www.gamry.com/application-notes/EIS/waveform-generation-and-frequency-resolution/

### Note:
**Sine discretization**: take one period sine wave - 2pi - and divide in 2^11 bits to get 2048 samples.
*Note that is the SinTable in the polyphonic_tunes_tables.h

**FTW (Frequency Tuning Word)**: parameter that the code uses to get the frequency note.
*Note that in the PITCHS table has the notes from c-2 to c8. Start in c-2 and goes one tone up until c8, so PITCHS[0] represent the FTW of c in octave -2.

**Get better**: use a RC filter to get cleanner sound!

### How to use song_egine:

This is the library that you will use, it cointains the functions to control music.

I) You have to initialize the timer interruption (initialize_song_engine and song_scheduler), to make use the library.
**ctrl_tim**: the timer that you will use.
**timer_freq**: timer frequency.
**htim**: interruption timer to know when a note ends - It is the same fo ctrl_tim.
#### Timer and PWM configuration:
**Your timer input frequency must has 20kHz**
**Put PWM in the max frequency possible and put the resolution to 2^11 bits** 

*void initialize_song_engine(double timer_freq, TIM_HandleTypeDef* ctrl_tim);

*void song_scheduler(TIM_HandleTypeDef* htim);

II) After that, you can treat output with pwm or use a custom treatment output.

*void set_pwm_output(TIM_HandleTypeDef* output_tim, uint8_t out_channel);

*void set_custom_output_handler(void (*output_handler)(uint32_t));

III) Set the music struct parameters and load the song. 

*void load_song(music musica);

IV) Just play and have fun.

You can use other functions to set/get the status of the song.

*void clear_song(music musica);
void play_song();

void pause_song();

void stop_song();

song_status get_song_status();

### How to use polyphonic_tune:

This library will join the 4 voices.

I) Set the pwm and timer necessaries, like the song_engine library. - You can use both library independently

II) You can customize your sound with diferents sets like sin wave(setwave).
But you have to set the function in **bold**.

*void setWave(uint8_t voice, uint8_t wave);

*void setPitch(uint8_t voice, uint8_t MIDInote);

*void setEnvelope(uint8_t voice, uint8_t env);

*void setLength(uint8_t voice, uint8_t length);

*void setMod(uint8_t voice, uint16_t mod);

*void mTrigger(uint8_t voice, uint8_t MIDInote);

*void setFrequency(uint8_t voice, float f);

*void setTime(uint8_t voice, float t);

*void trigger(uint8_t voice);


III) To set the 

void setupVoice(uint8_t voice, uint8_t wave, uint8_t pitch, uint8_t env, uint8_t length, uint16_t mod);



void audio_synthesis();
void setup_synth_engine(double timer_frequency, TIM_HandleTypeDef* ctrl_tim);
void setup_synth_custom_output_handler(void (*output_handler)(uint32_t));
void setup_synth_pwm_output_handler(TIM_HandleTypeDef* output_tim, uint8_t out_channel);
void setupVoice(uint8_t voice, uint8_t wave, uint8_t pitch, uint8_t env, uint8_t length, uint16_t mod);
void setWave(uint8_t voice, uint8_t wave);
void setPitch(uint8_t voice, uint8_t MIDInote);
void setEnvelope(uint8_t voice, uint8_t env);
void setLength(uint8_t voice, uint8_t length);
void setMod(uint8_t voice, uint16_t mod);
void mTrigger(uint8_t voice, uint8_t MIDInote);
void setFrequency(uint8_t voice, float f);
void setTime(uint8_t voice, float t);
void trigger(uint8_t voice);

void pause(uint8_t voice);

void synth_suspend();
void synth_resume();
