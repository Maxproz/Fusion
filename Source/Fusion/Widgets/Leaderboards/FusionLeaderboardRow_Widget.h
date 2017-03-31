// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "FusionLeaderboardRow_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionLeaderboardRow_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:


	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankText;
	
};
