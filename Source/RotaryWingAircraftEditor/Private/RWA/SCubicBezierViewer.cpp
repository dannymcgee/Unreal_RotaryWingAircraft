﻿#include "RWA/SCubicBezierViewer.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "RWA/EditorUtil.h"

#define Self SCubicBezierViewer


namespace RWA::Editor::CubicBezierViewer {

auto FRenderResources::DynamicMaterial() -> UMaterialInstanceDynamic*
{
	if (m_DynamicMaterial != nullptr)
		return m_DynamicMaterial;

	m_Material = Util::LoadAsset<UMaterial>("/RotaryWingAircraft/Editor/M_CubicBezier.M_CubicBezier");
	check(m_Material != nullptr);

	m_DynamicMaterial = UMaterialInstanceDynamic::Create(m_Material, GetTransientPackage());

	return m_DynamicMaterial;
}

auto FRenderResources::GetReferencerName() const -> FString
{
	return "RWA::Editor::CubicBezierViewer::FRenderResources";
}


void FRenderResources::AddReferencedObjects(FReferenceCollector& gc)
{
	gc.AddReferencedObject(m_Material);
	gc.AddReferencedObject(m_DynamicMaterial);
}

} // namespace RWA::Editor::CubicBezierViewer


Self::SCubicBezierViewer()
	: m_RenderResources(new RenderResources)
{}

Self::~SCubicBezierViewer()
{
	BeginCleanup(m_RenderResources);
}

void Self::Construct(FArguments const& args)
{
	m_ColorAndOpacity = args._ColorAndOpacity;
	m_CubicBezier = args._CubicBezier;

	if (auto cubicBezier = m_CubicBezier.Get())
		m_PrevCubicBezier = *cubicBezier;
	else
		m_PrevCubicBezier = {};

	auto* curveMat = m_RenderResources->DynamicMaterial();
	m_ImageBrush.SetImageSize(FVector2D{ 256, 256 });
	m_ImageBrush.SetResourceObject(curveMat);

	// TODO:
	// - Add synthetic "duration" field
	// - Show individual point values
	auto child = SNew(SBox)
		.Padding(8)
		.WidthOverride(128)
		.MinDesiredWidth(0)
		.HeightOverride(64)
	[
		SNew(SImage)
			.DesiredSizeOverride(FVector2D{ 256, 128 })
			.ColorAndOpacity(m_ColorAndOpacity)
			.Image(&m_ImageBrush)
	];

	m_ChildContainer = child;

	ChildSlot [ child ];

	UpdateView(m_PrevCubicBezier);
}

auto Self::OnPaint(
	FPaintArgs const& args,
	FGeometry const& geo,
	FSlateRect const& rect,
	FSlateWindowElementList& elements,
	int32 layer,
	FWidgetStyle const& style,
	bool parentEnabled)
	const -> int32
{
	if (auto const& cubicBezier = m_CubicBezier.Get())
		if (!FCubicBezier::IsNearlyEqual(*cubicBezier, m_PrevCubicBezier))
			UpdateView(*cubicBezier);

	return Super::OnPaint(args, geo, rect, elements, layer, style, parentEnabled);
}

void Self::UpdateView(FCubicBezier const& curve) const
{
	auto const& [p0, p1, p2, p3] = curve;
	auto* curveMat = m_RenderResources->DynamicMaterial();

	if (m_ChildContainer.IsValid()) {
		auto box = m_ChildContainer.Pin();
		box->SetWidthOverride(128 * curve.Duration());
	}

	curveMat->SetVectorParameterValue("P0-P1", FVector4f{ p0.X, p0.Y, p1.X, p1.Y });
	curveMat->SetVectorParameterValue("P2-P3", FVector4f{ p2.X, p2.Y, p3.X, p3.Y });
}


#undef Self
