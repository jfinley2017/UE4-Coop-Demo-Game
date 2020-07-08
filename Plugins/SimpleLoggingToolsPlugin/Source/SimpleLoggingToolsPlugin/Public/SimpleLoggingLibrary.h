// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "EngineGlobals.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimpleTransientCategory, Log, All);

/**
 * Library which consists of many different macros used to make logging easier in source.
 * ~~~ Clients should make their own .h and wrap these macros with their own to provide their own naming conventions. ~~~
 */

/**
 * Converts the current world's netmode to a string. Requires a class able to invoke GetWorld().
 * Used frequently in logging macros to provide additional context about where code is running. 
 */
#define SIMPLE_GET_NETMODE_STRING ((GetWorld() == nullptr) ? TEXT("") \
        : (GetWorld()->GetNetMode() == NM_Client) ? TEXT("[Client] ") \
        : (GetWorld()->GetNetMode() == NM_ListenServer) ? TEXT("[ListenServer] ") \
        : (GetWorld()->GetNetMode() == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
        : TEXT("[Standalone] "))

#define SIMPLE_GET_NETMODE_STRING_WORLDCONTEXT  (((WorldContextObject == nullptr) || (WorldContextObject->GetWorld() == nullptr)) ? TEXT("") \
        : (WorldContextObject->GetWorld()->GetNetMode() == NM_Client) ? TEXT("[Client] ") \
        : (WorldContextObject->GetWorld()->GetNetMode() == NM_ListenServer) ? TEXT("[ListenServer] ") \
        : (WorldContextObject->GetWorld()->GetNetMode() == NM_DedicatedServer) ? TEXT("[DedicatedServer] ") \
        : TEXT("[Standalone] "))

/**
 * Compatibility macros.
 */
#if _MSC_VER
#define SIMPLE_LOG_FUNC_NAME *FString(__FUNCTION__)
#else
#define SIMPLE_LOG_FUNC_NAME *FString(__func__)
#endif

 /**
  * Print enum macro. Requires enum to be marked as UENUM()
  *
  * Usage: SimpleEnumToString("YourEnumClass", {Value(can be variable)})
  * Example: SimpleEnumToString("MyEnumClass", MyEnumClass::EnumValue)
  */
#define SimpleEnumToString(InEnumType, InEnumValue) ( \
    (FindObject<UEnum>(ANY_PACKAGE, TEXT(InEnumType), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(InEnumType), true)->GetNameStringByIndex((uint8)InEnumValue) \
                                                                    : FString("Could not find string associated with enum value. Ensure that the enum is a UENUM and the index provided exists.") )
/**
 * Primary logging macro. Provides netmode and current function name alongside formatted text.
 * Usage: SimpleLog(MyLogCategory, Warning, "Some text and variables %s, %f, %d", *FString(), SomeFloat, SomeInteger);
 * Output: MyLogCategory: [NetMode] MyCallingClass::MyCallingFunction() : My Message.
 */
#define SimpleLog(LogCategory, Verbosity, Format, ...) \
{ \
    static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range."); \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    UE_LOG(LogCategory, Verbosity, TEXT("%s%s() : %s"),  SIMPLE_GET_NETMODE_STRING, SIMPLE_LOG_FUNC_NAME, *Msg); \
}

 /**
  * Logs when only a world context is available, functionally the same as @SimpleLog
  */
#define SimpleLog_WorldContext(LogCategory, Verbosity, Format, ...) \
{ \
    static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range.");\
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    UE_LOG(LogCategory, Verbosity, TEXT("%s%s() : %s"),  SIMPLE_GET_NETMODE_STRING_WORLDCONTEXT, SIMPLE_LOG_FUNC_NAME, *Msg);\
}

/**
 * Quick log, generally intended for one-off "I made it" messages where a breakpoint doesn't suffice. 
 * E.G. debugging tick with many complex conditionals.
 */
#define SimpleQuickLog(Format, ...) \
{\
    SimpleLog(LogSimpleTransientCategory, Warning, Format,  ##__VA_ARGS__); \
}

/**
 * Primary logging macro. Provides function name alongside formatted text. Does not require a reference to GetWorld() like its sibling SimpleLog.
 * As a result, it cannot provide netmode details.
 * Usage: SimpleLogStatic(MyLogCategory, Warning, "Some text and variables %s, %f, %d", *FString(), SomeFloat, SomeInteger);
 * Output: MyLogCategory: MyCallingClass::MyCallingFunction() : My Message.
 */
#define SimpleLogStatic(LogCategory, Verbosity, Format, ...) \
{ \
    static_assert((ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range."); \
    SET_WARN_COLOR(COLOR_CYAN); \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    UE_LOG(LogCategory, Verbosity, TEXT("%s() : %s"),  SIMPLE_LOG_FUNC_NAME, *Msg); \
    CLEAR_WARN_COLOR(); \
}

/**
 * Quick log, generally intended for one-off "I made it" messages where a breakpoint doesn't suffice.
 * E.G. debugging tick with many complex conditionals.
 */
#define SimpleQuickLogStatic(Format, ...) \
{\
    SimpleLogStatic(LogSimpleTransientCategory, Warning, Format,  ##__VA_ARGS__); \
}

 /**
  * Only logs the first time this macro is invoked. Will be ignored subsequent times.
  * Useful for logs that aren't fatal but can end up slamming the log.
  *
  * Note that for PIE versions, this will literally only log once. Subsequent PIE playthroughs will not hit this log again.
  * Running in non singleprocess mode does gets around this.
  */
#define SimpleLogOnce(LogCategory, Verbosity, Format, ...) \
{ \
    static bool bHasLoggedOnce = false; \
    if (bHasLoggedOnce == false) \
    { \
        SimpleLog(LogCategory, Verbosity, Format, ##__VA_ARGS__); \
        bHasLoggedOnce = true; \
    } \
}

/**
 * Only logs the first time this macro is invoked. Will be ignored subsequent times.
 * Useful for logs that aren't fatal but can end up spamming the log.
 *
 * Note that for PIE versions, this will literally only log once. Subsequent PIE playthroughs will not hit this log again.
 * Running in non singleprocess mode does get around this.
 */
#define SimpleLogOnceStatic(LogCategory, Verbosity, Format, ...) \
{ \
    static bool bHasLoggedOnce = false; \
    if (bHasLoggedOnce == false) \
    {\
        SimpleLogStatic(LogCategory, Verbosity, Format, ##__VA_ARGS__); \
        bHasLoggedOnce = true; \
    }\
}

/**
 * Prints to screen. Will not print to console or otherwise log.
 * Usage: ScreenDebug(0.0f, FColor::Red, "Printing some float %f", SomeFloat);
 */
#define SimpleScreenLog(Duration, Color, Format, ...)\
{\
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Msg); \
}




    