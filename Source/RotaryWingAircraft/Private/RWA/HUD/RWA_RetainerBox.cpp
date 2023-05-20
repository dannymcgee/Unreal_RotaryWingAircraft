#include "RWA/HUD/RWA_RetainerBox.h"

#include "RWA/HUD/SRetainerWidget.h"

#define LOCTEXT_NAMESPACE "RWA"
#define Self URWA_RetainerBox

namespace {
static FName g_DefaultTextureParamName = "Texture";
}

Self::URWA_RetainerBox(FObjectInitializer const& init)
	: Super(init)
	, TextureParameter(g_DefaultTextureParamName)
{
	SetVisibilityInternal(ESlateVisibility::Visible);
}

void Self::SetRetainedRendering(bool value)
{
	RetainedRendering = value;

	if (m_Widget.IsValid())
		m_Widget->SetRetainedRendering(value);
}

void Self::SetRenderingPhase(int32 phase, int32 phaseCount)
{
	Phase = phase;
	PhaseCount = FMath::Max(phaseCount, 1);

	if (m_Widget.IsValid())
		m_Widget->SetRenderingPhase(Phase, PhaseCount);
}

void Self::RequestRender()
{
	if (m_Widget.IsValid())
		m_Widget->RequestRender();
}

auto Self::GetEffectMaterial() const -> UMaterialInstanceDynamic*
{
	if (m_Widget.IsValid())
		return m_Widget->GetEffectMaterial();

	return nullptr;
}

void Self::SetEffectMaterial(UMaterialInterface* value)
{
	EffectMaterial = value;

	if (m_Widget.IsValid())
		m_Widget->SetEffectMaterial(value);
}

void Self::SetTextureParameter(FName value)
{
	TextureParameter = value;

	if (m_Widget.IsValid())
		m_Widget->SetTextureParameter(value);
}

void Self::ReleaseSlateResources(bool releaseChildren)
{
	Super::ReleaseSlateResources(releaseChildren);
	
	m_Widget.Reset();
}

auto Self::RebuildWidget() -> TSharedRef<SWidget>
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

	if (GetChildrenCount() > 0) {
		auto content = GetContentSlot()->Content
			? GetContentSlot()->Content->TakeWidget()
			: SNullWidget::NullWidget;
		
		m_Widget->SetContent(content);
	}

	return m_Widget.ToSharedRef();
}

void Self::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	m_Widget->SetRetainedRendering(IsDesignTime() ? false : RetainedRendering);
	m_Widget->SetEffectMaterial(EffectMaterial);
	m_Widget->SetTextureParameter(TextureParameter);
	m_Widget->SetWorld(GetWorld());
}

void Self::OnSlotAdded(UPanelSlot* slot)
{
	if (!m_Widget.IsValid())
		return;

	auto content = slot->Content
		? slot->Content->TakeWidget()
		: SNullWidget::NullWidget;
	
	m_Widget->SetContent(content);
}

void Self::OnSlotRemoved(UPanelSlot* slot)
{
	if (m_Widget.IsValid())
		m_Widget->SetContent(SNullWidget::NullWidget);
}

#if WITH_EDITOR
auto Self::GetPaletteCategory() -> FText const
{
	return LOCTEXT("RotaryWingAircraft", "Rotary-Wing Aircraft");
}
#endif

auto Self::GetCachedAllottedGeometry() const -> FGeometry
{
	if (m_Widget.IsValid())
		return m_Widget->GetTickSpaceGeometry();

	static FGeometry const s_empty;
	return s_empty;
}

#if WITH_EDITOR
auto Self::CanEditChange(FProperty const* property) const -> bool
{
	if (!Super::CanEditChange(property))
		return false;

	auto propName = property->GetFName(); 
	if (propName == GET_MEMBER_NAME_CHECKED(Self, Phase)
		|| propName == GET_MEMBER_NAME_CHECKED(Self, PhaseCount))
	{
		return RenderOnPhase && RetainedRendering;
	}

	return true;
}
#endif


#undef Self
#undef LOCTEXT_NAMESPACE
