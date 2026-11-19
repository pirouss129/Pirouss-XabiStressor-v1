/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/PeakRmsDetector.h"
#include "DSP/InputStage.h"
#include "DSP/SidechainFilter.h"
#include "DSP/HarmonicSaturation.h"
#include "DSP/MixPeakRmsDetector.h"
#include "DSP/CompressorCore.h"
#include "DSP/OutputStage.h"
#include "DSP/MixWetDry.h"

//==============================================================================
/**
*/
class PiroussXabiStressorv1AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PiroussXabiStressorv1AudioProcessor();
    ~PiroussXabiStressorv1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	void setOptoMode(); // Méthode pour activer le mode Opto

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    PeakRmsDetector inputMeterDetector;

	InputStage inputStage;

	SideChainFilter sideChainFilter;

	HarmonicSaturation harmonicSaturation;

	MixPeakRmsDetector mixPeakRmsDetector;

	CompressorCore compressorCore;

	OutputStage outputStage;

	WetDryMix wetDryMix;

	PeakRmsDetector outputMeterDetector;

    std::atomic<float>* compBypassParam = nullptr;

private:
    //==============================================================================
    // --- Cache de pointeurs atomiques pour les paramètres ---
    std::atomic<float>* inputStageParam = nullptr;
	std::atomic<float>* detectorInputParam = nullptr;
    std::atomic<float>* detectorFilterParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* listenScParam = nullptr;
    std::atomic<float>* driveInputParam = nullptr;
    std::atomic<float>* driveOutputParam = nullptr;
    std::atomic<float>* mixPeakRmsParam = nullptr;
	std::atomic<float>* attackParam = nullptr;
	std::atomic<float>* releaseParam = nullptr;
	std::atomic<float>* thresholdParam = nullptr;
	std::atomic<float>* ratioParam = nullptr;
	
    std::atomic<float>* oddSaturationParam = nullptr;
    std::atomic<float>* evenSaturationParam = nullptr;
    std::atomic<float>* outputStageParam = nullptr;
    std::atomic<float>* wetDryMixParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussXabiStressorv1AudioProcessor)
};
