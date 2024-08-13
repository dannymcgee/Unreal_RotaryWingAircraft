#include "RWA/HUD/RWA_RetainerBox.h"

#include "RWA/HUD/SRetainerWidget.h"

#define LOCTEXT_NAMESPACE "RWA"


namespace {
static FName g_DefaultTextureParamName = "Texture";
}

URWA_RetainerBox::URWA_RetainerBox(FObjectInitializer const& init)
	: Super(init)
	, TextureParameter(g_DefaultTextureParamName)
{
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void URWA_RetainerBox::SetRetainedRendering(bool value)
{
	RetainedRendering = value;

	if (m_Widget.IsValid())
		m_Widget->SetRetainedRendering(value);
}

void URWA_RetainerBox::SetRenderingPhase(int32 phase, int32 phaseCount)
{
	Phase = phase;
	PhaseCount = FMath::Max(phaseCount, 1);

	if (m_Widget.IsValid())
		m_Widget->SetRenderingPhase(Phase, PhaseCount);
}

void URWA_RetainerBox::RequestRender()
{
	if (m_Widget.IsValid())
		m_Widget->RequestRender();
}

UMaterialInstanceDynamic* URWA_RetainerBox::GetEffectMaterial() const
{
	if (m_Widget.IsValid())
		return m_Widget->GetEffectMaterial();

	return nullptr;
}

void URWA_RetainerBox::SetEffectMaterial(UMaterialInterface* value)
{
	EffectMaterial = value;

	if (m_Widget.IsValid())
		m_Widget->SetEffectMaterial(value);
}

void URWA_RetainerBox::SetTextureParameter(FName value)
{
	TextureParameter = value;

	if (m_Widget.IsValid())
		m_Widget->SetTextureParameter(value);
}

void URWA_RetainerBox::ReleaseSlateResources(bool releaseChildren)
{
	Super::ReleaseSlateResources(releaseChildren);
	
	m_Widget.Reset();
}

TSharedRef<SWidget> URWA_RetainerBox::RebuildWidget() 
{
	m_Widget = SNew(SRWA_RetainerWidget)
		.RenderOnInvalidation(RenderOnInvalidation)
		.RenderOnPhase(RenderOnPhase)
		.Phase(Phase)
		.PhaseCount(PhaseCount)
#if STATS
		.StatId(*FString::Printf(TEXT("%s [%s]"),
			*GetFName().ToString(),
			*GetClass()->GetName()))
#endif
		;

	if (GetChildrenCount() > 0)
	{
		TSharedRef<SWidget> content = GetContentSlot()->Content
			? GetContentSlot()->Content->TakeWidget()
			: SNullWidget::NullWidget;
		
		m_Widget->SetContent(content);
	}

	return m_Widget.ToSharedRef();
}

void URWA_RetainerBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	m_Widget->SetRetainedRendering(IsDesignTime() ? false : RetainedRendering);
	m_Widget->SetEffectMaterial(EffectMaterial);
	m_Widget->SetTextureParameter(TextureParameter);
	m_Widget->SetWorld(GetWorld());
}

void URWA_RetainerBox::OnSlotAdded(UPanelSlot* slot)
{
	if (!m_Widget.IsValid())
		return;

	TSharedRef<SWidget> content = slot->Content
		? slot->Content->TakeWidget()
		: SNullWidget::NullWidget;
	
	m_Widget->SetContent(content);
}

void URWA_RetainerBox::OnSlotRemoved(UPanelSlot* slot)
{
	if (m_Widget.IsValid())
		m_Widget->SetContent(SNullWidget::NullWidget);
}

#if WITH_EDITOR
FText const URWA_RetainerBox::GetPaletteCategory()
{
	return LOCTEXT("RotaryWingAircraft", "Rotary-Wing Aircraft");
}
#endif

FGeometry URWA_RetainerBox::GetCachedAllottedGeometry() const
{
	if (m_Widget.IsValid())
		return m_Widget->GetTickSpaceGeometry();

	static FGeometry const s_empty;
	return s_empty;
}

#if WITH_EDITOR
bool URWA_RetainerBox::CanEditChange(FProperty const* property) const
{
	if (!Super::CanEditChange(property))
		return false;

	FName propName = property->GetFName(); 
	if (propName == GET_MEMBER_NAME_CHECKED(URWA_RetainerBox, Phase)
		|| propName == GET_MEMBER_NAME_CHECKED(URWA_RetainerBox, PhaseCount))
	{
		return RenderOnPhase && RetainedRendering;
	}

	return true;
}
#endif


#undef LOCTEXT_NAMESPACE
