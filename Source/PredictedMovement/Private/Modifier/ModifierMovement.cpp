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
	bWantsToModifier = SavedMove.bWantsToModifier;
}

bool FModifierNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	Ar.SerializeBits(&bWantsToModifier, 1);
	
	return !Ar.IsError();
}

UModifierMovement::UModifierMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetworkMoveDataContainer(ModifierMoveDataContainer);
	
	MaxAccelerationModified = 2324.f;
	MaxWalkSpeedModified = 1600.f;
	BrakingDecelerationModified = 512.f;
	GroundFrictionModified = 12.f;
	BrakingFrictionModified = 4.f;

	bWantsToModifier = false;
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
	if (IsModified())
	{
		return MaxAccelerationModified;
	}
	return Super::GetMaxAcceleration();
}

float UModifierMovement::GetMaxSpeed() const
{
	if (IsModified())
	{
		return MaxWalkSpeedModified;
	}
	return Super::GetMaxSpeed();
}

float UModifierMovement::GetMaxBrakingDeceleration() const
{
	if (IsModified())
	{
		return BrakingDecelerationModified;
	}
	return Super::GetMaxBrakingDeceleration();
}

void UModifierMovement::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (IsModified() && IsMovingOnGround())
	{
		Friction = GroundFrictionModified;
	}
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

void UModifierMovement::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	if (IsModified() && IsMovingOnGround())
	{
		Friction = (bUseSeparateBrakingFriction ? BrakingFrictionModified : GroundFrictionModified);
	}
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

bool UModifierMovement::IsModified() const
{
	return ModifierCharacterOwner && ModifierCharacterOwner->bIsModified;
}

void UModifierMovement::Modifier(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanModifierInCurrentState())
	{
		return;
	}

	if (!bClientSimulation)
	{
		ModifierCharacterOwner->bIsModified = true;
	}
	ModifierCharacterOwner->OnStartModifier();
}

void UModifierMovement::UnModifier(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation)
	{
		ModifierCharacterOwner->bIsModified = false;
	}
	ModifierCharacterOwner->OnEndModifier();
}

bool UModifierMovement::CanModifierInCurrentState() const
{
	if (!UpdatedComponent || UpdatedComponent->IsSimulatingPhysics())
	{
		return false;
	}

	if (!IsFalling() && !IsMovingOnGround())
	{
		return false;
	}

	return true;
}

void UModifierMovement::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Proxies get replicated Modifier state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Check for a change in Modifier state. Players toggle Modifier by changing bWantsToModifier.
		const bool bIsModified = IsModified();
		if (bIsModified && (!bWantsToModifier || !CanModifierInCurrentState()))
		{
			UnModifier(false);
		}
		else if (!bIsModified && bWantsToModifier && CanModifierInCurrentState())
		{
			Modifier(false);
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UModifierMovement::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	// Proxies get replicated Modifier state.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// UnModifier if no longer allowed to be Modified
		if (IsModified() && !CanModifierInCurrentState())
		{
			UnModifier(false);
		}
	}

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UModifierMovement::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
	// Server updates from the client's move data
	// Use this instead of UpdateFromCompressedFlags()

	// Client >> CallServerMovePacked ➜ ClientFillNetworkMoveData ➜ ServerMovePacked_ClientSend >> Server
	// >> ServerMovePacked_ServerReceive ➜ ServerMove_HandleMoveData ➜ ServerMove_PerformMovement
	
	const FModifierNetworkMoveData& ModifierMoveData = static_cast<const FModifierNetworkMoveData&>(MoveData);

	bWantsToModifier = ModifierMoveData.bWantsToModifier;

	Super::ServerMove_PerformMovement(MoveData);
}

bool UModifierMovement::ClientUpdatePositionAfterServerUpdate()
{
	const bool bRealModifier = bWantsToModifier;
	const bool bResult = Super::ClientUpdatePositionAfterServerUpdate();
	bWantsToModifier = bRealModifier;

	return bResult;
}

void FSavedMove_Character_Modifier::Clear()
{
	Super::Clear();

	bWantsToModifier = false;
}

void FSavedMove_Character_Modifier::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	bWantsToModifier = Cast<AModifierCharacter>(C)->GetModifierCharacterMovement()->bWantsToModifier;
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
	
	if (bWantsToModifier != SavedMove->bWantsToModifier)
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
	
	bWantsToModifier = Cast<AModifierCharacter>(C)->GetModifierCharacterMovement()->bWantsToModifier;
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
