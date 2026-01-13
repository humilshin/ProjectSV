// Shin YeongSeop 2026

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"

/**
 * Base GameMode for ProjectSV
 * Handles Dedicated Server session registration automatically
 */
UCLASS()
class PROJECTSV_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;

	// Server name for session registration
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Server")
	FString ServerName = TEXT("ProjectSV Server");

	// Max players for session
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Server")
	int32 MaxPlayers = 16;

	// Match type for session search
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Server")
	FString MatchType = TEXT("FreeForAll");

protected:
	// Register dedicated server session with Steam
	void RegisterDedicatedServerSession();

	// Handle session creation complete
	UFUNCTION()
	void OnSessionCreated(bool bWasSuccessful);

private:
	bool bSessionRegistered = false;
};