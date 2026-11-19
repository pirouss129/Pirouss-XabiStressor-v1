#pragma once
#include <JuceHeader.h>

class HorizontalMeter : public juce::Component
{
public:
    HorizontalMeter() = default;
    ~HorizontalMeter() override = default;

    void setLevel(float newLevel)
    {
        currentLevel = currentLevel + 0.3f * (newLevel - currentLevel);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto totalBounds = getLocalBounds().toFloat();
        auto bounds = totalBounds.reduced(2.0f);

        // 1. Ombrage doux autour du bloc
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds.translated(0.0f, 1.5f), 4.0f);

        // 2. Fond du meter (style rack analogique mat)
        g.setColour(juce::Colour::fromRGB(30, 32, 35));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Bordure sombre
        g.setColour(juce::Colour::fromRGB(15, 16, 18));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        auto innerBounds = bounds.reduced(3.0f);

        float currentDb = juce::Decibels::gainToDecibels(currentLevel, minDb);
        float normalizedVal = juce::jmap(currentDb, minDb, maxDb, 0.0f, 1.0f);
        float clampedValue = juce::jlimit(0.0f, 1.0f, normalizedVal);

        if (clampedValue > 0.001f)
        {
            auto fillRect = innerBounds.withWidth(innerBounds.getWidth() * clampedValue);

            juce::ColourGradient gradient(
                colorGreen, innerBounds.getX(), innerBounds.getY(),
                colorRed, innerBounds.getRight(), innerBounds.getY(),
                false
            );

            float posYellow = juce::jmap(thresholdYellow, minDb, maxDb, 0.0f, 1.0f);
            float posOrange = juce::jmap(thresholdOrange, minDb, maxDb, 0.0f, 1.0f);
            float posRed = juce::jmap(thresholdRed, minDb, maxDb, 0.0f, 1.0f);

            gradient.addColour(posYellow, colorGreen);
            gradient.addColour(posOrange, colorYellow);
            gradient.addColour(posRed, colorOrange);

            g.setGradientFill(gradient);
            g.fillRoundedRectangle(fillRect, 2.0f);

            // Grille style VFD / Segments horizontaux
            g.setColour(juce::Colour::fromRGB(30, 32, 35).withAlpha(0.7f));
            float segmentWidth = 3.0f;
            float segmentGap = 1.5f;
            for (float currentX = innerBounds.getX(); currentX < fillRect.getRight(); currentX += (segmentWidth + segmentGap))
            {
                g.fillRect(currentX, innerBounds.getY(), segmentGap, innerBounds.getHeight());
            }

            // Texture de verre
            juce::ColourGradient glassReflect(
                juce::Colours::white.withAlpha(0.08f), innerBounds.getX(), innerBounds.getY(),
                juce::Colours::transparentWhite, innerBounds.getX(), innerBounds.getBottom(),
                false
            );
            g.setGradientFill(glassReflect);
            g.fillRoundedRectangle(fillRect, 2.0f);
        }
    }

private:
    float currentLevel = 0.0f;

    static constexpr float minDb = -40.0f;
    static constexpr float maxDb = 0.0f;

    const float thresholdYellow = -18.0f;
    const float thresholdOrange = -9.0f;
    const float thresholdRed = -6.0f;

    const juce::Colour colorGreen = juce::Colours::limegreen;
    const juce::Colour colorYellow = juce::Colours::gold;
    const juce::Colour colorOrange = juce::Colours::darkorange;
    const juce::Colour colorRed = juce::Colours::crimson;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HorizontalMeter)
};