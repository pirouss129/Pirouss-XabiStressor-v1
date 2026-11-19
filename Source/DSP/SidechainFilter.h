#pragma once

#include <JuceHeader.h>

class SideChainFilter
{
public:
    SideChainFilter() = default;
    ~SideChainFilter() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        // Initialisation obligatoire des processeurs pour le multi-canaux (Stéréo)
        hpFilter.prepare(spec);
        boostFilter.prepare(spec);

        hpFilter.reset();
        boostFilter.reset();

        updateFilters(currentMode);
    }

    void setMode(int modeIndex)
    {
        if (currentMode != modeIndex)
        {
            currentMode = modeIndex;
            updateFilters(currentMode);
        }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (currentMode == 0)
            return; // Mode Flat : on ne touche à rien

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        hpFilter.process(context);
        boostFilter.process(context);
    }

private:
    double sampleRate = 44100.0;
    int currentMode = 0;

    // Utilisation de ProcessorDuplicator pour gérer proprement la stéréo (L/R indépendants)
    using FilterType = juce::dsp::IIR::Filter<float>;
    juce::dsp::ProcessorDuplicator<FilterType, juce::dsp::IIR::Coefficients<float>> hpFilter;
    juce::dsp::ProcessorDuplicator<FilterType, juce::dsp::IIR::Coefficients<float>> boostFilter;

    void updateFilters(int mode)
    {
        auto coefficientsHP = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 120.0f); // HP à 120 Hz

        if (mode == 1) // HP simple
        {
            *hpFilter.state = *coefficientsHP;
            *boostFilter.state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, 1000.0f); // Neutre
        }
        else if (mode == 2) // HP & Boost1 (ex: accentuation autour de 2-3 kHz)
        {
            *hpFilter.state = *coefficientsHP;
            *boostFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 2500.0f, 1.0f, juce::Decibels::decibelsToGain(3.0f));
        }
        else if (mode == 3) // HP & Boost2 (boost plus agressif / haut-médium)
        {
            *hpFilter.state = *coefficientsHP;
            *boostFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 4000.0f, 1.5f, juce::Decibels::decibelsToGain(6.0f));
        }
        else // Flat
        {
            *hpFilter.state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, 1000.0f);
            *boostFilter.state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, 1000.0f);
        }
    }
};