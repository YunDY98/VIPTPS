// Fill out your copyright notice in the Description page of Project Settings.


#include "CountActor.h"
#include "ShootingGameCharacter.h"
// Sets default values
ACountActor::ACountActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACountActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACountActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ULevel* CurrentLevel = GetWorld()->GetCurrentLevel();

	CharacterCount = 0;
	// Iterate over all actors in the level and count the number of characters
	for (AActor* Actor : CurrentLevel->Actors)
	{


		if (ACharacter* Character = Cast<ACharacter>(Actor))
		{
			if (CurrentCnt < CharacterCount)
			{
				CurrentCnt = CharacterCount++;

			}
		}
	}
}

