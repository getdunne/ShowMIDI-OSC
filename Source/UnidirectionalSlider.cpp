#include "UnidirectionalSlider.h"

void UnidirectionalSlider::paint (Graphics& g)
{
    auto area = getLocalBounds();

    area.reduce(1, 1);

    if (value > 0.0f)
    {
        g.setColour(colour);
        int barHeight = int(value * area.getHeight());
        Rectangle<int> bar(area.getX(),
                           area.getY() + area.getHeight() - barHeight,
                           area.getWidth(),
                           barHeight);
        g.fillRect(bar);
    }

    g.setColour(colour.darker());
    g.drawText(String(outputCc), area.toFloat(), Justification::centredTop);
    g.drawText(String(inputCc), area.toFloat(), Justification::centredBottom);

    g.setColour(Colours::white);
    g.drawRect(area);
}

void UnidirectionalSlider::setValue(float v)
{
    value = v;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
	repaint();
}

void UnidirectionalSlider::mouseDown(const MouseEvent& evt)
{
    bool changingOutput = evt.getPosition().getY() < getHeight() / 2;

    if (evt.mods.isRightButtonDown() || evt.mods.isCtrlDown())
    {
        PopupMenu menu;
        for (int cn = 0; cn < 128; cn++)
        {
            String controllerNameString = "CC" + String(cn);
            const char* controllerName = MidiMessage::getControllerName(cn);
            if (controllerName)
                controllerNameString += " " + String(controllerName);
            menu.addItem(cn + 1, controllerNameString);
        }
        int id = menu.show();
        if (id)
        {
            if (changingOutput)
                outputCc = id - 1;
            else
                inputCc = id - 1;
            repaint();
        }
    }
}
