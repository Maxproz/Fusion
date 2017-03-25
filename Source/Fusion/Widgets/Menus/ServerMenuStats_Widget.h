// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"

#include "FusionGameInstance.h"

#include "ServerMenuStats_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UServerMenuStats_Widget : public UMasterWidget
{
	GENERATED_BODY()
	
	
public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedServerBrowserITemButton();

	void IsLan(const bool IsLanBook, FText& OutIsLanText);

	void SessionNameToText(const FString SessionString, FText& SessionText);


	void SetCustomResult(const FCustomFusionSessionResult InResult) { CustomResult = InResult; }

	void SetIndex(const int32 InIndex) { Index = InIndex; }

	void SetGameInstanceRef(UFusionGameInstance* InGameInstanceRef) { GameInstanceRef = InGameInstanceRef; }

protected:

	UPROPERTY(meta = (BindWidget))
	UButton* ServerBrowserITemButton;
	
	UPROPERTY(meta = (BindWidget))
	UImage* LockImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NetworkTextBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumberOfPlayersTextbox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NumberText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingTextBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerNameTextBox;

	FCustomFusionSessionResult CustomResult;

	int32 Index;

	class UFusionGameInstance* GameInstanceRef;

	int32 MaxRoomNameLength;



};

