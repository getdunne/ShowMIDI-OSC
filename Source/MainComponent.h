#pragma once
#include "JuceHeader.h"
#include "UnidirectionalSlider.h"
#include "BidirectionalSlider.h"
#include "BooleanIndicator.h"
#include "MyMidiKeyboardComponent.h"
#include "SustainPedalLogic.hpp"

class MainComponent : public Component
                    , protected MidiInputCallback
                    , protected ChangeBroadcaster
                    , protected ChangeListener
{
public:
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;
    void mouseDown(const MouseEvent&) override;

protected:
    // MidiInputCallback
    void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) override;

    // ChangeListener
    void changeListenerCallback(ChangeBroadcaster*) override;

    // MainComponent
    void setMidiInput(int index);
    void sendMidiOverOSC(uint8 byte1, uint8 byte2, uint8 byte3);

private:
    AudioDeviceManager deviceManager;

    bool pedalDown, newPedalDown;
    int keyCount, ccCount;
    bool ccCountChanged;
    int lastInputIndex;
    float pitchWheelValue, newPitchWheelValue;
    float ccValue[4], newCcValue[4];
    int inputCc[4], outputCc[4];
    MidiKeyboardState keyboardState;
    AudioKitCore::SustainPedalLogic pedalLogic;

    DatagramSocket socket;

    Image logo;
    const int offsetFromTop = 2;
    const int maxLogoHeight = 100;
    Rectangle<int> logoRect;

    //ComboBox midiInputList;
    //Label midiInputListLabel;

    MyMidiKeyboardComponent keyboardComponent;
    BooleanIndicator sustainPedal;
    BidirectionalSlider pitchWheel;
    UnidirectionalSlider modWheel;
    UnidirectionalSlider breathController, footController, softPedal;

    TextButton midiButton, keyboardButton, ccButton;
    TextButton ontopButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
