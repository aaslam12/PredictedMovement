// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModifierTypes.h"

using TModSize = uint8;  // If you want more than 255 modifiers, change this to uint16 or uint32
using TModifierStack = TArray<TModSize>;

/**
 * Represents a single modifier that can be applied to a character
 */
struct PREDICTEDMOVEMENT_API FMovementModifierData
{
	/** The requested input state, which requests modifiers of the specified level */
	TModifierStack WantsModifiers;
	
	/** The actual state, which represents the actual modifiers applied to the character */
	TModifierStack Modifiers;
};

/**
 * FSavedMove_Character
 */
struct PREDICTEDMOVEMENT_API FModifierSavedMove
{
	TModifierStack WantsModifiers;

	FModifierSavedMove()
	{}
	
	virtual ~FModifierSavedMove() = default;

	virtual void Clear()
	{
		WantsModifiers.Empty();
	}

	void SetMoveFor(const TModifierStack& Modifiers)
	{
		WantsModifiers = Modifiers;
	}

	bool CanCombineWith(const TModifierStack& Modifiers) const
	{
		return WantsModifiers == Modifiers;
	}

	void SetInitialPosition(const TModifierStack& Modifiers)
	{
		WantsModifiers = Modifiers;
	}

	bool IsImportantMove(const TModifierStack& Modifiers) const
	{
		return WantsModifiers != Modifiers;
	}
};

/**
 * FSavedMove_Character
 */
struct PREDICTEDMOVEMENT_API FModifierSavedMove_WithCorrection final : FModifierSavedMove
{
	using Super = FModifierSavedMove;
	
	TModifierStack Modifiers;

	FModifierSavedMove_WithCorrection()
	{}

	virtual void Clear() override
	{
		Super::Clear();
		Modifiers = {};
	}

	void PostUpdate(const TModifierStack& InModifiers)
	{
		Modifiers = InModifiers;
	}
};

/**
 * FSavedMove_Character
 */
struct PREDICTEDMOVEMENT_API FModifierSavedMove_ServerInitiated
{
	TModifierStack Modifiers;

	FModifierSavedMove_ServerInitiated()
	{}

	void Clear()
	{
		Modifiers.Empty();
	}

	void PostUpdate(const TModifierStack& InModifiers)
	{
		Modifiers = InModifiers;
	}
};

/**
 * FCharacterMoveResponseDataContainer
 * Only required when using WithCorrection or ServerInitiated modifiers
 */
struct PREDICTEDMOVEMENT_API FModifierMoveResponse
{
	TModifierStack Modifiers;

	void ServerFillResponseData(const TModifierStack& InModifiers)
	{
		Modifiers = InModifiers;
	}
};

/**
 * FCharacterNetworkMoveData
 * Sends wanted modifiers (via input) to the server to be applied to the character
 */
struct PREDICTEDMOVEMENT_API FModifierMoveData_LocalPredicted
{
	FModifierMoveData_LocalPredicted()
	{}
	
	TModifierStack WantsModifiers;

	void ClientFillNetworkMoveData(const TModifierStack& InWantsModifiers)
	{
		WantsModifiers = InWantsModifiers;
	}

	bool Serialize(FArchive& Ar, const FString& ErrorName, uint8 MaxSerializedModifiers=8);
};

/**
 * FCharacterNetworkMoveData
 * Sends wanted modifiers (via input) to the server to be applied to the character
 * Server compares the client and server modifiers to know when to send a net correction to the client with updated modifiers
 */
struct PREDICTEDMOVEMENT_API FModifierMoveData_WithCorrection
{
	FModifierMoveData_WithCorrection()
	{}

	TModifierStack WantsModifiers;
	TModifierStack Modifiers;

	void ClientFillNetworkMoveData(const TModifierStack& InWantsModifiers, const TModifierStack& InModifiers)
	{
		WantsModifiers = InWantsModifiers;
		Modifiers = InModifiers;
	}

	bool Serialize(FArchive& Ar, const FString& ErrorName, uint8 MaxSerializedModifiers=8);
};

/**
 * FCharacterNetworkMoveData
 * Used by server to compare between client and server, to know when to send a net correction to the client with updated modifiers
 */
struct PREDICTEDMOVEMENT_API FModifierMoveData_ServerInitiated
{
	FModifierMoveData_ServerInitiated()
	{}
	
	TModifierStack Modifiers;

	void ClientFillNetworkMoveData(const TModifierStack& InModifiers)
	{
		Modifiers = InModifiers;
	}

	bool Serialize(FArchive& Ar, const FString& ErrorName, uint8 MaxSerializedModifiers=8);
};

/**
 * Represents a single modifier that can be applied to a character
 * This is the base class for all modifiers, which can be local predicted, with correction, or server initiated
 */
struct PREDICTEDMOVEMENT_API FMovementModifier
{
	FMovementModifierData Data;
	
	TModSize GetWantedModifierLevel() const { return Data.WantsModifiers.Num(); }
	TModSize GetModifierLevel() const { return Data.Modifiers.Num(); }

	bool AddModifier(TModSize Level)
	{
		Data.WantsModifiers.Add(Level);
		return true;
	}

	bool RemoveModifier(TModSize Level, bool bRemoveAll)
	{
		if (Data.WantsModifiers.Contains(Level))
		{
			if (bRemoveAll)
			{
				Data.WantsModifiers.RemoveAll([Level](const TModSize& ModLevel) { return ModLevel == Level; });
			}
			else
			{
				Data.WantsModifiers.Remove(Level);
			}
			return true;
		}
		return false;
	}

	bool ResetModifiers()
	{
		if (Data.WantsModifiers.Num() > 0)
		{
			Data.WantsModifiers.Reset();
			return true;
		}
		return false;
	}

	TModSize GetNumWantedModifiersByLevel(TModSize Level) const
	{
		return Data.WantsModifiers.FilterByPredicate([Level](uint8 ModifierLevel)
		{
			return ModifierLevel == Level;
		}).Num();
	}

	TModSize GetNumModifiersByLevel(TModSize Level) const
	{
		return Data.Modifiers.FilterByPredicate([Level](uint8 ModifierLevel)
		{
			return ModifierLevel == Level;
		}).Num();
	}
	
	bool UpdateMovementState(bool bAllowedInCurrentState)
	{
		const TModifierStack Modifiers = bAllowedInCurrentState ? Data.WantsModifiers : TModifierStack();
		if (Data.Modifiers != Modifiers)
		{
			Data.Modifiers = Modifiers;
			return true;
		}
		return false;
	}
};

/**
 * Represents a single modifier that can be applied to a character
 * Local Predicted modifier is activated via player input and is predicted on the client
 * e.g. Sprint, Crouch, etc.
 */
struct PREDICTEDMOVEMENT_API FMovementModifier_LocalPredicted : FMovementModifier
{
	FMovementModifier_LocalPredicted()
	{}

	void ServerMove_PerformMovement(const TModifierStack& WantsModifiers)
	{
		Data.WantsModifiers = WantsModifiers;
	}

	void CombineWith(const TModifierStack& WantsModifiers)
	{
		Data.WantsModifiers = WantsModifiers;
	}
};

/**
 * Represents a single modifier that can be applied to a character
 * 
 * Local Predicted modifier is activated via a predicted state (GAS) and is predicted on the client
 * However, if the server disagrees with the client, it will correct the client
 *
 * Alternatively, initiated by the server and sent to the client
 * 
 * e.g. Speed increase after equipping a knife via predicted inventory ability, etc.
 */
struct PREDICTEDMOVEMENT_API FMovementModifier_WithCorrection final : FMovementModifier_LocalPredicted
{
	bool ServerCheckClientError(const TModifierStack& Modifiers) const
	{
		return Data.Modifiers != Modifiers;
	}

	void OnClientCorrectionReceived(const TModifierStack& Modifiers)
	{
		Data.WantsModifiers = Modifiers;
	}
};

struct PREDICTEDMOVEMENT_API FModifierStatics
{
	static bool NetSerialize(TModifierStack& Modifiers, FArchive& Ar, const FString& ErrorName, uint8 MaxSerializedModifiers=8);

	static TModSize UpdateModifierLevel(EModifierLevelMethod Method, const TModifierStack& Modifiers, TModSize MaxLevel, TModSize InvalidLevel);
	static TModSize CombineModifierLevels(EModifierLevelMethod Method, const TModifierStack& ModifierLevels, TModSize MaxLevel, TModSize InvalidLevel);
	
	template<
	typename TLocalPredicted,
	typename TCorrection,
	typename TServer>
	static bool ProcessModifiers(
	TModSize& CurrentLevel,
	EModifierLevelMethod Method,
	const TArray<FGameplayTag>& LevelTags,
	TModSize InvalidLevel,
	TLocalPredicted* Local,
	TCorrection* Correction,
	TServer* Server,
	TFunctionRef<bool()> CanActivateCallback)
	{
		const TModSize PrevLevel = CurrentLevel;
		bool bStateChanged = false;

		// Update state
		bStateChanged |= Local ? Local->UpdateMovementState(CanActivateCallback()) : false;
		bStateChanged |= Correction ? Correction->UpdateMovementState(CanActivateCallback()) : false;
		bStateChanged |= Server ? Server->UpdateMovementState(CanActivateCallback()) : false;

		if (bStateChanged)
		{
			// Determine the maximum level based on the available tags
			const TModSize MaxLevel = LevelTags.Num() > 0 ? static_cast<TModSize>(LevelTags.Num() - 1) : 0;

			// Update the modifier levels based on the method
			const TModSize LocalLevel      = Local		? UpdateModifierLevel(Method, Local->Data.Modifiers, MaxLevel, InvalidLevel) : InvalidLevel;
			const TModSize CorrectionLevel = Correction ? UpdateModifierLevel(Method, Correction->Data.Modifiers, MaxLevel, InvalidLevel) : InvalidLevel;
			const TModSize ServerLevel     = Server		? UpdateModifierLevel(Method, Server->Data.Modifiers, MaxLevel, InvalidLevel) : InvalidLevel;

			// Combine the levels into a single current level
			TArray<TModSize> Levels;
			if (LocalLevel != InvalidLevel)      { Levels.Add(LocalLevel); }
			if (CorrectionLevel != InvalidLevel) { Levels.Add(CorrectionLevel); }
			if (ServerLevel != InvalidLevel)     { Levels.Add(ServerLevel); }

			CurrentLevel = Levels.Num() > 0 ? CombineModifierLevels(Method, Levels, MaxLevel, InvalidLevel) : InvalidLevel;
			return CurrentLevel != PrevLevel;
		}

		return false;
	}
};