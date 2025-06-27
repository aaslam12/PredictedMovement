// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModifierCompression.h"
#include "ModifierTypes.h"
#include "Algo/Accumulate.h"
#include "Algo/MaxElement.h"
#include "Algo/MinElement.h"

template <typename T>
struct TIsValidModifierEnum : std::false_type {};

template <>
struct TIsValidModifierEnum<EModifierByte> : std::true_type {};

template <>
struct TIsValidModifierEnum<EModifierShort> : std::true_type {};

template <>
struct TIsValidModifierEnum<EModifierLong> : std::true_type {};

/**
 * FSavedMove_Character
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FModifierSavedMove
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
				  "FModifierSavedMove only supports uint8, uint16, or uint32 as T.");

	static_assert(TIsValidModifierEnum<TEnum>::value,
		"FModifierSavedMove only supports EModifierByte, EModifierShort, or EModifierLong as TEnum.");
	
	T WantsModifiers;

	FModifierSavedMove()
		: WantsModifiers(0)
	{}
	
	virtual ~FModifierSavedMove() = default;

	virtual void Clear()
	{
		WantsModifiers = 0;
	}

	void SetMoveFor(TArray<TEnum> Modifiers)
	{
		WantsModifiers = FModifierCompression::GetBitmaskFromModifiers(Modifiers);
	}

	bool CanCombineWith(T Modifiers) const
	{
		return WantsModifiers == Modifiers;
	}

	void SetInitialPosition(TArray<TEnum> Modifiers)
	{
		WantsModifiers = FModifierCompression::GetBitmaskFromModifiers(Modifiers);
	}

	bool IsImportantMove(T Modifiers) const
	{
		return WantsModifiers != Modifiers;
	}
};

/**
 * FSavedMove_Character
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FModifierSavedMove_WithCorrection final : FModifierSavedMove<T, TEnum>
{
	using Super = FModifierSavedMove<T, TEnum>;
	
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
				  "FModifierSavedMove only supports uint8, uint16, or uint32 as T.");

	static_assert(TIsValidModifierEnum<TEnum>::value,
		"FModifierSavedMove only supports EModifierByte, EModifierShort, or EModifierLong as TEnum.");
	
	T Modifiers;

	FModifierSavedMove_WithCorrection()
		: Super()
		, Modifiers(0)
	{}

	virtual void Clear() override
	{
		Super::Clear();
		Modifiers = 0;
	}

	void PostUpdate(TArray<TEnum> InModifiers)
	{
		Modifiers = FModifierCompression::GetBitmaskFromModifiers(InModifiers);
	}
};

/**
 * FSavedMove_Character
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FModifierSavedMove_ServerInitiated
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
				  "FModifierSavedMove only supports uint8, uint16, or uint32 as T.");

	static_assert(TIsValidModifierEnum<TEnum>::value,
		"FModifierSavedMove only supports EModifierByte, EModifierShort, or EModifierLong as TEnum.");
	
	T Modifiers;

	FModifierSavedMove_ServerInitiated()
		: Modifiers(0)
	{}

	void Clear()
	{
		Modifiers = 0;
	}

	void PostUpdate(TArray<TEnum> InModifiers)
	{
		Modifiers = FModifierCompression::GetBitmaskFromModifiers(InModifiers);
	}
};

/**
 * FCharacterMoveResponseDataContainer
 * Only required when using WithCorrection or ServerInitiated modifiers
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FModifierMoveResponse
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
	"FModifierMoveResponse only supports uint8, uint16, or uint32 as T.");

	static_assert(TIsValidModifierEnum<TEnum>::value,
		"FModifierMoveResponse only supports EModifierByte, EModifierShort, or EModifierLong as TEnum.");
	
	T Modifiers;

	void ServerFillResponseData(TArray<TEnum> InModifiers)
	{
		Modifiers = FModifierCompression::GetBitmaskFromModifiers(InModifiers);  // AUTH
	}
};

/**
 * FCharacterNetworkMoveData
 * Sends wanted modifiers (via input) to the server to be applied to the character
 */
template <typename T>
struct PREDICTEDMOVEMENT_API FModifierMoveData_LocalPredicted
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
	              "FModifierMoveData only supports uint8, uint16, or uint32 as T.");

	FModifierMoveData_LocalPredicted()
		: WantsModifiers(0)
	{}
	
	T WantsModifiers;

	void ClientFillNetworkMoveData(T InWantsModifiers)
	{
		WantsModifiers = InWantsModifiers;
	}
};

/**
 * FCharacterNetworkMoveData
 * Sends wanted modifiers (via input) to the server to be applied to the character
 * Server compares the client and server modifiers to know when to send a net correction to the client with updated modifiers
 */
template <typename T>
struct PREDICTEDMOVEMENT_API FModifierMoveData_WithCorrection
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
				  "FModifierMoveData only supports uint8, uint16, or uint32 as T.");

	FModifierMoveData_WithCorrection()
		: WantsModifiers(0)
		, Modifiers(0)
	{}

	T WantsModifiers;
	T Modifiers;

	void ClientFillNetworkMoveData(T InWantsModifiers, T InModifiers)
	{
		WantsModifiers = InWantsModifiers;
		Modifiers = InModifiers;
	}
};

/**
 * FCharacterNetworkMoveData
 * Used by server to compare between client and server, to know when to send a net correction to the client with updated modifiers
 */
template <typename T>
struct PREDICTEDMOVEMENT_API FModifierMoveData_ServerInitiated
{
	static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
				  "FModifierMoveData only supports uint8, uint16, or uint32 as T.");

	FModifierMoveData_ServerInitiated()
		: Modifiers(0)
	{}
	
	T Modifiers;

	void ClientFillNetworkMoveData(T InModifiers)
	{
		Modifiers = InModifiers;
	}
};

/**
 * Represents a single modifier that can be applied to a character
 */
template <typename TEnum>
struct PREDICTEDMOVEMENT_API FMovementModifierData
{
	/**
	 * The requested input state, which requests modifiers of the specified level
	 * uint8 allows 8 modifiers, uint16 allows 16 modifiers, uint32 allows 32 modifiers, and uint64 allows 64 modifiers.
	 */
	TArray<TEnum> WantsModifiers;
	
	/**
	 * The actual state, which represents the actual modifiers applied to the character
	 * uint8 allows 8 modifiers, uint16 allows 16 modifiers, uint32 allows 32 modifiers, and uint64 allows 64 modifiers.
	 */
	TArray<TEnum> Modifiers;
};

/**
 * Represents a single modifier that can be applied to a character
 * Local Predicted modifier is activated via player input and is predicted on the client
 * e.g. Sprint, Crouch, etc.
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FMovementModifier_LocalPredicted
{
	FMovementModifier_LocalPredicted()
		: BitSize(sizeof(T) * 8)
	{}

	const uint8 BitSize;
	
	FMovementModifierData<EModifierByte> Data;

	T GetWantedModifierLevel() const { return Data.WantsModifiers.Num(); }
	T GetModifierLevel() const { return Data.Modifiers.Num(); }

	bool AddModifier(T InLevel)
	{
		if (!FModifierCompression::IsValidBitPosition(InLevel)) { return false; }
		
		const EModifierByte Level = static_cast<EModifierByte>(1 << InLevel);
		if (!Data.WantsModifiers.Contains(Level))
		{
			Data.WantsModifiers.Add(Level);
			return true;
		}
		return false;
	}

	bool RemoveModifier(T InLevel)
	{
		if (!FModifierCompression::IsValidBitPosition(InLevel)) { return false; }
		
		const EModifierByte Level = static_cast<EModifierByte>(1 << InLevel);
		if (Data.WantsModifiers.Contains(Level))
		{
			Data.WantsModifiers.Remove(Level);
			return true;
		}
		return false;
	}

	bool UpdateMovementState(bool bAllowedInCurrentState)
	{
		const TArray<TEnum> Modifiers = bAllowedInCurrentState ? Data.WantsModifiers : TArray<TEnum>();
		if (Data.Modifiers != Modifiers)
		{
			Data.Modifiers = Modifiers;
			return true;
		}
		return false;
	}

	void ServerMove_PerformMovement(T WantsModifiers)
	{
		Data.WantsModifiers = FModifierCompression::SetModifiersFromBitmask<TEnum>(WantsModifiers);
	}

	void CombineWith(T WantsModifiers)
	{
		Data.WantsModifiers = FModifierCompression::SetModifiersFromBitmask<TEnum>(WantsModifiers);
	}
};

/**
 * Represents a single modifier that can be applied to a character
 * Local Predicted modifier is activated via a predicted state (GAS) and is predicted on the client
 * However, if the server disagrees with the client, it will correct the client
 * e.g. Speed increase after equipping a knife via predicted inventory ability, etc.
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FMovementModifier_LocalPredicted_WithCorrection final : FMovementModifier_LocalPredicted<T, TEnum>
{
	bool ServerCheckClientError(T Modifiers)
	{
		return FModifierCompression::GetBitmaskFromModifiers(this->Data.Modifiers) != Modifiers;
	}

	void OnClientCorrectionReceived(T Modifiers)
	{
		this->Data.WantsModifiers = FModifierCompression::SetModifiersFromBitmask<TEnum>(Modifiers);
	}
};

/**
 * Represents a single modifier that can be applied to a character
 * Server Authority modifier is activated by the server and is not predicted on the client
 * e.g. Snare from a damage event, speed increase after equipping a knife from a server event, etc.
 */
template <typename T, typename TEnum>
struct PREDICTEDMOVEMENT_API FMovementModifier_ServerInitiated
{
	FMovementModifier_ServerInitiated()
		: BitSize(sizeof(T) * 8)
	{}

	const uint8 BitSize;
	
	FMovementModifierData<EModifierByte> Data;

	T GetWantedModifierLevel() const { return Data.WantsModifiers.Num(); }
	T GetModifierLevel() const { return Data.Modifiers.Num(); }

	bool AddModifier(T InLevel)
	{
		if (!FModifierCompression::IsValidBitPosition(InLevel)) { return false; }
		
		const EModifierByte Level = static_cast<EModifierByte>(1 << InLevel);
		if (!Data.WantsModifiers.Contains(Level))
		{
			Data.WantsModifiers.Add(Level);
			return true;
		}
		return false;
	}

	// @TODO move everything to cpp

	bool RemoveModifier(T InLevel)
	{
		if (!FModifierCompression::IsValidBitPosition(InLevel)) { return false; }
		
		const EModifierByte Level = static_cast<EModifierByte>(1 << InLevel);
		if (Data.WantsModifiers.Contains(Level))
		{
			Data.WantsModifiers.Remove(Level);
			return true;
		}
		return false;
	}

	bool UpdateMovementState(bool bAllowedInCurrentState)
	{
		const TArray<TEnum> Modifiers = bAllowedInCurrentState ? Data.WantsModifiers : TArray<TEnum>();
		if (Data.Modifiers != Modifiers)
		{
			Data.Modifiers = Modifiers;
			return true;
		}
		return false;
	}
	
	bool ServerCheckClientError(T Modifiers)
	{
		return FModifierCompression::GetBitmaskFromModifiers(this->Data.Modifiers) != Modifiers;
	}

	void OnClientCorrectionReceived(T Modifiers)
	{
		Data.WantsModifiers = FModifierCompression::SetModifiersFromBitmask<TEnum>(Modifiers);
	}
};

struct PREDICTEDMOVEMENT_API FModifierStatics
{
	template<
	typename T, typename TEnum,
	typename TLocalPredicted,
	typename TCorrection,
	typename TServer>
	static bool ProcessModifiers(
	T& OutCurrentLevel,
	EModifierLevelMethod Method,
	const TArray<FGameplayTag>& LevelTags,
	TLocalPredicted* Local,
	TCorrection* Correction,
	TServer* Server,
	TFunctionRef<bool()> CanActivateCallback)
	{
		const T PrevLevel = OutCurrentLevel;
		bool bStateChanged = false;

		// Update state
		bStateChanged |= Local ? Local->UpdateMovementState(CanActivateCallback()) : false;
		bStateChanged |= Correction ? Correction->UpdateMovementState(CanActivateCallback()) : false;
		bStateChanged |= Server ? Server->UpdateMovementState(CanActivateCallback()) : false;

		if (bStateChanged)
		{
			// Determine the maximum level based on the available tags
			const T MaxLevel = LevelTags.Num() > 0 ? static_cast<T>(LevelTags.Num() - 1) : 0;

			// Update the modifier levels based on the method
			const T LocalLevel      = Local		 ? FModifierStatics::UpdateModifierLevel<T, TEnum>(Method, Local->Data.Modifiers, MaxLevel) : 0;
			const T CorrectionLevel = Correction ? FModifierStatics::UpdateModifierLevel<T, TEnum>(Method, Correction->Data.Modifiers, MaxLevel) : 0;
			const T ServerLevel     = Server	 ? FModifierStatics::UpdateModifierLevel<T, TEnum>(Method, Server->Data.Modifiers, MaxLevel) : 0;

			// Combine the levels into a single current level
			TArray<T> Levels;
			if (LocalLevel > 0)      { Levels.Add(LocalLevel); }
			if (CorrectionLevel > 0) { Levels.Add(CorrectionLevel); }
			if (ServerLevel > 0)     { Levels.Add(ServerLevel); }
			if (Levels.IsEmpty())
			{
				OutCurrentLevel = 0;
				return PrevLevel != OutCurrentLevel;
			}
			
			OutCurrentLevel = FModifierStatics::CombineModifierLevels<T>(Method, Levels, MaxLevel);
			return OutCurrentLevel != PrevLevel;
		}

		return false;
	}
	
	static int32 GetBitPosition(uint32 Value)
	{
		if (Value == 0) return -1;
		return FMath::FloorLog2(Value);
	}
	
	template <typename T, typename TEnum>
	static T UpdateModifierLevel(EModifierLevelMethod Method, const TArray<TEnum>& Modifiers, uint8 MaxLevel)
	{
		static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
			"UpdateModifierLevel only supports uint8, uint16, or uint32 as T.");

		static_assert(TIsValidModifierEnum<TEnum>::value,
			"UpdateModifierLevel only supports EModifierByte, EModifierShort, or EModifierLong as TEnum.");

		using Underlying = std::underlying_type_t<TEnum>;

		T NewLevel = 0;

		switch (Method)
		{
		case EModifierLevelMethod::Max:
			if (!Modifiers.IsEmpty())
			{
				auto MaxEnum = *Algo::MaxElement(Modifiers);
				NewLevel = static_cast<T>(GetBitPosition(static_cast<Underlying>(MaxEnum)));
			}
			break;

		case EModifierLevelMethod::Min:
			if (!Modifiers.IsEmpty())
			{
				auto MinEnum = *Algo::MinElement(Modifiers);
				NewLevel = static_cast<T>(GetBitPosition(static_cast<Underlying>(MinEnum)));
			}
			break;

		case EModifierLevelMethod::Stack:
			for (const TEnum& Level : Modifiers)
			{
				NewLevel += GetBitPosition(static_cast<Underlying>(Level));
			}
			break;

		case EModifierLevelMethod::Average:
			if (!Modifiers.IsEmpty())
			{
				Underlying Total = 0;
				for (const TEnum& Level : Modifiers)
				{
					Total += GetBitPosition(static_cast<Underlying>(Level));
				}
				NewLevel = static_cast<T>(Total / Modifiers.Num());
			}
			break;
		}

		// Don't exceed the maximum level
		NewLevel = FMath::Min(NewLevel, MaxLevel);
		
		return NewLevel;
	}

	template <typename T>
	static T CombineModifierLevels(EModifierLevelMethod Method, const TArray<T>& ModifierLevels, uint8 MaxLevel)
	{
		static_assert(std::is_same_v<T, uint8> || std::is_same_v<T, uint16> || std::is_same_v<T, uint32>,
			"CombineModifierLevels only supports uint8, uint16, or uint32 as T.");

		if (ModifierLevels.IsEmpty())
		{
			return 0;
		}

		T NewLevel = 0;

		switch (Method)
		{
		case EModifierLevelMethod::Max:
			NewLevel = *Algo::MaxElement(ModifierLevels);
			break;

		case EModifierLevelMethod::Min:
			NewLevel = *Algo::MinElement(ModifierLevels);
			break;

		case EModifierLevelMethod::Stack:
			for (const T& Level : ModifierLevels)
			{
				NewLevel += Level;
			}
			break;

		case EModifierLevelMethod::Average:
			{
				uint64 Total = 0; // Prevent overflow
				for (const T& Level : ModifierLevels)
				{
					Total += Level;
				}
				NewLevel = static_cast<T>(Total / ModifierLevels.Num());
			}
			break;
		}

		// Don't exceed the maximum level
		NewLevel = FMath::Min(NewLevel, MaxLevel);
		
		return NewLevel;
	}
};