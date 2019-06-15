#include "MainComponent.h"
#include "AboutBox.h"
#include "tinyosc.h"

//#define BRANDED_VERSION

MainComponent::MainComponent()
    : pedalDown(false), newPedalDown(false)
    , keyCount(61)
    , ccCount(2)
    , ccCountChanged(false)
    , lastInputIndex(0)
    , pitchWheelValue(0.0f), newPitchWheelValue(0.0f)
    , keyboardComponent(keyboardState, MidiKeyboardComponent::horizontalKeyboard)
    , modWheel(Colours::hotpink, inputCc[0], outputCc[0])
    , breathController(Colours::greenyellow, inputCc[1], outputCc[1])
    , footController(Colours::cyan, inputCc[2], outputCc[2])
    , softPedal(Colours::darkorange, inputCc[3], outputCc[3])
{
    setOpaque(true);

    logo = ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    logoRect.setSize(logo.getWidth(), maxLogoHeight);
    logoRect.setY(offsetFromTop);

    addAndMakeVisible(pitchWheel);
    pitchWheel.setValue(0.0f);

    addAndMakeVisible(modWheel);
    modWheel.setValue(0.0f);

    addChildComponent(breathController);
    breathController.setValue(0.0f);

    addChildComponent(footController);
    footController.setValue(0.0f);

    addChildComponent(softPedal);
    softPedal.setValue(0.0f);

    keyboardComponent.setOctaveForMiddleC(4);
    addAndMakeVisible(keyboardComponent);

    addAndMakeVisible(sustainPedal);
    sustainPedal.setValue(0.0f);

    addAndMakeVisible(midiButton);
    setMidiInput(MidiInput::getDefaultDeviceIndex());
    midiButton.setButtonText("M");
    midiButton.onClick = [this]()
    {
        PopupMenu menu;
        auto midiInputs = MidiInput::getDevices();
        for (int i = 0; i < midiInputs.size(); i++)
            menu.addItem(i + 1, midiInputs[i], true, i == lastInputIndex);
        int sel = menu.show();
        if (sel > 0) setMidiInput(sel - 1);
    };

    addAndMakeVisible(keyboardButton);
    keyboardButton.setButtonText("61");
    keyboardButton.onClick = [this]
    {
        PopupMenu menu;
        menu.addItem(25, "25 keys");
        menu.addItem(37, "37 keys");
        menu.addItem(49, "49 keys");
        menu.addItem(61, "61 keys");
        menu.addItem(73, "73 keys");
        menu.addItem(76, "76 keys");
        menu.addItem(88, "88 keys");
        int kc = menu.show();
        if (kc)
        {
            keyCount = kc;
            keyboardButton.setButtonText(String(kc));
            resized();
        }
    };

    addAndMakeVisible(ccButton);
    ccButton.setButtonText(String(ccCount));
    ccButton.onClick = [this]
    {
        PopupMenu menu;
        menu.addItem(1, "1: Pitch Bend");
        menu.addItem(2, "2: PB + Mod Wheel");
        menu.addItem(5, "5: PB + 4 CCs");
        int ccc = menu.show();
        if (ccc)
        {
            ccCount = ccc;
            ccButton.setButtonText(String(ccc));
            pitchWheel.setVisible(ccCount >= 1);
            modWheel.setVisible(ccCount >= 2);
            breathController.setVisible(ccCount >= 3);
            footController.setVisible(ccCount >= 4);
            softPedal.setVisible(ccCount >= 5);
            resized();
        }
    };

    addAndMakeVisible(ontopButton);
    ontopButton.setButtonText("OT");
    ontopButton.onClick = [this]()
    {
        auto window = getParentComponent();
        bool onTop = window->isAlwaysOnTop();
        PopupMenu menu;
        menu.addItem(1, "Stay On Top", true, onTop);
        menu.addItem(2, "Normal window", true, !onTop);
        int sel = menu.show();
        if (sel)
        {
            window->setAlwaysOnTop(sel == 1);
            ontopButton.setButtonText(sel == 1 ? "OT" : "NW");
        }
    };

    pitchWheelValue = 0.0f;

    for (int i = 0; i < 4; i++) ccValue[i] = newCcValue[i] = 0;
    outputCc[0] = inputCc[0] = 1;
    outputCc[1] = inputCc[1] = 2;
    outputCc[2] = inputCc[2] = 4;
    outputCc[3] = inputCc[3] = 67;

    addChangeListener(this);

    setSize(730, 100);
}

MainComponent::~MainComponent()
{
    removeChangeListener(this);
    deviceManager.removeMidiInputCallback(MidiInput::getDevices()[lastInputIndex], this);
    socket.shutdown();
}

void MainComponent::paint (Graphics& g)
{
    g.fillAll(Colours::black);

    using RP = RectanglePlacement;
    g.drawImage(logo, logoRect.toFloat(), RP::doNotResize | RP::xMid | RP::yBottom);
}

void MainComponent::mouseDown(const MouseEvent& evt)
{
#ifdef BRANDED_VERSION
    if (logoRect.contains(evt.getPosition()))
    {
        AboutBox::modal();
    }
#endif
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    const int buttonsWidth = 36;
    const int maxButtonHeight = 24;
    int sliderWidth = 30;
    if (ccCount >= 3) sliderWidth = 24;

    area.removeFromLeft(logo.getWidth());
    if (ccCount >= 1) pitchWheel.setBounds(area.removeFromLeft(sliderWidth));
    if (ccCount >= 2) modWheel.setBounds(area.removeFromLeft(sliderWidth));
    if (ccCount >= 3) breathController.setBounds(area.removeFromLeft(sliderWidth));
    if (ccCount >= 4) footController.setBounds(area.removeFromLeft(sliderWidth));
    if (ccCount >= 5) softPedal.setBounds(area.removeFromLeft(sliderWidth));
    if (ccCount >= 1) area.removeFromLeft(3);

    auto buttonsArea = area.removeFromRight(buttonsWidth);
    buttonsArea.removeFromLeft(3);
    int buttonHeight = (buttonsArea.getHeight() - 6) / 4;
    if (buttonHeight > maxButtonHeight) buttonHeight = maxButtonHeight;
    midiButton.setBounds(buttonsArea.removeFromTop(buttonHeight).reduced(2));
    keyboardButton.setBounds(buttonsArea.removeFromTop(buttonHeight).reduced(2));
    ccButton.setBounds(buttonsArea.removeFromTop(buttonHeight).reduced(2));
    ontopButton.setBounds(buttonsArea.removeFromTop(buttonHeight).reduced(2));

    sustainPedal.setBounds(area.removeFromBottom(6));
    area.removeFromBottom(2);

    keyboardComponent.setBounds(area);
    switch (keyCount)
    {
    case 25:
        keyboardComponent.setKeyWidth(area.getWidth() / 15.0f);
        keyboardComponent.setAvailableRange(48, 72);
        break;
    case 37:
        keyboardComponent.setKeyWidth(area.getWidth() / 22.0f);
        keyboardComponent.setAvailableRange(48, 84);
        break;
    case 49:
        keyboardComponent.setKeyWidth(area.getWidth() / 29.0f);
        keyboardComponent.setAvailableRange(36, 84);
        break;
    case 61:
        keyboardComponent.setKeyWidth(area.getWidth() / 36.0f);
        keyboardComponent.setAvailableRange(36, 96);
        break;
    case 73:
        keyboardComponent.setKeyWidth(area.getWidth() / 43.0f);
        keyboardComponent.setAvailableRange(24, 96);
        break;
    case 76:
        keyboardComponent.setKeyWidth(area.getWidth() / 45.0f);
        keyboardComponent.setAvailableRange(21, 96);
        break;
    default:
        keyboardComponent.setKeyWidth(area.getWidth() / 52.0f);
        keyboardComponent.setAvailableRange(21, 108);
        break;
    }
}

/** Starts listening to a MIDI input device, enabling it if necessary. */
void MainComponent::setMidiInput(int index)
{
    auto list = MidiInput::getDevices();

    deviceManager.removeMidiInputCallback(list[lastInputIndex], this);

    auto newInput = list[index];

    if (!deviceManager.isMidiInputEnabled(newInput))
        deviceManager.setMidiInputEnabled(newInput, true);

    deviceManager.addMidiInputCallback(newInput, this);

    lastInputIndex = index;
}

// These methods handle callbacks from the midi device + on-screen keyboard..
void MainComponent::handleIncomingMidiMessage(MidiInput* /*source*/, const MidiMessage& msg)
{
    const uint8* mdp = msg.getRawData();

    if (msg.isPitchWheel())
    {
        newPitchWheelValue = float(msg.getPitchWheelValue() - 8192) / 8192.0f;
        sendMidiOverOSC(mdp[0], mdp[1], mdp[2]);
        sendChangeMessage();
    }
    else if (msg.isControllerOfType(64))
    {
        newPedalDown = msg.getControllerValue() > 63.0f;
        sendMidiOverOSC(mdp[0], mdp[1], mdp[2]);
        sendChangeMessage();
    }
    else if (msg.isController())
    {
        if (msg.isControllerOfType(inputCc[0]))
        {
            newCcValue[0] = msg.getControllerValue() / 127.0f;
            sendMidiOverOSC(mdp[0], uint8(outputCc[0]), mdp[2]);
            sendChangeMessage();
        }
        if (msg.isControllerOfType(inputCc[1]))
        {
            newCcValue[1] = msg.getControllerValue() / 127.0f;
            sendMidiOverOSC(mdp[0], uint8(outputCc[1]), mdp[2]);
            sendChangeMessage();
        }
        if (msg.isControllerOfType(inputCc[2]))
        {
            newCcValue[2] = msg.getControllerValue() / 127.0f;
            sendMidiOverOSC(mdp[0], uint8(outputCc[2]), mdp[2]);
            sendChangeMessage();
        }
        if (msg.isControllerOfType(inputCc[3]))
        {
            newCcValue[3] = msg.getControllerValue() / 127.0f;
            sendMidiOverOSC(mdp[0], uint8(outputCc[3]), mdp[2]);
            sendChangeMessage();
        }
    }
    else if (msg.isNoteOn())
    {
        pedalLogic.keyDownAction(msg.getNoteNumber());
        keyboardState.noteOn(1, msg.getNoteNumber(), msg.getVelocity() / 127.0f);
        sendMidiOverOSC(mdp[0], mdp[1], mdp[2]);
    }
    else if (msg.isNoteOff())
    {
        if (pedalLogic.keyUpAction(msg.getNoteNumber()))
            keyboardState.noteOff(1, msg.getNoteNumber(), msg.getVelocity() / 127.0f);
        sendMidiOverOSC(mdp[0], mdp[1], mdp[2]);
    }
}

void MainComponent::sendMidiOverOSC(uint8 byte1, uint8 byte2, uint8 byte3)
{
    uint8 data[4];
    data[0] = 0;
    data[1] = byte3;
    data[2] = byte2;
    data[3] = byte1;

    uint8 buf[16];
    int len = tosc_writeMessage((char*)buf, sizeof(buf), "/midi", "m", data);
    socket.write("127.0.0.1", 12101, buf, len);
}

void MainComponent::changeListenerCallback(ChangeBroadcaster*)
{
    if (newPitchWheelValue != pitchWheelValue)
    {
        pitchWheelValue = newPitchWheelValue;
        pitchWheel.setValue(pitchWheelValue);
    }
    if (newCcValue[0] != ccValue[0])
    {
        ccValue[0] = newCcValue[0];
        modWheel.setValue(ccValue[0]);
    }
    if (newCcValue[1] != ccValue[1])
    {
        ccValue[1] = newCcValue[1];
        breathController.setValue(ccValue[1]);
    }
    if (newCcValue[2] != ccValue[2])
    {
        ccValue[2] = newCcValue[2];
        footController.setValue(ccValue[2]);
    }
    if (newCcValue[3] != ccValue[3])
    {
        ccValue[3] = newCcValue[3];
        softPedal.setValue(ccValue[3]);
    }
    if (newPedalDown != pedalDown)
    {
        pedalDown = newPedalDown;
        sustainPedal.setValue(pedalDown);
        if (pedalDown)
        {
            pedalLogic.pedalDown();
            keyboardComponent.pedalDown();
        }
        else
        {
            pedalLogic.pedalUp();
            keyboardComponent.pedalUp();
            for (int nn = 0; nn < 128; nn++)
            {
                if (pedalLogic.isNoteSustaining(nn))
                    keyboardState.noteOff(1, nn, 0);
            }
        }
    }
}
