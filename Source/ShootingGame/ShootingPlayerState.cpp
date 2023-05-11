// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ShootingGameHUD.h"

void AShootingPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingPlayerState, CurHp);
	DOREPLIFETIME(AShootingPlayerState, MaxHp);
	DOREPLIFETIME(AShootingPlayerState, Mag);
}

AShootingPlayerState::AShootingPlayerState()
{
	CurHp = 100.0f;
	MaxHp = 100.0f;
}



void AShootingPlayerState::OnRep_CurHp()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("OnRep_CurHp = %f"), CurHp));

	if(Fuc_Dele_UpdateHp_TwoParams.IsBound())
		Fuc_Dele_UpdateHp_TwoParams.Broadcast(CurHp, MaxHp);
}

void AShootingPlayerState::OnRep_MaxHp()
{
}

void AShootingPlayerState::OnRep_Mag()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("OnRep_Mag = %f"), Mag));

	if (Fuc_Dele_UpdateMag_OneParam.IsBound())
		Fuc_Dele_UpdateMag_OneParam.Broadcast(Mag);
}

void AShootingPlayerState::AddDamage(float Damage)
{
	CurHp = CurHp - Damage;
	CurHp = FMath::Clamp(CurHp, 0.0f, MaxHp);

	OnRep_CurHp();
}

void AShootingPlayerState::AddHeal(float Heal)
{
	CurHp = CurHp + Heal;
	CurHp = FMath::Clamp(CurHp, 0.0f, MaxHp);

	OnRep_CurHp();
}

void AShootingPlayerState::AddMag()
{
	Mag = Mag + 1;

	OnRep_Mag();
}

bool AShootingPlayerState::UseMag()
{
	bool result = IsCanUseMag();

	if (result)
	{
		Mag = Mag - 1;

		OnRep_Mag();
	}

	return result;
}

float AShootingPlayerState::MyCurHp()
{



	return CurHp;
}
