#pragma once

#include <JuceHeader.h>

class Button : public juce::TextButton
{
public:
    // Constructeur : un TextButton pur, directement cliquable et toggleable
    Button(const juce::String& buttonText, juce::Colour activeLedColour = juce::Colours::gold)
        : juce::TextButton(buttonText), ledColour(activeLedColour)
    {
        setClickingTogglesState(true);
        setWantsKeyboardFocus(false);
    }

    ~Button() override = default;

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // On réduit les bounds pour laisser de la marge à l'ombrage périphérique
        auto bounds = getLocalBounds().toFloat().reduced(10.0f);
        bool isOn = getToggleState();

        // 1. Ombrage doux autour du bloc
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds.translated(0.0f, 1.5f), 4.0f);

        // 2. Fond du bouton (style rack analogique mat)
        auto bgColour = isOn ? juce::Colour::fromRGB(42, 45, 49)
            : juce::Colour::fromRGB(30, 32, 35);

        if (shouldDrawButtonAsDown)
            bgColour = bgColour.darker(0.15f);
        else if (shouldDrawButtonAsHighlighted)
            bgColour = bgColour.brighter(0.08f);

        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 3.0f);

        // Bordure sombre
        g.setColour(juce::Colour::fromRGB(15, 16, 18));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // 3. Dessin de la LED intégrée (pastille ronde à gauche)
        auto ledBounds = juce::Rectangle<float>(bounds.getX() + 10.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);

        juce::Colour currentLedColour = isOn ? ledColour
            : juce::Colour::fromRGB(75, 80, 85); // Gris éteint

        // Halo lumineux si allumé
        if (isOn)
        {
            g.setColour(currentLedColour.withAlpha(0.35f));
            g.fillEllipse(ledBounds.expanded(3.5f));
        }

        // Corps de la LED
        g.setColour(currentLedColour);
        g.fillEllipse(ledBounds);

        // Reflet spéculaire de la LED
        g.setColour(juce::Colours::white.withAlpha(isOn ? 0.7f : 0.25f));
        g.fillEllipse(ledBounds.reduced(2.5f));

        // 4. Texte du bouton avec une police typée hardware vintage (Courier New en gras)
        auto textBounds = bounds.translated(12.0f, 0.0f);
        g.setColour(isOn ? juce::Colour::fromRGB(235, 233, 225) : juce::Colour::fromRGB(200, 198, 190));
        g.setFont(juce::Font("Courier New", 13.0f, juce::Font::bold));
        g.drawText(getButtonText(), textBounds, juce::Justification::centred, true);
    }

private:
    juce::Colour ledColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Button)
};