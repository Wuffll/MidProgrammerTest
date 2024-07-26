// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MidProgrammerTestCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
struct FInputActionValue;

class UHealthComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMidProgrammerTestCharacter : public ACharacter
{
	GENERATED_BODY()

// ---------------------------------------------- Variables ---------------------------------------------- 

#pragma region Components

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

#pragma endregion

#pragma region Input
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

#pragma endregion

#pragma region Explosion

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	const float ExplosionDamage = 30.0f;
	const float ExplosionRadius = 200.0f; // in cm
	const float ShotRange = 2000.0f; // in cm

#pragma endregion


#pragma region HUD

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "HUD")
	TObjectPtr<UUserWidget> HUDWidget;

#pragma endregion

#pragma region Health

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

#pragma endregion

// ---------------------------------------------- Functions ---------------------------------------------- 

#pragma region General

public:

	AMidProgrammerTestCharacter();

protected:
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void BeginPlay();

#pragma endregion

#pragma region Move&Look

protected:

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

#pragma endregion

#pragma region Fire

protected:

	void Fire(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void ProcessFire();

	UFUNCTION(NetMulticast, Reliable)
	void SpawnExplosionEmitterMulticast(FVector position);

#pragma endregion

#pragma region Health
public:
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:

	UFUNCTION(Client, Reliable)
	void Die();

#pragma endregion

#pragma region Getters

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma endregion


};

