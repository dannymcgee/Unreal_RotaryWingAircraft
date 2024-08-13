#include "RWA/Heli.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "RWA/HeliMovement.h"

DEFINE_LOG_CATEGORY(LogHeli)

#define HELI_LOG(msg, ...) UE_LOG(LogHeli, Log, TEXT(msg), __VA_ARGS__)
#define HELI_WARN(msg, ...) UE_LOG(LogHeli, Warning, TEXT(msg), __VA_ARGS__)


// Initialization --------------------------------------------------------------

ARWA_Heli::ARWA_Heli(FObjectInitializer const& init)
	: Super(init)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	m_Mesh = InitSkelMesh();
	RootComponent = m_Mesh;

	m_VehicleMovement = InitVehicleMovement(m_Mesh);
}

void ARWA_Heli::BeginPlay()
{
	Super::BeginPlay();

	AddInputMappingContext();
}

USkeletalMeshComponent* ARWA_Heli::InitSkelMesh()
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

URWA_HeliMovementComponent* ARWA_Heli::InitVehicleMovement(USkeletalMeshComponent* mesh) 
{
	auto* cmp = CreateDefaultSubobject<URWA_HeliMovementComponent>(TEXT("VehicleMovement"));
	cmp->SetIsReplicated(true);
	cmp->UpdatedComponent = mesh;

	return cmp;
}


// Getters ---------------------------------------------------------------------

USkeletalMeshComponent* ARWA_Heli::GetMesh() const
{
	return m_Mesh;
}

URWA_HeliMovementComponent* ARWA_Heli::GetVehicleMovement() const
{
	return m_VehicleMovement;
}

UPawnMovementComponent* ARWA_Heli::GetMovementComponent() const
{
	return m_VehicleMovement;
}


// Input handling --------------------------------------------------------------

void ARWA_Heli::AddInputMappingContext() const
{
	if (IEnhancedInputSubsystemInterface* inputSubsys = GetInputSubsystem())
		inputSubsys->AddMappingContext(DefaultMappingContext, 0);
}

void ARWA_Heli::RemoveInputMappingContext() const
{
	if (IEnhancedInputSubsystemInterface* inputSubsys = GetInputSubsystem())
		inputSubsys->RemoveMappingContext(DefaultMappingContext);
}

IEnhancedInputSubsystemInterface* ARWA_Heli::GetInputSubsystem() const
{
	auto* pc = Cast<APlayerController>(Controller);
	if (pc == nullptr) return nullptr;

	ULocalPlayer* lp = pc->GetLocalPlayer();
	if (lp == nullptr) return nullptr;

	return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(lp);
}

void ARWA_Heli::SetupPlayerInputComponent(UInputComponent* inputCmp)
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
void ARWA_Heli::OnCyclic(FInputActionValue const& value)
{
	auto input = value.Get<FVector2D>();

	m_VehicleMovement->SetPitchInput(input.Y);
	m_VehicleMovement->SetRollInput(input.X);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ARWA_Heli::OnCollective(FInputActionValue const& value)
{
	m_VehicleMovement->SetCollectiveInput(value.Get<float>());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ARWA_Heli::OnAntiTorque(FInputActionValue const& value)
{
	m_VehicleMovement->SetYawInput(value.Get<float>());
}


#undef HELI_LOG
#undef HELI_WARN
