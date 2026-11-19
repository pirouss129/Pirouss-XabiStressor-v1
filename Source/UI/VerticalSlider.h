#pragma once

#include <JuceHeader.h>

class VerticalSlider : public juce::Component
{
public:
    // Constructeur : s'instancie avec un libellé (ex: VerticalSlider myFader {"OUTPUT"};)
    VerticalSlider(const juce::String& labelText)
    {
        // Configuration du slider en mode linéaire vertical
        slider.setSliderStyle(juce::Slider::LinearVertical);

        // TextBox positionnée en dessous du fader
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible(slider);

        // Configuration du label descriptif
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);
    }

    ~VerticalSlider() override = default;

    void resized() override
    {
        auto area = getLocalBounds();

        // Le label textuel prend une bande en haut (20 pixels)
        label.setBounds(area.removeFromTop(20));

        // Le reste de la zone est occupé par le fader vertical et sa textBox
        slider.setBounds(area);
    }

    void paint(juce::Graphics& g) override
    {
        juce::ignoreUnused(g);
    }

    // Helper pour lier facilement avec l'APVTS
    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    juce::Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalSlider)
};