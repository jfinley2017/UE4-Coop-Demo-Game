// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleLoggingBlueprintLibrary.h"
#include "SimpleLoggingLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Windows/WindowsPlatformStackWalk.h"

void USimpleLoggingBlueprintLibrary::SimpleBlueprintLogImpl(const FFrame& StackFrame, UObject* WorldContextObject, FString Message /*= "Hello"*/, FName LogCategory /*= "LogSimpleTransientCategory"*/, ESimpleLogVerbosityBlueprint Verbosity /*= ESimpleLogVerbosityBlueprint::Log*/, bool bPrintToScreen /*= false */, float ScreenDuration /*= 5.0f */)
{
    
    FString FormattedMessage;
    FormatStringFromStackAndWorldContext(FormattedMessage, Message, WorldContextObject, StackFrame);

    if (bPrintToScreen)
    {
        FColor ScreenDebugColor;
        switch (Verbosity)
        {
            case ESimpleLogVerbosityBlueprint::Error:
                ScreenDebugColor = FColor::Red;
                break;
            case ESimpleLogVerbosityBlueprint::Warning:
                ScreenDebugColor = FColor::Yellow;
                break;
            default:
                ScreenDebugColor = FColor::Cyan;
                break;
        }

        GEngine->AddOnScreenDebugMessage(-1, ScreenDuration, ScreenDebugColor, FormattedMessage);
    }

    GLog->Log(LogCategory, (ELogVerbosity::Type)Verbosity, FormattedMessage);
}

void USimpleLoggingBlueprintLibrary::FormatStringFromStackAndWorldContext(FString& OutFormattedString, const FString& Message, UObject* WorldContextObject, const FFrame& StackFrame)
{
    FString StackTraceString = StackFrame.GetStackTrace();
    TArray<FString> StackFrames;
    StackTraceString.ParseIntoArrayLines(StackFrames);

    FString CallingMethodString;

    if (StackFrames.Num() >= 2 && StackFrames[StackFrames.Num() - 1].Contains("ExecuteUbergraph_"))
    {
        // Blueprint graph, generally something like ExecuteUbergraph_{SomeClass}.
        // Not super useful, so we go 1 up to find the owning function.
        CallingMethodString = StackFrames[StackFrames.Num() - 2];
    }
    else if (StackFrames.Num() >= 1)
    {
        // A blueprint function, no Ubergraph
        CallingMethodString = StackFrames.Last();
    }
    else
    {
        // Called from native code? 
        CallingMethodString = TEXT("None");
        ensureMsgf(false, TEXT("SimpleBlueprintLog should not be used natively. Intended for blueprint use only."));
    }
    CallingMethodString.RemoveFromStart("\tFunction ");
    OutFormattedString = FString::Printf(TEXT("%s%s : %s"), SIMPLE_GET_NETMODE_STRING_WORLDCONTEXT, *CallingMethodString, *Message);

}

