#include "RWA/HeliMovement.h"

#include "RWA/Util.h"

DEFINE_LOG_CATEGORY(LogHeliMvmt)

#define HELI_LOG(msg, ...) UE_LOG(LogHeliMvmt, Log, TEXT(msg), __VA_ARGS__)
#define HELI_WARN(msg, ...) UE_LOG(LogHeliMvmt, Warning, TEXT(msg), __VA_ARGS__)


URWA_HeliMovementComponent::URWA_HeliMovementComponent() : Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	OnCalculateCustomPhysics.BindUObject(this, &Self::SubstepTick);
}


// Lifecycle & Events ----------------------------------------------------------

void URWA_HeliMovementComponent::TickComponent(float deltaTime, ELevelTick type, TickFn* fn)
{
	Super::TickComponent(deltaTime, type, fn);

	if (FBodyInstance* body = GetBodyInstance())
		body->AddCustomPhysics(OnCalculateCustomPhysics);
	else {
		HELI_WARN("Failed to get body instance!");
	}
}

void URWA_HeliMovementComponent::SetUpdatedComponent(USceneComponent* cmp)
{
	Super::SetUpdatedComponent(cmp);

	PawnOwner = cmp ? Cast<APawn>(cmp->GetOwner()) : nullptr;

	if (auto* mesh = Cast<USkeletalMeshComponent>(cmp))
		mesh->bLocalSpaceKinematics = true;
	else {
		HELI_WARN("Failed to cast component to USkeletalMeshComponent!");
	}
}

void URWA_HeliMovementComponent::SubstepTick(float deltaTime, FBodyInstance* body)
{
	UpdateEngineState(deltaTime);
	UpdatePhysicsState(deltaTime, body);
	UpdateSimulation(deltaTime, body);
}

void URWA_HeliMovementComponent::UpdateEngineState(float deltaTime)
{
	using namespace RWA;

	FEngineState& state = m_EngineState;

	switch (state.Phase) {
		case EEngineState::SpoolingUp: {
			state.SpoolAlpha += (1 / SpoolUpTime) * deltaTime;

			auto sinAlpha = Util::CurveSin(state.SpoolAlpha);
			state.PowerAlpha = Util::InverseLerp(sinAlpha, 0.667, 1.0);

			if (state.SpoolAlpha >= 1) {
				state.SpoolAlpha = 1;
				state.PowerAlpha = 1;
				state.Phase = EEngineState::Running;
				state.RPM = RPM;
			} else {
				state.RPM = RPM * sinAlpha;
			}
		} break;

		case EEngineState::SpoolingDown: {
			state.SpoolAlpha -= (1 / SpoolUpTime) * deltaTime;

			float sinAlpha = Util::CurveSin(state.SpoolAlpha);
			state.PowerAlpha = Util::InverseLerp(sinAlpha, 0.667, 1.0);

			if (state.SpoolAlpha <= 0) {
				state.SpoolAlpha = 0;
				state.PowerAlpha = 0;
				state.Phase = EEngineState::Off;
				state.RPM = 0;
			} else {
				state.RPM = RPM * sinAlpha;
			}
		} break;

		default: break;
	}
}

void URWA_HeliMovementComponent::UpdatePhysicsState(float deltaTime, FBodyInstance* body)
{
	FTransform transform = GetOwner()->GetActorTransform();

	FPhysicsCommand::ExecuteRead(body->ActorHandle, [&](FPhysicsActorHandle const& handle)
	{
		float mass = FPhysicsInterface::GetMass_AssumesLocked(handle);
		FVector com = FPhysicsInterface::GetComTransform_AssumesLocked(handle).GetLocation();
		FVector lv = body->GetUnrealWorldVelocity_AssumesLocked();
		FVector av = body->GetUnrealWorldAngularVelocityInRadians_AssumesLocked();
		FVector dv = lv - m_PhysicsState.LinearVelocity;
		float aoa = FMath::Asin((Up() | lv) / lv.Size());
		FVector gForce = transform.InverseTransformVector(dv / (k_Gravity * deltaTime));

		m_PhysicsState.Mass = mass;
		m_PhysicsState.CoM = com;
		m_PhysicsState.LinearVelocity = lv;
		m_PhysicsState.AngularVelocity = av;
		m_PhysicsState.DeltaVelocity = dv;
		m_PhysicsState.GForce = gForce;
		m_PhysicsState.AngleOfAttack = aoa;

		if (lv.Size() < 100)
		{
			m_PhysicsState.CrossSectionalArea = 0;
			return;
		}

		m_PhysicsState.CrossSectionalArea =
			ComputeCrossSectionalArea(body, handle, lv.GetSafeNormal());
	});
}

void URWA_HeliMovementComponent::UpdateSimulation(float deltaTime, FBodyInstance* body) const
{
	float mass = m_PhysicsState.Mass;
	FVector com = m_PhysicsState.CoM;
	FVector lv = m_PhysicsState.LinearVelocity;
	FVector av = m_PhysicsState.AngularVelocity;
	float surfArea = m_PhysicsState.CrossSectionalArea;
	float aoa = m_PhysicsState.AngleOfAttack;

	FVector dv = ComputeThrust(com, mass);
	FVector drag = ComputeDrag(lv, aoa, surfArea);
	FVector torque = lv.IsNearlyZero(10.f)
		? FVector::ZeroVector
		: ComputeTorque(av, mass);

	if (DebugPhysics)
		DebugPhysicsSimulation(com, lv, dv, drag, surfArea);

	if (AeroTorqueInfluence)
		ComputeAeroTorque(lv, mass, torque);

	body->AddForce(dv + drag);
	body->AddTorqueInRadians(torque);
}


// Physics Calculations --------------------------------------------------------

float URWA_HeliMovementComponent::ComputeCrossSectionalArea(
	FBodyInstance const* body,
	FPhysicsActorHandle const& handle,
	FVector const& velocityDirection)
	const
{
	FBox bb = FPhysicsInterface::GetBounds_AssumesLocked(handle);
	float extent = bb.GetExtent().GetAbsMax();

	// dp = direction plane: a plane perpendicular to the direction of travel.
	// We'll fire a bunnch of line traces from various points on this plane
	// toward the vehicle, and use the proportion of hits to roughly estimate
	// the cross-sectional area of the vehicle for drag calculations.
	FVector dpCenter = bb.GetCenter() + velocityDirection * extent;
	FVector dpNormal = velocityDirection * -1;
	FVector dpTan, dpBinorm;
	dpNormal.FindBestAxisVectors(dpTan, dpBinorm);

	float step = extent / 16.0;
	FHitResult hit;
	int32 hits = 0, total = 0;

	for (float x = -extent; x < extent; x += step)
	{
		for (float y = -extent; y < extent; y += step)
		{
			++total;

			FVector p1 = dpCenter + (dpTan * x) + (dpBinorm * y);
			FVector p2 = p1 + (dpNormal * extent * 4.0);

			if (body->LineTrace(hit, p1, p2, false))
				++hits;
		}
	}

	return ((float) hits / (float) total) * extent * 4.0;
}


FVector URWA_HeliMovementComponent::ComputeThrust(FVector const& pos, float mass) const
{
	using namespace RWA;

	// Scale the collective input by the current engine power
	float scaledInput = m_Input.Collective * m_EngineState.PowerAlpha;

	// Compute the base thrust magnitude
	float thrust = scaledInput >= 0
		? FMath::Lerp(0.0, -k_Gravity + EnginePower, scaledInput)
		: FMath::Lerp(0.0, k_Gravity - EnginePower, FMath::Abs(scaledInput));

	// Ground effect - increases rotor efficiency when altitude < 80m
	// TODO: Make the ground effect altitude curve configurable
	float agl = GetRadarAltitude();
	float geAlpha = Util::InverseLerp(agl, 80'00, 0);
	float groundEffect = FMath::Clamp(geAlpha * EnginePower * scaledInput, 0, EnginePower);

	// Altitude penalty - decreases rotor efficiency at high altitudes
	float altPenalty = 1.0;
	if (AltitudePenaltyCurve)
		altPenalty = AltitudePenaltyCurve->GetFloatValue(pos.Z / 100.0);

	return mass * ((thrust * altPenalty) + groundEffect) * Up();
}

FVector URWA_HeliMovementComponent::ComputeDrag(
	FVector const& velocity,
	float aoa,
	float area)
	const 
{
	using namespace RWA;

	float aoaAbs = FMath::Abs(aoa);
	float cd = 0.0;
	if (DragCoefficientCurve)
	{
		cd = DragCoefficientCurve->GetFloatValue(FMath::RadiansToDegrees(aoaAbs));
	}
	else
	{
		float aoaAlpha = Util::InverseLerp(aoaAbs, 0, PI / 2);
		cd = FMath::Lerp(0.667, 1.5, aoaAlpha);
	}

	float rho = 0.01225; // TODO: Modulate air density by altitude
	float v = velocity.Size() / 15.0;
	float drag = 0.5 * cd * rho * v * v * area;

	// Convert a portion of drag to lift when pitching up (i.e. "cyclic climb")
	float stallAngle = FMath::DegreesToRadians(30);
	float lift = 0.f;
	
	if (aoa < 0 && aoaAbs < stallAngle)
	{
		float ideal = stallAngle * 0.5f;
		float factor = 1.f - (FMath::Abs(aoaAbs - ideal) / ideal);
		lift = factor * drag;
		drag -= lift;
	}

	FVector dragVector = velocity.GetSafeNormal() * -drag;
	FVector liftVector = Up() * lift;

	return dragVector + liftVector;
}

FVector URWA_HeliMovementComponent::ComputeTorque(FVector const& angularVelocity, float mass) const
{
	FVector pitch = Right() * m_Input.Pitch * CyclicSensitivity;
	FVector roll = Forward() * -m_Input.Roll * CyclicSensitivity;
	FVector yaw = Up() * m_Input.Yaw * AntiTorqueSensitivity;

	FVector target = pitch + roll + yaw;
	FVector inputTorque = target - angularVelocity;

	return inputTorque * mass * 1'000'00 * Agility;
}

void URWA_HeliMovementComponent::ComputeAeroTorque(
	FVector const& velocity,
	float mass,
	FVector& inout_torque)
	const
{
	APawn* pawn = GetPawn();
	if (!ensure(pawn)) return;

	FVector vRel = pawn
		->GetTransform()
		.InverseTransformVector(velocity)
		.RotateAngleAxis(15, FVector::RightVector);

	float thetaZ = FMath::Atan2(vRel.Y, vRel.X);
	float thetaX = FMath::Atan2(vRel.Z, vRel.Y);

	FVector latVel { vRel.X, vRel.Y, 0 };
	float influence = AeroTorqueInfluence->GetFloatValue(latVel.Size() / 100.0);

	inout_torque += (Up() * thetaZ * mass * 120 * 1000 * influence);
	inout_torque += (Right() * -thetaX * mass * 120 * 50 * influence);
}


// Utility ---------------------------------------------------------------------

APawn* URWA_HeliMovementComponent::GetPawn() const 
{
	if (!UpdatedComponent) return nullptr;
	return Cast<APawn>(UpdatedComponent->GetOwner());
}

FBodyInstance* URWA_HeliMovementComponent::GetBodyInstance() const 
{
	if (!UpdatedComponent) return nullptr;

	auto* cmp = Cast<USkeletalMeshComponent>(UpdatedComponent);
	if (!cmp) return nullptr;

	return cmp->GetBodyInstance();
}

FVector URWA_HeliMovementComponent::Forward() const 
{
	APawn* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorForwardVector();

	return FVector::ForwardVector;
}

FVector URWA_HeliMovementComponent::Right() const 
{
	APawn* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorRightVector();

	return FVector::RightVector;
}

FVector URWA_HeliMovementComponent::Up() const 
{
	APawn* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorUpVector();

	return FVector::UpVector;
}


// Debug -----------------------------------------------------------------------

void URWA_HeliMovementComponent::DebugPhysicsSimulation(
	FVector const& centerOfMass,
	FVector const& linearVelocity,
	FVector const& thrust,
	FVector const& drag,
	float crossSectionalArea)
	const
{
	// Draw a sphere around the center of mass
	DrawDebugSphere(GetWorld(), centerOfMass, 35, 8, FColor::White, false, -1, 128, 5);

	// Draw a line indicating linear velocity
	DrawDebugLine(
		GetWorld(),
		centerOfMass,
		centerOfMass + linearVelocity,
		FColor::Cyan,
		false, -1, 0,
		(linearVelocity.Size() / 200.0));

	// Draw a line for the thrust vector
	DrawDebugLine(
		GetWorld(),
		centerOfMass,
		centerOfMass + (thrust / 1000.0),
		FColor::Green,
		false, -1, 0,
		(thrust.Size() / 50'000.0)
	);

	// Draw a circle to represent the cross-sectional area
	FVector circleY, circleZ;
	linearVelocity.GetSafeNormal().FindBestAxisVectors(circleY, circleZ);
	DrawDebugCircle(
		GetWorld(),
		centerOfMass,
		FMath::Sqrt((crossSectionalArea * 100.0) / PI),
		32,
		FColor::Magenta,
		false,
		-1,
		255,
		5,
		circleY,
		circleZ
	);

	// Draw a line for the drag vector
	DrawDebugLine(
		GetWorld(),
		centerOfMass,
		centerOfMass + (drag / 500.0),
		FColor::Red,
		false, -1, 0,
		(drag.Size() / 50'000.0)
	);
}


// Blueprint Getters -----------------------------------------------------------

float URWA_HeliMovementComponent::GetCurrentRPM() const
{
	return m_EngineState.RPM;
}

float URWA_HeliMovementComponent::GetCurrentCollective() const
{
	return m_Input.Collective;
}

FVector2D URWA_HeliMovementComponent::GetCurrentCyclic() const 
{
	return { m_Input.Roll, m_Input.Pitch };
}

float URWA_HeliMovementComponent::GetCurrentTorque() const
{
	return m_Input.Yaw;
}

FVector URWA_HeliMovementComponent::GetVelocity() const 
{
	return m_PhysicsState.LinearVelocity;
}

float URWA_HeliMovementComponent::GetLateralAirspeed() const
{
	FVector lv = m_PhysicsState.LinearVelocity;
	FVector latVel { lv.X, lv.Y, 0 };

	return latVel.Size();
}

float URWA_HeliMovementComponent::GetLateralAirspeedKnots() const
{
	return GetLateralAirspeed() * k_CmPerSecToKnots;
}

float URWA_HeliMovementComponent::GetVerticalAirspeed() const
{
	return m_PhysicsState.LinearVelocity.Z;
}

float URWA_HeliMovementComponent::GetHeadingDegrees() const
{
	FVector direction = FVector::VectorPlaneProject(Forward(), FVector::UpVector);
	direction.Normalize();

	return FMath::RadiansToDegrees(FMath::Atan2(-direction.Y, -direction.X)) + 180.0;
}

float URWA_HeliMovementComponent::GetRadarAltitude() const
{
	APawn* pawn = GetPawn();
	if (pawn == nullptr) return INFINITY;

	auto params = FCollisionQueryParams::DefaultQueryParam;
	params.AddIgnoredActor(pawn);

	FVector start = m_PhysicsState.CoM;
	FVector end = start + FVector::DownVector * 2000'00.f;
	FHitResult hit;

	if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic, params))
		return FVector::Dist(start, hit.Location);

	return INFINITY;
}


// Blueprint Methods -----------------------------------------------------------

void URWA_HeliMovementComponent::StartEngine()
{
	if (m_EngineState.Phase != EEngineState::Running)
		m_EngineState.Phase = EEngineState::SpoolingUp;
}

void URWA_HeliMovementComponent::StopEngine()
{
	if (m_EngineState.Phase != EEngineState::Off)
		m_EngineState.Phase = EEngineState::SpoolingDown;
}

void URWA_HeliMovementComponent::SetCollectiveInput(float value)
{
	if (value > 0 && m_EngineState.Phase == EEngineState::Off)
		StartEngine();

	m_Input.Collective = value;
}

void URWA_HeliMovementComponent::SetPitchInput(float value)
{
	m_Input.Pitch = value;
}

void URWA_HeliMovementComponent::SetRollInput(float value)
{
	m_Input.Roll = value;
}

void URWA_HeliMovementComponent::SetYawInput(float value)
{
	m_Input.Yaw = value;
}


#undef HELI_LOG
#undef HELI_WARN
