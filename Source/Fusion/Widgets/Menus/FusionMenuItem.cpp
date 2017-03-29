// @Maxpro 2017

#include "Fusion.h"

#include "FusionPlayerController.h"

#include "FusionMenuItem.h"


#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "UI"/ RelativePath + TEXT(".ttf"), __VA_ARGS__ )

void SFusionMenuItem::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;

	UWorld* World = PlayerOwner->GetGameInstance()->GetWorld();
	AFusionPlayerController* FPC = Cast<AFusionPlayerController>(PlayerOwner->GetPlayerController(World));

	BorderBrush = new FSlateBrush();
	BorderBrush->ImageSize = FVector2D(403.f, 36.f);
	BorderBrush->DrawAs = ESlateBrushDrawType::Image;
	BorderBrush->TintColor = FSlateColor(FLinearColor(FColor::White));
	if (FPC)
	{
		BorderBrush->SetResourceObject(FPC->MenuItemTexture);
	}

	LeftArrowBrush = new FSlateBrush();
	LeftArrowBrush->ImageSize = FVector2D(18.f, 21.f);
	LeftArrowBrush->DrawAs = ESlateBrushDrawType::Image;
	LeftArrowBrush->TintColor = FSlateColor(FLinearColor(FColor::White));
	if (FPC)
	{
		LeftArrowBrush->SetResourceObject(FPC->LeftArrowTexture);
	}

	RightArrowBrush = new FSlateBrush();
	RightArrowBrush->ImageSize = FVector2D(18.f, 21.f);
	RightArrowBrush->DrawAs = ESlateBrushDrawType::Image;
	RightArrowBrush->TintColor = FSlateColor(FLinearColor(FColor::White));
	if (FPC)
	{
		RightArrowBrush->SetResourceObject(FPC->RightArrowTexture);
	}


	Text = InArgs._Text;
	OptionText = InArgs._OptionText;
	OnClicked = InArgs._OnClicked;
	OnArrowPressed = InArgs._OnArrowPressed;
	bIsMultichoice = InArgs._bIsMultichoice;
	bIsActiveMenuItem = false;
	LeftArrowVisible = EVisibility::Collapsed;
	RightArrowVisible = EVisibility::Collapsed;
	//if attribute is set, use its value, otherwise uses default
	InactiveTextAlpha = InArgs._InactiveTextAlpha.Get(1.0f);

	const float ArrowMargin = 3.0f;
	ItemMargin = 10.0f;
	TextColor = FLinearColor(FColor(155, 164, 182));


	// Fonts still need to be specified in code for now
	Style.Set("Fusion.MenuServerListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.ScoreboardListTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 14))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuProfileNameStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 20))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.MenuHeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 26))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.WelcomeScreen.WelcomeTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 32))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.HeaderTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 24))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DefaultScoreboard.Row.StatTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 18))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.SplitScreenLobby.StartMatchTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 16))
		.SetColorAndOpacity(FLinearColor::Green)
		.SetShadowOffset(FVector2D(0, 1))
	);

	Style.Set("Fusion.DemoListCheckboxTextStyle", FTextBlockStyle()
		.SetFont(TTF_FONT("HUD/Roboto18", 12))
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FIntPoint(-1, 1))
	);




	ChildSlot
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(374.0f)
		.HeightOverride(23.0f)
		[
			SNew(SImage)
			.ColorAndOpacity(this, &SFusionMenuItem::GetButtonBgColor)
		.Image(BorderBrush)
		]
		]
	+ SOverlay::Slot()
		.HAlign(bIsMultichoice ? HAlign_Left : HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(ItemMargin, 0, 0, 0))
		[
			SAssignNew(TextWidget, STextBlock)
			.TextStyle(Style, "Fusion.MenuTextStyle")
		.ColorAndOpacity(this, &SFusionMenuItem::GetButtonTextColor)
		.ShadowColorAndOpacity(this, &SFusionMenuItem::GetButtonTextShadowColor)
		.Text(Text)
		]
	+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		.Padding(FMargin(0, 0, ArrowMargin, 0))
		.Visibility(this, &SFusionMenuItem::GetLeftArrowVisibility)
		.OnMouseButtonDown(this, &SFusionMenuItem::OnLeftArrowDown)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(LeftArrowBrush)
		]
		]
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(TAttribute<FMargin>(this, &SFusionMenuItem::GetOptionPadding))
		[
			SNew(STextBlock)
			.TextStyle(Style, "Fusion.MenuTextStyle")
		.Visibility(bIsMultichoice ? EVisibility::Visible : EVisibility::Collapsed)
		.ColorAndOpacity(this, &SFusionMenuItem::GetButtonTextColor)
		.ShadowColorAndOpacity(this, &SFusionMenuItem::GetButtonTextShadowColor)
		.Text(OptionText)
		]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		.Padding(FMargin(ArrowMargin, 0, ItemMargin, 0))
		.Visibility(this, &SFusionMenuItem::GetRightArrowVisibility)
		.OnMouseButtonDown(this, &SFusionMenuItem::OnRightArrowDown)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(RightArrowBrush)
		]
		]
		]
		]

		];
}

FReply SFusionMenuItem::OnRightArrowDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Result = FReply::Unhandled();
	const int32 MoveRight = 1;
	if (OnArrowPressed.IsBound() && bIsActiveMenuItem)
	{
		OnArrowPressed.Execute(MoveRight);
		Result = FReply::Handled();
	}
	return Result;
}

FReply SFusionMenuItem::OnLeftArrowDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Result = FReply::Unhandled();
	const int32 MoveLeft = -1;
	if (OnArrowPressed.IsBound() && bIsActiveMenuItem)
	{
		OnArrowPressed.Execute(MoveLeft);
		Result = FReply::Handled();
	}
	return Result;
}

EVisibility SFusionMenuItem::GetLeftArrowVisibility() const
{
	return LeftArrowVisible;
}

EVisibility SFusionMenuItem::GetRightArrowVisibility() const
{
	return RightArrowVisible;
}

FMargin SFusionMenuItem::GetOptionPadding() const
{
	return RightArrowVisible == EVisibility::Visible ? FMargin(0) : FMargin(0, 0, ItemMargin, 0);
}

FSlateColor SFusionMenuItem::GetButtonTextColor() const
{
	FLinearColor Result;
	if (bIsActiveMenuItem)
	{
		Result = TextColor;
	}
	else
	{
		Result = FLinearColor(TextColor.R, TextColor.G, TextColor.B, InactiveTextAlpha);
	}
	return Result;
}

FLinearColor SFusionMenuItem::GetButtonTextShadowColor() const
{
	FLinearColor Result;
	if (bIsActiveMenuItem)
	{
		Result = FLinearColor(0, 0, 0, 1);
	}
	else
	{
		Result = FLinearColor(0, 0, 0, InactiveTextAlpha);
	}
	return Result;
}


FSlateColor SFusionMenuItem::GetButtonBgColor() const
{
	const float MinAlpha = 0.1f;
	const float MaxAlpha = 1.f;
	const float AnimSpeedModifier = 1.5f;

	float AnimPercent = 0.f;
	ULocalPlayer* const Player = PlayerOwner.Get();
	if (Player)
	{
		// @fixme, need a world get delta time?
		UWorld* const World = Player->GetWorld();
		if (World)
		{
			const float GameTime = World->GetRealTimeSeconds();
			AnimPercent = FMath::Abs(FMath::Sin(GameTime*AnimSpeedModifier));
		}
	}

	const float BgAlpha = bIsActiveMenuItem ? FMath::Lerp(MinAlpha, MaxAlpha, AnimPercent) : 0.f;
	return FLinearColor(1.f, 1.f, 1.f, BgAlpha);
}

FReply SFusionMenuItem::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//execute our "OnClicked" delegate, if we have one
	if (OnClicked.IsBound() == true)
	{
		return OnClicked.Execute();
	}

	return FReply::Handled();
}


FReply SFusionMenuItem::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SFusionMenuItem::SetMenuItemActive(bool bIsMenuItemActive)
{
	this->bIsActiveMenuItem = bIsMenuItemActive;
}

void SFusionMenuItem::UpdateItemText(const FText& UpdatedText)
{
	Text = UpdatedText;
	if (TextWidget.IsValid())
	{
		TextWidget->SetText(Text);
	}
}


#undef TTF_FONT