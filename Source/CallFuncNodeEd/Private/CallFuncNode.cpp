#include "CallFuncNode.h"
#include "CallFuncNodeLibrary.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Literal.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"

// define our custom pin names
static const FName ObjectPinName = FName(TEXT("Object"));
static const FName NamePinName = FName(TEXT("Name"));
static const FName ResultPinName = FName(TEXT("Result"));

#define LOCTEXT_NAMESPACE "CallFuncNode"
// the offset in pins for the start of the option pins
#define OPTION_PINS_OFFSET 5

static const TCHAR* SlotNames[5] = {
	TEXT("A"),
	TEXT("B"),
	TEXT("C"),
	TEXT("D"),
	TEXT("E")
};

// function names in UCallFuncNodeLibrary, must match!
static const FName FunNames[6] = {
	FName(TEXT("CallNodeFunc_Internal")),
	FName(TEXT("CallNodeFuncOneParam_Internal")),
	FName(TEXT("CallNodeFuncTwoParam_Internal")),
	FName(TEXT("CallNodeFuncThreeParam_Internal")),
	FName(TEXT("CallNodeFuncFourParam_Internal")),
	FName(TEXT("CallNodeFuncFiveParam_Internal")),
};

UK2Node_CallFunc::UK2Node_CallFunc(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UK2Node_CallFunc::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	bReconstructNode = false;

	// input exec pin
	UEdGraphPin* InPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	InPin->PinFriendlyName = FText();
	// input object pin
	ObjectPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, ObjectPinName);
	ObjectPin->PinType.PinSubCategoryObject = UObject::StaticClass();
	// input name pin
	NamePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, NamePinName);
	// output exec pin
	UEdGraphPin* OutPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	OutPin->PinFriendlyName = FText();
	// output result
	UEdGraphPin* OutResult = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Byte, FindObject<UEnum>(ANY_PACKAGE, TEXT("ECallFuncResult"), true), ResultPinName);

	if (NumOptionPins > 0)
	{
		// all parameter pins
		for (int32 i = 0; i < NumOptionPins && i < Entries.Num(); i++)
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, Entries[i]);

			if (!EntryFriendlyNames.IsValidIndex(i))
				EntryFriendlyNames.Add(FTextStringHelper::CreateFromBuffer(SlotNames[i]));
			Pin->PinFriendlyName = EntryFriendlyNames[i];
		}
	}
	else
	{
		for (int32 i = 0; i < AutoPins.Num(); i++)
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Input, AutoPins[i].NormalType.Type, AutoPins[i].Name);
			Pin->PinType.PinSubCategoryObject = AutoPins[i].NormalType.SubType;
			Pin->PinType.ContainerType = AutoPins[i].ContainerType;
			if (AutoPins[i].ContainerType == EPinContainerType::Map)
			{
				// need additional data
				Pin->PinType.PinValueType.TerminalCategory = AutoPins[i].TerminalType.Type;
				Pin->PinType.PinValueType.TerminalSubCategoryObject = AutoPins[i].TerminalType.SubType;
			}
		}
	}
}

void UK2Node_CallFunc::PostReconstructNode()
{
	Super::PostReconstructNode();

	bReconstructNode = false;
}

FCallFuncAutoParameter::PinType UK2Node_CallFunc::GetParameterInfo(FField* Parameter, FName& OutType, UObject*& OutSubType, EPinContainerType& OutContainerType)
{
	FCallFuncAutoParameter::PinType OutPinType;

	// find specific property type, then assign correct type data
	if (FNumericProperty *NumericProperty = CastField<FNumericProperty>(Parameter))
	{
		if (FByteProperty* ByteProperty = CastField<FByteProperty>(NumericProperty))
		{
			OutPinType.Type = UEdGraphSchema_K2::PC_Byte;
			OutPinType.SubType = ByteProperty->Enum;
		}
		else if (NumericProperty->IsFloatingPoint())
		{
			OutPinType.Type = UEdGraphSchema_K2::PC_Float;
		}
		else if (NumericProperty->IsInteger())
		{
			// int64 vs normal int
			OutPinType.Type = NumericProperty->HasAllCastFlags(CASTCLASS_FInt64Property) ? UEdGraphSchema_K2::PC_Int64 : UEdGraphSchema_K2::PC_Int;
		}
	}
	else if (FBoolProperty* Bool = CastField<FBoolProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Boolean;
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Name;
	}
	else if (FTextProperty* TextProperty = CastField<FTextProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Text;
	}
	else if (FClassProperty* ClassProperty = CastField<FClassProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Class;
		OutPinType.SubType = ClassProperty->PropertyClass;
	}
	else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Object;
		OutPinType.SubType = ObjectProperty->PropertyClass;
	}
	else if (FSoftClassProperty* SoftClass = CastField<FSoftClassProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_SoftClass;
		OutPinType.SubType = SoftClass->MetaClass;
	}
	else if (FSoftObjectProperty* SoftObject = CastField<FSoftObjectProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_SoftObject;
		OutPinType.SubType = SoftObject->PropertyClass;
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_String;
	}
	else if (FArrayProperty* ArrProperty = CastField<FArrayProperty>(Parameter))
	{
		OutPinType = GetParameterInfo(ArrProperty->Inner, OutPinType.Type, OutPinType.SubType, OutContainerType);
		OutContainerType = EPinContainerType::Array;
	}
	else if (FSetProperty* SetProperty = CastField<FSetProperty>(Parameter))
	{
		OutPinType = GetParameterInfo(SetProperty->ElementProp, OutPinType.Type, OutPinType.SubType, OutContainerType);
		OutContainerType = EPinContainerType::Set;
	}
	else if (FMapProperty* MapProperty = CastField<FMapProperty>(Parameter))
	{
		OutPinType = GetParameterInfo(MapProperty->KeyProp, OutType, OutSubType, OutContainerType);
		FCallFuncAutoParameter::PinType ValueType = GetParameterInfo(MapProperty->ValueProp, OutType, OutSubType, OutContainerType);
		OutType = ValueType.Type;
		OutSubType = ValueType.SubType;
		OutContainerType = EPinContainerType::Map;
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Struct;
		OutPinType.SubType = StructProperty->Struct;
	}
	// this might not be used anymore, but worth checking, just incase
	else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Parameter))
	{
		OutPinType.Type = UEdGraphSchema_K2::PC_Byte;
		OutPinType.SubType = EnumProperty->GetEnum();
	}

	return OutPinType;
}

void UK2Node_CallFunc::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (Pin == NamePin && NumOptionPins == 0)
	{
		// get object pin, check if we can evaluate it easily, and if we can find
		// the function associated with that object, then reallocate the pins
		bool bSuccess = false;
		if (ObjectPin->LinkedTo.Num() == 1)
		{
			UFunction* Func = nullptr;
			UEdGraphPin* OtherPin = ObjectPin->LinkedTo[0];
			// try and find type via the pin itself
			UObject* OtherPinType = OtherPin->PinType.PinSubCategoryObject.Get();
			// the pin sub category object may be a class, in which case use that to find func
			if (UClass* OtherPinClass = Cast<UClass>(OtherPinType))
				Func = OtherPinClass->FindFunctionByName(FName(Pin->DefaultValue));
			else
				Func = OtherPinType->FindFunction(FName(Pin->DefaultValue));

			if (Func != nullptr)
			{
				// found it! parse the parameters and reset the pins
				AutoPins.Reset(Func->NumParms);
				FField* Parameter = Func->ChildProperties;
				for (int32 i = 0; i < Func->NumParms; i++)
				{
					FCallFuncAutoParameter AutoParameter;
					AutoParameter.Name = Parameter->NamePrivate;
					// determine type
					AutoParameter.NormalType = GetParameterInfo(Parameter, AutoParameter.TerminalType.Type,
						AutoParameter.TerminalType.SubType, AutoParameter.ContainerType);

					AutoPins.Add(AutoParameter);
					Parameter = Parameter->Next;
				}

				bSuccess = true;
			}
		}

		// if no function found, revert back
		if (!bSuccess)
		{
			AutoPins.Reset(0);
		}

		ReconstructNode();
	}
}

void UK2Node_CallFunc::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	TArray<UEdGraphPin*> OptionPins;
	for (int32 i = OPTION_PINS_OFFSET; i < Pins.Num() && i < OldPins.Num(); i++)
	{
		Pins[i]->PinType = OldPins[i]->PinType;
	}
}

void UK2Node_CallFunc::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the 
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

void UK2Node_CallFunc::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	{
		TArray < UEdGraphPin*> PinParams;
		GetOptionsPins(PinParams);

		// See if this pin is one of the wildcard pins
		const bool bIsWildcardPin = (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard);

		TFunction<bool(UEdGraphPin*)> PinInUse = [&PinInUse](UEdGraphPin* PinToConsider)
		{
			bool bPinInUse = ((PinToConsider->LinkedTo.Num() > 0) || (PinToConsider->ParentPin != nullptr) || !PinToConsider->DoesDefaultValueMatchAutogenerated());
			if (!bPinInUse)
			{
				for (UEdGraphPin* SubPin : PinToConsider->SubPins)
				{
					bPinInUse = PinInUse(SubPin);
					if (bPinInUse)
					{
						break;
					}
				}
			}
			return bPinInUse;
		};

		bool bPinsInUse = PinInUse(Pin);

		bool bPinTypeChanged = false;

		if (bPinsInUse)
		{
			// If the pin was one of the wildcards we have to handle it specially
			if (bIsWildcardPin)
			{
				// If the pin is linked, make sure the other wildcard pins match
				if (Pin->LinkedTo.Num() > 0)
				{
					UEdGraphPin* LinkPin = Pin->LinkedTo[0];

					if (Pin->PinType != LinkPin->PinType)
					{
						Pin->PinType = LinkPin->PinType;
						bPinTypeChanged = true;
					}
				}
			}
		}
		else
		{
			bPinTypeChanged = true;
			Pin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
			Pin->PinType.PinSubCategory = NAME_None;
			Pin->PinType.PinSubCategoryObject = nullptr;
		}

		if (bPinTypeChanged)
		{
			const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

			for (UEdGraphPin* OptionPin : PinParams)
			{
				// Recombine the sub pins back into the OptionPin
				if (OptionPin->ParentPin == nullptr && OptionPin->SubPins.Num() > 0)
				{
					Schema->RecombinePin(OptionPin->SubPins[0]);
				}
			}

			// Get the options again and set them
			GetOptionsPins(PinParams);
			for (UEdGraphPin* OptionPin : PinParams)
			{
				if (!Schema->IsPinDefaultValid(OptionPin, OptionPin->DefaultValue, OptionPin->DefaultObject, OptionPin->DefaultTextValue).IsEmpty())
				{
					Schema->ResetPinToAutogeneratedDefaultValue(OptionPin);
				}
			}

			bReconstructNode = true;
		}
	}
}

void UK2Node_CallFunc::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();

	if (bReconstructNode)
	{
		ReconstructNode();
	}
}

void UK2Node_CallFunc::AddInputPin()
{
	Modify();

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Increment the pin count
	if (NumOptionPins < 5)
	{
		Entries.Add(FName(SlotNames[NumOptionPins]));
		EntryFriendlyNames.Add(FTextStringHelper::CreateFromBuffer(SlotNames[NumOptionPins]));

		NumOptionPins++;
		// We will let the AllocateDefaultPins call handle the actual addition via ReconstructNode
		ReconstructNode();
	}
}

void UK2Node_CallFunc::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UFunction* Function = UCallFuncNodeLibrary::StaticClass()->FindFunctionByName(FunNames[NumOptionPins]);
	if (Function == NULL)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("InvalidFunctionName", "UK2Node_CallFunc: Function not supported or not initialized. @@").ToString(), this);
		return;
	}

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	// The call function does all the real work of actually calling our input function
	UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);

	CallFunction->SetFromFunction(Function);
	CallFunction->AllocateDefaultPins();
	UEdGraphPin *OutPin = CallFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	// move all the pins from our node to the call function node (matching the UCallFuncNodeLibrary functions)
	CompilerContext.MovePinLinksToIntermediate(*FindPin(ObjectPinName), *CallFunction->FindPin(TEXT("Object")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(NamePinName), *CallFunction->FindPin(TEXT("Name")));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(ResultPinName), *OutPin);

	CompilerContext.MovePinLinksToIntermediate(*FindPin(UEdGraphSchema_K2::PN_Execute), *CallFunction->FindPin(UEdGraphSchema_K2::PN_Execute));
	CompilerContext.MovePinLinksToIntermediate(*FindPin(UEdGraphSchema_K2::PN_Then), *CallFunction->FindPin(UEdGraphSchema_K2::PN_Then));

	// move all the parameter pins
	int32 PinCount = FMath::Max(NumOptionPins, AutoPins.Num());
	for (int32 i = 0; i < PinCount; i++)
	{
		UEdGraphPin* SrcPin = FindPin(FName(SlotNames[i]));
		UEdGraphPin* FuncPin = CallFunction->FindPin(SlotNames[i]);
		CompilerContext.MovePinLinksToIntermediate(*SrcPin, *FuncPin);
		FuncPin->PinType = SrcPin->PinType;
	}

	BreakAllNodeLinks();
}

void UK2Node_CallFunc::GetOptionsPins(TArray<UEdGraphPin*>& OutPins)
{
	OutPins.Reset();

	for (int32 i = OPTION_PINS_OFFSET; i < Pins.Num(); i++)
	{
		OutPins.Add(Pins[i]);
	}
}

#undef LOCTEXT_NAMESPACE