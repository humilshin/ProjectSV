// Shin YeongSeop 2026

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(FString TypeOfMatch)
{
	// Dedicated Server doesn't need UI
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	MatchType = TypeOfMatch;

	// Add to viewport
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	// Setup input mode for UI
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->IsLocalController())
			{
				FInputModeUIOnly InputModeData;
				InputModeData.SetWidgetToFocus(TakeWidget());
				InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PlayerController->SetInputMode(InputModeData);
				PlayerController->SetShowMouseCursor(true);
			}
		}
	}

	// Get subsystem and bind delegates
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	}

	// Auto-refresh server list on setup
	RefreshServerList();
}

void UMenu::RefreshServerList()
{
	if (MultiplayerSessionsSubsystem)
	{
		if (JoinButton)
		{
			JoinButton->SetIsEnabled(false);
		}
		if (RefreshButton)
		{
			RefreshButton->SetIsEnabled(false);
		}

		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &ThisClass::RefreshButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	// Cache results
	CachedSessionResults = SessionResults;

	// Re-enable buttons
	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(true);
	}

	if (bWasSuccessful && SessionResults.Num() > 0)
	{
		if (JoinButton)
		{
			JoinButton->SetIsEnabled(true);
		}

		// Log found servers
		for (const auto& Result : SessionResults)
		{
			FString ServerName;
			Result.Session.SessionSettings.Get(FName("ServerName"), ServerName);
			UE_LOG(LogTemp, Log, TEXT("Found server: %s"), *ServerName);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("Found server: %s"), *ServerName));
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("No servers found. Try refreshing."));
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Get server address and travel
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
			if (SessionInterface.IsValid())
			{
				FString Address;
				SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

				UE_LOG(LogTemp, Warning, TEXT("Travel Address: [%s]"), *Address);

				if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
				{
					// UI cleanup first
					MenuTearDown();

					PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to join server!"));
		}

		if (JoinButton)
		{
			JoinButton->SetIsEnabled(true);
		}
	}
}

void UMenu::JoinButtonClicked()
{
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(false);
	}

	if (MultiplayerSessionsSubsystem && CachedSessionResults.Num() > 0)
	{
		// Find first server matching our MatchType
		for (const auto& Result : CachedSessionResults)
		{
			FString SettingsValue;
			Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

			if (SettingsValue == MatchType)
			{
				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
		}

		// If no match found, just join the first available server
		MultiplayerSessionsSubsystem->JoinSession(CachedSessionResults[0]);
		return;
	}

	// No servers to join
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No servers available to join!"));
	}

	if (JoinButton)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::RefreshButtonClicked()
{
	RefreshServerList();
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
