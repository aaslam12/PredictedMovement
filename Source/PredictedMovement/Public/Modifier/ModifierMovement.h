// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModifierImpl.h"
#include "ModifierTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "System/PredictedMovementVersioning.h"
#include "ModifierMovement.generated.h"

class AModifierCharacter;

using TMod_Local = FMovementModifier_LocalPredicted;
using TMod_LocalCorrection = FMovementModifier_WithCorrection;
using TMod_Server = FMovementModifier_WithCorrection;


struct PREDICTEDMOVEMENT_API FModifierMoveResponseDataContainer : FCharacterMoveResponseDataContainer
{  // Server ➜ Client
	using Super = FCharacterMoveResponseDataContainer;

	FModifierMoveResponse BoostCorrection;
	FModifierMoveResponse BoostServer;
	FModifierMoveResponse SnareServer;

	virtual void ServerFillResponseData(const UCharacterMovementComponent& CharacterMovement, const FClientAdjustment& PendingAdjustment) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap) override;
};

struct PREDICTEDMOVEMENT_API FModifierNetworkMoveData : FCharacterNetworkMoveData
{  // Client ➜ Server
public:
	typedef FCharacterNetworkMoveData Super;
 
	FModifierNetworkMoveData()
	{}

	FModifierMoveData_LocalPredicted BoostLocal;
	FModifierMoveData_WithCorrection BoostCorrection;
	FModifierMoveData_ServerInitiated BoostServer;
	FModifierMoveData_ServerInitiated SnareServer;
	
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
	/**
	 * Boost increases movement speed and acceleration
	 * Scaling applied on a per-boost-level basis
	 * Every tag defined here must also be defined in the FModifierData Boost property
	 */
	UPROPERTY(Category="Character Movement: Modifiers", EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FMovementModifierParams> Boost;

	/** Indexed list of Boost levels, used to determine the current Boost level */
	UPROPERTY()
	TArray<FGameplayTag> BoostLevels;

	/** The method used to calculate boost levels */
	UPROPERTY(Category="Character Movement: Modifiers", EditAnywhere, BlueprintReadWrite)
	EModifierLevelMethod BoostLevelMethod;
	
	/** Local Predicted Boost based on Player Input, that can be corrected by the server when a mismatch occurs */
	TMod_Local BoostLocal;
	TMod_LocalCorrection BoostCorrection;
	TMod_Server BoostServer;

public:
	/**
	 * Boost increases movement speed and acceleration
	 * Scaling applied on a per-boost-level basis
	 * Every tag defined here must also be defined in the FModifierData Boost property
	 */
	UPROPERTY(Category="Character Movement: Modifiers", EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FMovementModifierParams> Snare;

	/** Indexed list of Boost levels, used to determine the current Boost level */
	UPROPERTY()
	TArray<FGameplayTag> SnareLevels;

	/** The method used to calculate boost levels */
	UPROPERTY(Category="Character Movement: Modifiers", EditAnywhere, BlueprintReadWrite)
	EModifierLevelMethod SnareLevelMethod;
	
	/** Local Predicted Boost based on Player Input, that can be corrected by the server when a mismatch occurs */
	TMod_Server SnareServer;
	
public:
	UModifierMovement(const FObjectInitializer& ObjectInitializer);

	virtual bool HasValidData() const override;
	virtual void PostLoad() override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

public:
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetGroundFriction(float DefaultGroundFriction) const;
	virtual float GetBrakingFriction() const;
	virtual float GetRootMotionTranslationScalar() const;
	
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;

public:
	/* Boost Implementation */

	uint8 BoostLevel = UINT8_MAX;
	const FMovementModifierParams* GetBoostParams() const { return Boost.Find(GetBoostLevel()); }
	FGameplayTag GetBoostLevel() const { return BoostLevels.IsValidIndex(BoostLevel) ? BoostLevels[BoostLevel] : FGameplayTag::EmptyTag; }
	uint8 GetBoostLevelIndex(const FGameplayTag& Level) const { return BoostLevels.IndexOfByKey(Level) > INDEX_NONE ? BoostLevels.IndexOfByKey(Level) : UINT8_MAX; }
	virtual bool CanBoostInCurrentState() const;

	float GetBoostSpeedScalar() const { return GetBoostParams() ? GetBoostParams()->MaxWalkSpeed : 1.f; }
	float GetBoostAccelScalar() const { return GetBoostParams() ? GetBoostParams()->MaxAcceleration : 1.f; }
	float GetBoostBrakingScalar() const { return GetBoostParams() ? GetBoostParams()->BrakingDeceleration : 1.f; }
	float GetBoostGroundFrictionScalar() const { return GetBoostParams() ? GetBoostParams()->GroundFriction : 1.f; }
	float GetBoostBrakingFrictionScalar() const { return GetBoostParams() ? GetBoostParams()->BrakingFriction : 1.f; }
	bool BoostAffectsRootMotion() const { return GetBoostParams() ? GetBoostParams()->bAffectsRootMotion : false; }
	
	/* ~Boost Implementation */

public:
	/* Snare Implementation */

	uint8 SnareLevel = UINT8_MAX;
	const FMovementModifierParams* GetSnareParams() const { return Snare.Find(GetSnareLevel()); }
	FGameplayTag GetSnareLevel() const { return SnareLevels.IsValidIndex(SnareLevel) ? SnareLevels[SnareLevel] : FGameplayTag::EmptyTag; }
	uint8 GetSnareLevelIndex(const FGameplayTag& Level) const { return SnareLevels.IndexOfByKey(Level) > INDEX_NONE ? SnareLevels.IndexOfByKey(Level) : UINT8_MAX; }
	virtual bool CanSnareInCurrentState() const;

	float GetSnareSpeedScalar() const { return GetSnareParams() ? GetSnareParams()->MaxWalkSpeed : 1.f; }
	float GetSnareAccelScalar() const { return GetSnareParams() ? GetSnareParams()->MaxAcceleration : 1.f; }
	float GetSnareBrakingScalar() const { return GetSnareParams() ? GetSnareParams()->BrakingDeceleration : 1.f; }
	float GetSnareGroundFrictionScalar() const { return GetSnareParams() ? GetSnareParams()->GroundFriction : 1.f; }
	float GetSnareBrakingFrictionScalar() const { return GetSnareParams() ? GetSnareParams()->BrakingFriction : 1.f; }
	bool SnareAffectsRootMotion() const { return GetSnareParams() ? GetSnareParams()->bAffectsRootMotion : false; }
	
	/* ~Snare Implementation */
	
public:
	void UpdateModifierMovementState();

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	
public:
	virtual void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData) override;
	
	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel,
		const FVector& ClientWorldLocation, const FVector& RelativeClientLocation,
		UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	
	virtual void OnClientCorrectionReceived(class FNetworkPredictionData_Client_Character& ClientData, float TimeStamp,
		FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase,
		bool bBaseRelativePosition, uint8 ServerMovementMode
#if UE_5_03_OR_LATER
		, FVector ServerGravityDirection) override;
#else
	) override;
#endif

protected:
	virtual bool ClientUpdatePositionAfterServerUpdate() override;

protected:
	virtual void TickCharacterPose(float DeltaTime) override;  // ACharacter::GetAnimRootMotionTranslationScale() is non-virtual so we have to duplicate this entire function
	
private:
	FModifierNetworkMoveDataContainer ModifierMoveDataContainer;
	FModifierMoveResponseDataContainer ModifierMoveResponseDataContainer;
	
public:
	/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class PREDICTEDMOVEMENT_API FSavedMove_Character_Modifier : public FSavedMove_Character
{
	using Super = FSavedMove_Character;

public:
	FSavedMove_Character_Modifier()
	{}

	virtual ~FSavedMove_Character_Modifier() override
	{}

	FModifierSavedMove BoostLocal;
	FModifierSavedMove_WithCorrection BoostCorrection;
	FModifierSavedMove_ServerInitiated BoostServer;
	
	FModifierSavedMove_ServerInitiated SnareServer;
	
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
	
	/** Set the properties describing the final position, etc. of the moved pawn. */
	virtual void PostUpdate(ACharacter* C, EPostUpdateMode PostUpdateMode) override;

	/** Returns true if this move is an "important" move that should be sent again if not acked by the server */
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
