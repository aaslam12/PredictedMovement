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

	DOREPLIFETIME_CONDITION(ThisClass, bIsModified, COND_SimulatedOnly);
}

void AModifierCharacter::OnRep_IsModified()
{
	if (ModifierMovement)
	{
		if (bIsModified)
		{
			ModifierMovement->bWantsToModifier = true;
			ModifierMovement->Modifier(true);
		}
		else
		{
			ModifierMovement->bWantsToModifier = false;
			ModifierMovement->UnModifier(true);
		}
		ModifierMovement->bNetworkUpdateReceived = true;
	}
}

void AModifierCharacter::Modifier(bool bClientSimulation)
{
	if (ModifierMovement)
	{
		ModifierMovement->bWantsToModifier = true;
	}
}

void AModifierCharacter::UnModifier(bool bClientSimulation)
{
	if (ModifierMovement)
	{
		ModifierMovement->bWantsToModifier = false;
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