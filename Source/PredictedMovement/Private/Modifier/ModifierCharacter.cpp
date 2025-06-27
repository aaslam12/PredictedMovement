// Copyright (c) Jared Taylor


#include "Modifier/ModifierCharacter.h"

#include "Modifier/ModifierTags.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "PredictedMovement/Public/Modifier/ModifierMovement.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModifierCharacter)

void AModifierCharacter::OnModifierChanged(const FGameplayTag& ModifierType, uint8 ModifierLevel,
	uint8 PrevModifierLevel)
{
	if (ModifierLevel > 0 && PrevModifierLevel == 0)
	{
		OnModifierAdded(ModifierType, ModifierLevel, PrevModifierLevel);
	}
	else if (ModifierLevel == 0 && PrevModifierLevel > 0)
	{
		OnModifierRemoved(ModifierType, ModifierLevel, PrevModifierLevel);
	}

	K2_OnModifierChanged(ModifierType, ModifierLevel, PrevModifierLevel);

	// Replicate to simulated proxies
	if (HasAuthority())
	{
		if (ModifierType == FModifierTags::Modifier_Type_Boost)
		{
			SimulatedBoost = ModifierLevel;
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, SimulatedBoost, this);
		}
	}
}

void AModifierCharacter::OnModifierAdded(const FGameplayTag& ModifierType, uint8 ModifierLevel, uint8 PrevModifierLevel)
{
	
}

void AModifierCharacter::OnModifierRemoved(const FGameplayTag& ModifierType, uint8 ModifierLevel,
	uint8 PrevModifierLevel)
{
	
}

AModifierCharacter::AModifierCharacter(const FObjectInitializer& FObjectInitializer)
	: Super(FObjectInitializer.SetDefaultSubobjectClass<UModifierMovement>(CharacterMovementComponentName))
{
	ModifierMovement = Cast<UModifierMovement>(GetCharacterMovement());
}

void AModifierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Push Model
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.Condition = COND_SimulatedOnly;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SimulatedBoost, SharedParams);
}

void AModifierCharacter::OnRep_SimulatedBoost(uint8 PrevLevel)
{
	if (ModifierMovement)
	{
		// @TODO if (SimulatedModifierLevel != PrevLevel)
		// {
		// 	ModifierMovement->WantsModifierLevel = ModifierLevel;
		// 	ModifierMovement->ChangeModifier(ModifierLevel, true, PrevLevel);
		// 	ModifierMovement->bNetworkUpdateReceived = true;
		// }
	}
}

void AModifierCharacter::AddModifier(uint8 InLevel, bool bClientSimulation)
{
	if (ModifierMovement)
	{
		ModifierMovement->BoostServer.AddModifier(InLevel);
		// ModifierMovement->AddWantedModifier(InLevel);
	}
}

void AModifierCharacter::RemoveModifier(uint8 InLevel, bool bClientSimulation)
{
	if (ModifierMovement)
	{
		ModifierMovement->BoostServer.RemoveModifier(InLevel);
		// ModifierMovement->RemoveWantedModifier(InLevel);
	}
}

void AModifierCharacter::OnEndModifier()
{
	K2_OnEndModifier();
}

void AModifierCharacter::OnStartModifier()
{
	K2_OnStartModifier();
}