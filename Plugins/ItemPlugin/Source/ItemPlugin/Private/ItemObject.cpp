// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemObject.h"
#include "ItemInterface.h"

// Sets default values
AItemObject::AItemObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = StaticMesh;

	StaticMesh->SetCollisionProfileName("OverlapAllDynamic");

	StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &AItemObject::OnBeginOverlap);
}

// Called when the game starts or when spawned
void AItemObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemObject::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("OnBeginOverlap OtherActor=%s"), *OtherActor->GetName()));

	IItemInterface* InterfaceObj = Cast<IItemInterface>(OtherActor);

	if (InterfaceObj)
	{
		InterfaceObj->Execute_AddHeal(OtherActor, 10.0f);
	}
}

