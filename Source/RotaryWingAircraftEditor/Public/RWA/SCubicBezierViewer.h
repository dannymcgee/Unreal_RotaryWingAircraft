#pragma once

#include "CoreMinimal.h"
#include "RWA/Input/CubicBezier.h"


namespace RWA::Editor::CubicBezierViewer {

class FRenderResources
	: public FDeferredCleanupInterface
	, public FGCObject
{
public:
	FRenderResources() = default;

	UMaterialInstanceDynamic* DynamicMaterial();
	FString GetReferencerName() const override;
	void AddReferencedObjects(FReferenceCollector& gc) override;

private:
	TObjectPtr<UMaterialInterface> m_Material = nullptr;
	TObjectPtr<UMaterialInstanceDynamic> m_DynamicMaterial = nullptr;
};

} // namespace RWA::Editor::CubicBezierViewer


class ROTARYWINGAIRCRAFTEDITOR_API SCubicBezierViewer
	: public SCompoundWidget
{
private:
	using RenderResources = RWA::Editor::CubicBezierViewer::FRenderResources;
	using Super = SCompoundWidget;

public:
	SLATE_BEGIN_ARGS(SCubicBezierViewer)
			: _ColorAndOpacity(FLinearColor::White)
		{}
		SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)
		SLATE_ATTRIBUTE(TOptional<FCubicBezier>, CubicBezier)
	SLATE_END_ARGS()

	SCubicBezierViewer();
	~SCubicBezierViewer();

	void Construct(FArguments const& args);

	auto OnPaint(
		FPaintArgs const& args,
		FGeometry const& geo,
		FSlateRect const& rect,
		FSlateWindowElementList& elements,
		int32 layer,
		FWidgetStyle const& style,
		bool parentEnabled)
		const -> int32 override;

private:
	void UpdateView(FCubicBezier const& curve) const;

private:
	mutable TWeakPtr<SBox,ESPMode::ThreadSafe> m_ChildContainer;

	TAttribute<TOptional<FCubicBezier>> m_CubicBezier;
	FCubicBezier m_PrevCubicBezier;

	TAttribute<FSlateColor> m_ColorAndOpacity;
	FSlateBrush m_ImageBrush;

	RenderResources* m_RenderResources;
};
