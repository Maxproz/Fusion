// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "FusionLeaderboard_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UFusionLeaderboard_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:

	virtual void NativeConstruct() override;


protected:

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* LeaderboardScrollList;



	
};
