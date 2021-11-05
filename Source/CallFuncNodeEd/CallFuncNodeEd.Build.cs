// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CallFuncNodeEd : ModuleRules
{
	public CallFuncNodeEd(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "BlueprintGraph"
            }
			);

        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CallFuncNode" });
        PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraph", "BlueprintGraph", "Persona", "UnrealEd", "SlateCore", "BlueprintGraph", "KismetCompiler" });
    }
}
