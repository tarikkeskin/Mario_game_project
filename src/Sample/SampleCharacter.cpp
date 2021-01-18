#include "SampleCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SampleCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

// ASampleCharacter

ASampleCharacter::ASampleCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USampleCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ******** size of our collision capsule.********
	GetCapsuleComponent()->SetCapsuleHalfHeight(27.0f);
	GetCapsuleComponent()->SetCapsuleRadius(14.0f);

	// ******* an orthographic camera and attach the Root Comp.********
	SideViewCameraComponent=CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 512.0f;
	SideViewCameraComponent->AspectRatio = 8.0f / 7.0f;
	SideViewCameraComponent->SetupAttachment(RootComponent);

	SideViewCameraComponent->SetAbsolute(true, true);
	SideViewCameraComponent->SetWorldLocationAndRotation(
		FVector(256.0f, 1000.0f, -224.0f),
		FQuat::MakeFromEuler(FVector(0.0f, 0.0f, -90.0f))
	);

	// ****** character movement ********
	GetCharacterMovement()->GravityScale = 2.2f;
	GetCharacterMovement()->AirControl = 0.90f;
	GetCharacterMovement()->JumpZVelocity = 900.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 255.0f;
	GetCharacterMovement()->MaxFlySpeed = 650.0f;

	/* Lock character motion onto the X-Z plane,
         the character can't move in or out of the screen
     */
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
   // GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

/*
     	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
     	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
     	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
     	TextComponent->SetupAttachment(RootComponent);
*/
	// ***Enable replication on the Sprite component**********
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}

// ********** Begin play **********
void ASampleCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = (APlayerController*)Controller;

	if (PlayerController)
	{
		PlayerController->ConsoleCommand(TEXT("showflag.postprocessing 0"));
		PlayerController->ConsoleCommand(TEXT("r.SetRes 512x448w"));
	}
}

// ********** Animation **********

void ASampleCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* NextAnimation = nullptr;
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());
    
// ***check isClimbing to set the Next Flipbook animatiıon*****
	if (MoveComponent && MoveComponent->IsClimbing())
	{
        NextAnimation = (PlayerSpeedSqr > 0.0f) ? ClimbingAnimation : ClimbingFreeAnimation;
	}
	else
	{
        NextAnimation = (PlayerSpeedSqr > 0.0f) ? RunningAnimation : freeAnimation;
	}

	if(NextAnimation && GetSprite()->GetFlipbook() != NextAnimation)
	{
		GetSprite()->SetFlipbook(NextAnimation);
	}
}

void ASampleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateCharacter();	
}



void ASampleCharacter::MoveRight(float Value)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
}

void ASampleCharacter::MoveUp(float Value)
{
	// ******* if currently climbing , add MoveUP movement ***********
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());

	if (MoveComponent && MoveComponent->IsClimbing())
	{
		AddMovementInput(FVector(0.0f, 0.0f, 1.0f), Value);
	}
}

void ASampleCharacter::UpdateCharacter()
{
	UpdateAnimation();

	const FVector PlayerVelocity = GetVelocity();
	float TravelDirection = PlayerVelocity.X;
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}
}

/*
 *
 Enable climbing if the character is overlapping at least one climbing volume
 *
 */
void ASampleCharacter::AddClimbableVolume(ASampleClimbableVolume* Volume)
{
	Volumes.Add(Volume);
	SetClimbEnabled(true);
}

void ASampleCharacter::RemoveClimbableVolume(ASampleClimbableVolume* Volume)
{
	Volumes.Remove(Volume);
	if (Volumes.Num() == 0)
	{
		SetClimbEnabled(false);
	}
}

// **** set inputs ******
void ASampleCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{)
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this,&ACharacter::StopJumping);
    PlayerInputComponent->BindAction("Climb", IE_Pressed,this,&ASampleCharacter::StartClimb);
    PlayerInputComponent->BindAction("Climb", IE_Released, this,&ASampleCharacter::StopClimb);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASampleCharacter::MoveRight);
    PlayerInputComponent->BindAxis("MoveUp", this, &ASampleCharacter::MoveUp);
}
/*
 *
    toggle a boolean toggleClimb in the movement component
    when pressing/releasing the Climb input
 *
 */

void ASampleCharacter::SetClimbEnabled(bool bIsEnabled)
{
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());

	if (MoveComponent)
	{
		MoveComponent->bClimbEnabled = bIsEnabled;
	}
}

void ASampleCharacter::StartClimb()
{
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());

	if (MoveComponent)
	{
		if (CanClimb())
		{
			MoveComponent->toggleClimb = true;
		}
	}
}

void ASampleCharacter::StopClimb()
{
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());

	if (MoveComponent)
	{
		MoveComponent->toggleClimb = false;
	}
}

bool ASampleCharacter::CanClimb() const
{
	USampleCharacterMovementComponent* MoveComponent = Cast<USampleCharacterMovementComponent>(GetMovementComponent());

	if (!MoveComponent || MoveComponent->IsClimbing())
		return false;
	
	return GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}
