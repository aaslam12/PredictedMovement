// Copyright (c) 2023 Jared Taylor. All Rights Reserved.


#include "Modifier/ModifierCharacter.h"

#include "Net/UnrealNetwork.h"
#include "PredictedMovement/Public/Modifier/ModifierMovement.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModifierCharacter)

AModifierCharacter::AModifierCharacter(const FObjectInitializer& FObjectInitializer)
	: Super(FObjectInitializer.SetDefaultSubobjectClass<UModifierMovement>(CharacterMovementComponentName))
{
	ModifierMovement = Cast<UModifierMovement>(GetCharacterMovement());
}

void AModifierCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, SimulatedModifierLevel, COND_SimulatedOnly);
}

void AModifierCharacter::OnRep_SimulatedModifierLevel(uint8 PrevLevel)
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
		ModifierMovement->AddWantedModifier(InLevel);
	}
}

void AModifierCharacter::RemoveModifier(uint8 InLevel, bool bClientSimulation)
{
	if (ModifierMovement)
	{
		ModifierMovement->RemoveWantedModifier(InLevel);
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