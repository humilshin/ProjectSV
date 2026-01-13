// Shin YeongSeop 2026

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/World.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (IsDedicatedServerMode())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("MultiplayerSessionsSubsystem: Initialized in Dedicated Server mode"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("MultiplayerSessionsSubsystem: Initialized in Client mode"));
		}
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}

	Super::Deinitialize();
}

bool UMultiplayerSessionsSubsystem::IsDedicatedServerMode() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetNetMode() == NM_DedicatedServer;
	}
	return IsRunningDedicatedServer();
}

//    SERVER FUNCTIONS
void UMultiplayerSessionsSubsystem::CreateServerSession(int32 NumPublicConnections, FString MatchType, FString ServerName)
{
	if (!IsValidSessionInterface())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CreateServerSession: Invalid session interface"));
		}
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}

	// Destroy existing session first
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		LastServerName = ServerName;
		DestroySession();
		return;
	}

	// Register callback
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// Configure session settings for Dedicated Server
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	LastSessionSettings->bIsLANMatch = Subsystem && Subsystem->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bAllowInvites = true;

	// Dedicated Server specific settings (NO Lobby/Presence)
	LastSessionSettings->bUsesPresence = false;
	LastSessionSettings->bUseLobbiesIfAvailable = false;
	LastSessionSettings->bAllowJoinViaPresence = false;
	LastSessionSettings->bIsDedicated = true;

	// Server metadata
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("ServerName"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(SEARCH_KEYWORDS, ServerName, EOnlineDataAdvertisementType::ViaOnlineService);
	LastSessionSettings->BuildUniqueId = GetTypeHash(FGuid::NewGuid().ToString());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
			FString::Printf(TEXT("CreateServerSession: Creating '%s' with %d slots"), *ServerName, NumPublicConnections));
	}

	// Dedicated server uses index 0 (no local player)
	if (!SessionInterface->CreateSession(0, NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CreateServerSession: Failed to create session"));
		}
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

//  CLIENT FUNCTIONS
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!IsValidSessionInterface())
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	LastSessionSearch->bIsLanQuery = Subsystem && Subsystem->GetSubsystemName() == "NULL";

	// Dedicated Servers : no Lobby/Presence
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Searching for dedicated servers..."));
	}

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer == nullptr)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (LocalPlayer == nullptr)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("JoinSession: Joining server..."));

	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

//  UTILITIES

bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
	if (!SessionInterface.IsValid())
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			SessionInterface = Subsystem->GetSessionInterface();
			UE_LOG(LogTemp, Log, TEXT("IsValidSessionInterface: Using '%s'"), *Subsystem->GetSubsystemName().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("IsValidSessionInterface: No OnlineSubsystem available"));
		}
	}
	return SessionInterface.IsValid();
}


//    INTERNAL CALLBACKS

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
				FString::Printf(TEXT("Session '%s' created successfully"), *SessionName.ToString()));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				FString::Printf(TEXT("Failed to create session '%s'"), *SessionName.ToString()));
		}
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
				FString::Printf(TEXT("Found %d servers"), LastSessionSearch->SearchResults.Num()));
		}

		// Log details for each found server
		for (int32 i = 0; i < LastSessionSearch->SearchResults.Num(); i++)
		{
			const FOnlineSessionSearchResult& Result = LastSessionSearch->SearchResults[i];

			FString ServerName;
			FString MatchType;
			Result.Session.SessionSettings.Get(FName("ServerName"), ServerName);
			Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

			int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;


			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White,
					FString::Printf(TEXT("[%d] %s | %s | %d/%d | %dms"),
						i, *ServerName, *MatchType, CurrentPlayers, MaxPlayers, Result.PingInMs));
			}
		}

		if (LastSessionSearch->SearchResults.Num() <= 0)
		{
			MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
			return;
		}

		MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete: LastSessionSearch is invalid"));
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("OnJoinSessionComplete: Result = %d"), static_cast<int32>(Result));
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateServerSession(LastNumPublicConnections, LastMatchType, LastServerName);
	}

	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("OnStartSessionComplete: Session '%s' started = %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}