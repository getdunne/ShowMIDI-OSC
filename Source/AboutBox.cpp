#include "AboutBox.h"

LinkLabel::LinkLabel(StringRef url)
    : Label(),
    linkUrl(url)
{
    setColour(Label::textColourId, Colours::skyblue);
    normal = getFont();
    underlined = Font(normal.getHeight(), Font::underlined);
}

void LinkLabel::mouseDown(const MouseEvent&)
{
    linkUrl.launchInDefaultBrowser();
}

void LinkLabel::mouseEnter(const MouseEvent&)
{
    setFont(underlined);
}

void LinkLabel::mouseExit(const MouseEvent&)
{
    setFont(normal);
}


AboutBox::AboutBox()
    : guruLine("https://www.pluginguru.com")
    , usageLine("https://getdunne.net/download/showmidi-osc/")
    , fossLine("https://github.com/getdunne/ShowMIDI-OSC")
{
    logo = ImageCache::getFromMemory(BinaryData::aboutlogo_png, BinaryData::aboutlogo_pngSize);

    guruLine.setText("ShowMIDI-OSC by PlugInGuru", NotificationType::dontSendNotification);
    guruLine.setJustificationType(Justification::horizontallyCentred);
    addAndMakeVisible(guruLine);

    copyrightLine.setColour(Label::textColourId, Colours::white);
    copyrightLine.setJustificationType(Justification::horizontallyCentred);
    copyrightLine.setText("(c)2019 by Shane Dunne",
                          NotificationType::dontSendNotification);
    addAndMakeVisible(copyrightLine);

    usageLine.setText("Download and usage instructions", NotificationType::dontSendNotification);
    usageLine.setJustificationType(Justification::horizontallyCentred);
    addAndMakeVisible(usageLine);

    fossLine.setText("Open-source code on GitHub", NotificationType::dontSendNotification);
    fossLine.setJustificationType(Justification::horizontallyCentred);
    addAndMakeVisible(fossLine);
}

void AboutBox::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    g.drawImageAt(logo, 0, 0);
}

void AboutBox::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromLeft(logo.getWidth());
    bounds.removeFromTop(3);
    guruLine.setBounds(bounds.removeFromTop(16));
    copyrightLine.setBounds(bounds.removeFromTop(16));
    fossLine.setBounds(bounds.removeFromTop(16));
    usageLine.setBounds(bounds.removeFromTop(16));
    vstLine.setBounds(bounds.removeFromTop(16));
}

void AboutBox::modal()
{
    DialogWindow::LaunchOptions options;
    options.dialogTitle = "About ShowMIDI-OSC";
    AboutBox* view = new AboutBox;
    view->setBounds(0, 0, 380, 80);
    options.content.set(view, true);
    options.useNativeTitleBar = false;
    options.launchAsync();
}
