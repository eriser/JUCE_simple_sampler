/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED


class MainComponent : public Component,
                      private AudioIODeviceCallback,  // [1]
                    private MidiInputCallback,
                    public Slider::Listener
{
public:
    //==============================================================================
    MainComponent()
        : audioSetupComp (audioDeviceManager, 0, 0, 0, 256,
                          true, // showMidiInputOptions must be true
                          false, true, false),
           keyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard)
    {
        
        setSize (650, 560);

        audioDeviceManager.initialise (0, 2, nullptr, true, String(), nullptr);
        audioDeviceManager.addMidiInputCallback (String(), this); // [6]
        audioDeviceManager.addAudioCallback (this);
        const StringArray midiInputs (MidiInput::getDevices());
        audioDeviceManager.setMidiInputEnabled(midiInputs[0],true);
        addAndMakeVisible (audioSetupComp);
        addAndMakeVisible (keyboardComponent);
        addAndMakeVisible (midinote_label);
        

        
        for (int indice=0;indice<8;indice++)
        {
            addAndMakeVisible (pitchSlider[indice]);
            pitchSlider[indice].setRange(-24,24);
            pitchSlider[indice].addListener(this);
        }
        
        midinote_label.setText("None",dontSendNotification);
    }

    ~MainComponent()
    {
        audioDeviceManager.removeMidiInputCallback (String(), this);
    }

    //==============================================================================
    void resized() override
    {
        //Rectangle<int> r (getLocalBounds());
        //audioSetupComp.setBounds (r);
        
        keyboardComponent.setBounds (8, 10, getWidth() - 16, 64);
        
        midinote_label.setBounds (8, 100, getWidth() - 16, 20);
        
        for (int indice=0;indice<8;indice++)
        {
           pitchSlider[indice].setBounds (8, 130+30*indice, getWidth() - 16, 20);
        }
        
        
    }

    //==============================================================================
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override
    {
        // make buffer
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        
        // clear it to silence
        buffer.clear();
        MidiBuffer incomingMidi;
        
        // get the MIDI messages for this audio block
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        const double sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void audioDeviceStopped() override
    {
    }
    
   
    
    void sliderValueChanged (Slider* slider) override
    {
        for (int indice=0;indice<8;indice++)
        {
            if (slider == &(pitchSlider[indice]))
                {
                    int pitch=pitchSlider[indice].getValue();
                    synth.samplemap[indice]->detune=pitch;
                    
                }
        }
    }
 

private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message) override
    {
        keyboardState.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
        //midinote_label.setText(message.getDescription(),dontSendNotification);
    }

    //==============================================================================
    AudioDeviceManager audioDeviceManager;         // [3]
    AudioDeviceSelectorComponent audioSetupComp;   // [4]
    
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;
    
    DrumSynthesiser synth;
    MidiMessageCollector midiCollector;            // [5]
    
    Label midinote_label;
    Slider pitchSlider[8];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
