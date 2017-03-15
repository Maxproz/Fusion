// Fill out your copyright notice in the Description page of Project Settings.

#include "Fusion.h"
#include "MasterWidget.h"


void UMasterWidget::AssignAnimations()
{
	UProperty* prop = GetClass()->PropertyLink;

	// Run through all properties of this class to find any widget animations
	while (prop != nullptr)
	{
		// Only interested in object properties
		if (prop->GetClass() == UObjectProperty::StaticClass())
		{
			UObjectProperty* objectProp = Cast<UObjectProperty>(prop);

			// Only want the properties that are widget animations
			if (objectProp->PropertyClass == UWidgetAnimation::StaticClass())
			{
				UObject* object = objectProp->GetObjectPropertyValue_InContainer(this);

				UWidgetAnimation* widgetAnim = Cast<UWidgetAnimation>(object);

				if (widgetAnim != nullptr)
				{
					// DO SOMETHING TO STORE OFF THE ANIM PTR HERE!
					// E.g. add to a TArray of some struct that holds info for each anim
					WidgetAnimations.Add(widgetAnim);
				}
			}
		}

		prop = prop->PropertyLinkNext;
	}
}

