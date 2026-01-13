// Shin YeongSeop 2026

#include "AnimInstance/BaseAnimInstance.h"
#include "Character/BaseCharacter.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"



void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BaseCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());
}


void UBaseAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BaseCharacter == nullptr)
	{
		BaseCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());
	}
	if (BaseCharacter == nullptr) return;

	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, BaseCharacter->GetActorRotation());
	
	Velocity = BaseCharacter->GetVelocity();
	
	FVector GroundVelocity;
	GroundVelocity.X = Velocity.X;
	GroundVelocity.Y = Velocity.Y;
	GroundVelocity.Z = 0.f;
	GroundSpeed = GroundVelocity.Size();

	bIsInAir = BaseCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ( GroundSpeed > 3.0f) && ( BaseCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ) ? true : false;
}