#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "HeliMovement.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHeliMvmt, Log, All);


USTRUCT(DisplayName="Rotor Setup")
struct ROTARYWINGAIRCRAFT_API FRWA_RotorSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Rotor Setup")
	FName BoneName = EName::None;

	UPROPERTY(EditAnywhere, Category="Rotor Setup")
	FVector TorqueNormal = FVector::UpVector;
};


UCLASS(
	ClassGroup=(Custom),
	DisplayName="Heli Movement Component",
	HideCategories=(
		"PlanarMovement", "Components|Movement|Planar",
		"Activation", "Components|Activation"
	),
	meta=(BlueprintSpawnableComponent)
)
class ROTARYWINGAIRCRAFT_API URWA_HeliMovementComponent
	: public UPawnMovementComponent
{
	GENERATED_BODY()

	using Self = URWA_HeliMovementComponent;

public:

	using TickFn = FActorComponentTickFunction;

	URWA_HeliMovementComponent();


	// Blueprint Configuration --------------------------------------------------

public:

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float RPM = 350;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float EnginePower = 400;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float SpoolUpTime = 10;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float CyclicSensitivity = 1;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float AntiTorqueSensitivity = 1;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	float Agility = 1;

	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	TArray<FRWA_RotorSetup> Rotors;

	/**
	 * X-Axis: Altitude (meters)
	 * Y-Axis: Main rotor effectiveness (0-1)
	 */
	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	TObjectPtr<UCurveFloat> AltitudePenaltyCurve;

	/**
	 * X-Axis: Angle of Attack (degrees)
	 * Y-Axis: Drag Coefficient
	 */
	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup")
	TObjectPtr<UCurveFloat> DragCoefficientCurve;

	/**
	 * When the aircraft is traveling at moderate airspeeds, the aerodynamic
	 * shape of the vehicle will tend to "pull" the vehicle's orientation to face
	 * the direction of travel. This curve determines the strength of that
	 * effect.
	 *
	 * X-Axis: Airspeed (m/s)
	 * Y-Axis: Influence of the torque applied by aerodynamics (0-1)
	 */
	UPROPERTY(EditDefaultsOnly, Category="VehicleSetup",
		DisplayName="Aerodynamic Torque Influence"
	)
	TObjectPtr<UCurveFloat> AeroTorqueInfluence;

	UPROPERTY(EditAnywhere, Category="Vehicle")
	bool DebugPhysics = false;


	// Blueprint Getters --------------------------------------------------------

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetCurrentRPM() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetCurrentCollective() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	FVector2D GetCurrentCyclic() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetCurrentTorque() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	FVector GetVelocity() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetLateralAirspeed() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetLateralAirspeedKnots() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetVerticalAirspeed() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetHeadingDegrees() const;

	UFUNCTION(BlueprintPure, Category="Components|Movement|Heli")
	float GetRadarAltitude() const;


	// Blueprint Methods --------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void StartEngine();

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void StopEngine();

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void SetCollectiveInput(float value);

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void SetPitchInput(float value);

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void SetRollInput(float value);

	UFUNCTION(BlueprintCallable, Category="Components|Movement|Heli")
	void SetYawInput(float value);


	// Lifecycle & Events -------------------------------------------------------

public:

	void TickComponent(float deltaTime, ELevelTick type, TickFn* fn) override;


protected:

	FCalculateCustomPhysics OnCalculateCustomPhysics;

	void SetUpdatedComponent(USceneComponent* cmp) override;

	void SubstepTick(float deltaTime, FBodyInstance* body);

	virtual void UpdateEngineState(float deltaTime);
	virtual void UpdatePhysicsState(float deltaTime, FBodyInstance* body);
	virtual void UpdateSimulation(float deltaTime, FBodyInstance* body) const;


private:

	// Data Structures ----------------------------------------------------------

	struct FInput
	{
		float Collective = 0;
		float Pitch = 0;
		float Roll = 0;
		float Yaw = 0;
	};

	enum class EEngineState : uint8
	{
		Off,
		SpoolingUp,
		Running,
		SpoolingDown,
	};

	// Helper for tracking engine state transitions during the "spooling" phases
	struct FEngineState
	{
		EEngineState Phase = EEngineState::Off;
		float SpoolAlpha = 0;
		float PowerAlpha = 0;
		float RPM = 0;
	};

	// Container for data read from the physics body
	struct FPhysicsState
	{
		float Mass = 0;
		float CrossSectionalArea = 0;
		/** In radians */
		float AngleOfAttack = 0;
		FVector CoM = FVector::ZeroVector;
		FVector LinearVelocity = FVector::ZeroVector;
		FVector AngularVelocity = FVector::ZeroVector;
		FVector DeltaVelocity = FVector::ZeroVector;
		FVector GForce = FVector::ZeroVector;
	};

	// Details ------------------------------------------------------------------

	FInput m_Input;
	FEngineState m_EngineState;
	FPhysicsState m_PhysicsState;

	inline static float const k_Gravity = -981;
	inline static float const k_CmPerSecToKnots = 0.019438;

	APawn* GetPawn() const;
	FBodyInstance* GetBodyInstance() const;

	/**
	 * IMPORTANT: This should only be called from a FPhysicsCommand read/write
	 * callback with a locked mutex.
	 */
	float ComputeCrossSectionalArea(
		FBodyInstance const* body,
		FPhysicsActorHandle const& handle,
		FVector const& velocityDirection)
		const;

	FVector ComputeThrust(FVector const& pos, float mass) const;
	FVector ComputeDrag(FVector const& velocity, float aoa, float area) const;
	FVector ComputeTorque(FVector const& angularVelocity, float mass) const;
	void ComputeAeroTorque(FVector const& velocity, float mass, FVector& inout_torque) const;

	FVector Forward() const;
	FVector Right() const;
	FVector Up() const;

	void DebugPhysicsSimulation(
		FVector const& centerOfMass,
		FVector const& linearVelocity,
		FVector const& thrust,
		FVector const& drag,
		float crossSectionalArea)
		const;
};
