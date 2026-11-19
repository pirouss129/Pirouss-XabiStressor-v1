#pragma once

#include <JuceHeader.h>

class Meter : public juce::Component
{
public:
    Meter() = default;
    ~Meter() override = default;

    // Met à jour les valeurs (en dBFS)
    void setLevels(float leftPeak, float rightPeak, float leftRms, float rightRms)
    {
        lPeak = leftPeak;
        rPeak = rightPeak;
        lRms = leftRms;
        rRms = rightRms;

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // On réduit les bounds exactement comme sur le bouton pour laisser de la marge à l'ombrage périphérique
        auto bounds = getLocalBounds().toFloat().reduced(5.0f, 10.0f);

        // 1. Ombrage doux autour du bloc (identique au bouton)
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds.translated(0.0f, 1.5f), 4.0f);

        // 2. Fond du meter (style rack analogique mat identique aux boutons)
        g.setColour(juce::Colour::fromRGB(30, 32, 35));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Bordure sombre (identique au bouton)
        g.setColour(juce::Colour::fromRGB(15, 16, 18));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // Zone interne utile (légèrement réduite pour l'agencement interne)
        auto innerBounds = bounds.reduced(4.0f);

        // On élargit la zone de droite pour que les textes respirent
        float scaleWidth = 25.0f;
        auto meterArea = innerBounds.removeFromLeft(innerBounds.getWidth() - scaleWidth);
        auto scaleArea = innerBounds;

        // 3. Dessin des 4 colonnes (RMS L, Peak L, Peak R, RMS R)
        int numCols = 4;
        float totalWidth = meterArea.getWidth();
        float padding = 2.0f;
        float colWidth = (totalWidth - (padding * (numCols + 1))) / numCols;

        auto startX = meterArea.getX() + padding;
        auto startY = meterArea.getY() + padding;
        auto colHeight = meterArea.getHeight() - (padding * 2.0f);

        drawSingleBar(g, startX + 0 * (colWidth + padding), startY, colWidth, colHeight, lRms);
        drawSingleBar(g, startX + 1 * (colWidth + padding), startY, colWidth, colHeight, lPeak);
        drawSingleBar(g, startX + 2 * (colWidth + padding), startY, colWidth, colHeight, rPeak);
        drawSingleBar(g, startX + 3 * (colWidth + padding), startY, colWidth, colHeight, rRms);

        // 4. Dessin de l'échelle graduée sérigraphiée indépendante
        drawScale(g, scaleArea);
    }

    void resized() override
    {
    }

private:
    // --- PARAMÈTRES AUDIO ---
    static constexpr float dBMin = -33.0f;
    static constexpr float dBMax = 1.0f;

    // Seuils en dB pour les changements de couleur du dégradé
    static constexpr float thresholdOrange = -18.0f;
    static constexpr float thresholdDarkOrange = -9.0f;
    static constexpr float thresholdRed = -6.0f;
    // ------------------------

    // --- GRADUATIONS PERSONNALISÉES (Valeur dB, Texte affiché) ---
    struct ScaleMark {
        float db;
        const char* label;
    };

    const ScaleMark scaleMarks[5] = {
        { 0.0f, "0" },
        { -6.0f, "-6" },
        { -9.0f, "-9" },
        { -18.0f, "-18" },
        { -32.0f, "-32" }
    };
    // -------------------------------------------------------------

    float lPeak = dBMin, rPeak = dBMin;
    float lRms = dBMin, rRms = dBMin;

    // Helper pour dessiner une barre verticale avec dégradé PPM, grille VFD et reflet verre
    void drawSingleBar(juce::Graphics& g, float x, float y, float w, float h, float dbLevel)
    {
        juce::Rectangle<float> rect(x, y, w, h);

        // Calcul des positions normalisées des seuils pour le dégradé
        auto norm = [](float db) {
            return juce::jlimit(0.0f, 1.0f, juce::jmap(db, dBMin, dBMax, 0.0f, 1.0f));
            };

        // Dégradé vertical (vert en bas, rouge en haut)
        juce::ColourGradient gradient(
            juce::Colours::limegreen, rect.getX(), rect.getBottom(),
            juce::Colours::crimson, rect.getX(), rect.getY(),
            false
        );

        gradient.addColour(norm(thresholdOrange), juce::Colours::gold);
        gradient.addColour(norm(thresholdDarkOrange), juce::Colours::darkorange);
        gradient.addColour(norm(thresholdRed), juce::Colours::orangered);

        g.setGradientFill(gradient);
        g.fillRect(rect);

        // Grille style VFD / Segments PPM (aux couleurs du fond du bloc)
        g.setColour(juce::Colour::fromRGB(30, 32, 35).withAlpha(0.8f));
        float segmentHeight = 3.0f;
        float segmentGap = 1.5f;
        for (float currentY = rect.getY(); currentY < rect.getBottom(); currentY += (segmentHeight + segmentGap))
        {
            g.fillRect(rect.getX(), currentY, rect.getWidth(), segmentGap);
        }

        // Masque de niveau (partie éteinte de la barre)
        float normalized = juce::jmap(dbLevel, dBMin, dBMax, 0.0f, 1.0f);
        normalized = juce::jlimit(0.0f, 1.0f, normalized);
        float levelY = rect.getBottom() - (normalized * rect.getHeight());

        g.setColour(juce::Colour::fromRGB(30, 32, 35));
        g.fillRect(rect.getX(), rect.getY(), rect.getWidth(), levelY - rect.getY());

        // Texture de verre : léger reflet spéculaire diagonal
        juce::ColourGradient glassReflect(
            juce::Colours::white.withAlpha(0.08f), rect.getX(), rect.getY(),
            juce::Colours::transparentWhite, rect.getRight(), rect.getBottom() * 0.5f,
            false
        );
        g.setGradientFill(glassReflect);
        g.fillRect(rect);
    }

    // Helper pour tracer les graduations et les chiffres (Courier New) de manière fixe
    void drawScale(juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour(juce::Colour(200, 198, 190)); // Blanc cassé rétro
        g.setFont(juce::Font("Courier New", 9.0f, juce::Font::bold));

        for (const auto& mark : scaleMarks)
        {
            // Position Y calculée par rapport à l'étendue globale du meter (dBMin à dBMax)
            float normalized = juce::jmap(mark.db, dBMin, dBMax, 0.0f, 1.0f);

            // On s'assure que la graduation reste dans les limites visuelles du meter
            if (normalized < 0.0f || normalized > 1.0f) continue;

            float y = area.getBottom() - (normalized * area.getHeight());

            // Petit trait de repère (tick) sérigraphié
            g.fillRect(area.getX(), y, 4.0f, 1.0f);

            // Texte de la graduation (entièrement piloté par le tableau scaleMarks)
            g.drawText(mark.label,
                static_cast<int>(area.getX() + 6),
                static_cast<int>(y - 5),
                static_cast<int>(area.getWidth() - 6),
                10,
                juce::Justification::centredLeft,
                false);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Meter)
};