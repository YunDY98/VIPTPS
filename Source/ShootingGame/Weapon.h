// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponInterface.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FST_Weapon : public FTableRowBase
{
	GENERATED_BODY()

public:
	FST_Weapon()
		: StaticMesh(nullptr)
		, ShootMontage(nullptr)
		, ReloadMontage(nullptr)
		, SoundBase(nullptr)
		, FireEffect(nullptr)
		, MaxAmmo(30)
		, Damage(10)
		, WeaponClass(nullptr)
	{}


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ShootMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* SoundBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* FireEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeapon> WeaponClass;
};

UCLASS()
class SHOOTINGGAME_API AWeapon : public AActor, public IWeaponInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PressTrigger(bool isPressed);

	virtual void PressTrigger_Implementation(bool isPressed) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NotifyShoot();

	virtual void NotifyShoot_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NotifyReload();

	virtual void NotifyReload_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PressReload();

	virtual void PressReload_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IsCanUse(bool& IsCanUse);

	virtual void IsCanUse_Implementation(bool& IsCanUse) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AttachWeapon(ACharacter* targetChar);

	virtual void AttachWeapon_Implementation(ACharacter* targetChar) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DetachWeapon(ACharacter* targetChar);

	virtual void DetachWeapon_Implementation(ACharacter* targetChar) override;

public:
	UFUNCTION(Server, Reliable)
	void ReqShoot(const FVector vStart, const FVector vEnd);

	UFUNCTION()
	void OnRep_Ammo();

	UFUNCTION()
	void OnRep_RowName();

	UFUNCTION(BlueprintCallable)
	void UpdateAmmoToHud(int NewAmmo);

	UFUNCTION(BlueprintCallable)
	void DoReload();

	UFUNCTION(BlueprintCallable)
	bool UseAmmo();

	void SetRowName(FName name);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* Audio;

	UPROPERTY(Replicated, BlueprintReadWrite)
	ACharacter* OwnChar;
	
	UPROPERTY(ReplicatedUsing = OnRep_RowName)
	FName RowName;

	FST_Weapon* WeaponData;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int Ammo;
};
