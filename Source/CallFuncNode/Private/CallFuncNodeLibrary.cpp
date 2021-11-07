#include "CallFuncNodeLibrary.h"

ECallFuncResult UCallFuncNodeLibrary::CallNodeFunc_Generic(UObject* Object, FName Name)
{
	if (Object && Object->IsValidLowLevel())
	{
		if (UFunction* Func = Object->FindFunction(Name))
		{
			if (Func->NumParms == 0)
			{
				Object->ProcessEvent(Func, nullptr);
				return ECallFuncResult::CallFunc_Success;
			}
			return ECallFuncResult::CallFunc_ParamMismatch;
		}
		return ECallFuncResult::CallFunc_NoFunc;
	}
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFunc_Params(UObject* Object, FName Name, TArray<FCallFuncParameter>& Properties)
{
	if (Object && Object->IsValidLowLevel())
	{
		if (UFunction* Func = Object->FindFunction(Name))
		{
			int32 TotalMemory = 0;

			for (FCallFuncParameter Property : Properties)
			{
				// if the property in null that means its a reference to self
				if (Property.Property == nullptr)
					TotalMemory += 8;
				else
				{
					// all parameters need to be padded to 4 bytes
					int32 Size = Property.Property->ElementSize * Property.Property->ArrayDim;
					int32 Alignment = Size % 4;
					TotalMemory += Size + (Alignment == 0 ? 0 : (4 - Alignment)); // pad to 4 bytes
				}
			}

			// check if theres a mismatch between our input memory or the num properties
			if (Func->NumParms == Properties.Num() && Func->ParmsSize == TotalMemory)
			{
				uint8* Memory = (uint8*)FMemory_Alloca(TotalMemory);
				FMemory::Memzero(Memory, TotalMemory);
				uint8* MemoryPtr = Memory;

				// copy each parameter to our chunk of memory
				FField* FuncProperty = Func->ChildProperties;
				for (int32 i = 0; i < Properties.Num(); i++)
				{
					FCallFuncParameter Property = Properties[i];

					if (!Property.Property->IsA(FuncProperty->GetClass()))
					{
						// mismatch
						return ECallFuncResult::CallFunc_ParamMismatch;
					}

					if (Property.Property == nullptr)
					{
						FMemory::Memcpy(MemoryPtr, &Object, sizeof(8));
						MemoryPtr += 8;
					}
					else
					{
						CopyToMemory(MemoryPtr, Property);
					}

					FuncProperty = FuncProperty->Next;
				}

				// finally call the function with the parameter memory
				Object->ProcessEvent(Func, Memory);

				return ECallFuncResult::CallFunc_Success;
			}
			else
			{
				// mismatch
				return ECallFuncResult::CallFunc_ParamMismatch;
			}
		}

		return ECallFuncResult::CallFunc_NoFunc;
	}

	return ECallFuncResult::CallFunc_InvalidObj;
}

// All our declared function definitions that don't get used, still need to define to avoid NoExport on the class
ECallFuncResult UCallFuncNodeLibrary::CallNodeFunc_Internal(UObject* Object, FName Name)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFuncOneParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFuncTwoParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFuncThreeParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFuncFourParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C, TFieldPath<FProperty> D)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

ECallFuncResult UCallFuncNodeLibrary::CallNodeFuncFiveParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C, TFieldPath<FProperty> D, TFieldPath<FProperty> E)
{
	check(0);
	return ECallFuncResult::CallFunc_InvalidObj;
}

// helper function to get the properties from the blueprint stack
FORCEINLINE void ParseInputs(TArray<UCallFuncNodeLibrary::FCallFuncParameter>& InOutParams, FFrame& Stack, int32 ParamNum)
{
	for (int32 i = 0; i < ParamNum; i++)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FObjectProperty>(NULL);

		FProperty* ValueProp = Stack.MostRecentProperty;
		void* ValuePtr = Stack.MostRecentPropertyAddress;
		InOutParams.Add({ ValueProp, ValuePtr });
	}
}

/*
* All our functions CustomThunk implementations
*/
DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFunc_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FObjectProperty>(NULL);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Generic(Object, Name);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFuncOneParam_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	TArray<FCallFuncParameter> Properties;
	ParseInputs(Properties, Stack, 1);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Params(Object, Name, Properties);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFuncTwoParam_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	TArray<FCallFuncParameter> Properties;
	ParseInputs(Properties, Stack, 2);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Params(Object, Name, Properties);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFuncThreeParam_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	TArray<FCallFuncParameter> Properties;
	ParseInputs(Properties, Stack, 3);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Params(Object, Name, Properties);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFuncFourParam_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	TArray<FCallFuncParameter> Properties;
	ParseInputs(Properties, Stack, 4);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Params(Object, Name, Properties);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UCallFuncNodeLibrary::execCallNodeFuncFiveParam_Internal)
{
	P_GET_OBJECT(UObject, Object);
	P_GET_PROPERTY(FNameProperty, Name);

	TArray<FCallFuncParameter> Properties;
	ParseInputs(Properties, Stack, 5);

	P_FINISH;

	P_NATIVE_BEGIN;
	*(ECallFuncResult*)Z_Param__Result = UCallFuncNodeLibrary::CallNodeFunc_Params(Object, Name, Properties);
	P_NATIVE_END;
}

// Ensure all memory is padded to 4 bytes
FORCEINLINE int32 MemPadding(int32 InSize)
{
	int32 Alignment = InSize % 4;
	return InSize + (Alignment == 0 ? 0 : (4 - Alignment));
}

void UCallFuncNodeLibrary::CopyToMemory(uint8*& MemoryPtr, FCallFuncParameter Property, bool bPad)
{
	struct FArrayTypeBasic
	{
		uint8* Ptr;
		int32 ArrayNum;
		int32 ArrayMax;
	};

	// Simply check what type of property the input is and call specific functions to get the
	// memory/value and then copy it to our buffer

	if (FNumericProperty *NumericProperty = CastField<FNumericProperty>(Property.Property))
	{
		if (NumericProperty->IsFloatingPoint())
		{
			float FloatValue = NumericProperty->GetFloatingPointPropertyValue(Property.Memory);
			FMemory::Memcpy(MemoryPtr, &FloatValue, Property.Property->ElementSize);
		}
		else if (NumericProperty->IsInteger())
		{
			int32 IntValue = NumericProperty->GetSignedIntPropertyValue(Property.Memory);
			FMemory::Memcpy(MemoryPtr, &IntValue, Property.Property->ElementSize);
		}
	} 
	else if (FBoolProperty* Bool = CastField<FBoolProperty>(Property.Property))
	{
		bool BoolVal = Bool->GetPropertyValue(Property.Memory);
		FMemory::Memcpy(MemoryPtr, &BoolVal, Property.Property->ElementSize);
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property.Property))
	{
		FName NameValue = NameProperty->GetPropertyValue(Property.Memory);
		FMemory::Memcpy(MemoryPtr, &NameValue, Property.Property->ElementSize);
	}
	else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property.Property))
	{
		FText TextValue = TextProperty->GetPropertyValue(Property.Memory);
		FMemory::Memcpy(MemoryPtr, &TextValue, Property.Property->ElementSize);
	}
	else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property.Property))
	{
		UObject* Obj = ObjectProperty->GetPropertyValue(Property.Memory);
		FMemory::Memcpy(MemoryPtr, &Obj, Property.Property->ElementSize);
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property.Property))
	{
		FString* StrValue = StringProperty->GetPropertyValuePtr(Property.Memory);
		FMemory::Memcpy(MemoryPtr, StrValue, Property.Property->ElementSize);
	}
	else if (FArrayProperty* ArrProperty = CastField<FArrayProperty>(Property.Property))
	{
		FScriptArrayHelper Helper(ArrProperty, Property.Memory);
		int32 ArrSize = Helper.Num();

		FArrayTypeBasic Arr;
		Arr.Ptr = Helper.GetRawPtr(0);
		Arr.ArrayNum = ArrSize;
		// TODO: figure out how to get the actual Max
		Arr.ArrayMax = ArrSize;
		
		FMemory::Memcpy(MemoryPtr, &Arr, sizeof(FArrayTypeBasic));
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property.Property))
	{
		StructProperty->CopyValuesInternal(MemoryPtr, Property.Memory, 1);
	}

	MemoryPtr += bPad ? MemPadding(Property.Property->ElementSize) : Property.Property->ElementSize;
}

// No longer used function, but useful for debugging
void UCallFuncNodeLibrary::CopyToMemory_Struct(uint8*& Memory, FCallFuncParameter Parameter)
{
	FStructProperty* Struct = CastField<FStructProperty>(Parameter.Property);

	if (Struct == nullptr)
		return;

	UScriptStruct* Script = Struct->Struct;
	for (TFieldIterator<FProperty> It(Script); It; ++It)
	{
		FProperty* Property = *It;

		// Never assume ArrayDim is always 1
		for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++)
		{
			// This grabs the pointer to where the property value is stored
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Parameter.Memory, ArrayIndex);

			FCallFuncParameter RecursiveParameter;
			RecursiveParameter.Memory = ValuePtr;
			RecursiveParameter.Property = Property;
			// Parse this property
			CopyToMemory(Memory, RecursiveParameter);
		}
	}
}