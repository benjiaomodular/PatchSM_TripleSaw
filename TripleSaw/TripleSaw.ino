#include "DaisyDuino.h"

DaisyHardware patch;
Oscillator osc_a, osc_b, osc_c;
float voltsPerNote = 0.0833;
float note = 0;

void AudioCallback(float**  in, float** out, size_t size)
{

    // Get the raw course tune value in volts (0 - 5)
    float coarse_tune = 12.f + (patch.GetAdcValue(0) * 72.f);

    /** Using the second analog input we'll use a fine tune of the primary frequency. */
    float fine_tune = patch.GetAdcValue(1) * 10.f;

    /** Convert those values from midi notes to frequency */
    float freq_a = mtof(note + coarse_tune + fine_tune);

    /** Our third control will detune the voices from each other (0 - 5V) */
    float detune_amt = patch.GetAdcValue(2);

    /**  Detuning each of the other oscillators by upto 5% of the primary
   * frequency */
    float freq_b = freq_a + (0.05 * freq_a * detune_amt);
    float freq_c = freq_a - (0.05 * freq_a * detune_amt);

    /** Set the oscillators to those frequencies */
    osc_a.SetFreq(freq_a);
    osc_b.SetFreq(freq_b);
    osc_c.SetFreq(freq_c);

    /** This loop will allow us to process the individual samples of audio */
    for(size_t i = 0; i < size; i++)
    {
        /** We'll combine both oscillator signals using simple addition
     *
     *  All of the DSP modules within DaisySP feature a "Process" function
     *  that will return the next sample of the synthesized sound, or effect.
     */
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();

        /** In this example both outputs will be the same */
        out[0][i] = out[1][i] = sig;
    }
}

void setup()
{
    /** Initialize the patch_sm hardware object */
    patch = DAISY.init(DAISY_PATCH_SM);

    /** Initialize the oscillator modules */
    osc_a.Init(DAISY.AudioSampleRate());
    osc_b.Init(DAISY.AudioSampleRate());
    osc_c.Init(DAISY.AudioSampleRate());

    /** Let's set some specific parameters that won't change during the example
   *
   *  The POLYBLEP waveforms are bandlimited which means that they won't alias.
   */
    osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

    /** Start Processing the audio */
    DAISY.StartAudio(AudioCallback);
}

void loop(){
    /** First we'll tell the hardware to process the 8 analog inputs */
    patch.ProcessAnalogControls();

    int value = analogRead(PIN_PATCH_SM_CV_5);
    float volts = patch.AnalogReadToVolts(value);
    note = int(volts / voltsPerNote);

    patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_2, volts);
    patch.WriteCvOut(PIN_PATCH_SM_CV_OUT_1, 5);
}
