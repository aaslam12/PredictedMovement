// Copyright (c) Jared Taylor


#include "Modifier/ModifierMovement.h"

#include "Modifier/ModifierCharacter.h"
#include "Modifier/ModifierTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModifierMovement)

void FModifierMoveResponseDataContainer::ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement,
	const FClientAdjustment& PendingAdjustment)
{
	Super::ServerFillResponseData(CharacterMovement, PendingAdjustment);

	// Server ➜ Client
	const UModifierMovement* MoveComp = Cast<UModifierMovement>(&CharacterMovement);
	
	BoostCorrection.ServerFillResponseData(MoveComp->BoostCorrection.Data.Modifiers);  // AUTH
	BoostServer.ServerFillResponseData(MoveComp->BoostServer.Data.Modifiers);  // AUTH
	
	// Modifiers = FModifierCompression::GetBitmaskFromModifiers(MoveComp->Modifiers);  // AUTH
}

bool FModifierMoveResponseDataContainer::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
	UPackageMap* PackageMap)
{
	if (!Super::Serialize(CharacterMovement, Ar, PackageMap))
	{
		return false;
	}

	// Server ➜ Client
	if (IsCorrection())
	{
		Ar << BoostCorrection.Modifiers;  // AUTH
		Ar << BoostServer.Modifiers;  // AUTH
	}

	return !Ar.IsError();
}

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
	BoostLocal.ClientFillNetworkMoveData(SavedMove.BoostLocal.WantsModifiers);
	BoostCorrection.ClientFillNetworkMoveData(SavedMove.BoostCorrection.WantsModifiers, SavedMove.BoostCorrection.Modifiers);
	BoostServer.ClientFillNetworkMoveData(SavedMove.BoostServer.Modifiers);
	// WantsModifiers = SavedMove.WantsModifiers;
	// Modifiers = SavedMove.Modifiers;  // AUTH
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

	Ar << BoostLocal.WantsModifiers;
	Ar << BoostCorrection.WantsModifiers;
	Ar << BoostCorrection.Modifiers;  // AUTH
	Ar << BoostServer.Modifiers;  // AUTH
	
	// Ar << WantsModifiers;
	// Ar << Modifiers;  // AUTH
	
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
	SetMoveResponseDataContainer(ModifierMoveResponseDataContainer);
	
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
	const float Modified = MaxAccelerationModified * GetBoostLevel();
	return Super::GetMaxAcceleration() + Modified;
}

float UModifierMovement::GetMaxSpeed() const
{
	const float Modified = MaxWalkSpeedModified * GetBoostLevel();
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

bool UModifierMovement::CanBoostInCurrentState() const
{
	return UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics() &&
		(IsFalling() || IsMovingOnGround());
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

		const uint8 PrevBoost = GetBoostLevel();
		BoostLocal.UpdateMovementState(			CanBoostInCurrentState() );
		BoostCorrection.UpdateMovementState(	CanBoostInCurrentState() );
		BoostServer.UpdateMovementState(		CanBoostInCurrentState() );
		if (GetBoostLevel() != PrevBoost)
		{
			// @TODO ModifierCharacterOwner->OnModifierChanged(ModifierType, GetBoostLevel(), PrevBoost);
		}
		
		// const TArray<EModifierByte>& NewModifiers = GetBoostModifiersForCurrentState();
		// if (NewModifiers != Modifiers)
		// {
		// 	ChangeModifiers(NewModifiers);
		// }
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

	// WantsModifiers = FModifierCompression::SetModifiersFromBitmask<EModifierByte>(ModifierMoveData.WantsModifiers);
	BoostLocal.ServerMove_PerformMovement(ModifierMoveData.BoostLocal.WantsModifiers);
	BoostCorrection.ServerMove_PerformMovement(ModifierMoveData.BoostCorrection.WantsModifiers);

	Super::ServerMove_PerformMovement(MoveData);
}

bool UModifierMovement::ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel,
	const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase,
	FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	// ServerMovePacked_ServerReceive ➜ ServerMove_HandleMoveData ➜ ServerMove_PerformMovement
	// ➜ ServerMoveHandleClientError ➜ ServerCheckClientError
	
	if (Super::ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode))
	{
		return true;
	}
    
	/*
	 * This will trigger a client correction if the value in the Client differs
	 */
	const FModifierNetworkMoveData* CurrentMoveData = static_cast<const FModifierNetworkMoveData*>(GetCurrentNetworkMoveData());
	
	// if (FModifierCompression::GetBitmaskFromModifiers(Modifiers) != CurrentMoveData->Modifiers)
	// {
	// 	return true;  // AUTH
	// }

	if (BoostCorrection.ServerCheckClientError(CurrentMoveData->BoostCorrection.Modifiers))
	{
		return true;  // AUTH
	}

	if (BoostServer.ServerCheckClientError(CurrentMoveData->BoostServer.Modifiers))
	{
		return true;  // AUTH
	}

	return false;
}

void UModifierMovement::OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData,
	float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName,
	bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode
#if UE_5_03_OR_LATER
	, FVector ServerGravityDirection
#endif
	)
{
	// This occurs on AutonomousProxy, when the server sends the move response
	// This is where we receive the snare, and can override the server's location, assuming it has given us authority

	// Server >> SendClientAdjustment() ➜ ServerSendMoveResponse() ➜ ServerFillResponseData() + MoveResponsePacked_ServerSend() >> Client
	// >> ClientMoveResponsePacked() ➜ ClientHandleMoveResponse() ➜ ClientAdjustPosition_Implementation() ➜ OnClientCorrectionReceived()
	
	const FModifierMoveResponseDataContainer& MoveResponse = static_cast<const FModifierMoveResponseDataContainer&>(GetMoveResponseDataContainer());
	// WantsModifiers = FModifierCompression::SetModifiersFromBitmask<EModifierByte>(MoveResponse.Modifiers);  // AUTH

	BoostCorrection.OnClientCorrectionReceived(MoveResponse.BoostCorrection.Modifiers);
	BoostServer.OnClientCorrectionReceived(MoveResponse.BoostServer.Modifiers);
	
	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName,
		bHasBase, bBaseRelativePosition, ServerMovementMode, ServerGravityDirection);
}

bool UModifierMovement::ClientUpdatePositionAfterServerUpdate()
{
	// const TArray<EModifierByte> RealWantsModifiers = WantsModifiers;

	const TArray RealBoostLocal = BoostLocal.Data.WantsModifiers;
	const TArray RealBoostCorrection = BoostCorrection.Data.WantsModifiers;
	
	const bool bResult = Super::ClientUpdatePositionAfterServerUpdate();
	
	// WantsModifiers = RealWantsModifiers;

	BoostLocal.Data.WantsModifiers = RealBoostLocal;
	BoostCorrection.Data.WantsModifiers = RealBoostCorrection;

	return bResult;
}

void FSavedMove_Character_Modifier::Clear()
{
	Super::Clear();

	// WantsModifiers = 0;
	// Modifiers = 0;  // AUTH

	BoostLocal.Clear();
	BoostCorrection.Clear();
	BoostServer.Clear();
}

void FSavedMove_Character_Modifier::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	// Client ➜ Server (ReplicateMoveToServer)
	
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UModifierMovement* MoveComp = Cast<AModifierCharacter>(C)->GetModifierCharacterMovement())
	{
		// Bit pack the modifiers into a bitfield.
		// WantsModifiers = FModifierCompression::GetBitmaskFromModifiers(MoveComp->WantsModifiers);
		BoostLocal.SetMoveFor(MoveComp->BoostLocal.Data.WantsModifiers);
		BoostCorrection.SetMoveFor(MoveComp->BoostCorrection.Data.WantsModifiers);
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
	
	// if (WantsModifiers != SavedMove->WantsModifiers)
	// {
	// 	return false;
	// }

	if (!BoostLocal.CanCombineWith(SavedMove->BoostLocal.WantsModifiers)) { return false; }
	if (!BoostCorrection.CanCombineWith(SavedMove->BoostCorrection.WantsModifiers)) { return false; }
	
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
		BoostLocal.SetInitialPosition(MoveComp->BoostLocal.Data.WantsModifiers);
		BoostCorrection.SetInitialPosition(MoveComp->BoostCorrection.Data.WantsModifiers);
		// WantsModifiers = FModifierCompression::GetBitmaskFromModifiers(MoveComp->WantsModifiers);
	}
}

void FSavedMove_Character_Modifier::CombineWith(const FSavedMove_Character* OldMove, ACharacter* C,
	APlayerController* PC, const FVector& OldStartLocation)
{
	Super::CombineWith(OldMove, C, PC, OldStartLocation);

	const FSavedMove_Character_Modifier* SavedOldMove = static_cast<const FSavedMove_Character_Modifier*>(OldMove);

	if (UModifierMovement* MoveComp = C ? Cast<UModifierMovement>(C->GetCharacterMovement()) : nullptr)
	{
		MoveComp->BoostLocal.CombineWith(SavedOldMove->BoostLocal.WantsModifiers);
		MoveComp->BoostCorrection.CombineWith(SavedOldMove->BoostCorrection.WantsModifiers);
		// MoveComp->WantsModifiers = FModifierCompression::SetModifiersFromBitmask<EModifierByte>(SavedOldMove->WantsModifiers);
	}
}

void FSavedMove_Character_Modifier::PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode)
{
	// When considering whether to delay or combine moves, we need to compare the move at the start and the end
	if (const UModifierMovement* MoveComp = C ? Cast<UModifierMovement>(C->GetCharacterMovement()) : nullptr)
	{
		BoostCorrection.PostUpdate(MoveComp->BoostCorrection.Data.Modifiers);  // AUTH
		BoostServer.PostUpdate(MoveComp->BoostServer.Data.Modifiers);  // AUTH
		// Modifiers = FModifierCompression::GetBitmaskFromModifiers(MoveComp->Modifiers);  // AUTH
	}

	Super::PostUpdate(C, PostUpdateMode);
}

bool FSavedMove_Character_Modifier::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	const TSharedPtr<FSavedMove_Character_Modifier>& SavedMove = StaticCastSharedPtr<FSavedMove_Character_Modifier>(LastAckedMove);
	// if (WantsModifiers != SavedMove->WantsModifiers)
	// {
	// 	return true;
	// }

	if (BoostLocal.IsImportantMove(SavedMove->BoostLocal.WantsModifiers))
	{
		return true;
	}

	if (BoostCorrection.IsImportantMove(SavedMove->BoostCorrection.WantsModifiers))
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
