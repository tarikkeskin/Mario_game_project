#include "SampleClimbableVolume.h"
#include "Components/BoxComponent.h"
#include "SampleCharacter.h"

ASampleClimbableVolume::ASampleClimbableVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComponent->InitBoxExtent(FVector(16.0f, 16.0f, 16.0f));
	BoxComponent->SetupAttachment(RootComponent);
}

/*
 *
    Detection of  overlapping with the character:
 *
 */
void ASampleClimbableVolume::NotifyActorBeginOverlap(class AActor* Other)
{
    Super::NotifyActorBeginOverlap(Other);

    if (IsValid(Other) && !IsPendingKill())
    {
        StaticCast<ASampleCharacter*>(Other)->AddClimbableVolume(this);
    }
}

void ASampleClimbableVolume::NotifyActorEndOverlap(class AActor* Other)
{
    Super::NotifyActorEndOverlap(Other);

    if (IsValid(Other) && !IsPendingKill())
    {
        StaticCast<ASampleCharacter*>(Other)->RemoveClimbableVolume(this);
    }
}
