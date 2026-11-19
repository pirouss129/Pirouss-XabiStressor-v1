#pragma once

#include <JuceHeader.h>

class BigRotSlider : public juce::Component
{
public:
    BigRotSlider(const juce::String& labelText)
    {
        // 1. Configuration du Label (au-dessus)
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font("Courier New", 13.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(200, 198, 190));
        addAndMakeVisible(label);

        // Application du LookAndFeel pour le grand potard crème patiné
        slider.setLookAndFeel(&bigKnobLookAndFeel);

        // 2. Configuration du Slider (Rotatif)
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 18);

        // Style de la boîte de texte vintage
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(235, 233, 225));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(18, 20, 23));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGB(45, 48, 52));

        addAndMakeVisible(slider);
    }

    ~BigRotSlider() override
    {
        slider.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(20));
        slider.setBounds(bounds);
    }

    void paint(juce::Graphics& g) override
    {
        juce::ignoreUnused(g);
    }

    juce::Slider& getSlider() { return slider; }

private:
    // --- LOOK AND FEEL GRAND POTARD CRÈME / PATINÉ ---
    class BigKnobLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPosProportional, float rotaryStartAngle,
            float rotaryEndAngle, juce::Slider& slider) override
        {
            auto localBounds = juce::Rectangle<int>(x, y, width, height).toFloat();
            auto size = juce::jmin(localBounds.getWidth(), localBounds.getHeight());
            auto bounds = juce::Rectangle<float>(size, size).withCentre(localBounds.getCentre()).reduced(4.0f);

            auto radius = bounds.getWidth() * 0.5f;
            auto center = bounds.getCentre();

            // Teintes blanc cassé / crème légèrement patiné (aspect plastique vieilli)
            auto knobBgTop = juce::Colour::fromRGB(215, 210, 198);
            auto knobBgBottom = juce::Colour::fromRGB(180, 175, 163);
            auto knobOutline = juce::Colour::fromRGB(110, 105, 95);
            auto darkMark = juce::Colour::fromRGB(35, 33, 30); // Noir pour les repères

            // 1. Ombre portée douce sous le grand potard
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillEllipse(bounds.translated(0.0f, 3.0f));

            // 2. Corps principal avec dégradé crème patiné
            juce::ColourGradient knobGradient(
                knobBgTop, center.x, center.y - radius,
                knobBgBottom, center.x, center.y + radius,
                true
            );
            g.setGradientFill(knobGradient);
            g.fillEllipse(bounds);

            // Contour du potard
            g.setColour(knobOutline);
            g.drawEllipse(bounds, 1.2f);

            // 3. Chapeau central (insert) avec ombre portée sous le cercle du milieu
            auto capRadius = radius * 0.68f;
            auto innerBounds = juce::Rectangle<float>(capRadius * 2.0f, capRadius * 2.0f).withCentre(center);

            // Ombre portée sous le chapeau central
            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.fillEllipse(innerBounds.translated(0.0f, 2.0f));

            juce::ColourGradient innerGradient(
                juce::Colour::fromRGB(165, 160, 150), center.x, center.y - capRadius,
                juce::Colour::fromRGB(195, 190, 180), center.x, center.y + capRadius,
                true
            );
            g.setGradientFill(innerGradient);
            g.fillEllipse(innerBounds);

            g.setColour(knobOutline.withAlpha(0.6f));
            g.drawEllipse(innerBounds, 1.0f);

            // 4. Calcul de l'angle pour le point indicateur noir
            auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // 5. Dessin du point indicateur plus gros en noir
            auto pointDistance = capRadius * 0.70f;
            auto pointX = center.x + std::sin(angle) * pointDistance;
            auto pointY = center.y - std::cos(angle) * pointDistance;

            auto pointBounds = juce::Rectangle<float>(8.0f, 8.0f).withCentre({ pointX, pointY });

            g.setColour(darkMark);
            g.fillEllipse(pointBounds);
        }

        juce::Font getLabelFont(juce::Label& label) override
        {
            return juce::Font("Courier New", 12.0f, juce::Font::bold);
        }
    };

    juce::Label label;
    juce::Slider slider;
    BigKnobLookAndFeel bigKnobLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BigRotSlider)
};