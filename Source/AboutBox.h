#pragma once
#include "JuceHeader.h"

class LinkLabel : public Label
{
public:
    LinkLabel(StringRef url);

    void mouseDown(const MouseEvent&) override;
    void mouseEnter(const MouseEvent&) override;
    void mouseExit(const MouseEvent&) override;

private:
    URL linkUrl;
    Font normal, underlined;
};

class AboutBox    : public Component
{
public:
    AboutBox();

    void paint (Graphics&) override;
    void resized() override;

    static void modal();

private:
    Image logo;

    LinkLabel guruLine;
    Label copyrightLine;
    LinkLabel usageLine;
    LinkLabel fossLine;
    Label vstLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutBox)
};
