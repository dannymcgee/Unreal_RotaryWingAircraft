#include "RWA/HeliMovement.h"

#include "RWA/Util.h"

DEFINE_LOG_CATEGORY(LogHeliMvmt)

#define HELI_LOG(msg, ...) UE_LOG(LogHeliMvmt, Log, TEXT(msg), __VA_ARGS__)
#define HELI_WARN(msg, ...) UE_LOG(LogHeliMvmt, Warning, TEXT(msg), __VA_ARGS__)

#define Self URWA_HeliMovementComponent


Self::URWA_HeliMovementComponent() : Super() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	OnCalculateCustomPhysics.BindUObject(this, &Self::SubstepTick);
}


// Lifecycle & Events ----------------------------------------------------------

void Self::TickComponent(float deltaTime, ELevelTick type, TickFn* fn) {
	Super::TickComponent(deltaTime, type, fn);

	if (auto* body = GetBodyInstance())
		body->AddCustomPhysics(OnCalculateCustomPhysics);
	else {
		HELI_WARN("Failed to get body instance!");
	}
}

void Self::SetUpdatedComponent(USceneComponent* cmp) {
	Super::SetUpdatedComponent(cmp);

	PawnOwner = cmp ? Cast<APawn>(cmp->GetOwner()) : nullptr;

	if (auto* mesh = Cast<USkeletalMeshComponent>(cmp))
		mesh->bLocalSpaceKinematics = true;
	else {
		HELI_WARN("Failed to cast component to USkeletalMeshComponent!");
	}
}

void Self::SubstepTick(float deltaTime, FBodyInstance* body) {
	UpdateEngineState(deltaTime);
	UpdatePhysicsState(deltaTime, body);
	UpdateSimulation(deltaTime, body);
}

void Self::UpdateEngineState(float deltaTime) {
	using namespace RWA;

	auto& state = _EngineState;

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

			auto sinAlpha = Util::CurveSin(state.SpoolAlpha);
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

void Self::UpdatePhysicsState(float deltaTime, FBodyInstance* body) {
	auto transform = GetOwner()->GetActorTransform();

	FPhysicsCommand::ExecuteRead(body->ActorHandle, [&](const FPhysicsActorHandle& handle) {
		auto mass = FPhysicsInterface::GetMass_AssumesLocked(handle);
		auto com = FPhysicsInterface::GetComTransform_AssumesLocked(handle).GetLocation();
		auto lv = body->GetUnrealWorldVelocity_AssumesLocked();
		auto av = body->GetUnrealWorldAngularVelocityInRadians_AssumesLocked();
		auto dv = lv - _PhysicsState.LinearVelocity;
		auto aoa = FMath::Asin((Up() | lv) / lv.Size());
		auto gForce = transform.InverseTransformVector(dv / (k_Gravity * deltaTime));

		_PhysicsState.Mass = mass;
		_PhysicsState.CoM = com;
		_PhysicsState.LinearVelocity = lv;
		_PhysicsState.AngularVelocity = av;
		_PhysicsState.DeltaVelocity = dv;
		_PhysicsState.GForce = gForce;
		_PhysicsState.AngleOfAttack = aoa;

		if (lv.Size() < 100) {
			_PhysicsState.CrossSectionalArea = 0;
			return;
		}

		_PhysicsState.CrossSectionalArea =
			ComputeCrossSectionalArea(body, handle, lv.GetSafeNormal());
	});
}

void Self::UpdateSimulation(float deltaTime, FBodyInstance* body) const {
	auto mass = _PhysicsState.Mass;
	auto com = _PhysicsState.CoM;
	auto lv = _PhysicsState.LinearVelocity;
	auto av = _PhysicsState.AngularVelocity;
	auto surfArea = _PhysicsState.CrossSectionalArea;
	auto aoa = _PhysicsState.AngleOfAttack;

	auto dv = ComputeThrust(com, mass);
	auto drag = ComputeDrag(lv, aoa, surfArea);
	auto torque = lv.IsNearlyZero(10.f)
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

auto Self::ComputeCrossSectionalArea(
	const FBodyInstance* body,
	const FPhysicsActorHandle& handle,
	FVector velocityNormal
) const -> float {
	auto bb = FPhysicsInterface::GetBounds_AssumesLocked(handle);
	auto extent = bb.GetExtent().GetAbsMax();

	// dp = direction plane: a plane perpendicular to the direction of travel.
	// We'll fire a bunnch of line traces from various points on this plane
	// toward the vehicle, and use the proportion of hits to roughly estimate
	// the cross-sectional area of the vehicle for drag calculations.
	auto dpCenter = bb.GetCenter() + velocityNormal * extent;
	auto dpNormal = velocityNormal * -1;
	auto dpTan = FVector {};
	auto dpBinorm = FVector {};
	dpNormal.FindBestAxisVectors(dpTan, dpBinorm);

	auto step = extent / 16.0;
	auto hit = FHitResult {};
	auto hits = 0;
	auto total = 0;

	for (auto x = -extent; x < extent; x += step) {
		for (auto y = -extent; y < extent; y += step) {
			++total;

			auto p1 = dpCenter + (dpTan * x) + (dpBinorm * y);
			auto p2 = p1 + (dpNormal * extent * 4.0);

			if (body->LineTrace(hit, p1, p2, false))
				++hits;
		}
	}

	return ((double) hits / (double) total) * extent * 4.0;
}


auto Self::ComputeThrust(const FVector& pos, float mass) const -> FVector {
	using namespace RWA;

	// Scale the collective input by the current engine power
	auto scaledInput = _Input.Collective * _EngineState.PowerAlpha;

	// Compute the base thrust magnitude
	auto magicTuningValue = 3.4525;
	auto thrust = scaledInput * EnginePower * magicTuningValue;

	// Ground effect - increases rotor efficiency when altitude < 80m
	// TODO: Make the ground effect altitude curve configurable
	auto agl = GetRadarAltitude();
	auto geAlpha = Util::InverseLerp(agl, 80'00, 0);
	auto groundEffect = FMath::Clamp(geAlpha * EnginePower * scaledInput, 0, EnginePower);

	// Altitude penalty - decreases rotor efficiency at high altitudes
	auto altPenalty = 1.0;
	if (AltitudePenaltyCurve)
		altPenalty = AltitudePenaltyCurve->GetFloatValue(pos.Z / 100.0);

	return mass * ((thrust * altPenalty) + groundEffect) * Up();
}

auto Self::ComputeDrag(const FVector& velocity, float aoa, float area) const -> FVector {
	using namespace RWA;

	auto aoaAbs = FMath::Abs(aoa);
	auto cd = 0.0;
	if (DragCoefficientCurve) {
		cd = DragCoefficientCurve->GetFloatValue(FMath::RadiansToDegrees(aoaAbs));
	} else {
		auto aoaAlpha = Util::InverseLerp(aoaAbs, 0, PI / 2);
		cd = FMath::Lerp(0.667, 1.5, aoaAlpha);
	}

	auto rho = 0.01225; // TODO: Modulate air density by altitude
	auto v = velocity.Size() / 15.0;
	auto drag = 0.5 * cd * rho * v * v * area;

	// Convert a portion of drag to lift when pitching up (i.e. "cyclic climb")
	auto stallAngle = FMath::DegreesToRadians(30);
	auto lift = 0.f;
	if (aoa < 0 && aoaAbs < stallAngle) {
		auto ideal = stallAngle * 0.5f;
		auto factor = 1.f - (FMath::Abs(aoaAbs - ideal) / ideal);
		lift = factor * drag;
		drag -= lift;
	}

	auto dragVector = velocity.GetSafeNormal() * -drag;
	auto liftVector = Up() * lift;

	return dragVector + liftVector;
}

auto Self::ComputeTorque(const FVector& angularVelocity, float mass) const -> FVector {
	auto pitch = Right() * _Input.Pitch * CyclicSensitivity;
	auto roll = Forward() * -_Input.Roll * CyclicSensitivity;
	auto yaw = Up() * _Input.Yaw * AntiTorqueSensitivity;

	auto target = pitch + roll + yaw;
	auto inputTorque = target - angularVelocity;

	return inputTorque * mass * 1'000'00 * Agility;
}

void Self::ComputeAeroTorque(
	const FVector& velocity,
	float mass,
	FVector& inout_torque
) const {
	auto* pawn = GetPawn();
	if (!ensure(pawn)) return;

	auto vRel = pawn
		->GetTransform()
		.InverseTransformVector(velocity)
		.RotateAngleAxis(15, FVector::RightVector);

	auto thetaZ = FMath::Atan2(vRel.Y, vRel.X);
	auto thetaX = FMath::Atan2(vRel.Z, vRel.Y);

	auto latVel = FVector { vRel.X, vRel.Y, 0 };
	auto influence = AeroTorqueInfluence->GetFloatValue(latVel.Size() / 100);

	inout_torque += (Up() * thetaZ * mass * 120 * 1000 * influence);
	inout_torque += (Right() * -thetaX * mass * 120 * 50 * influence);
}


// Utility ---------------------------------------------------------------------

auto Self::GetPawn() const -> APawn* {
	if (!UpdatedComponent) return nullptr;
	return Cast<APawn>(UpdatedComponent->GetOwner());
}

auto Self::GetBodyInstance() const -> FBodyInstance* {
	if (!UpdatedComponent) return nullptr;

	auto* cmp = Cast<USkeletalMeshComponent>(UpdatedComponent);
	if (!cmp) return nullptr;

	return cmp->GetBodyInstance();
}

auto Self::Forward() const -> FVector {
	auto* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorForwardVector();

	return FVector::ForwardVector;
}

auto Self::Right() const -> FVector {
	auto* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorRightVector();

	return FVector::RightVector;
}

auto Self::Up() const -> FVector {
	auto* pawn = GetPawn();
	if (ensure(pawn != nullptr))
		return pawn->GetActorUpVector();

	return FVector::UpVector;
}


// Debug -----------------------------------------------------------------------

void Self::DebugPhysicsSimulation(
	const FVector& centerOfMass,
	const FVector& linearVelocity,
	const FVector& thrust,
	const FVector& drag,
	double crossSectionalArea
) const {
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
	auto circleY = FVector {};
	auto circleZ = FVector {};
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

auto Self::GetCurrentRPM() const -> float {
	return _EngineState.RPM;
}

auto Self::GetCurrentCollective() const -> float {
	return _Input.Collective;
}

auto Self::GetCurrentCyclic() const -> FVector2D {
	return { _Input.Roll, _Input.Pitch };
}

auto Self::GetCurrentTorque() const -> float {
	return _Input.Yaw;
}

auto Self::GetVelocity() const -> FVector {
	return _PhysicsState.LinearVelocity;
}

auto Self::GetLateralAirspeed() const -> float {
	auto lv = _PhysicsState.LinearVelocity;
	auto latVel = FVector { lv.X, lv.Y, 0 };

	return latVel.Size();
}

auto Self::GetLateralAirspeedKnots() const -> float {
	return GetLateralAirspeed() * k_CmPerSecToKnots;
}

auto Self::GetVerticalAirspeed() const -> float {
	return _PhysicsState.LinearVelocity.Z;
}

auto Self::GetHeadingDegrees() const -> float {
	auto direction = FVector
		::VectorPlaneProject(Forward(), FVector::UpVector)
		.GetSafeNormal();

	return FMath::RadiansToDegrees(FMath::Atan2(-direction.Y, -direction.X)) + 180.0;
}

auto Self::GetRadarAltitude() const -> float {
	auto* pawn = GetPawn();
	if (pawn == nullptr) return INFINITY;

	auto params = FCollisionQueryParams::DefaultQueryParam;
	params.AddIgnoredActor(pawn);

	auto start = _PhysicsState.CoM;
	auto end = start + FVector::DownVector * 2000'00.f;
	auto hit = FHitResult {};

	if (GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_WorldStatic, params))
		return FVector::Dist(start, hit.Location);

	return INFINITY;
}


// Blueprint Methods -----------------------------------------------------------

void Self::StartEngine() {
	if (_EngineState.Phase != EEngineState::Running)
		_EngineState.Phase = EEngineState::SpoolingUp;
}

void Self::StopEngine() {
	if (_EngineState.Phase != EEngineState::Off)
		_EngineState.Phase = EEngineState::SpoolingDown;
}

void Self::SetCollectiveInput(float value) {
	if (value > 0 && _EngineState.Phase == EEngineState::Off)
		StartEngine();

	_Input.Collective = value;
}

void Self::SetPitchInput(float value) {
	_Input.Pitch = value;
}

void Self::SetRollInput(float value) {
	_Input.Roll = value;
}

void Self::SetYawInput(float value) {
	_Input.Yaw = value;
}


#undef Self
#undef HELI_LOG
#undef HELI_WARN
