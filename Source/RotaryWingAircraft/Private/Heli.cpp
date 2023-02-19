#include "Heli.h"
#include "HeliMvmtCmp.h"


// Initialization --------------------------------------------------------------

AHeli::AHeli() : Super() {
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	_Mesh = InitSkelMesh();
	RootComponent = _Mesh;

	_VehicleMovement = InitVehicleMovement(_Mesh);
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

// TODO: Look at Lyra to come up with a better way to setup these inputs
void AHeli::SetupPlayerInputComponent(UInputComponent* input) {
	input->BindAction("StartEngine", IE_Pressed, _VehicleMovement, &UHeliMvmtCmp::StartEngine);
	input->BindAction("StopEngine", IE_Pressed, _VehicleMovement, &UHeliMvmtCmp::StopEngine);

	input->BindAxis("CollectiveUp", this, &AHeli::OnCollectiveUp);
	input->BindAxis("CollectiveDown", this, &AHeli::OnCollectiveDown);

	input->BindAction("YawRight", IE_Pressed, this, &AHeli::IncYaw);
	input->BindAction("YawRight", IE_Released, this, &AHeli::DecYaw);
	input->BindAction("YawLeft", IE_Pressed, this, &AHeli::DecYaw);
	input->BindAction("YawLeft", IE_Released, this, &AHeli::IncYaw);

	input->BindAxis("Pitch", _VehicleMovement, &UHeliMvmtCmp::SetPitchInput);
	input->BindAxis("Roll", _VehicleMovement, &UHeliMvmtCmp::SetRollInput);
}


// Update ----------------------------------------------------------------------

void AHeli::Tick(float deltaTime) {
	Super::Tick(deltaTime);

	_VehicleMovement->SetYawInput(_YawInput);
	_VehicleMovement->SetCollectiveInput(_CollectiveInput);

	_CollectiveInput = 0;
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

void AHeli::OnCollectiveUp(float input) {
	_CollectiveInput = FMath::Clamp(_CollectiveInput + input, -1, 1);
}

void AHeli::OnCollectiveDown(float input) {
	_CollectiveInput = FMath::Clamp(_CollectiveInput - input, -1, 1);
}

void AHeli::IncYaw() {
	_YawInput += 1;
}

void AHeli::DecYaw() {
	_YawInput -= 1;
}
