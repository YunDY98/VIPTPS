// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBurst.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"

void AWeaponBurst::PressTrigger_Implementation(bool isPressed)
{
	if (isPressed)
	{
		FTimerManager& timerManager = GetWorld()->GetTimerManager();
		timerManager.SetTimer(th_BurstShooting, this, &AWeaponBurst::BurstShooting, 0.2f, true);
		BurstShooting();
	}
	else
	{
		FTimerManager& timerManager = GetWorld()->GetTimerManager();
		timerManager.ClearTimer(th_BurstShooting);
	}
}

void AWeaponBurst::BurstShooting()
{
	if (OwnChar == nullptr)
	{
		FTimerManager& timerManager = GetWorld()->GetTimerManager();
		timerManager.ClearTimer(th_BurstShooting);
		return;
	}

	OwnChar->PlayAnimMontage(WeaponData->ShootMontage);
}
