/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/header.h"
#include "UI/Button.h"
#include "UI/RotSlider.h"
#include "UI/BigRotSlider.h"
#include "UI/VerticalSlider.h"
#include "UI/HorizontalSlider.h"
#include "UI/Meter.h"
#include "UI/GRMeter.h"
#include "UI/HorizontalMeter.h"



//==============================================================================
/**
*/
class PiroussXabiStressorv1AudioProcessorEditor : public juce::AudioProcessorEditor, 
	public juce::Timer
{
public:
    PiroussXabiStressorv1AudioProcessorEditor (PiroussXabiStressorv1AudioProcessor&);
    ~PiroussXabiStressorv1AudioProcessorEditor() override;

    //==============================================================================
	void timerCallback() override;
    void paint (juce::Graphics&) override;
    void resized() override;
	void mouseDown(const juce::MouseEvent& event);

	//void parameterChanged(const juce::String& parameterID, float newValue) override; // <-- La méthode appelée en cas de changement
	

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PiroussXabiStressorv1AudioProcessor& audioProcessor;

	header headerComponent{ "Pirouss XabiStressor v1.1" };
	Button bypassButton{ "Bypass", juce::Colours::blueviolet };
	Button compBypassButton{ "COMP" };
	Button listenSCButton{ "Listen SC", juce::Colours::red };
	Button driveInputButton{ "Drive In" };
	Button driveOutputButton{ "Drive Out" };
	BigRotSlider inputSlider{ "Input" };
	BigRotSlider attackSlider{ "Attack" };
	BigRotSlider releaseSlider{ "Release" };
	BigRotSlider outputSlider{ "Output" };
	RotSlider mixWetDrySlider{ "Mix Wet Dry" };
	RotSlider detectorInputSlider{ "Detect Input" };
	RotSlider detectorFilterSlider{ "Detect Filter" };
	BigRotSlider ratioSlider{ "Ratio" };
	RotSlider distOddSlider{ "Out Dist Odd" };
	RotSlider distEvenSlider{ "Out Dist Even" };
	HorizontalSlider thresholdSlider{ "Thr" };
	RotSlider peakRmsSlider{ "Detect Peak/RMS" };
	Meter inputMeter;
	Meter outputMeter;
	HorizontalMeter horizontalMeter;
	GRMeter grMeter;
	

	// pointeurs atomiques pour les paramètres
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compBypassAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> listenScAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> driveInputAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> driveOutputAttachment;

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputStageAttachment;

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detectorInputAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detectorFilterAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakRmsAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixPeakRmsAttachment;

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;

	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> oddSaturationAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> evenSaturationAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputStageAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetDryMixAttachment;

	const int inputMeterWidth = 100;
	const int buttonZoneWidth = 120;
	const int rotSliderWidth = 120; // et il y en a 5 alignés horizontalement
	const int outputMeterWidth = 100;

	const int plugWidth = inputMeterWidth + buttonZoneWidth + (rotSliderWidth * 5) + outputMeterWidth;
	const int plugHeight = 320;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussXabiStressorv1AudioProcessorEditor)
};
