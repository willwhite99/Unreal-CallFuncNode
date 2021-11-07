#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Templates/SubclassOf.h"
#include "Textures/SlateIcon.h"
#include "Engine/MemberReference.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "K2Node.h"
#include "K2Node_AddPinInterface.h"
#include "CallFuncNode.generated.h"

/*
* Custom editor node for CallFuncNode
*/
UCLASS(MinimalAPI)
class UK2Node_CallFunc : public UK2Node, public IK2Node_AddPinInterface
{
	GENERATED_UCLASS_BODY()

public:
	/** The number of selectable options this node currently has */
	UPROPERTY()
	int32 NumOptionPins;

	/** The pin type of the index pin */
	UPROPERTY()
		TArray<FEdGraphPinType> IndexPinType;

	/** List of the current entries in the enum (Pin Names) */
	UPROPERTY()
		TArray<FName> Entries;

	/** List of the current entries in the enum (Pin Friendly Names) */
	UPROPERTY(Transient)
		TArray<FText> EntryFriendlyNames;

	/** Whether we need to reconstruct the node after the pins have changed */
	UPROPERTY(Transient)
		uint8 bReconstructNode : 1;

	virtual void AddInputPin() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void AllocateDefaultPins() override;
	virtual void PostReconstructNode() override;

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void NodeConnectionListChanged() override;
	virtual FText GetTooltipText() const override { return FText::FromString(TEXT("Call Function by Name")); }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return FText::FromString(TEXT("Call Function by Name")); }
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual FText GetMenuCategory() const { return FText::FromString(TEXT("Utilities")); }

	void GetOptionsPins(TArray<UEdGraphPin*>& Pins);
};