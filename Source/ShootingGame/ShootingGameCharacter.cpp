// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShootingGameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "WeaponInterface.h"
#include "ShootingPlayerState.h"
#include "Weapon.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "NameTagInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "ShootingGameInstance.h"
//#include "NiagaraComponent.h"  // 나이아가라 컴포넌트 헤더 파일
//#include "NiagaraFunctionLibrary.h"  // 나이아가라 함수 라이브러리 헤더 파일
//#include "NiagaraSystem.h"  // 나이아가라 시스템 헤더 파일

//////////////////////////////////////////////////////////////////////////
// AShootingGameCharacter




//
AShootingGameCharacter::AShootingGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	ConstructorHelpers::FObjectFinder<UAnimMontage> montage(TEXT("AnimMontage'/Game/RifleAnimsetPro/Animations/InPlace/Rifle_ShootOnce_Montage.Rifle_ShootOnce_Montage'"));

	AnimMontage = montage.Object;

	IsRagdoll = false;
}

void AShootingGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	// UGameplayStatics를 사용하여 현재 게임 인스턴스에 액세스합니다.
	UGameInstance* CurrentGameInstance = GetGameInstance();

	// 해당 클래스로 캐스팅하기 전에, 게임 인스턴스가 널(null)이 아닌지 확인합니다.
	if (CurrentGameInstance)
	{
		// 현재 게임 인스턴스를 사용자 지정 게임 인스턴스 클래스(예: MyGameInstance)로 캐스팅합니다.
		UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(CurrentGameInstance);

		// 캐스팅이 성공적으로 수행된 경우, CustomGameInstance 변수를 사용하여 사용자 지정 게임 인스턴스에 액세스할 수 있습니다.
		if (GameInstance)
		{
			// 여기에 사용자 지정 게임 인스턴스 클래스 내 함수 호출 및 기타 작업을 수행할 수 있습니다.
			GameInstance->CharCount = CharCount();
		}
	}
	
	
	BindPlayerState();
}

void AShootingGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() == true)
	{
		ControlPitch = GetControlRotation().Pitch;
	}

	if (IsRagdoll)
	{
		SetActorLocation(GetMesh()->GetSocketLocation("spine_01") + FVector(0.0f, 0.0f, 60.0f));

		
	}

	
	
	
	
}

float AShootingGameCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
		FString::Printf(TEXT("TakeDamage Damage=%f EventInstigator=%s"), DamageAmount, *EventInstigator->GetName()));

	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (ps)
	{
		ps->AddDamage(DamageAmount);
		// 나이아가라 시스템 애셋 로드
		//FString NiagaraSystemPath = "NiagaraSystem'/Game/BloodFX/FX/NS_BloodImpact_Medium.NS_BloodImpact_Medium'"; // 나이아가라 시스템 애셋의 경로
		//UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr, *NiagaraSystemPath));

		//// 나이아가라 시스템 컴포넌트 생성 및 초기화
		//if (NiagaraSystem)
		//{
		//	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraSystem, GetActorLocation());
		//	if (NiagaraComponent)
		//	{
		//		NiagaraComponent->ActivateSystem();
		//		NiagaraComponent->SetAutoDestroy(true);
		//		NiagaraComponent->RegisterComponent();
		//	}
		//}
	}

	return 0.0f;
}

void AShootingGameCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShootingGameCharacter, ControlPitch);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AShootingGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShootingGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShootingGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AShootingGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AShootingGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AShootingGameCharacter::OnResetVR);

	// Shoot
	PlayerInputComponent->BindAction("Trigger", IE_Pressed, this, &AShootingGameCharacter::PressTrigger);
	PlayerInputComponent->BindAction("Trigger", IE_Released, this, &AShootingGameCharacter::ReleaseTrigger);

	// Reload
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShootingGameCharacter::PressReload);

	// PickUp
	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &AShootingGameCharacter::PressPickUp);

	// DropWeapon
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AShootingGameCharacter::PressDropWeapon);

	// MagTest
	PlayerInputComponent->BindAction("MagTest", IE_Pressed, this, &AShootingGameCharacter::PressMagTest);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AShootingGameCharacter::Menu);
}


AActor* AShootingGameCharacter::SetEquipWeapon(AActor* Weapon)
{
	if (IsValid(EquipWeapon))
	{
		DisableOwnerWeapon();
	}

	ResPickUp(Weapon);

	EnableOwnerWeapon(Weapon);

	return EquipWeapon;
}

void AShootingGameCharacter::OnNotifyShoot()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_NotifyShoot(EquipWeapon);
	}
}

void AShootingGameCharacter::OnNotifyReload()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_NotifyReload(EquipWeapon);
	}
}

void AShootingGameCharacter::OnUpdateHp_Implementation(float CurrentHp, float MaxHp)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("OnUpdateHp CurrentHp : %f"), CurrentHp));

	if (CurrentHp <= 0)
	{
		//this->Destroy();
		DoRagdoll();
		
		if (!HasAuthority())
		{
			MyCurHp();
		}

		//if (CharCount() <= 2 && HasAuthority())

		//{
		//	this->Destroy();
		//}
		
		// UGameplayStatics를 사용하여 현재 게임 인스턴스에 액세스합니다.
		UGameInstance* CurrentGameInstance = GetGameInstance();

		// 해당 클래스로 캐스팅하기 전에, 게임 인스턴스가 널(null)이 아닌지 확인합니다.
		if (CurrentGameInstance)
		{
			// 현재 게임 인스턴스를 사용자 지정 게임 인스턴스 클래스(예: MyGameInstance)로 캐스팅합니다.
			UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(CurrentGameInstance);

			// 캐스팅이 성공적으로 수행된 경우, CustomGameInstance 변수를 사용하여 사용자 지정 게임 인스턴스에 액세스할 수 있습니다.
			if (GameInstance)
			{
				// 여기에 사용자 지정 게임 인스턴스 클래스 내 함수 호출 및 기타 작업을 수행할 수 있습니다.
				GameInstance->CharCount -= 1;
				if (GameInstance->CharCount <= 2 && HasAuthority())
				{
					//this->Destroy();
				}

				
			}
		}

		
		
	}
}

void AShootingGameCharacter::DoRagdoll()
{
	IsRagdoll = true;
	
	
	


	GetMesh()->SetSimulatePhysics(true);
}

void AShootingGameCharacter::DoGetup()
{
	IsRagdoll = false;

	GetMesh()->SetSimulatePhysics(false);

	GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	FVector loc = { 0.0f, 0.0f, -97.0f };
	FRotator Rot = { 0.0f, 270.0f, 0.0f };
	GetMesh()->SetRelativeLocationAndRotation(loc, Rot);
}

void AShootingGameCharacter::AddHeal_Implementation(float Heal)
{
	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (ps)
	{
		ps->AddHeal(Heal);
	}
}

void AShootingGameCharacter::AddMag_Implementation()
{
	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (ps)
	{
		ps->AddMag();
	}
}



void AShootingGameCharacter::ReqPressTrigger_Implementation(bool isPressed)
{
	ResPressTrigger(isPressed);
}

void AShootingGameCharacter::ResPressTrigger_Implementation(bool isPressed)
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_PressTrigger(EquipWeapon, isPressed);
	}
}

void AShootingGameCharacter::ReqPressC_Implementation()
{
	ResPressC();
}

void AShootingGameCharacter::ResPressC_Implementation()
{
	if (IsRagdoll)
	{
		DoGetup();
	}
	else
	{
		DoRagdoll();
	}
}

void AShootingGameCharacter::ReqPressReload_Implementation()
{
	ResPressReload();
}

void AShootingGameCharacter::ResPressReload_Implementation()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_PressReload(EquipWeapon);
	}
}

void AShootingGameCharacter::ReqPickUp_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("PressPickUp")));

	AActor* nearestActor = GetNearestWeapon();
	if (nearestActor)
	{
		SetEquipWeapon(nearestActor);
	}
}

void AShootingGameCharacter::ResPickUp_Implementation(AActor* weapon)
{
	if (IsValid(EquipWeapon))
	{
		IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);
		if (InterfaceObj)
		{
			InterfaceObj->Execute_DetachWeapon(EquipWeapon, this);
		}
	}

	AttachWeapon(weapon);
}





void AShootingGameCharacter::ReqGameEnd_Implementation()
{
	ResGameEnd();

}

void AShootingGameCharacter::ResGameEnd_Implementation()
{
	if (WBP_GameOver)
	{
		// Create the UI widget
		UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_GameOver);
		if (WBP_GameOver)
		{
			// Add the UI widget to the viewport
			MyBlueprintUIWidget->AddToViewport();
			APlayerController* MyController = GetWorld()->GetFirstPlayerController();
			MyController->SetInputMode(FInputModeUIOnly());
			MyController->bShowMouseCursor = true;
		}
	}
	
}
void AShootingGameCharacter::OnResetVR()
{
	// If ShootingGame is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in ShootingGame.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AShootingGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AShootingGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AShootingGameCharacter::PressTrigger()
{
	ReqPressTrigger(true);
	
}

void AShootingGameCharacter::ReleaseTrigger()
{
	ReqPressTrigger(false);
}

void AShootingGameCharacter::PressTestKey()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("PressTestKey")));

	ReqPressC();
}

void AShootingGameCharacter::BindPlayerState()
{
	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (IsValid(ps))
	{
		ps->Fuc_Dele_UpdateHp_TwoParams.AddUFunction(this, FName("OnUpdateHp"));
		OnUpdateHp(ps->GetCurHp(), ps->GetMaxHp());
		return;
	}

	FTimerManager& timerManager = GetWorld()->GetTimerManager();
	timerManager.SetTimer(th_BindPlayerState, this, &AShootingGameCharacter::BindPlayerState, 0.1f, false);
}

void AShootingGameCharacter::PressReload()
{
	ReqPressReload();
}

void AShootingGameCharacter::PressPickUp()
{
	ReqPickUp();
}

void AShootingGameCharacter::PressDropWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("PressDropWeapon")));

	ReqDropWeapon();
}

void AShootingGameCharacter::PressMagTest()
{
	AShootingPlayerState* ps = Cast<AShootingPlayerState>(GetPlayerState());
	if (ps)
	{
		ps->AddMag();
	}
}

void AShootingGameCharacter::Menu()
{
	Winner();
	if (IsRagdoll)
	{
		if (!HasAuthority())
		{



			if (WBP_GameOver)
			{
				// Create the UI widget
				UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_GameOver);
				if (WBP_GameOver)
				{
					// Add the UI widget to the viewport
					MyBlueprintUIWidget->AddToViewport();
					APlayerController* MyController = GetWorld()->GetFirstPlayerController();
					MyController->SetInputMode(FInputModeUIOnly());
					MyController->bShowMouseCursor = true;
				}
			}
		}
		else
		{
		
			this->Destroy();

			if (WBP_DontOut)
			{
				// Create the UI widget
				UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_DontOut);
				if (WBP_DontOut)
				{
					// Add the UI widget to the viewport
					MyBlueprintUIWidget->AddToViewport();
					APlayerController* MyController = GetWorld()->GetFirstPlayerController();
					MyController->SetInputMode(FInputModeUIOnly());
					MyController->bShowMouseCursor = true;
				}
			}
			
		}
		
	}
	
	// Get a reference to the current level


	
	//UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_GameOver);
	//if (WBP_GameOver)
	//{
	//	APlayerController* MyController = GetWorld()->GetFirstPlayerController();
	//	
	//	if (UiCheck)
	//	{
	//		// Add the UI widget to the viewport
	//		MyBlueprintUIWidget->AddToViewport();

	//		FInputModeUIOnly InputModeUIOnly;
	//		InputModeUIOnly.SetWidgetToFocus(MyBlueprintUIWidget->TakeWidget());
	//		InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);

	//		MyController->SetInputMode(InputModeUIOnly);
	//		MyController->bShowMouseCursor = true;
	//		UiCheck = false;
	//	}
	//	else
	//	{
	//		MyBlueprintUIWidget->RemoveFromViewport();

	//		FInputModeGameOnly InputModeGameOnly;
	//		MyController->SetInputMode(InputModeGameOnly);

	//		MyController->bShowMouseCursor = false;
	//		UiCheck = true;
	//	}
	//		
	//		
	//	
			
		
			
		
	}


	
	
}

void AShootingGameCharacter::ReqDropWeapon_Implementation()
{
	DisableOwnerWeapon();
	ResDropWeapon();
}

void AShootingGameCharacter::ResDropWeapon_Implementation()
{
	DetachWeapon(EquipWeapon);
}

void AShootingGameCharacter::OnRep_EquipWeapon()
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_AttachWeapon(EquipWeapon, this);
	}
}

AActor* AShootingGameCharacter::GetNearestWeapon()
{
	float nearestDist = 9999999.0f;
	AActor* nearestActor = nullptr;
	TArray<AActor*> actors;
	GetCapsuleComponent()->GetOverlappingActors(actors, AWeapon::StaticClass());

	for (AActor* weapon : actors)
	{
		IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(weapon);

		if (InterfaceObj == nullptr)
			continue;

		float dist = FVector::Distance(GetActorLocation(), weapon->GetActorLocation());

		if (dist > nearestDist)
			continue;

		nearestDist = dist;
		nearestActor = weapon;
	}

	return nearestActor;
}

void AShootingGameCharacter::EnableOwnerWeapon(AActor* actor)
{
	actor->SetOwner(GetController());
}

void AShootingGameCharacter::DisableOwnerWeapon()
{
	if(IsValid(EquipWeapon))
		EquipWeapon->SetOwner(nullptr);
}

void AShootingGameCharacter::AttachWeapon(AActor* weapon)
{
	EquipWeapon = weapon;

	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(weapon);
	if (InterfaceObj)
	{
		InterfaceObj->Execute_AttachWeapon(weapon, this);
	}

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AShootingGameCharacter::DetachWeapon(AActor* weapon)
{
	IWeaponInterface* InterfaceObj = Cast<IWeaponInterface>(EquipWeapon);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_DetachWeapon(EquipWeapon, this);
	}

	EquipWeapon = nullptr;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AShootingGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShootingGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShootingGameCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AShootingGameCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	
}

void  AShootingGameCharacter::LoadLevel()
{
	
	UWorld* World = GetWorld(); // 현재 월드 얻기
	if (World)
	{
		FString LevelToLoad = "Entry"; // 여기에 원하는 레벨 이름 적기
		UGameplayStatics::OpenLevel(World, FName(*LevelToLoad));
	}
	
	
}

void AShootingGameCharacter::LoadLevelDelay()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("delatyasfddddddddddddddddd")));


	float DelaySeconds = 3.0f;


	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AShootingGameCharacter::LoadLevel, 3.0f, false);
}

void AShootingGameCharacter::MyCurHp()
{
	
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		AShootingPlayerState* CustomPlayerState = Cast<AShootingPlayerState>(PlayerController->PlayerState);
		if (CustomPlayerState)
		{
			if (0 >= CustomPlayerState->MyCurHp())
			{
				LoadLevel();
			}
			// UGameplayStatics를 사용하여 현재 게임 인스턴스에 액세스합니다.
			UGameInstance* CurrentGameInstance = GetGameInstance();

			// 해당 클래스로 캐스팅하기 전에, 게임 인스턴스가 널(null)이 아닌지 확인합니다.
			if (CurrentGameInstance)
			{
				// 현재 게임 인스턴스를 사용자 지정 게임 인스턴스 클래스(예: MyGameInstance)로 캐스팅합니다.
				UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(CurrentGameInstance);

				// 캐스팅이 성공적으로 수행된 경우, CustomGameInstance 변수를 사용하여 사용자 지정 게임 인스턴스에 액세스할 수 있습니다.
				if (GameInstance)
				{
					// 여기에 사용자 지정 게임 인스턴스 클래스 내 함수 호출 및 기타 작업을 수행할 수 있습니다.
					
					if (GameInstance->CharCount==1 &&(0 < CustomPlayerState->MyCurHp()))
					{
						
							Winner();
		
						
					}


				}
			}
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CustomPlayerState 캐스팅에 실패했습니다."));
		}
	}
}

int AShootingGameCharacter::CharCount()
{
	ULevel* CurrentLevel = GetWorld()->GetCurrentLevel();
	CharacterCount = 0;

	// Iterate over all actors in the level and count the number of characters
	for (AActor* Actor : CurrentLevel->Actors)
	{


		if (ACharacter* Character = Cast<ACharacter>(Actor))
		{
			CharacterCount++;
		}





	}
	return CharacterCount;
}

void AShootingGameCharacter::Winner()
{
	// UGameplayStatics를 사용하여 현재 게임 인스턴스에 액세스합니다.
	UGameInstance* CurrentGameInstance = GetGameInstance();
	
	// 해당 클래스로 캐스팅하기 전에, 게임 인스턴스가 널(null)이 아닌지 확인합니다.
	if (CurrentGameInstance)
	{
		// 현재 게임 인스턴스를 사용자 지정 게임 인스턴스 클래스(예: MyGameInstance)로 캐스팅합니다.
		UShootingGameInstance* GameInstance = Cast<UShootingGameInstance>(CurrentGameInstance);

		if (GameInstance->CharCount == 1)
		{

			ReqGameEnd();
			if (WBP_GameEnd)
			{
				// Create the UI widget
				UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_GameEnd);
				if (WBP_GameEnd)
				{
					APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
					if (PlayerController)
					{
						AShootingPlayerState* CustomPlayerState = Cast<AShootingPlayerState>(PlayerController->PlayerState);
						if (CustomPlayerState)
						{
							if (0 < CustomPlayerState->MyCurHp())
							{
								// Add the UI widget to the viewport
								MyBlueprintUIWidget->AddToViewport();
								APlayerController* MyController = GetWorld()->GetFirstPlayerController();
								MyController->SetInputMode(FInputModeUIOnly());
								MyController->bShowMouseCursor = true;
							}
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("CustomPlayerState 캐스팅에 실패했습니다."));
						}
					}


				}
			}


		}
	}

	
	/*UUserWidget* MyBlueprintUIWidget = CreateWidget<UUserWidget>(GetWorld(), WBP_GameEnd);
	MyBlueprintUIWidget->AddToViewport();
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();
	MyController->SetInputMode(FInputModeUIOnly());
	MyController->bShowMouseCursor = true;*/
}


