// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "NiagaraComponent.h"  // 나이아가라 컴포넌트 헤더 파일
#include "NiagaraFunctionLibrary.h"  // 나이아가라 함수 라이브러리 헤더 파일
#include "NiagaraSystem.h"  // 나이아가라 시스템 헤더 파일

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "ShootingGameHUD.h"
#include "ShootingGameInstance.h"
#include "ShootingPlayerState.h"


// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;
	Mesh->SetCollisionProfileName("Weapon");
	Mesh->SetSimulatePhysics(true);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(RootComponent);

	bReplicates = true;
	SetReplicateMovement(true);

	Ammo = 30;
}

void AWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, OwnChar);
	DOREPLIFETIME(AWeapon, RowName);
	DOREPLIFETIME(AWeapon, Ammo);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::PressTrigger_Implementation(bool isPressed)
{
	if(isPressed)
		OwnChar->PlayAnimMontage(WeaponData->ShootMontage);
}

void AWeapon::NotifyShoot_Implementation()
{
	bool isUse = false;
	IsCanUse(isUse);

	if (isUse == false)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponData->FireEffect, Mesh->GetSocketLocation("Muzzle"), Mesh->GetSocketRotation("Muzzle"), FVector(0.3f, 0.3f, 0.3f));

	Audio->Play();

	APlayerController* shooter = GetWorld()->GetFirstPlayerController();
	if (shooter == OwnChar->GetController())
	{
		FVector forward = shooter->PlayerCameraManager->GetActorForwardVector();

		FVector start = (forward * 350) + shooter->PlayerCameraManager->GetCameraLocation();
		FVector end = (forward * 5000) + shooter->PlayerCameraManager->GetCameraLocation();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Client - ReqShoot")));
		ReqShoot(start, end);
		ReqBlood(start, end);
	}
}

void AWeapon::NotifyReload_Implementation()
{
	DoReload();
}

void AWeapon::PressReload_Implementation()
{
	check(OwnChar);

	AShootingPlayerState* playerState = Cast<AShootingPlayerState>(OwnChar->GetPlayerState());
	if (playerState == nullptr)
		return;

	if (playerState->IsCanUseMag() == false)
		return;

	OwnChar->PlayAnimMontage(WeaponData->ReloadMontage);
}

void AWeapon::IsCanUse_Implementation(bool& IsCanUse)
{
	if (Ammo <= 0)
	{
		IsCanUse = false;
		return;
	}

	IsCanUse = true;
}

void AWeapon::AttachWeapon_Implementation(ACharacter* targetChar)
{
	OwnChar = targetChar;

	Mesh->SetSimulatePhysics(false);

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttachToComponent(targetChar->GetMesh()
		, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("weapon"));

	UpdateAmmoToHud(Ammo);
}

void AWeapon::DetachWeapon_Implementation(ACharacter* targetChar)
{
	UpdateAmmoToHud(0);

	OwnChar = nullptr;

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Mesh->SetSimulatePhysics(true);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeapon::OnRep_Ammo()
{
	UpdateAmmoToHud(Ammo);
}

void AWeapon::OnRep_RowName()
{
	UShootingGameInstance* gameInstance = Cast<UShootingGameInstance>(GetGameInstance());
	WeaponData = gameInstance->GetWeaponRowData(RowName);
	Mesh->SetStaticMesh(WeaponData->StaticMesh);
	Audio->SetSound(WeaponData->SoundBase);
}

void AWeapon::UpdateAmmoToHud(int NewAmmo)
{
	//UI 출력 연결
	APlayerController* firstPlayer = GetWorld()->GetFirstPlayerController();

	if (OwnChar && OwnChar->GetController() == firstPlayer)
	{
		AShootingGameHUD* Hud = Cast<AShootingGameHUD>(firstPlayer->GetHUD());
		if (IsValid(Hud))
		{
			Hud->OnUpdateMyAmmo(NewAmmo);
		}
	}
}

void AWeapon::DoReload()
{
	check(OwnChar);

	AShootingPlayerState* playerState = Cast<AShootingPlayerState>(OwnChar->GetPlayerState());
	if (playerState == nullptr)
		return;

	if (playerState->UseMag() == false)
		return;

	Ammo = WeaponData->MaxAmmo;
	OnRep_Ammo();
}

bool AWeapon::UseAmmo()
{
	if (Ammo <= 0)
	{
		return false;
	}

	Ammo = Ammo - 1;
	OnRep_Ammo();
	return true;
}

void AWeapon::SetRowName(FName name)
{
	RowName = name;

	OnRep_RowName();
}



void AWeapon::ReqShoot_Implementation(const FVector vStart, const FVector vEnd)
{
	if (UseAmmo() == false)
		return;

	FHitResult result;
	bool isHit = GetWorld()->LineTraceSingleByObjectType(result, vStart, vEnd, ECollisionChannel::ECC_Pawn);

	//DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Yellow, false, 5.0f);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Server - ReqShoot")));

	if (isHit)
	{
		ACharacter* HitChar = Cast<ACharacter>(result.GetActor());
		if (HitChar)
		{
			UGameplayStatics::ApplyDamage(HitChar, 10, OwnChar->GetController(), this, UDamageType::StaticClass());
			
			
		}
	}


}



void  AWeapon::ResBlood_Implementation(const FVector vStart, const FVector vEnd)
{
	if (UseAmmo() == false)
		return;

	FHitResult result;
	bool isHit = GetWorld()->LineTraceSingleByObjectType(result, vStart, vEnd, ECollisionChannel::ECC_Pawn);

	
	if (isHit)
	{
		ACharacter* HitChar = Cast<ACharacter>(result.GetActor());
		if (HitChar)
		{
			
			
			// 나이아가라 시스템 애셋 로드
			FString NiagaraSystemPath = "NiagaraSystem'/Game/BloodFX/FX/NS_BloodImpact_Medium.NS_BloodImpact_Medium'"; // 나이아가라 시스템 애셋의 경로
			UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr, *NiagaraSystemPath));

			// 나이아가라 시스템 컴포넌트 생성 및 초기화
			if (NiagaraSystem)
			{
				FVector HitLocation = result.Location; // 맞은 위치
				FRotator HitRotation = result.ImpactNormal.Rotation(); // 맞은 위치의 법선을 회전값으로 변환하여 컴포넌트의 초기 회전값으로 사용
				UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, HitLocation, HitRotation);
				if (NiagaraComponent)
				{
					NiagaraComponent->ActivateSystem();
					NiagaraComponent->SetAutoDestroy(true);
					NiagaraComponent->RegisterComponent();
				}
			}
		}
	}


}

void  AWeapon::ReqBlood_Implementation(const FVector vStart, const FVector vEnd)
{
	ResBlood(vStart, vEnd);
}
