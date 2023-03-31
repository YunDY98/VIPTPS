// Fill out your copyright notice in the Description page of Project Settings.


#include "MagObject.h"
#include "ItemInterface.h"

// Sets default values
AMagObject::AMagObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = StaticMesh;

	StaticMesh->SetCollisionProfileName("OverlapAllDynamic");

	StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &AMagObject::OnBeginOverlap);

}

// Called when the game starts or when spawned
void AMagObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagObject::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("OnBeginOverlap OtherActor=%s"), *OtherActor->GetName()));

	if (HasAuthority())
	{
		IItemInterface* InterfaceObj = Cast<IItemInterface>(OtherActor);

		if (InterfaceObj)
		{
			InterfaceObj->Execute_AddMag(OtherActor);
		}

		Destroy();
	}
}

