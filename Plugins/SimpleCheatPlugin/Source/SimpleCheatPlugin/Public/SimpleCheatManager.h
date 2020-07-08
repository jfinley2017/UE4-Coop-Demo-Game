// Copyright 2020 Joe Finley. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "SimpleCheatManager.generated.h"

class USimpleCheatComponent;
namespace ForwardDeclare { class UWorld; }

/**
 * Helper macro to grab the current function name.
 * Useful for passing the command to the server (see explanation of USimpleCheatManager below for why/how you would do this).
 */
#define FUNC_NAME FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2)

/**
 * Handles executing cheats. Disabled on shipping builds, lives on PlayerControllers.
 * Define cheats with UFUNCTION(exec) to have them appear in the console (` in-game) under the name of the function.
 * EG. MyCheat() will appear as MyCheat
 * Cheat arguments are deliminated by spaces, MyCheat(VarOne, VarTwo) is invoked with "MyCheat VarOne VarTwo"
 *
 * You should derive from this class, specify the new derived class in your custom PlayerController (with a SimpleCheatComponent attached
   to that PlayerController), and write your own cheats. See example cheats here for how to create cheats that run on client/server/both.
 *
 * To create a new cheat:
 * 1.) Declare the new cheat here, with UFUNCTION(exec). The cheat will appear in the console as the function name. 
       Consider using a common prefix for your cheats to make searching easier.
 * 2.) Define the function in MyNewSimpleCheatManager.cpp, doing any logic you wish. Use GetOuterAPlayerController() to retreive the player invoking the cheat.
 *     a.) If the cheat requires authority, route your cheat through ServerCheat(). The string passed to ServerCheat() must be equivilent to the console command.
           eg. If you are invoking MyCheat VarOne VarTwo, the string passed to ServerCheat() must be "MyCheat Var1 Var2".
           eg. using above FUNC_NAME macro: ServerCheat(FString::Printf(TEXT("%s"), *FUNC_NAME))
 *
 *
 * High level goals:
 * 1.) Enumerable cheat list in console.
 * 2.) Easy for designers/testers to use the cheats they wish to use, immediately. (eg, no worrying about whether this cheat needs to run on server).
 * 3.) Non-invasive architecture. No need to insert a child into your PlayerController class hierarchy.
 *
 *
 * Issues:
 * 1.) Passing to server is clunky/ugly, currently going with it as the designer (`) menu is not clunky/ugly. 
 */
UCLASS(Within=PlayerController)
class SIMPLECHEATPLUGIN_API USimpleCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:

    /**
     * Prints to screen on both client and server. Used to verify basic functionality is working.
     */
    UFUNCTION(exec)
    void SimpleCheatTest();

    /**
     * Prints to screen only on server. Used to verify basic functionality is working.
     */
    UFUNCTION(exec)
    void SimpleCheatTest_Server();

    /**
     * Prints to screen only on client. Used to verify basic functionality is working.
     */
    UFUNCTION(exec)
    void SimpleCheatTest_Client();

protected:

    /**
     * Routes the cheat to the server. Does nothing if we are already the authority.
     */
    void ServerCheat(const FString& CheatMsg);

    /**
     * Returns whether or not this manager is the authoritative version.
     * In most situations, authoritative versions of the cheat manager will be the one to make
     * significant changes to the game state.
     */
    bool HasAuthority();

    /**
     * Returns the world of the outer PC.
     */
    UWorld* GetWorld() const;

    void SendScreenMessage(FColor Color, FString Message);

    /**
     * Retrieves the outer simple cheat component.
     */
    USimpleCheatComponent* GetOuterSimpleCheatComponent();

    UPROPERTY()
    USimpleCheatComponent* CachedOuterSimpleCheatComponent;
	
};
