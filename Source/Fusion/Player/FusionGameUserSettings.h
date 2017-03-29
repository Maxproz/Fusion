// @Maxpro 2017

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "FusionGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

	/** Applies all current user settings to the game and saves to permanent storage (e.g. file), optionally checking for command line overrides. */
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	int32 GetGraphicsQuality() const
	{
		return GraphicsQuality;
	}

	void SetGraphicsQuality(int32 InGraphicsQuality)
	{
		GraphicsQuality = InGraphicsQuality;
	}

	bool IsLanMatch() const
	{
		return bIsLanMatch;
	}

	void SetLanMatch(bool InbIsLanMatch)
	{
		bIsLanMatch = InbIsLanMatch;
	}

	// interface UGameUserSettings
	virtual void SetToDefaults() override;

private:
	/**
	* Graphics Quality
	*	0 = Low
	*	1 = High
	*/
	UPROPERTY(config)
	int32 GraphicsQuality;

	/** is lan match? */
	UPROPERTY(config)
	bool bIsLanMatch;

};