// Copyright (c) 2023 Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ModifierMovement.generated.h"

class AModifierCharacter;

struct PREDICTEDMOVEMENT_API FModifierNetworkMoveData : FCharacterNetworkMoveData
{  // Client ➜ Server
public:
	typedef FCharacterNetworkMoveData Super;
 
	FModifierNetworkMoveData()
		: WantsModifierLevel(false)
	{}

	uint8 WantsModifierLevel;
	
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
};
 
struct PREDICTEDMOVEMENT_API FModifierNetworkMoveDataContainer : FCharacterNetworkMoveDataContainer
{  // Client ➜ Server
public:
	typedef FCharacterNetworkMoveDataContainer Super;
 
	FModifierNetworkMoveDataContainer()
	{
		NewMoveData = &MoveData[0];
		PendingMoveData = &MoveData[1];
		OldMoveData = &MoveData[2];
	}
 
private:
	FModifierNetworkMoveData MoveData[3];
};

/**
 *
 */
UCLASS()
class PREDICTEDMOVEMENT_API UModifierMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
private:
	/** Character movement component belongs to */
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<AModifierCharacter> ModifierCharacterOwner;

public:
	/** Max Acceleration (rate of change of velocity) */
	UPROPERTY(Category="Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxAccelerationModified;
	
	/** The maximum ground speed when Modified. */
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWalkSpeedModified;

public:
	/** If true, try to Modifier (or keep Modified) on next update. If false, try to stop Modified on next update. */
	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 WantsModifierLevel;

public:
	UModifierMovement(const FObjectInitializer& ObjectInitializer);

	virtual bool HasValidData() const override;
	virtual void PostLoad() override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

public:
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;

public:
	virtual uint8 GetModifierLevel() const;

	/**
	 * Call CharacterOwner->OnStartModifier() if successful.
	 * In general you should set WantsModifierLevel instead to have the Modifier persist during movement, or just use the Modifier functions on the owning Character.
	 * @param	bClientSimulation	true when called when bIsModifiered is replicated to non owned clients.
	 */
	virtual void ChangeModifier(uint8 NewLevel, bool bClientSimulation = false, uint8 PrevSimulatedLevel = 0);

	/** Returns true if the character is allowed to Modifier in the current state. */
	virtual uint8 GetModifierLevelForCurrentState() const;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData) override;

protected:
	virtual bool ClientUpdatePositionAfterServerUpdate() override;

private:
	FModifierNetworkMoveDataContainer ModifierMoveDataContainer;
	
public:
	/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class PREDICTEDMOVEMENT_API FSavedMove_Character_Modifier : public FSavedMove_Character
{
	using Super = FSavedMove_Character;

public:
	FSavedMove_Character_Modifier()
		: WantsModifierLevel(0)
	{
	}

	virtual ~FSavedMove_Character_Modifier() override
	{}

	uint8 WantsModifierLevel;
		
	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear() override;

	/** Called to set up this saved move (when initially created) to make a predictive correction. */
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;

	/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	/** Set the properties describing the position, etc. of the moved pawn at the start of the move. */
	virtual void SetInitialPosition(ACharacter* C) override;

	/** Combine this move with an older move and update relevant state. */
	virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;

	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
};

class PREDICTEDMOVEMENT_API FNetworkPredictionData_Client_Character_Modifier : public FNetworkPredictionData_Client_Character
{
	using Super = FNetworkPredictionData_Client_Character;

public:
	FNetworkPredictionData_Client_Character_Modifier(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
	{}

	virtual FSavedMovePtr AllocateNewMove() override;
};
