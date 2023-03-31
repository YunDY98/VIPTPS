// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawner.h"
#include "ShootingGameInstance.h"
#include "Engine/World.h"
#include "Weapon.h"

// Sets default values
AWeaponSpawner::AWeaponSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		UShootingGameInstance* gameInstance = Cast<UShootingGameInstance>(GetGameInstance());
		FName rowName = gameInstance->GetWeaponRandomRowName();
		FST_Weapon* data = gameInstance->GetWeaponRowData(rowName);

		AWeapon* weapon = GetWorld()->SpawnActor<AWeapon>(data->WeaponClass, GetActorTransform());
		if (weapon)
		{
			weapon->SetRowName(rowName);
		}
	}
}

// Called every frame
void AWeaponSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

