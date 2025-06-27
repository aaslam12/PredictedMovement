// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "ModifierCompression.generated.h"

UENUM()
enum class EModifierByte : uint8
{
	MOD_Level0 = 0x01, // 1 << 0
	MOD_Level1 = 0x02, // 1 << 1
	MOD_Level2 = 0x04, // 1 << 2
	MOD_Level3 = 0x08, // 1 << 3
	MOD_Level4 = 0x10, // 1 << 4
	MOD_Level5 = 0x20, // 1 << 5
	MOD_Level6 = 0x40, // 1 << 6
	MOD_Level7 = 0x80  // 1 << 7
};

UENUM()
enum class EModifierShort : uint16
{
	MOD_Level0  = 0x0001,  // 1 << 0
	MOD_Level1  = 0x0002,  // 1 << 1
	MOD_Level2  = 0x0004,  // 1 << 2
	MOD_Level3  = 0x0008,  // 1 << 3
	MOD_Level4  = 0x0010,  // 1 << 4
	MOD_Level5  = 0x0020,  // 1 << 5
	MOD_Level6  = 0x0040,  // 1 << 6
	MOD_Level7  = 0x0080,  // 1 << 7
	MOD_Level8  = 0x0100,  // 1 << 8
	MOD_Level9  = 0x0200,  // 1 << 9
	MOD_Level10 = 0x0400,  // 1 << 10
	MOD_Level11 = 0x0800,  // 1 << 11
	MOD_Level12 = 0x1000,  // 1 << 12
	MOD_Level13 = 0x2000,  // 1 << 13
	MOD_Level14 = 0x4000,  // 1 << 14
	MOD_Level15 = 0x8000   // 1 << 15
};

UENUM()
enum class EModifierLong : uint32
{
	MOD_Level0  = 0x00000001,  // 1 << 0
	MOD_Level1  = 0x00000002,  // 1 << 1
	MOD_Level2  = 0x00000004,  // 1 << 2
	MOD_Level3  = 0x00000008,  // 1 << 3
	MOD_Level4  = 0x00000010,  // 1 << 4
	MOD_Level5  = 0x00000020,  // 1 << 5
	MOD_Level6  = 0x00000040,  // 1 << 6
	MOD_Level7  = 0x00000080,  // 1 << 7
	MOD_Level8  = 0x00000100,  // 1 << 8
	MOD_Level9  = 0x00000200,  // 1 << 9
	MOD_Level10 = 0x00000400,  // 1 << 10
	MOD_Level11 = 0x00000800,  // 1 << 11
	MOD_Level12 = 0x00001000,  // 1 << 12
	MOD_Level13 = 0x00002000,  // 1 << 13
	MOD_Level14 = 0x00004000,  // 1 << 14
	MOD_Level15 = 0x00008000,  // 1 << 15
	MOD_Level16 = 0x00010000,  // 1 << 16
	MOD_Level17 = 0x00020000,  // 1 << 17
	MOD_Level18 = 0x00040000,  // 1 << 18
	MOD_Level19 = 0x00080000,  // 1 << 19
	MOD_Level20 = 0x00100000,  // 1 << 20
	MOD_Level21 = 0x00200000,  // 1 << 21
	MOD_Level22 = 0x00400000,  // 1 << 22
	MOD_Level23 = 0x00800000,  // 1 << 23
	MOD_Level24 = 0x01000000,  // 1 << 24
	MOD_Level25 = 0x02000000,  // 1 << 25
	MOD_Level26 = 0x04000000,  // 1 << 26
	MOD_Level27 = 0x08000000,  // 1 << 27
	MOD_Level28 = 0x10000000,  // 1 << 28
	MOD_Level29 = 0x20000000,  // 1 << 29
	MOD_Level30 = 0x40000000,  // 1 << 30
	MOD_Level31 = 0x80000000   // 1 << 31
};

template <typename TEnum>
TArray<TEnum> GetAllModifiers(); // Declaration

template <>
inline TArray<EModifierByte> GetAllModifiers<EModifierByte>()
{
	return {
		EModifierByte::MOD_Level0,
		EModifierByte::MOD_Level1,
		EModifierByte::MOD_Level2,
		EModifierByte::MOD_Level3,
		EModifierByte::MOD_Level4,
		EModifierByte::MOD_Level5,
		EModifierByte::MOD_Level6,
		EModifierByte::MOD_Level7
	};
}

template <>
inline TArray<EModifierShort> GetAllModifiers<EModifierShort>()
{
	return {
		EModifierShort::MOD_Level0,
		EModifierShort::MOD_Level1,
		EModifierShort::MOD_Level2,
		EModifierShort::MOD_Level3,
		EModifierShort::MOD_Level4,
		EModifierShort::MOD_Level5,
		EModifierShort::MOD_Level6,
		EModifierShort::MOD_Level7,
		EModifierShort::MOD_Level8,
		EModifierShort::MOD_Level9,
		EModifierShort::MOD_Level10,
		EModifierShort::MOD_Level11,
		EModifierShort::MOD_Level12,
		EModifierShort::MOD_Level13,
		EModifierShort::MOD_Level14,
		EModifierShort::MOD_Level15
	};
}

template <>
inline TArray<EModifierLong> GetAllModifiers<EModifierLong>()
{
	return {
		EModifierLong::MOD_Level0,
		EModifierLong::MOD_Level1,
		EModifierLong::MOD_Level2,
		EModifierLong::MOD_Level3,
		EModifierLong::MOD_Level4,
		EModifierLong::MOD_Level5,
		EModifierLong::MOD_Level6,
		EModifierLong::MOD_Level7,
		EModifierLong::MOD_Level8,
		EModifierLong::MOD_Level9,
		EModifierLong::MOD_Level10,
		EModifierLong::MOD_Level11,
		EModifierLong::MOD_Level12,
		EModifierLong::MOD_Level13,
		EModifierLong::MOD_Level14,
		EModifierLong::MOD_Level15,
		EModifierLong::MOD_Level16,
		EModifierLong::MOD_Level17,
		EModifierLong::MOD_Level18,
		EModifierLong::MOD_Level19,
		EModifierLong::MOD_Level20,
		EModifierLong::MOD_Level21,
		EModifierLong::MOD_Level22,
		EModifierLong::MOD_Level23,
		EModifierLong::MOD_Level24,
		EModifierLong::MOD_Level25,
		EModifierLong::MOD_Level26,
		EModifierLong::MOD_Level27,
		EModifierLong::MOD_Level28,
		EModifierLong::MOD_Level29,
		EModifierLong::MOD_Level30,
		EModifierLong::MOD_Level31
	};
}

class FModifierCompression
{
	template <typename TEnum>
	static constexpr auto ToUnderlying(TEnum e) noexcept
	{
		return static_cast<std::underlying_type_t<TEnum>>(e);
	}

public:
	template <typename T>
	static bool IsValidBitPosition(T Index)
	{
		static_assert(std::is_unsigned_v<T>, "IsValidBitPosition only supports unsigned integer types.");
		return Index < sizeof(T) * 8;
	}

	// GetBitmaskFromModifiers is more performant than SetModifiersFromBitmask
	
	template <typename TEnum, typename TUnderlying = std::underlying_type_t<TEnum>>
	static TUnderlying GetBitmaskFromModifiers(const TArray<TEnum>& Modifiers)
	{
		TUnderlying Bitmask = 0;
		for (const TEnum& Mod : Modifiers)
		{
			Bitmask |= ToUnderlying(Mod);
		}
		return Bitmask;
	}

	template <typename TEnum, typename TUnderlying = std::underlying_type_t<TEnum>>
	static TArray<TEnum> SetModifiersFromBitmask(TUnderlying Bitmask)
	{
		TArray<TEnum> Result;
		const TArray<TEnum> KnownModifiers = GetAllModifiers<TEnum>();

		for (const TEnum& Mod : KnownModifiers)
		{
			const TUnderlying Flag = ToUnderlying(Mod);
			if ((Bitmask & Flag) == Flag)
			{
				Result.Add(Mod);
				Bitmask &= ~Flag;
			}
		}
		return Result;
	}
};
