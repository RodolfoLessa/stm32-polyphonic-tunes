# stm32-polyphonic-tunes



## Tutorial to read music scores with the API

### How to write song: 

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

The control of the voice string is done through 3 parameters of the struct song_ctrl.

**note**: contains the FTW table index corresponding to the note.
**duration**: contains the time the note will be played based on the following equation.
**position**: the index of the string related to note and tempo.

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

### How to read a song

To read the note you just have to know the control variables, and **note_uptade** gives it. 
**Before using the note update, it is important to initialize the position field with null values.**
##### uint8_t note_update(music *song, uint8_t instrument)

**instrument** represent the voice of the score. 
*Usually in classical music, they have several instruments in which each represents a voice.*

The function will uptade duration, position, note and will return 1 when the string have more notes to read. If not, return 0.
The function return it to other the programmer know when the music it is over.


example: uint8_t violine_I;
note_update(&song, violine_I);

song.control.note[violine_I] will be the number that represent c4 in the FTW table. 
song.contol.duration[violine_I] will be the time -> 4(nota_ref)/8(time); 


