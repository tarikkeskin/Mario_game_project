#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "SampleCharacterMovementComponent.generated.h"

enum class ESampleMovementMode : uint8
{
    MOVE_None = 0,
    MOVE_Climbing
};

UCLASS(ClassGroup = "Sample")
class SAMPLE_API USampleCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    USampleCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

    virtual float GetMaxSpeed() const override;
    virtual float GetMaxBrakingDeceleration() const;
    virtual bool CanAttemptJump() const override;
    virtual bool DoJump(bool bReplayingMoves) override;
    virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
    virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
    virtual bool CanClimbInCurrentState() const;
    virtual bool IsClimbing() const;
    virtual void Climb(bool bClientSimulation);
    virtual void UnClimb(bool bClientSimulation);
    virtual void PhysCustom(float deltaTime, int32 Iterations) override;
    virtual void PhysCustomClimbing(float deltaTime, int32 Iterations);

    
     //try to climb  on next update. If false try to stop climbing on next update.
    UPROPERTY(Category = "Sample", VisibleInstanceOnly, BlueprintReadOnly)
    bool bClimbEnabled;
    
	// The maximum climbing speed
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxClimbSpeed;

	
	// Deceleration when climbing and not applying acceleration.
    UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float BrakingDecelerationClimbing;

    
    // Cooldown before the character can climb again after leaving the climbing state
    UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
    float ClimbCooldown;

    //Remaining time before the character can climb again.
    UPROPERTY(Category = "Sample", VisibleInstanceOnly, BlueprintReadOnly)
    float ClimbTimer;

    // try to climb  on next update. If false  try to stop climbing on next update
    UPROPERTY(Category = "Sample", VisibleInstanceOnly, BlueprintReadOnly)
    bool bWantsToClimb;
};

