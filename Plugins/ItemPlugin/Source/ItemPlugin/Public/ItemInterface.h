// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ITEMPLUGIN_API IItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddHeal(float Heal);

	virtual void AddHeal_Implementation(float Heal) {};

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddMag();

	virtual void AddMag_Implementation() {};
};
