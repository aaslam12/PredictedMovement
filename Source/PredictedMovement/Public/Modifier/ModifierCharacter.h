// Copyright (c) 2023 Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ModifierCharacter.generated.h"

class UModifierMovement;
UCLASS()
class PREDICTEDMOVEMENT_API AModifierCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Movement component used for movement logic in various movement modes (walking, falling, etc), containing relevant settings and functions to control movement. */
	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UModifierMovement> ModifierMovement;

	friend class FSavedMove_Character_Modifier;
protected:
	FORCEINLINE UModifierMovement* GetModifierCharacterMovement() const { return ModifierMovement; }
	
public:
	/** Set by character movement to specify that this Character is currently Modified. */
	UPROPERTY(BlueprintReadOnly, replicatedUsing=OnRep_IsModified, Category=Character)
	uint32 bIsModified:1;
	
public:
	AModifierCharacter(const FObjectInitializer& FObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Handle Crouching replicated from server */
	UFUNCTION()
	virtual void OnRep_IsModified();

	/**
	 * Request the character to start Modified. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnStartModifier
	 * @see IsModified
	 * @see CharacterMovement->WantsToModifier
	 */
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void Modifier(bool bClientSimulation = false);

	/**
	 * Request the character to stop Modified. The request is processed on the next update of the CharacterMovementComponent.
	 * @see OnEndModifier
	 * @see IsModified
	 * @see CharacterMovement->WantsToModifier
	 */
	UFUNCTION(BlueprintCallable, Category=Character, meta=(HidePin="bClientSimulation"))
	virtual void UnModifier(bool bClientSimulation = false);

	/** Called when Character stops Modified. Called on non-owned Characters through bIsModified replication. */
	virtual void OnEndModifier();

	/** Event when Character stops Modified. */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndModifier", ScriptName="OnEndModifier"))
	void K2_OnEndModifier();

	/** Called when Character Modifieres. Called on non-owned Characters through bIsModified replication. */
	virtual void OnStartModifier();

	/** Event when Character Modifieres. */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartModifier", ScriptName="OnStartModifier"))
	void K2_OnStartModifier();
};
