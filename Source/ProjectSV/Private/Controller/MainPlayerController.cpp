// Shin YeongSeop 2026

#include "Controller/MainPlayerController.h"
#include "EnhancedInputSubsystems.h"


AMainPlayerController::AMainPlayerController()
	:
	InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr)
	{}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem
			= LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}

