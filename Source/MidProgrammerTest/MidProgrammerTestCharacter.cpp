// Copyright Epic Games, Inc. All Rights Reserved.

#include "MidProgrammerTestCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Blueprint/UserWidget.h"

#include "Kismet/GameplayStatics.h"

#include "HealthComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMidProgrammerTestCharacter::AMidProgrammerTestCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetIsReplicated(true);
}

void AMidProgrammerTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetController<APlayerController>();
	if (PC)
		UE_LOG(LogTemplateCharacter, Log, TEXT("PC is set!"));

}

void AMidProgrammerTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Look);

		// LeftClick
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMidProgrammerTestCharacter::Fire);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

#pragma region Move&Look

void AMidProgrammerTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMidProgrammerTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma endregion

#pragma region Fire

void AMidProgrammerTestCharacter::Fire(const FInputActionValue& Value)
{
	// Process on server
	ProcessFire();
}

void AMidProgrammerTestCharacter::ProcessFire_Implementation()
{
	const FVector DirectionVector = FollowCamera->GetForwardVector();
	const FVector CameraPosition = FollowCamera->GetComponentLocation();

	FVector ExplosionPos = CameraPosition + ShotRange * DirectionVector;

	FHitResult HitResult;
	bool HitAnything = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraPosition,
		ExplosionPos, {});

	if (HitAnything)// && HitResult.GetActor()->GetOwner() != GetController())
	{
		ExplosionPos = HitResult.Location;
	}

	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, ExplosionPos, ExplosionRadius, nullptr, {});
	SpawnExplosionEmitterMulticast(ExplosionPos);
}

void AMidProgrammerTestCharacter::SpawnExplosionEmitterMulticast_Implementation(FVector position)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, position, FRotator(0.0, 0.0, 0.0));
}

#pragma endregion

#pragma region Health

float AMidProgrammerTestCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float DamageApplied = 0.0f;

	// Processing on server; replicated on clients
	if (GetLocalRole() == ROLE_Authority)
	{
		const float HealthBeforeDamage = HealthComponent->GetCurrentHealth();

		if (HealthBeforeDamage > 0.0f)
		{
			HealthComponent->SetCurrentHealth(HealthBeforeDamage - DamageAmount);

			const float HealthAfterDamage = HealthComponent->GetCurrentHealth();
			DamageApplied = HealthBeforeDamage - HealthAfterDamage;

			UE_LOG(LogTemplateCharacter, Log, TEXT("Health: %f || Damage taken: %f \n"), HealthAfterDamage, DamageApplied);

			if (HealthAfterDamage <= 0.0f)
			{
				Die();
			}
		}
	}

	return DamageApplied;
}

void AMidProgrammerTestCharacter::Die_Implementation()
{
	// Destroy(); // kills the character

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->SetInputMode(FInputModeUIOnly());
	}

	UE_LOG(LogTemplateCharacter, Log, TEXT("Character has died! X("));
}

#pragma endregion