#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CallFuncNodeLibrary.generated.h"

UENUM(BlueprintType)
enum ECallFuncResult
{
	CallFunc_Success UMETA(DisplayName = "Success"),
	CallFunc_NoFunc UMETA(DisplayName = "Function Missing"),
	CallFunc_ParamMismatch UMETA(DisplayName = "Parameters Mismatch"),
	CallFunc_InvalidObj UMETA(DisplayName = "Invalid Object")
};

/*
* Blueprint library for the CallFuncNode plugin
*/
UCLASS()
class CALLFUNCNODE_API UCallFuncNodeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	* Function declarations, all marked as CustomThunk as we need to parse the inputs manually
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, meta=(BlueprintInternalUseOnly="true"))
		static ECallFuncResult CallNodeFunc_Internal(UObject* Object, FName Name);
	DECLARE_FUNCTION(execCallNodeFunc_Internal);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "A"))
		static ECallFuncResult CallNodeFuncOneParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A);
	DECLARE_FUNCTION(execCallNodeFuncOneParam_Internal);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "A,B"))
		static ECallFuncResult CallNodeFuncTwoParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B);
	DECLARE_FUNCTION(execCallNodeFuncTwoParam_Internal);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "A,B,C"))
		static ECallFuncResult CallNodeFuncThreeParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C);
	DECLARE_FUNCTION(execCallNodeFuncThreeParam_Internal);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "A,B,C,D"))
		static ECallFuncResult CallNodeFuncFourParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C, TFieldPath<FProperty> D);
	DECLARE_FUNCTION(execCallNodeFuncFourParam_Internal);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (BlueprintInternalUseOnly = "true", CustomStructureParam = "A,B,C,D,E"))
		static ECallFuncResult CallNodeFuncFiveParam_Internal(UObject* Object, FName Name, TFieldPath<FProperty> A, TFieldPath<FProperty> B, TFieldPath<FProperty> C, TFieldPath<FProperty> D, TFieldPath<FProperty> E);
	DECLARE_FUNCTION(execCallNodeFuncFiveParam_Internal);

	struct FCallFuncParameter
	{
		FProperty* Property;
		void* Memory;
	};

	// Function for calling a named function with no parameters
	static ECallFuncResult CallNodeFunc_Generic(UObject* Object, FName Name);
	// Function for calling a named function with parameters, using FCallFuncParameter to define the parameters
	static ECallFuncResult CallNodeFunc_Params(UObject* Object, FName Name, TArray<FCallFuncParameter>& Properties);
	// Copy a FCallFuncParameter to memory, this parses the property into it's actual memory
	static void CopyToMemory(uint8*& Memory, FCallFuncParameter Parameter, bool bPad = true);
	// Copy a struct to memory, no longer used but can be used for debugging purposes
	static void CopyToMemory_Struct(uint8*& Memory, FCallFuncParameter Parameter);
};