// WavetableSynth.cpp
// Mason Kaschube (mason.kaschube)
// Cs245 S24 4/5/24

#include "WavetableSynth.h"
#include <cmath>
#include <iostream>

WavetableSynth::WavetableSynth(int devno, int R) :
    MidiIn(devno),
    sampleRate(R),
    velocity(1),
    time(0.0f),
    currNote(0),
    pitchWheelChange(0.0f),
    modWheelChange(0),
    scale(1),
    volume(1),
    ad("WavetableSynth.wav"),
    sample(&ad, 0, double(440.0f/452.0f) * (44100.0f / R), 5224, 8908),
    adsr(0.1f, 0.5f, 0.5f, 1.85f, R)
{
    MidiIn::start();
    adsr.sustainOff();
}

WavetableSynth::~WavetableSynth()
{
    MidiIn::stop();
    MidiIn::~MidiIn();
}

float WavetableSynth::output()
{
    // grab the current sample and adsr slices
    float res = sample.output() * adsr.output();

    // scale with volume
    res *= volume * velocity;

    // grab the base pitch offset
    double currOffset = double(currNote - 69) * 100;

    // scale by the pitch wheel change
    currOffset += pitchWheelChange;

    // scale by the mod wheel 
    double pi = 2 * cos(0.0f);
    currOffset += modWheelChange * sin(2 * pi * 5 * time);

    // apply the pitch
    sample.pitchOffset(currOffset);

    // return the audio slice
    return res;
}

void WavetableSynth::next()
{
    // advance the sample and adsr envelopes
    sample.next();
    adsr.next();

    // advance the time (used for the mod wheel and this value worked the best)
    time += (scale / sampleRate);
}

void WavetableSynth::onNoteOn(int channel, int note, int velocity_)
{
    // change the note pitch to this new note, 
    sample.pitchOffset((note-69) * 100);
    sample.reset();
    adsr.reset();

    // save out our note's volume
    velocity = double(velocity_ / 127.0f);

    // save out the index of the current note
    currNote = note;
}

void WavetableSynth::onNoteOff(int channel, int note)
{
    // turn off the adsr sustain phase
    if (note == currNote)
    {
        adsr.sustainOff();
    }
}

void WavetableSynth::onPitchWheelChange(int channel, float value)
{
    // calculate the value for the pitch change in [-200, 200] cents
    pitchWheelChange = 200.0f * value;

    scale = pow(2, pitchWheelChange / 1200.0f);
}

void WavetableSynth::onModulationWheelChange(int channel, int value)
{
    // save out the mod wheel change
    modWheelChange = value;
}

void WavetableSynth::onVolumeChange(int channel, int level) 
{ 
    // save out our volume level
    volume = double(level / 127.0f);
}