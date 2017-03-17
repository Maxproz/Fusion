// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Fusion : ModuleRules
{
    public Fusion(TargetInfo Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "UMG", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils" });

       // DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

        // TODO: Implement the string below once I am ready to get the loading screen stuff working.
        /*"ShooterGameLoadingScreen",*/
        PrivateDependencyModuleNames.AddRange(new string[] { "InputCore", "Slate", "SlateCore", "Json", "FusionGameLoadingScreen" });


    }
}
    

