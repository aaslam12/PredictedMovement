// Copyright (c) 2023 Jared Taylor. All Rights Reserved.


#include "Modifier/ModifierMovement.h"

#include "Modifier/ModifierCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModifierMovement)

void FModifierNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove,
	ENetworkMoveType MoveType)
{
	// Client packs move data to send to the server
	// Use this instead of GetCompressedFlags()
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	// Client ➜ Server
	
	// CallServerMovePacked ➜ ClientFillNetworkMoveData ➜ ServerMovePacked_ClientSend >> Server
	// >> ServerMovePacked_ServerReceive ➜ ServerMove_HandleMoveData ➜ ServerMove_PerformMovement
	// ➜ MoveAutonomous (UpdateFromCompressedFlags)
	
	const FSavedMove_Character_Modifier& SavedMove = static_cast<const FSavedMove_Character_Modifier&>(ClientMove);
	WantsModifiers = SavedMove.WantsModifiers;
}

bool FModifierNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap, ENetworkMoveType MoveType)
{  // Client ➜ Server
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	// constexpr uint8 MaxModifiers = 8;
	// if (MaxModifiers <= 1)
	// {
	// 	return !Ar.IsError();
	// }

	Ar << WantsModifiers;
	
	// // Serialize the number of elements
	// uint8 NumModifiers = WantsModifiers.Num();
	// if (Ar.IsSaving())
	// {
	// 	NumModifiers = FMath::Min(MaxModifiers, NumModifiers);
	// }
	// Ar << NumModifiers;
	//
	// // Resize the array if needed
	// if (Ar.IsLoading())
	// {
	// 	const FName ModifierType = TEXT("DebugType");  // @TODO
	// 	if (!ensureMsgf(NumModifiers <= MaxModifiers,
	// 		TEXT("Deserializing modifier %s array with %d elements when max is %d -- Check packet serialization logic"), *ModifierType.ToString(), NumModifiers, MaxModifiers))
	// 	{
	// 		NumModifiers = MaxModifiers;
	// 	}
	// 	WantsModifiers.SetNum(NumModifiers);
	// }
	//
	// // Serialize the elements
	// for (uint8 i = 0; i < NumModifiers; ++i)
	// {
	// 	Ar << WantsModifiers[i];
	// }

	return !Ar.IsError();
}

UModifierMovement::UModifierMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetworkMoveDataContainer(ModifierMoveDataContainer);
	
	MaxAccelerationModified = 250.f;
	MaxWalkSpeedModified = 250.f;
}

bool UModifierMovement::HasValidData() const
{
	return Super::HasValidData() && IsValid(ModifierCharacterOwner);
}

void UModifierMovement::PostLoad()
{
	Super::PostLoad();

	ModifierCharacterOwner = Cast<AModifierCharacter>(PawnOwner);
}

void UModifierMovement::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	ModifierCharacterOwner = Cast<AModifierCharacter>(PawnOwner);
}

float UModifierMovement::GetMaxAcceleration() const
{
	const float Modified = MaxAccelerationModified * GetModifierLevel();
	return Super::GetMaxAcceleration() + Modified;
}

float UModifierMovement::GetMaxSpeed() const
{
	const float Modified = MaxWalkSpeedModified * GetModifierLevel();
	return Super::GetMaxSpeed() + Modified;
}

float UModifierMovement::GetMaxBrakingDeceleration() const
{
	return Super::GetMaxBrakingDeceleration();
}

void UModifierMovement::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

void UModifierMovement::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

void UModifierMovement::ChangeModifiers(const TArray<uint8>& NewModifiers, bool bClientSimulation, uint8 PrevSimulatedLevel)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation)
	{
		const TArray<uint8> PrevModifiers = Modifiers;
		if (PrevModifiers != NewModifiers)
		{
			const uint8 PrevLevel = GetModifierLevel();
			Modifiers = NewModifiers;
			
			if (ModifierCharacterOwner->HasAuthority())
			{
				ModifierCharacterOwner->SimulatedModifierLevel = GetModifierLevel();
			}
			// @TODO ModifierCharacterOwner->OnModifierChanged(ModifierType, ModifierLevel, PrevLevel);
		}
	}
	else
	{
		// @TODO ModifierCharacterOwner->OnModifierChanged(ModifierType, ModifierLevel, PrevSimulatedLevel);
	}
}

TArray<uint8> UModifierMovement::GetModifiersForCurrentState() const
{
	if (!UpdatedComponent || UpdatedComponent->IsSimulatingPhysics())
	{
		return {};
	}

	if (!IsFalling() && !IsMovingOnGround())
	{
		return {};
	}

	return WantsModifiers;
}

uint8 UModifierMovement::GetModifierLevelForCurrentState() const
{
	if (!UpdatedComponent || UpdatedComponent->IsSimulatingPhysics())
	{
		return 0;
	}

	if (!IsFalling() && !IsMovingOnGround())
	{
		return 0;
	}

	return GetWantedModifierLevel();
}

void UModifierMovement::UpdateModifierMovementState()
{
	if (!HasValidData())
	{
		return;
	}

	// Proxies get replicated Modifier state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Check for a change in Modifier state. Players toggle Modifier by changing WantsModifierLevel.
		const TArray<uint8>& NewModifiers = GetModifiersForCurrentState();
		if (NewModifiers != Modifiers)
		{
			ChangeModifiers(NewModifiers);
		}
	}
}

void UModifierMovement::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	UpdateModifierMovementState();

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UModifierMovement::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	UpdateModifierMovementState();

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UModifierMovement::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
	// Server updates from the client's move data
	// Use this instead of UpdateFromCompressedFlags()

	// Client >> CallServerMovePacked ➜ ClientFillNetworkMoveData ➜ ServerMovePacked_ClientSend >> Server
	// >> ServerMovePacked_ServerReceive ➜ ServerMove_HandleMoveData ➜ ServerMove_PerformMovement
	
	const FModifierNetworkMoveData& ModifierMoveData = static_cast<const FModifierNetworkMoveData&>(MoveData);

	WantsModifiers = SetModifiersFromBitmask(ModifierMoveData.WantsModifiers);

	Super::ServerMove_PerformMovement(MoveData);
}

bool UModifierMovement::ClientUpdatePositionAfterServerUpdate()
{
	const TArray<uint8> RealWantsModifiers = WantsModifiers;
	
	const bool bResult = Super::ClientUpdatePositionAfterServerUpdate();
	
	WantsModifiers = RealWantsModifiers;

	return bResult;
}

void FSavedMove_Character_Modifier::Clear()
{
	Super::Clear();

	WantsModifiers = 0;
}

void FSavedMove_Character_Modifier::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UModifierMovement* MoveComp = Cast<AModifierCharacter>(C)->GetModifierCharacterMovement())
	{
		// Bit pack the modifiers into a bitfield.
		WantsModifiers = GetBitmaskFromModifiers(MoveComp->WantsModifiers);
	}
}

bool FSavedMove_Character_Modifier::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
	float MaxDelta) const
{
	// We combine moves for the purpose of reducing the number of moves sent to the server, especially when exceeding
	// 60 fps (by default, see ClientNetSendMoveDeltaTime).
	// By combining moves, we can send fewer moves, but still have the same outcome.
	
	// If we didn't handle move combining, and then we used OnStartModifier() to modify our Velocity directly, it would
	// de-sync if we exceed 60fps. This is where move combining kicks in and starts using Pending Moves instead.
	
	// When combining moves, the PendingMove is passed into the NewMove. Locally, before sending a Move to the Server,
	// the AutonomousProxy Client will already have processed the current PendingMove (it's only pending for being sent,
	// not processed).

	// Since combining will happen before processing a move, PendingMove might end up being processed twice; once last
	// frame, and once as part of the new combined move.
	
	const TSharedPtr<FSavedMove_Character_Modifier> SavedMove = StaticCastSharedPtr<FSavedMove_Character_Modifier>(NewMove);

	// We can only combine moves if they will result in the same state as if both moves were processed individually,
	// because the AutonomousProxy Client processes them individually prior to sending them to the server.
	
	if (WantsModifiers != SavedMove->WantsModifiers)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Character_Modifier::SetInitialPosition(ACharacter* C)
{
	// To counter the PendingMove potentially being processed twice, we need to make sure to reset the state of the CMC
	// back to the "InitialPosition" (state) it had before the PendingMove got processed.
	
	Super::SetInitialPosition(C);

	// Retrieve the value from our CMC to revert the saved move value back to this.
	if (const UModifierMovement* MoveComp = Cast<AModifierCharacter>(C)->GetModifierCharacterMovement())
	{
		WantsModifiers = GetBitmaskFromModifiers(MoveComp->WantsModifiers);
	}
}

void FSavedMove_Character_Modifier::CombineWith(const FSavedMove_Character* OldMove, ACharacter* C,
	APlayerController* PC, const FVector& OldStartLocation)
{
	Super::CombineWith(OldMove, C, PC, OldStartLocation);

	const FSavedMove_Character_Modifier* SavedOldMove = static_cast<const FSavedMove_Character_Modifier*>(OldMove);

	if (UModifierMovement* MoveComp = C ? Cast<UModifierMovement>(C->GetCharacterMovement()) : nullptr)
	{
		MoveComp->WantsModifiers = MoveComp->SetModifiersFromBitmask(SavedOldMove->WantsModifiers);
	}
}

bool FSavedMove_Character_Modifier::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	const TSharedPtr<FSavedMove_Character_Modifier>& SavedMove = StaticCastSharedPtr<FSavedMove_Character_Modifier>(LastAckedMove);
	if (WantsModifiers != SavedMove->WantsModifiers)
	{
		return true;
	}
	
	return Super::IsImportantMove(LastAckedMove);
}

FSavedMovePtr FNetworkPredictionData_Client_Character_Modifier::AllocateNewMove()
{
	return MakeShared<FSavedMove_Character_Modifier>();
}

FNetworkPredictionData_Client* UModifierMovement::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UModifierMovement* MutableThis = const_cast<UModifierMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_Modifier(*this);
	}

	return ClientPredictionData;
}
