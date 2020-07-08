// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Stack.h"
#include "SimpleLoggingBlueprintLibrary.generated.h"

/**
 * Expose ELogVerbosity to blueprint by mirroring ELogVerbosity and casting.
 */
UENUM(BlueprintType)
enum class ESimpleLogVerbosityBlueprint : uint8
{
    None UMETA(Hidden), 
    Error = 2,
    Warning = 3,
    Log = 4,
    Verbose = 5,
    VeryVerbose = 6
};

/**
 * 
 */
UCLASS()
class SIMPLELOGGINGTOOLSPLUGIN_API USimpleLoggingBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:


    /**
    * Logs the message @Message to the category @LogCategory with verbosity @Verbosity.
    * Includes information on the netmode/outer function invoking this log. Optionally printing to the screen.
    * Only intended for blueprint use. Use macros provided in SimpleLoggingLibrary.h/.cpp for native logging.
    * @param WorldContextObject The world context of the invoking blueprint. Used for netmode.
    * @param Message The message to be logged.
    * @param LogCategory The log category that the message should be logged to. Note that this is not a formal UE log category, but behaves similarly in the output log.
    * @param Verbosity The verbosity at which to show the log.
    * @param bPrintToScreen Whether or not the message should also be printed to screen. The message will be color coded in the usual Error/Warning color scheme.
    * @param ScreenDuration The duration which the message should appear on screen. Does nothing if bPrintToScreen is false.
    */
    UFUNCTION(BlueprintCallable, CustomThunk, meta=(WorldContext="WorldContextObject"), Category = "SimpleLoggingLibrary")
    static void SimpleBlueprintLog(UObject* WorldContextObject, FString Message = "Hello", FName LogCategory = "LogSimpleTransientCategory",
                                       ESimpleLogVerbosityBlueprint Verbosity = ESimpleLogVerbosityBlueprint::Log, bool bPrintToScreen = false, float ScreenDuration = 5.0f);

    static void SimpleBlueprintLogImpl(const FFrame& StackFrame, UObject* WorldContextObject, FString Message = "Hello", FName LogCategory = "LogSimpleTransientCategory",
                                       ESimpleLogVerbosityBlueprint Verbosity = ESimpleLogVerbosityBlueprint::Log, bool bPrintToScreen = false, float ScreenDuration = 5.0f);

    DECLARE_FUNCTION(execSimpleBlueprintLog)
    {
        P_GET_OBJECT(UObject, Z_Param_WorldContextObject);
        P_GET_PROPERTY(UStrProperty, Z_Param_Message);
        P_GET_PROPERTY(UNameProperty, Z_Param_LogCategory);
        P_GET_ENUM(ESimpleLogVerbosityBlueprint, Z_Param_Verbosity);
        P_GET_UBOOL(Z_Param_bPrintToScreen);
        P_GET_PROPERTY(UFloatProperty, Z_Param_ScreenDuration);
        P_FINISH;
        P_NATIVE_BEGIN;
        USimpleLoggingBlueprintLibrary::SimpleBlueprintLogImpl(Stack, Z_Param_WorldContextObject, Z_Param_Message,
                                                               Z_Param_LogCategory, Z_Param_Verbosity, Z_Param_bPrintToScreen, Z_Param_ScreenDuration);
        P_NATIVE_END;
    }

protected:

    /**
     * Attempts to format a log message in {Netmode} {Function} : {Message} format.
     */
    static void FormatStringFromStackAndWorldContext(FString& OutFormattedString, const FString& Message, UObject* WorldContextObject, const FFrame& StackFrame);

};
