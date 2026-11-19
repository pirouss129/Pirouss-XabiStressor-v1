/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// --- CONSTRUCTEUR / DESTRUCTEUR ---
//==============================================================================
PiroussXabiStressorv1AudioProcessorEditor::PiroussXabiStressorv1AudioProcessorEditor (PiroussXabiStressorv1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
	addAndMakeVisible(inputMeter);
	addAndMakeVisible(headerComponent);
	addAndMakeVisible(bypassButton);
	addAndMakeVisible(compBypassButton);
	addAndMakeVisible(listenSCButton);
	addAndMakeVisible(driveInputButton);
	addAndMakeVisible(driveOutputButton);
	addAndMakeVisible(inputSlider);
	addAndMakeVisible(attackSlider);
	addAndMakeVisible(releaseSlider);
	addAndMakeVisible(outputSlider);
	addAndMakeVisible(mixWetDrySlider);
	addAndMakeVisible(detectorInputSlider);
	addAndMakeVisible(detectorFilterSlider);
	addAndMakeVisible(ratioSlider);
	addAndMakeVisible(distOddSlider);
	addAndMakeVisible(distEvenSlider);
	addAndMakeVisible(thresholdSlider);
	addAndMakeVisible(peakRmsSlider);
	addAndMakeVisible(outputMeter);
	addAndMakeVisible(horizontalMeter);
	addAndMakeVisible(grMeter);

	// INTERCEPTION CLICS
	headerComponent.setInterceptsMouseClicks(false, false); // le composant laisse passer les clicks, les enfants contenus dans le composant laisse passer les clicks
	

	// liaison apvts
	bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.apvts, "BYPASS", bypassButton);

	compBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.apvts, "COMP_BYPASS", compBypassButton);

	listenScAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.apvts, "LISTEN_SC", listenSCButton);

	driveInputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.apvts, "DRIVE_INPUT", driveInputButton);

	driveOutputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.apvts, "DRIVE_OUTPUT", driveOutputButton);


	inputStageAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "INPUT_STAGE", inputSlider.getSlider());

	detectorInputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "DETECTOR_INPUT", detectorInputSlider.getSlider());

	detectorFilterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "DETECTOR_FILTER", detectorFilterSlider.getSlider());

	mixPeakRmsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "MIX_PEAK_RMS", peakRmsSlider.getSlider());

	

	attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "ATTACK", attackSlider.getSlider());

	releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "RELEASE", releaseSlider.getSlider());

	thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "THRESHOLD", thresholdSlider.getSlider());

	ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "RATIO", ratioSlider.getSlider());

	

	oddSaturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "DIST_ODD", distOddSlider.getSlider());

	evenSaturationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "DIST_EVEN", distEvenSlider.getSlider());

	outputStageAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "OUTPUT_STAGE", outputSlider.getSlider());

	wetDryMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts, "WET_DRY_MIX", mixWetDrySlider.getSlider());

    setSize (plugWidth, plugHeight);

	startTimerHz(30); // 30 Hz pour mettre à jour les vumètres
}

PiroussXabiStressorv1AudioProcessorEditor::~PiroussXabiStressorv1AudioProcessorEditor()
{
	stopTimer();
	
}
//==============================================================================
// --- TIMER CALLBACK ---
//==============================================================================
void PiroussXabiStressorv1AudioProcessorEditor::timerCallback()
{
	// Mettre à jour les valeurs des vumètres
	inputMeter.setLevels(audioProcessor.inputMeterDetector.getLeftPeakDb(),
		audioProcessor.inputMeterDetector.getRightPeakDb(),
		audioProcessor.inputMeterDetector.getLeftRmsDb(),
		audioProcessor.inputMeterDetector.getRightRmsDb());

	float scLevel = std::max(audioProcessor.mixPeakRmsDetector.getLeftLevel(),
		audioProcessor.mixPeakRmsDetector.getRightLevel());
	horizontalMeter.setLevel(scLevel);

	// Mettre à jour le bargraph de Gain Reduction (GR)
	float grValue = 0.0f;
	// Si le compresseur n'est PAS bypassé, on récupère le vrai GR, sinon on met à 0
	if (*audioProcessor.compBypassParam > 0.5f)
	{
		grValue = audioProcessor.compressorCore.getCurrentGrDb();
	}
	grMeter.setGainReduction(grValue);

	outputMeter.setLevels(audioProcessor.outputMeterDetector.getLeftPeakDb(),
		audioProcessor.outputMeterDetector.getRightPeakDb(),
		audioProcessor.outputMeterDetector.getLeftRmsDb(),
		audioProcessor.outputMeterDetector.getRightRmsDb());

}

//==============================================================================
// --- PAINT / RESIZED ---
//==============================================================================
void PiroussXabiStressorv1AudioProcessorEditor::paint(juce::Graphics& g)
{
	
	// Fond général : un dégradé vertical très subtil pour casser l'effet plat
	juce::ColourGradient backgroundGradient(
		juce::Colour::fromRGB(28, 30, 33),  // Gris très sombre légèrement bleuté en haut
		0.0f, 0.0f,
		juce::Colour::fromRGB(15, 16, 18),  // Noir mat profond en bas
		0.0f, static_cast<float>(getHeight()),
		false
	);

	g.setGradientFill(backgroundGradient);
	g.fillAll();

	// Optionnel : un petit filet de bordure ou de séparation haut/bas si tu veux structurer les racks
	g.setColour(juce::Colour::fromRGB(45, 48, 52));
	g.drawRect(getLocalBounds(), 1);
	
}

void PiroussXabiStressorv1AudioProcessorEditor::resized()
{
	auto area = getLocalBounds();

	inputMeter.setBounds(area.removeFromLeft(inputMeterWidth));
	auto buttonZone = area.removeFromLeft(buttonZoneWidth);
	int buttonHeight = buttonZone.getHeight() / 5;	// on divise la hauteur du buttonZone par 5 pour les cinq boutons
	bypassButton.setBounds(buttonZone.removeFromTop(buttonHeight));	// on réserve 1/5 de la hauteur de buttonZone pour le bouton Bypass
	compBypassButton.setBounds(buttonZone.removeFromTop(buttonHeight));	// on réserve 1/5 de la hauteur de buttonZone pour le bouton Comp
	listenSCButton.setBounds(buttonZone.removeFromTop(buttonHeight));	// on réserve 1/5 de la hauteur de buttonZone pour le bouton Listen SC
	driveInputButton.setBounds(buttonZone.removeFromTop(buttonHeight));	// on réserve 1/5 de la hauteur de buttonZone pour le bouton Drive In
	driveOutputButton.setBounds(buttonZone);	// le reste de buttonZone pour le bouton Drive Out
	

	outputMeter.setBounds(area.removeFromRight(outputMeterWidth));

	auto topArea = area.removeFromTop(plugHeight * 0.2); // 0,2 pour le header et le bloc du haut
	int topAreaWidth = topArea.getWidth();
	headerComponent.setBounds(topArea.removeFromLeft(topAreaWidth * 0.4)); // 40% pour le header

	auto topMeterArea = topArea.reduced(0.0f, 10.0f); // Le reste pour le bloc de mesure
	int topMeterAreaHeight = topMeterArea.getHeight();

	
	// 2. Maintenant on découpe dans des copies ou on utilise des sous-rectangles pour positionner les éléments
	auto workArea = topMeterArea;

	// --- Horizontal Meter ---
	auto horizontalMeterArea = workArea.removeFromTop(topMeterAreaHeight / 3);
	auto paddedMeterArea = horizontalMeterArea;
	paddedMeterArea.removeFromLeft(40);
	paddedMeterArea.removeFromRight(60);
	horizontalMeter.setBounds(paddedMeterArea);

	// --- Threshold Slider ---
	auto thresholdArea = workArea.removeFromTop(topMeterAreaHeight / 3);
	// Si tu veux aussi un peu de padding horizontal sur le slider, tu peux faire pareil :
	// auto paddedSliderArea = thresholdArea;
	// paddedSliderArea.removeFromLeft(40);
	// paddedSliderArea.removeFromRight(60);
	thresholdSlider.setBounds(thresholdArea);

	// --- GR Meter ---
	auto grMeterArea = workArea; // Le dernier tiers restant
	auto paddedGrMeterArea = grMeterArea;
	paddedGrMeterArea.removeFromLeft(40);
	paddedGrMeterArea.removeFromRight(60);
	grMeter.setBounds(paddedGrMeterArea);
	

	auto middleArea = area.removeFromTop(plugHeight * 0.3);	// 0,3 pour les 5 RotSliders
	int middleAreaWidth = middleArea.getWidth();	// on récupère la largeur du middleArea pour positionner les Sliders
	int sliderWidth = middleAreaWidth / 6;	// on divise la largeur du middleArea par 6 pour le bouton et les 5 Sliders
	
	detectorInputSlider.setBounds(middleArea.removeFromLeft(sliderWidth));	// on réserve 1/6 de la largeur du middleArea pour le Slider
	detectorFilterSlider.setBounds(middleArea.removeFromLeft(sliderWidth));	// on réserve 1/6 de la largeur du middleArea pour le Slider
	peakRmsSlider.setBounds(middleArea.removeFromLeft(sliderWidth));	// on réserve 1/6 de la largeur du middleArea pour le Slider
	distOddSlider.setBounds(middleArea.removeFromLeft(sliderWidth));	// on réserve 1/6 de la largeur du middleArea pour le Slider
	distEvenSlider.setBounds(middleArea.removeFromLeft(sliderWidth));	// on réserve 1/6 de la largeur du middleArea pour le Slider
	mixWetDrySlider.setBounds(middleArea);	// le reste du middleArea pour le Slider

	auto bottomArea = area;	// dernier 0,5 pour les 5 RotSliders
	int bottomAreaWidth = bottomArea.getWidth();	// on récupère la largeur du bottomArea pour positionner les Sliders
	int bottomSliderWidth = bottomAreaWidth / 5;	// on divise la largeur du bottomArea par 5 pour les 5 Sliders
	inputSlider.setBounds(bottomArea.removeFromLeft(bottomSliderWidth).reduced(10.0f));	// on réserve 1/5 de la largeur du bottomArea pour le Slider
	ratioSlider.setBounds(bottomArea.removeFromLeft(bottomSliderWidth).reduced(10.0f));	// on réserve 1/5 de la largeur du bottomArea pour le Slider
	attackSlider.setBounds(bottomArea.removeFromLeft(bottomSliderWidth).reduced(10.0f));	// on réserve 1/5 de la largeur du bottomArea pour le Slider
	releaseSlider.setBounds(bottomArea.removeFromLeft(bottomSliderWidth).reduced(10.0f));	// on réserve 1/5 de la largeur du bottomArea pour le Slider
	outputSlider.setBounds(bottomArea.removeFromLeft(bottomSliderWidth).reduced(10.0f));	// on réserve 1/5 de la largeur du bottomArea pour le Slider
	
}

//==============================================================================
// --- MENU CLIC DROIT ---
//==============================================================================

void PiroussXabiStressorv1AudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
	if (event.mods.isPopupMenu()) // si clic droit
	{
		juce::PopupMenu menu; // Préparation du Menu

		menu.addItem(1, "Made with JUCE", true);
		menu.addSeparator();
		menu.addItem(2, "Mode Opto : Ratio Opto, SC Feedback, SC full RMS, Atk/Rel Max", true);

		menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) // Menu
			{
				if (result == 1)
				{
					juce::URL("https://juce.com/").launchInDefaultBrowser();
				}
				else if (result == 2)
				{
					audioProcessor.setOptoMode(); // L'éditeur demande au Processor de faire le boulot
				}
			});
	}
}