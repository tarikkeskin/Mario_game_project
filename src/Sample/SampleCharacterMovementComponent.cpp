#include "SampleCharacterMovementComponent.h"
#include "SampleCharacter.h"
#include "GameFramework/Character.h"

USampleCharacterMovementComponent::USampleCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bClimbEnabled(false)
    , BrakingDecelerationClimbing(0.0f)
    , ClimbCooldown(0.0f)
    , ClimbTimer(0.0f)
    , bWantsToClimb(false)
{}

float USampleCharacterMovementComponent::GetMaxSpeed() const
{
    if (IsClimbing())
    {
        return MaxClimbSpeed;
    }

    return Super::GetMaxSpeed();
}

float USampleCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
    if (IsClimbing())
    {
        return BrakingDecelerationClimbing;
    }

    return Super::GetMaxBrakingDeceleration();
}

//Allow to jump when climbing
bool USampleCharacterMovementComponent::CanAttemptJump() const
{
    if (CanEverJump() && IsClimbing())
    {
        return true;
    }

    return Super::CanAttemptJump();
}

bool USampleCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
    bool bWasClimbing = IsClimbing();

    if (Super::DoJump(bReplayingMoves))
    {
        if (bWasClimbing)
        {
            ClimbTimer = ClimbCooldown;
        }

        return true;
    }

    return false;
}

// Apply bWantsToClimb before movement

void USampleCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

    if (ClimbTimer > 0.0f)
    {
        ClimbTimer -= DeltaSeconds;
        if (ClimbTimer < 0.0f)
        {
            ClimbTimer = 0.0f;
        }
    }
    if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
    {
        // Check for a change in climb state. Players toggle climb by changing bWantsToClimb.
        const bool bIsClimbing = IsClimbing();
        if (bIsClimbing && (!bWantsToClimb || !CanClimbInCurrentState()))
        {
            UnClimb(false);
        }
        else if (!bIsClimbing && bWantsToClimb && CanClimbInCurrentState())
        {
            Climb(false);
        }
    }
}

void USampleCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

    // Proxies get replicated climb state.
    if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
    {
        // Unclimb if no longer allowed to be climbing
        if (IsClimbing() && !CanClimbInCurrentState())
        {
            UnClimb(false);
        }
    }
}

bool USampleCharacterMovementComponent::CanClimbInCurrentState() const
{
    return bClimbEnabled && ClimbTimer <= 0.0f && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

//If we are in the MOVE_Climbing movement mode
bool USampleCharacterMovementComponent::IsClimbing() const
{
    return (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ESampleMovementMode::MOVE_Climbing) && UpdatedComponent;
}

//Change movement mode to MOVE_Climbing
void USampleCharacterMovementComponent::Climb(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanClimbInCurrentState())
	{
		return;
	}

    ASampleCharacter* Owner = StaticCast<ASampleCharacter*>(CharacterOwner);
    SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ESampleMovementMode::MOVE_Climbing);
    Velocity = FVector::ZeroVector;
    Owner->ResetJumpState();
}

// Change movement mode to MOVE_Falling
void USampleCharacterMovementComponent::UnClimb(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

    ASampleCharacter* Owner = StaticCast<ASampleCharacter*>(CharacterOwner);
    SetMovementMode(EMovementMode::MOVE_Falling);
    ClimbTimer = ClimbCooldown;
}
// Custom physics for MOVE_Climbing movement mode
void USampleCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    switch ((ESampleMovementMode)CustomMovementMode)
    {
    case ESampleMovementMode::MOVE_Climbing:
        PhysCustomClimbing(deltaTime, Iterations);
        break;
    default:
        Super::PhysCustom(deltaTime, Iterations);
    }
}

void USampleCharacterMovementComponent::PhysCustomClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

    RestorePreAdditiveRootMotionVelocity();

	// Apply acceleration
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, GroundFriction, false, GetMaxBrakingDeceleration());
	}

    ApplyRootMotionToVelocity(deltaTime);

    Iterations++;
    bJustTeleported = false;

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FVector Adjusted = Velocity * deltaTime;
    FHitResult Hit(1.f);
    SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }
}




