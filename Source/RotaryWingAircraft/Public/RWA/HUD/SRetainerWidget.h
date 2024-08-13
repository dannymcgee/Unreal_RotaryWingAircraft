#pragma once

#include "CoreMinimal.h"

class SVirtualWindow;
class FRWA_RenderResources;

DECLARE_MULTICAST_DELEGATE(FRWA_OnRetainedModeChanged);

class ROTARYWINGAIRCRAFT_API SRWA_RetainerWidget
	: public SCompoundWidget
	, public FSlateInvalidationRoot
{
private:
	using Self = SRWA_RetainerWidget;
	using Super = SCompoundWidget;
	using RenderResources = FRWA_RenderResources;
	
public:
	static int32 s_MaxRetainerWorkPerFrame;

	SLATE_BEGIN_ARGS(SRWA_RetainerWidget)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
		_Phase = 0;
		_PhaseCount = 1;
		_RenderOnPhase = true;
		_RenderOnInvalidation = false;
		_RenderWithLocalTransform = true;
	}
	SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_ARGUMENT(bool, RenderOnPhase)
		SLATE_ARGUMENT(bool, RenderOnInvalidation)
		SLATE_ARGUMENT(bool, RenderWithLocalTransform)
		SLATE_ARGUMENT(int32, Phase)
		SLATE_ARGUMENT(int32, PhaseCount)
		SLATE_ARGUMENT(FName, StatId)
	SLATE_END_ARGS()

	SRWA_RetainerWidget();
	~SRWA_RetainerWidget();

	void Construct(FArguments const& args);

	/** Requests that the retainer redraw the hosted content next time it's painted. */
	void RequestRender();

	void SetRenderingPhase(int32 phase, int32 phaseCount);
	void SetRetainedRendering(bool value);
	void SetContent(TSharedRef<SWidget> const& content);
	void SetTextureParameter(FName value);
	void SetWorld(UWorld* value);

	UMaterialInstanceDynamic* GetEffectMaterial() const;
	void SetEffectMaterial(UMaterialInterface* value);

	FChildren* GetChildren() override;
#if WITH_SLATE_DEBUGGING
	FChildren* Debug_GetChildrenForReflector() override;
#endif

protected:
	int32 OnPaint(
		FPaintArgs const& args,
		FGeometry const& geo,
		FSlateRect const& cullingRect,
		FSlateWindowElementList& out_drawElements,
		int32 layerId,
		FWidgetStyle const& style,
		bool parentEnabled)
		const override;

	FVector2D ComputeDesiredSize(float scale) const override;
	
	virtual bool Advanced_IsInvalidationRoot() const { return m_EnableRetainedRendering; }

	FSlateInvalidationRoot const* Advanced_AsInvalidationRoot() const override
	{
		return m_EnableRetainedRendering ? this : nullptr;
	}

	bool CustomPrepass(float layoutScaleMultiplier) override;

	TSharedRef<SWidget> GetRootWidget() override;
	int32 PaintSlowPath(FSlateInvalidationContext const& ctx) override;

	enum class EPaintRetainedContentResult
	{
		NotPainted,
		Painted,
		Queued,
		TextureSizeTooBig,
		TextureSizeZero,
	};
	EPaintRetainedContentResult PaintRetainedContentImpl(
		FSlateInvalidationContext const& ctx,
		FGeometry const& geo,
		int32 layerId);

	void RefreshRenderingMode();
	bool ShouldBeRenderingOffscreen() const;
	bool IsAnythingVisibleToRender() const;
	void OnRetainerModeChanged();
	void OnRootInvalidated();

private:
	void OnGlobalInvalidationToggled(bool value);
	void UpdateWidgetRenderer();

#if !UE_BUILD_SHIPPING
	static void OnRetainerModeCVarChanged(IConsoleVariable* cvar);
	static FRWA_OnRetainedModeChanged s_OnRetainedModeChangedDelegate;
#endif

	mutable FSlateBrush m_SurfaceBrush;

	FIntPoint m_PrevRenderSize;
	FGeometry m_PrevGeo;
	FIntPoint m_PrevClipRectSize;
	TOptional<FSlateClippingState> m_PrevClippingState;
	FColor m_PrevColorAndOpacity;
	int32 m_LastIncomingLayerId;

	TSharedPtr<SWidget> m_Widget;
	TSharedRef<SVirtualWindow> m_VirtualWindow;
	TSharedRef<FHittestGrid> m_HitTestGrid;

	int32 m_Phase;
	int32 m_PhaseCount;

	bool m_EnableRetainedRenderingDesire;
	bool m_EnableRetainedRendering;
	bool m_EnableRenderWithLocalTransform;

	bool m_RenderOnPhase;
	bool m_RenderOnInvalidation;

	bool m_RenderRequested;
	bool m_InvalidSizeLogged;

	double m_LastDrawTime;
	int64 m_LastTickedFrame;

	TWeakObjectPtr<UWorld> m_OuterWorld;
	
	RenderResources* m_RenderResources;

	STAT(TStatId m_StatId);

	FSlateBrush m_DynamicBrush;

	FName m_DynamicEffectTextureParam;

	static TArray<Self*,TInlineAllocator<3>> s_WaitingToRender;
	static TFrameValue<int32> s_RetainerWorkThisFrame;
};
