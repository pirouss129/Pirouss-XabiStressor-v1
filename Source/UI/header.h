#pragma once
#include <JuceHeader.h>

class header : public juce::Component
{
public:
    header(const juce::String& pluginName)
        : name(pluginName)
    {
        // Chargement direct et sécurisé via le BinaryData de Projucer
        logoImage = juce::ImageCache::getFromMemory(BinaryData::juce_logo_png, BinaryData::juce_logo_pngSize);

        // Configuration du label de titre (Police vintage, souligné, encre noire)
        auto font = juce::Font("Courier New", 18.0f, juce::Font::bold);
        font.setUnderline(true);

        titleLabel.setText(name, juce::dontSendNotification);
        titleLabel.setFont(font);
        titleLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(25, 27, 30)); // Noir encre vieillie
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(titleLabel);
    }

    ~header() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Zone plaque encadrée avec ombrage et angles arrondis pour le bloc logo + titre
        auto plateBounds = bounds.reduced(8.0f, 10.0f);

        // 1. Ombrage sous la plaque (légèrement plus diffus pour l'effet relief clair)
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillRoundedRectangle(plateBounds.translated(0.0f, 2.5f), 6.0f);

        // 2. Fond de la plaque : Blanc cassé patiné / plastique vieilli (style gros potards)
        juce::ColourGradient plateGradient(
            juce::Colour::fromRGB(220, 216, 204), plateBounds.getX(), plateBounds.getY(),
            juce::Colour::fromRGB(195, 190, 178), plateBounds.getRight(), plateBounds.getBottom(),
            false
        );
        g.setGradientFill(plateGradient);
        g.fillRoundedRectangle(plateBounds, 6.0f);

        // 3. Effet d'usure / grain subtil ou biseauté sur les bords (contour foncé et patiné)
        g.setColour(juce::Colour::fromRGB(130, 125, 115));
        g.drawRoundedRectangle(plateBounds, 6.0f, 1.0f);

        // Contour interne plus sombre pour incruster la plaque
        g.setColour(juce::Colour::fromRGB(160, 155, 145));
        g.drawRoundedRectangle(plateBounds.reduced(1.0f), 5.0f, 1.0f);

        // 4. Dessin du logo JUCE si chargé correctement
        if (logoImage.isValid())
        {
            auto logoArea = juce::Rectangle<float>(plateBounds.getX() + 12.0f, plateBounds.getCentreY() - 12.0f, 24.0f, 24.0f);
            g.drawImage(logoImage, logoArea);
        }

        // Ligne de séparation tout en bas de l'en-tête
        //g.setColour(juce::Colour::fromRGB(10, 11, 13));
        //g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(8, 6);

        int leftMargin = logoImage.isValid() ? 44 : 12;
        area.removeFromLeft(leftMargin);

        titleLabel.setBounds(area.removeFromLeft(350));
    }

private:
    juce::String name;
    juce::Label titleLabel;
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(header)
};