// @Maxpro 2017

#pragma once

#include "Widgets/MasterWidget.h"
#include "PasswordEnterPopup_Widget.generated.h"

/**
 * 
 */
UCLASS()
class FUSION_API UPasswordEnterPopup_Widget : public UMasterWidget
{
	GENERATED_BODY()


public:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedBackButton();

	UFUNCTION()
	void OnClickedJoinRoomButton();

	UFUNCTION()
	void OnTextChangedPasswordTextBox(const FText &Text);

	// TODO: Filter this out into a static function.. its inside of main menu and here now.
	void LimitTextBoxText(const FText InText, const int32 MaxTextSize, FString& OutString);

	void SetMaxPasswordLength(const int32 Length) { MaxPasswordLength = Length; }

	void SetPassword(const FString InPassword) { Password = InPassword; }

	void SetGameInstanceRef(UFusionGameInstance* InGameInstanceRef) { GameInstanceRef = InGameInstanceRef; }
protected:

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinRoomButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordTextBox;

	class UFusionGameInstance* GameInstanceRef;

	int32 SessionIndex;

	FString Password;

	int32 MaxPasswordLength;
};
