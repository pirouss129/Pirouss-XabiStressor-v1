#pragma once
#include <JuceHeader.h>

class GRMeter : public juce::Component
{
public:
    GRMeter() = default;
    ~GRMeter() override = default;

    void setGainReduction(float reductionDb)
    {
        currentReductionDb = juce::jlimit(dbMin, dbMax, reductionDb);
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

        auto meterRect = bounds.reduced(3.0f);

        // Si on a une vraie réduction de gain (supérieure à un tout petit seuil)
        if (currentReductionDb > 0.05f)
        {
            auto dbToX = [meterRect](float db)
                {
                    float normalized = juce::jmap(db, dbMin, dbMax, 0.0f, 1.0f);
                    return meterRect.getRight() - (normalized * meterRect.getWidth());
                };

            float currentX = dbToX(currentReductionDb);

            // La zone active va de la position actuelle du GR jusqu'à l'extrême droite (0 dB)
            float fillWidth = meterRect.getRight() - currentX;
            auto fillRect = juce::Rectangle<float>(currentX, meterRect.getY(), fillWidth, meterRect.getHeight());

            // 3. Dégradé de couleur global (Vert à droite -> Rouge à gauche)
            juce::ColourGradient gradient(
                juce::Colours::limegreen, meterRect.getRight(), meterRect.getY(),
                juce::Colours::crimson, meterRect.getX(), meterRect.getY(),
                false
            );

            float posYellow = juce::jmap(6.0f, dbMin, dbMax, 0.0f, 1.0f);
            float posOrange = juce::jmap(18.0f, dbMin, dbMax, 0.0f, 1.0f);
            float posRed = juce::jmap(30.0f, dbMin, dbMax, 0.0f, 1.0f);

            gradient.addColour(posYellow, juce::Colours::gold);
            gradient.addColour(posOrange, juce::Colours::orange);
            gradient.addColour(posRed, juce::Colours::crimson);

            g.setGradientFill(gradient);
            g.fillRoundedRectangle(fillRect, 2.0f);

            // 4. Grille style VFD / Segments horizontaux sur la zone active
            g.setColour(juce::Colour::fromRGB(30, 32, 35).withAlpha(0.75f));
            float segmentWidth = 3.0f;
            float segmentGap = 1.5f;
            for (float x = meterRect.getRight() - segmentWidth; x > currentX; x -= (segmentWidth + segmentGap))
            {
                g.fillRect(x - segmentGap, meterRect.getY(), segmentGap, meterRect.getHeight());
            }

            // 5. Texture de verre : reflet spéculaire sur la partie active
            juce::ColourGradient glassReflect(
                juce::Colours::white.withAlpha(0.08f), fillRect.getX(), fillRect.getY(),
                juce::Colours::transparentWhite, fillRect.getX(), fillRect.getBottom(),
                false
            );
            g.setGradientFill(glassReflect);
            g.fillRoundedRectangle(fillRect, 2.0f);
        }
    }

private:
    float currentReductionDb = 0.0f;
    static constexpr float dbMin = 0.0f;
    static constexpr float dbMax = 40.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GRMeter)
};