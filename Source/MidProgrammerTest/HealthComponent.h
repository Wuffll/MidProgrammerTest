// All Rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MIDPROGRAMMERTEST_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

// ---------------------------------------------- Variables ---------------------------------------------- 
protected:

	float MaxHealth;

	UPROPERTY(Replicated)
	float CurrentHealth;

// ---------------------------------------------- Functions ----------------------------------------------
public:	
	// Sets default values for this component's properties
	UHealthComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	void SetCurrentHealth(float value);

	void GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

};
