// Shin YeongSeop 2026

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * Server Browser Menu Widget
 * Allows clients to find and join dedicated servers
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// Setup the menu with search parameters
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void MenuSetup(FString TypeOfMatch = FString(TEXT("FreeForAll")));

	// Refresh the server list
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void RefreshServerList();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// Callbacks for MultiplayerSessionsSubsystem delegates
	//
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	// UI Elements
	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget))
	class UButton* RefreshButton;

	// Button callbacks
	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void RefreshButtonClicked();

	void MenuTearDown();

	// Subsystem reference
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	// Search parameters
	FString MatchType{ TEXT("FreeForAll") };

	// Cached search results
	TArray<FOnlineSessionSearchResult> CachedSessionResults;
};
