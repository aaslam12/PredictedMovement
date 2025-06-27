// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "ModifierCompression.h"

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

	void UpdateMovementState(bool bAllowedInCurrentState)
	{
		Data.Modifiers = bAllowedInCurrentState ? Data.WantsModifiers : TArray<TEnum>();
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

	void UpdateMovementState(bool bAllowedInCurrentState)
	{
		Data.Modifiers = bAllowedInCurrentState ? Data.WantsModifiers : TArray<TEnum>();
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