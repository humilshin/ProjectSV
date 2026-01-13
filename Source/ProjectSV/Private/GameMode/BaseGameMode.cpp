// Shin YeongSeop 2026

#include "GameMode/BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MultiplayerSessionsSubsystem.h"

ABaseGameMode::ABaseGameMode()
{
	// Default settings
}

void ABaseGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Parse command line options for server name
	if (UGameplayStatics::HasOption(Options, TEXT("ServerName")))
	{
		ServerName = UGameplayStatics::ParseOption(Options, TEXT("ServerName"));
	}

	// Parse max players
	if (UGameplayStatics::HasOption(Options, TEXT("MaxPlayers")))
	{
		FString ParsedMaxPlayers = UGameplayStatics::ParseOption(Options, TEXT("MaxPlayers"));
		MaxPlayers = FCString::Atoi(*ParsedMaxPlayers);
	}

	// Parse match type
	if (UGameplayStatics::HasOption(Options, TEXT("MatchType")))
	{
		MatchType = UGameplayStatics::ParseOption(Options, TEXT("MatchType"));
	}

	UE_LOG(LogTemp, Log, TEXT("BaseGameMode::InitGame - Map: %s, ServerName: %s, MaxPlayers: %d"), *MapName, *ServerName, MaxPlayers);
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Register session for Dedicated Server
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		RegisterDedicatedServerSession();
	}
}

void ABaseGameMode::RegisterDedicatedServerSession()
{
	if (bSessionRegistered)
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseGameMode: Session already registered"));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseGameMode: No GameInstance available"));
		return;
	}

	UMultiplayerSessionsSubsystem* SessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (!SessionsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseGameMode: MultiplayerSessionsSubsystem not found"));
		return;
	}

	// Bind to session creation complete delegate
	SessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ABaseGameMode::OnSessionCreated);

	// Create the dedicated server session
	UE_LOG(LogTemp, Log, TEXT("BaseGameMode: Registering dedicated server session '%s' with %d max players"), *ServerName, MaxPlayers);
	SessionsSubsystem->CreateServerSession(MaxPlayers, MatchType, ServerName);
}

void ABaseGameMode::OnSessionCreated(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		bSessionRegistered = true;
		UE_LOG(LogTemp, Log, TEXT("BaseGameMode: Dedicated server session registered successfully"));

		// Optionally start the session immediately
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UMultiplayerSessionsSubsystem* SessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
			if (SessionsSubsystem)
			{
				SessionsSubsystem->StartSession();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BaseGameMode: Failed to register dedicated server session"));

		// Retry after delay
		FTimerHandle RetryTimerHandle;
		GetWorldTimerManager().SetTimer(RetryTimerHandle, this, &ABaseGameMode::RegisterDedicatedServerSession, 5.0f, false);
	}
}