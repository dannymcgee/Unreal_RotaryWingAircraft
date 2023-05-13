#include "RWA/Heli.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RWA/HeliMovement.h"

DEFINE_LOG_CATEGORY(LogHeli)

#define HELI_LOG(msg, ...) UE_LOG(LogHeli, Log, TEXT(msg), __VA_ARGS__)
#define HELI_WARN(msg, ...) UE_LOG(LogHeli, Warning, TEXT(msg), __VA_ARGS__)

#define Self ARWA_Heli


// Initialization --------------------------------------------------------------

Self::ARWA_Heli() : Super()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	m_Mesh = InitSkelMesh();
	RootComponent = m_Mesh;

	m_VehicleMovement = InitVehicleMovement(m_Mesh);
}

void Self::BeginPlay()
{
	Super::BeginPlay();

	AddInputMappingContext();
}

auto Self::InitSkelMesh() -> USkeletalMeshComponent*
{
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

auto Self::InitVehicleMovement(USkeletalMeshComponent* mesh) -> URWA_HeliMovementComponent*
{
	auto* cmp = CreateDefaultSubobject<URWA_HeliMovementComponent>(TEXT("VehicleMovement"));
	cmp->SetIsReplicated(true);
	cmp->UpdatedComponent = mesh;

	return cmp;
}


// Getters ---------------------------------------------------------------------

auto Self::GetMesh() const -> USkeletalMeshComponent*
{
	return m_Mesh;
}

auto Self::GetVehicleMovement() const -> URWA_HeliMovementComponent*
{
	return m_VehicleMovement;
}

auto Self::GetMovementComponent() const -> UPawnMovementComponent*
{
	return m_VehicleMovement;
}


// Input handling --------------------------------------------------------------

void Self::AddInputMappingContext() const
{
	if (auto* inputSubsys = GetInputSubsystem())
		inputSubsys->AddMappingContext(DefaultMappingContext, 0);
}

void Self::RemoveInputMappingContext() const
{
	if (auto* inputSubsys = GetInputSubsystem())
		inputSubsys->RemoveMappingContext(DefaultMappingContext);
}

auto Self::GetInputSubsystem() const -> IEnhancedInputSubsystemInterface*
{
	auto* pc = Cast<APlayerController>(Controller);
	if (pc == nullptr) return nullptr;

	auto* lp = pc->GetLocalPlayer();
	if (lp == nullptr) return nullptr;

	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(lp);
}

void Self::SetupPlayerInputComponent(UInputComponent* inputCmp)
{
	if (auto* input = CastChecked<UEnhancedInputComponent>(inputCmp)) {
		input->BindAction(CyclicAction, ETriggerEvent::Triggered, this, &Self::OnCyclic);
		input->BindAction(CyclicAction, ETriggerEvent::Completed, this, &Self::OnCyclic);

		input->BindAction(CollectiveAction, ETriggerEvent::Triggered, this, &Self::OnCollective);
		input->BindAction(CollectiveAction, ETriggerEvent::Completed, this, &Self::OnCollective);

		input->BindAction(AntiTorqueAction, ETriggerEvent::Triggered, this, &Self::OnAntiTorque);
		input->BindAction(AntiTorqueAction, ETriggerEvent::Completed, this, &Self::OnAntiTorque);
	}
	else {
		HELI_WARN(
			"Failed to cast InputComponent to EnhancedInputComponent. The Heli "
			"actor requires an EnhancedInputComponent by default."
		);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Self::OnCyclic(FInputActionValue const& value)
{
	auto input = value.Get<FVector2D>();

	m_VehicleMovement->SetPitchInput(input.Y);
	m_VehicleMovement->SetRollInput(input.X);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Self::OnCollective(FInputActionValue const& value)
{
	m_VehicleMovement->SetCollectiveInput(value.Get<float>());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Self::OnAntiTorque(FInputActionValue const& value)
{
	m_VehicleMovement->SetYawInput(value.Get<float>());
}


#undef Self
#undef HELI_LOG
#undef HELI_WARN
