// Shin YeongSeop 2026
#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Controller/MainPlayerController.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	NormalSpeed = 300.0f;
	SprintSpeedMultiplier = 2.0f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMainPlayerController* MainPlayerController = Cast<AMainPlayerController>(GetController()))
		{
			if (MainPlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					MainPlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::Move
				);
			}
			
			if (MainPlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					MainPlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::Look
				);
			}
			
			if (MainPlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					MainPlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerCharacter::StartJump
				);
				
				EnhancedInput->BindAction(
					MainPlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&APlayerCharacter::StopJump
				);
			}
            
			if (MainPlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					MainPlayerController->SprintAction,
					ETriggerEvent::Triggered, 
					this, 
					&APlayerCharacter::StartSprint
				);
				EnhancedInput->BindAction(
					MainPlayerController->SprintAction, 
					ETriggerEvent::Completed, 
					this, 
					&APlayerCharacter::StopSprint
				);
			}    
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	if (Controller == nullptr) return;
    
	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
    
	AddMovementInput(Forward, InputVector.Y);
	AddMovementInput(Right, InputVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	if (Controller == nullptr) return;
	
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}


void APlayerCharacter::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Super::Jump();
	}
}

void APlayerCharacter::StopJump(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		Super::StopJumping();
	}
}

void APlayerCharacter::StartSprint(const FInputActionValue& Value) 
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value) 
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}


