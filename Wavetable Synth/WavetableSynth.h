// WavetableSynth.h
// Mason Kaschube (mason.kaschube)
// -- interface to wavetable synthesizer class
// cs245 2024.03

#ifndef CS245_WAVETABLESYNTH_H
#define CS245_WAVETABLESYNTH_H

#include "MidiIn.h"
#include "AudioData.h"
#include "Resample.h"
#include "ADSR.h"


class WavetableSynth : private MidiIn {
  public:
    WavetableSynth(int devno, int R);
    ~WavetableSynth(void);
    float output(void);
    void next(void);
  private:
    int sampleRate;
    double velocity;
    double time;
    int currNote;
    double pitchWheelChange;
    int modWheelChange;
    float scale;
    double volume;
    AudioData ad;
    Resample sample;
    ADSR adsr;
    void onNoteOn(int channel, int note, int velocity) override;
    void onNoteOff(int channel, int note) override;
    void onPitchWheelChange(int channel, float value) override;
    void onModulationWheelChange(int channel, int value) override;
    void onVolumeChange(int channel, int level) override;
};


#endif

