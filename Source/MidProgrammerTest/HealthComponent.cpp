// All Rights reserved. 


#include "HealthComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

static constexpr float MAX_HEALTH = 120.0f;
static constexpr float STARTING_HEALTH = 100.0f;

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
	:
	MaxHealth(MAX_HEALTH),
	CurrentHealth(STARTING_HEALTH)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::SetCurrentHealth(float value)
{
	CurrentHealth = FMath::Clamp(value, 0.0f, MaxHealth);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}