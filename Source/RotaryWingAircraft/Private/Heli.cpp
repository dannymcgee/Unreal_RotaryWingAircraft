#include "Heli.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeliMvmtCmp.h"

DEFINE_LOG_CATEGORY(LogHeli)

#define HELI_LOG(msg, ...) UE_LOG(LogHeli, Log, TEXT(msg), __VA_ARGS__)
#define HELI_WARN(msg, ...) UE_LOG(LogHeli, Warning, TEXT(msg), __VA_ARGS__)


// Initialization --------------------------------------------------------------

AHeli::AHeli() : Super() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	_Mesh = InitSkelMesh();
	RootComponent = _Mesh;

	_VehicleMovement = InitVehicleMovement(_Mesh);
}

void AHeli::BeginPlay() {
	Super::BeginPlay();

	AddInputMappingContext();
}

auto AHeli::InitSkelMesh() -> USkeletalMeshComponent* {
	auto* mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
	mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	mesh->BodyInstance.bSimulatePhysics = true;
	mesh->BodyInstance.bNotifyRigidBodyCollision = true;
	mesh->BodyInstance.bUseCCD = true;
	mesh->bBlendPhysics = true;
	mesh->SetGenerateOverlapEvents(true);
	mesh->SetCanEverAffectNavigation(false); // TODO: Is this correct?

	return mesh;
}

auto AHeli::InitVehicleMovement(USkeletalMeshComponent* mesh) -> UHeliMvmtCmp* {
	auto* cmp = CreateDefaultSubobject<UHeliMvmtCmp>(TEXT("VehicleMovement"));
	cmp->SetIsReplicated(true);
	cmp->UpdatedComponent = mesh;

	return cmp;
}


// Getters ---------------------------------------------------------------------

auto AHeli::GetMesh() const -> USkeletalMeshComponent* {
	return _Mesh;
}

auto AHeli::GetVehicleMovement() const -> UHeliMvmtCmp* {
	return _VehicleMovement;
}

auto AHeli::GetMovementComponent() const -> UPawnMovementComponent* {
	return _VehicleMovement;
}


// Input handling --------------------------------------------------------------

void AHeli::AddInputMappingContext() const {
	if (auto* inputSubsys = GetInputSubsystem())
		inputSubsys->AddMappingContext(DefaultMappingContext, 0);
}

void AHeli::RemoveInputMappingContext() const {
	if (auto* inputSubsys = GetInputSubsystem())
		inputSubsys->RemoveMappingContext(DefaultMappingContext);
}

auto AHeli::GetInputSubsystem() const -> IEnhancedInputSubsystemInterface* {
	auto* pc = Cast<APlayerController>(Controller);
	if (pc == nullptr) return nullptr;

	auto* lp = pc->GetLocalPlayer();
	if (lp == nullptr) return nullptr;

	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(lp);
}

void AHeli::SetupPlayerInputComponent(UInputComponent* inputCmp) {
	if (auto* input = CastChecked<UEnhancedInputComponent>(inputCmp)) {
		input->BindAction(CyclicAction, ETriggerEvent::Triggered, this, &AHeli::OnCyclic);
		input->BindAction(CyclicAction, ETriggerEvent::Completed, this, &AHeli::OnCyclic);

		input->BindAction(CollectiveAction, ETriggerEvent::Triggered, this, &AHeli::OnCollective);
		input->BindAction(CollectiveAction, ETriggerEvent::Completed, this, &AHeli::OnCollective);

		input->BindAction(AntiTorqueAction, ETriggerEvent::Triggered, this, &AHeli::OnAntiTorque);
		input->BindAction(AntiTorqueAction, ETriggerEvent::Completed, this, &AHeli::OnAntiTorque);
	}
	else {
		HELI_WARN(
			"Failed to cast InputComponent to EnhancedInputComponent. The Heli "
			"actor requires an EnhancedInputComponent by default."
		);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AHeli::OnCyclic(const FInputActionValue& value) {
	auto input = value.Get<FVector2D>();

	_VehicleMovement->SetPitchInput(input.Y);
	_VehicleMovement->SetRollInput(input.X);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AHeli::OnCollective(const FInputActionValue& value) {
	_VehicleMovement->SetCollectiveInput(value.Get<float>());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AHeli::OnAntiTorque(const FInputActionValue& value) {
	_VehicleMovement->SetYawInput(value.Get<float>());
}


#undef HELI_LOG
#undef HELI_WARN
