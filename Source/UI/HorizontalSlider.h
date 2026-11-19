#pragma once
#include <JuceHeader.h>

class HorizontalSlider : public juce::Component
{
public:
    HorizontalSlider(const juce::String& labelText, int labelWidth = 30, int textBoxWidth = 50, int textBoxHeight = 18)
        : lWidth(labelWidth), tbWidth(textBoxWidth), tbHeight(textBoxHeight)
    {
        // Application du LookAndFeel pour le curseur triangulaire
        slider.setLookAndFeel(&sliderLookAndFeel);

        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, tbWidth, tbHeight);

        // Style de la boîte de texte vintage assorti au rack
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(235, 233, 225));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(18, 20, 23));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB(45, 48, 52));
        addAndMakeVisible(slider);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setFont(juce::Font("Courier New", 13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(200, 198, 190));
        addAndMakeVisible(label);
    }

    ~HorizontalSlider() override
    {
        slider.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromLeft(lWidth));
        slider.setBounds(bounds);
    }

    void paint(juce::Graphics& g) override
    {
        /*
        // --- COULEUR DE DEBUG ---
        // Affiche un fond vert translucide sur tout le composant Slider pour vérifier l'alignement
        g.setColour(juce::Colours::limegreen.withAlpha(0.25f));
        g.fillRect(getLocalBounds().toFloat());

        // Affiche un cadre rouge vif autour de la zone du slider (sans le label) pour voir ses limites exactes
        g.setColour(juce::Colours::red.withAlpha(0.8f));
        g.drawRect(slider.getBounds().toFloat(), 1.0f);
        */
    }

    juce::Slider& getSlider() { return slider; }

private:
    // --- LOOK AND FEEL SLIDER LINÉAIRE (Rail creusé & Curseur triangle) ---
    class SliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPos, float minSliderPos, float maxSliderPos,
            juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            juce::ignoreUnused(minSliderPos, maxSliderPos, style);

            auto trackWidth = 6.0f;
            auto trackBounds = juce::Rectangle<float>(
                (float)x,
                (float)y + ((float)height - trackWidth) * 0.5f,
                (float)width,
                trackWidth
            );

            // 1. Ombre portée / renfoncement du rail
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillRoundedRectangle(trackBounds.translated(0.0f, 1.0f), 2.0f);

            // 2. Fond du rail creusé
            g.setColour(juce::Colour::fromRGB(15, 17, 20));
            g.fillRoundedRectangle(trackBounds, 2.0f);

            // Contour du rail
            g.setColour(juce::Colour::fromRGB(45, 48, 52));
            g.drawRoundedRectangle(trackBounds, 2.0f, 1.0f);

            // 3. Dessin du curseur : Triangle blanc cassé pointant vers le haut
            auto thumbWidth = 14.0f;
            auto thumbHeight = 10.0f;
            auto thumbY = (float)y + ((float)height - thumbHeight) * 0.5f;

            juce::Path thumbPath;
            thumbPath.startNewSubPath(sliderPos, thumbY);
            thumbPath.lineTo(sliderPos + thumbWidth * 0.5f, thumbY + thumbHeight);
            thumbPath.lineTo(sliderPos - thumbWidth * 0.5f, thumbY + thumbHeight);
            thumbPath.closeSubPath();

            // Ombre du curseur
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.fillPath(thumbPath, juce::AffineTransform::translation(0.0f, 1.5f));

            // Corps du curseur (Blanc cassé rétro)
            g.setColour(juce::Colour::fromRGB(215, 210, 198));
            g.fillPath(thumbPath);

            // Contour du curseur
            g.setColour(juce::Colour::fromRGB(110, 105, 95));
            g.strokePath(thumbPath, juce::PathStrokeType(1.0f));
        }

        juce::Font getLabelFont(juce::Label& label) override
        {
            return juce::Font("Courier New", 12.0f, juce::Font::bold);
        }
    };

    juce::Slider slider;
    juce::Label label;
    SliderLookAndFeel sliderLookAndFeel;
    int lWidth;
    int tbWidth;
    int tbHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalSlider)
};