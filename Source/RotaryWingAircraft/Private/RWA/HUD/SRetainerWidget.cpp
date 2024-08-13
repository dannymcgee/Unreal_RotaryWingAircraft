#include "RWA/HUD/SRetainerWidget.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Input/HittestGrid.h"
#include "Slate/WidgetRenderer.h"


DECLARE_CYCLE_STAT(
	TEXT("RWA Retainer Widget Tick"),
	STAT_RWA_RetainerWidgetTick,
	STATGROUP_Slate);
DECLARE_CYCLE_STAT(
	TEXT("RWA Retainer Widget Paint"),
	STAT_RWA_RetainerWidgetPaint,
	STATGROUP_Slate);

#if !UE_BUILD_SHIPPING
FRWA_OnRetainedModeChanged SRWA_RetainerWidget::s_OnRetainedModeChangedDelegate {};
#endif


// CVars =======================================================================

int32 g_EnableRetainedRendering = 1;
FAutoConsoleVariableRef RWA_EnableRetainedRendering {
	TEXT("Slate.RWAEnableRetainedRendering"),
	g_EnableRetainedRendering,
	TEXT("Whether to attempt to render things in SRWA_RenderEffectsWidgets to "
		"render targets first"),
};

bool g_SlateEnableRenderWithLocalTransform = true;
FAutoConsoleVariableRef CVarGlateRWA_EnableRenderWithLocalTransform {
	TEXT("Slate.RWAEnableRetainedRenderingWithLocalTransform"),
	g_SlateEnableRenderWithLocalTransform,
	TEXT("Whether to render with the local transform or the one passed down "
		"from the parent widget."),
};

static bool IsRetainedRenderingEnabled()
{
	return g_EnableRetainedRendering != 0;
}


// Render Resources ============================================================

class FRWA_RenderResources
	: public FDeferredCleanupInterface
	, public FGCObject
{
public:
	FWidgetRenderer* WidgetRenderer = nullptr;
	TObjectPtr<UTextureRenderTarget2D> RenderTarget = nullptr;
	TObjectPtr<UMaterialInstanceDynamic> DynamicEffect = nullptr;

public:
	FRWA_RenderResources() = default;
	
	~FRWA_RenderResources()
	{
		if (WidgetRenderer) delete WidgetRenderer;
	}

	void AddReferencedObjects(FReferenceCollector& collector) override
	{
		collector.AddReferencedObject(RenderTarget);
		collector.AddReferencedObject(DynamicEffect);
	}

	FString GetReferencerName() const override
	{
		return TEXT("FRWA_RenderResources");
	}
};


// SRWA_RetainerWidget =========================================================

TArray<SRWA_RetainerWidget*,TInlineAllocator<3>> SRWA_RetainerWidget::s_WaitingToRender {};

int32 SRWA_RetainerWidget::s_MaxRetainerWorkPerFrame = 0;

TFrameValue<int32> SRWA_RetainerWidget::s_RetainerWorkThisFrame = 3;

SRWA_RetainerWidget::SRWA_RetainerWidget()
	: m_PrevRenderSize(FIntPoint::NoneValue)
	, m_PrevClipRectSize(FIntPoint::NoneValue)
	, m_PrevColorAndOpacity(FColor::Transparent)
	, m_LastIncomingLayerId(0)
	, m_VirtualWindow(SNew(SVirtualWindow))
	, m_HitTestGrid(MakeShared<FHittestGrid>())
	, m_RenderResources(new RenderResources)
{
	FSlateApplicationBase::Get()
		.OnGlobalInvalidationToggled()
		.AddRaw(this, &Self::OnGlobalInvalidationToggled);

	// FIXME: Why is this not wrapping the previous statement?
	if (FSlateApplication::IsInitialized()) {
	#if !UE_BUILD_SHIPPING
		// FIXME: Why are we clearing this delegate in ctor and dtor?
		s_OnRetainedModeChangedDelegate.RemoveAll(this);
	#endif
	}

	bHasCustomPrepass = true;

	SetInvalidationRootWidget(*this);
	SetInvalidationRootHittestGrid(m_HitTestGrid.Get());
	SetCanTick(false);
}

SRWA_RetainerWidget::~SRWA_RetainerWidget()
{
	if (FSlateApplication::IsInitialized()) {
		FSlateApplicationBase::Get()
			.OnGlobalInvalidationToggled()
			.RemoveAll(this);

	#if !UE_BUILD_SHIPPING
		s_OnRetainedModeChangedDelegate.RemoveAll(this);
	#endif
	}

	// Begin deferred cleanup of rendering resources.
	// DO NOT delete here. Will be deleted when safe.
	BeginCleanup(m_RenderResources);

	s_WaitingToRender.Remove(this);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SRWA_RetainerWidget::UpdateWidgetRenderer()
{
	// We can't write out linear. If we write out linear, then we end up with
	// premultiplied alpha in linear space, which blending with gamma space later
	// is difficult (impossible?) to get right since the rest of slate does
	// blending in gamma space.
	bool const writeContentInGammaSpace = true;
	
	FWidgetRenderer*& wr = m_RenderResources->WidgetRenderer;
	if (!wr) wr = new FWidgetRenderer(writeContentInGammaSpace);

	wr->SetUseGammaCorrection(writeContentInGammaSpace);
	
	// This will be handled by the main Slate rendering pass
	wr->SetApplyColorDeficiencyCorrection(false);

	wr->SetIsPrepassNeeded(false);
	wr->SetClearHitTestGrid(false);

	// Update the render target to match the current gamma rendering prefs
	UTextureRenderTarget2D* rt = m_RenderResources->RenderTarget;
	if (rt && rt->SRGB != writeContentInGammaSpace) {
		// NOTE: We do the opposite here of whatever write is. If we're writing
		// out gamma, then sRGB writes were not supported, so it won't be an sRGB
		// texture.
		// FIXME: writeContentInGammaSpace is a constant, so why are we checking it?
		rt->TargetGamma = !writeContentInGammaSpace ? 0 : 1;
		rt->SRGB = !writeContentInGammaSpace;

		rt->UpdateResource();
	}
}

void SRWA_RetainerWidget::Construct(FArguments const& args)
{
	STAT(m_StatId = FDynamicStats::CreateStatId<FStatGroup_STATGROUP_Slate>(args._StatId));

	auto* rt = NewObject<UTextureRenderTarget2D>();
	rt->ClearColor = FLinearColor::Transparent;
	rt->RenderTargetFormat = RTF_RGBA8_SRGB;

	m_RenderResources->RenderTarget = rt;
	m_SurfaceBrush.SetResourceObject(rt);

	// deubanks: We don't want Retainer Widgets blocking hit testing for tooltips
	m_VirtualWindow->SetVisibility(EVisibility::SelfHitTestInvisible);
	m_VirtualWindow->SetShouldResolveDeferred(false);

	UpdateWidgetRenderer();

	m_Widget = args._Content.Widget;

	m_RenderOnPhase = args._RenderOnPhase;
	m_RenderOnInvalidation = args._RenderOnInvalidation;

	m_Phase = args._Phase;
	m_PhaseCount = args._PhaseCount;

	m_LastDrawTime = FApp::GetCurrentTime();
	m_LastTickedFrame = 0;

	m_EnableRetainedRenderingDesire = true;
	m_EnableRetainedRendering = false;
	m_EnableRenderWithLocalTransform = args._RenderWithLocalTransform;
	SetVolatilePrepass(m_EnableRetainedRendering);

	RefreshRenderingMode();
	m_RenderRequested = true;
	m_InvalidSizeLogged = false;

	ChildSlot[ m_Widget.ToSharedRef() ];

	if (FSlateApplication::IsInitialized()) {
	#if !UE_BUILD_SHIPPING
		s_OnRetainedModeChangedDelegate.AddRaw(this, &Self::OnRetainerModeChanged);

		static bool s_initialized = false;
		if (!s_initialized) {
			s_initialized = true;
			
			RWA_EnableRetainedRendering->SetOnChangedCallback(
				FConsoleVariableDelegate::CreateStatic(&Self::OnRetainerModeCVarChanged));
		}
	#endif
	}
}

bool SRWA_RetainerWidget::ShouldBeRenderingOffscreen() const
{
	return m_EnableRetainedRenderingDesire && IsRetainedRenderingEnabled();
}

bool SRWA_RetainerWidget::IsAnythingVisibleToRender() const
{
	return m_Widget.IsValid() && m_Widget->GetVisibility().IsVisible();
}

void SRWA_RetainerWidget::OnRetainerModeChanged()
{
	if (m_Widget.IsValid())
		InvalidateChildRemovedFromTree(*m_Widget.Get());

	// Invalidate self
	Advanced_ResetInvalidation(true);

	// Invalidate my invalidation root, since all my children were once it's children
	// it needs to force a generation bump just like me.
	if (FSlateInvalidationRoot* invalidationRoot = GetProxyHandle()
		.GetInvalidationRootHandle()
		.GetInvalidationRoot())
	{
		invalidationRoot->Advanced_ResetInvalidation(true);
	}

	RefreshRenderingMode();

	m_RenderRequested = true;
}

void SRWA_RetainerWidget::OnRootInvalidated()
{
	RequestRender();
}

#if !UE_BUILD_SHIPPING
void SRWA_RetainerWidget::OnRetainerModeCVarChanged( IConsoleVariable* CVar )
{
	s_OnRetainedModeChangedDelegate.Broadcast();
}
#endif

void SRWA_RetainerWidget::SetRetainedRendering(bool value)
{
	if (m_EnableRetainedRenderingDesire != value) {
		m_EnableRetainedRenderingDesire = value;
		OnRetainerModeChanged();
	}
}

void SRWA_RetainerWidget::RefreshRenderingMode()
{
	bool const renderOffscreen = ShouldBeRenderingOffscreen();
	
	if (m_EnableRetainedRendering != renderOffscreen) {
		m_EnableRetainedRendering = renderOffscreen;
		SetVolatilePrepass(m_EnableRetainedRendering);
		InvalidateRootChildOrder();
	}
}

void SRWA_RetainerWidget::SetContent(TSharedRef<SWidget> const& content)
{
	m_Widget = content;
	ChildSlot[ content ];
}

UMaterialInstanceDynamic* SRWA_RetainerWidget::GetEffectMaterial() const
{
	return m_RenderResources->DynamicEffect;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SRWA_RetainerWidget::SetEffectMaterial(UMaterialInterface* value)
{
	if (value)
	{
		auto* effect = Cast<UMaterialInstanceDynamic>(value);
		if (!effect) effect = UMaterialInstanceDynamic::Create(value, GetTransientPackage());

		m_RenderResources->DynamicEffect = effect;
		m_SurfaceBrush.SetResourceObject(m_RenderResources->DynamicEffect);
	}
	else
	{
		m_RenderResources->DynamicEffect = nullptr;
		m_SurfaceBrush.SetResourceObject(m_RenderResources->RenderTarget);
	}

	UpdateWidgetRenderer();
}

void SRWA_RetainerWidget::SetTextureParameter(FName value)
{
	m_DynamicEffectTextureParam = value;
}

void SRWA_RetainerWidget::SetWorld(UWorld* value)
{
	m_OuterWorld = value;
}

FChildren* SRWA_RetainerWidget::GetChildren() 
{
	if (m_EnableRetainedRendering)
		return &FNoChildren::NoChildrenInstance;

	return Super::GetChildren();
}

#if WITH_SLATE_DEBUGGING
FChildren* SRWA_RetainerWidget::Debug_GetChildrenForReflector() 
{
	return Super::GetChildren();
}
#endif

void SRWA_RetainerWidget::SetRenderingPhase(int32 phase, int32 phaseCount)
{
	m_Phase = phase;
	m_PhaseCount = phaseCount;
}

void SRWA_RetainerWidget::RequestRender()
{
	m_RenderRequested = true;
	InvalidateRootChildOrder();
}

auto SRWA_RetainerWidget::PaintRetainedContentImpl(
	FSlateInvalidationContext const& ctx,
	FGeometry const& geo,
	int32 layerId)
	-> EPaintRetainedContentResult
{
	if (m_RenderOnPhase
		&& m_LastTickedFrame != GFrameCounter
		&& (GFrameCounter % m_PhaseCount) == m_Phase)
	{
		// If doing some phase based invalidation, just redraw everything again
		m_RenderRequested = true;
		InvalidateRootLayout();
	}

	FPaintGeometry paintGeo = geo.ToPaintGeometry();
	FSlateRenderTransform xform = paintGeo.GetAccumulatedRenderTransform();

	FVector2f const& scale2d = xform.GetMatrix().GetScale().GetVector();
	FVector2f const& localSize = FVector2f(paintGeo.GetLocalSize());
	// UE_LOG(LogTemp, Log, TEXT("Local Size:  %f x %f"), localSize.X, localSize.Y);
	// UE_LOG(LogTemp, Log, TEXT("Scale 2D:    %f x %f"), scale2d.X, scale2d.Y);

	FVector2f renderSize = localSize * scale2d;
	// UE_LOG(LogTemp, Log, TEXT("Render Size: %f x %f"), renderSize.X, renderSize.Y);

	FIntPoint renderSizeRounded = renderSize.IntPoint();

	// Handle cache invalidation
	if (m_RenderOnInvalidation)
	{
		// the invalidation root will take care of whether or not we actually rendered
		m_RenderRequested = true;
		
		// Aggressively re-layout when a base state changes
		FIntPoint clipRectSize = ctx.CullingRect.GetSize().IntPoint();
		TOptional<FSlateClippingState> clippingState = ctx.WindowElementList->GetClippingState();

		if (renderSizeRounded != m_PrevRenderSize
			|| geo != m_PrevGeo
			|| geo.GetAccumulatedRenderTransform() != m_PrevGeo.GetAccumulatedRenderTransform()
			|| clipRectSize != m_PrevClipRectSize
			|| clippingState != m_PrevClippingState)
		{
			m_PrevRenderSize = renderSizeRounded;
			m_PrevGeo = geo;
			m_PrevClipRectSize = clipRectSize;
			m_PrevClippingState = clippingState;

			InvalidateRootLayout();
		}

		// Aggressively re-paint when a base state changes
		FColor colorAndOpacityTint = ctx.WidgetStyle.GetColorAndOpacityTint().ToFColor(false);

		if (layerId != m_LastIncomingLayerId
			|| colorAndOpacityTint != m_PrevColorAndOpacity)
		{
			m_LastIncomingLayerId = layerId;
			m_PrevColorAndOpacity = colorAndOpacityTint;
		}
	}
	else if (renderSizeRounded != m_PrevRenderSize)
	{
		m_RenderRequested = true;
		InvalidateRootLayout();
		m_PrevRenderSize = renderSizeRounded;
	}

	// Defer work if we're over the frame budget
	if (s_MaxRetainerWorkPerFrame > 0
		&& s_RetainerWorkThisFrame.TryGetValue(0) > s_MaxRetainerWorkPerFrame)
	{
		s_WaitingToRender.AddUnique(this);

		return EPaintRetainedContentResult::Queued;
	}

	// Skip paint if it was not requested or our child widget is invisible
	if (!m_RenderRequested || !m_Widget->GetVisibility().IsVisible())
		return EPaintRetainedContentResult::NotPainted;

	// In order to get material parameter collections to function properly, we
	// need the current world's Scene properly propagated through to any
	// widgets that depend on that functionality. The SceneViewport and
	// RetainerWidget are the only locations where this information exists in
	// Slate, so we push the current scene onto the current Slate application
	// so that we can leverage it in later calls.
	UWorld* world = m_OuterWorld.Get();
	if (world && world->Scene && IsInGameThread())
		FSlateApplication::Get().GetRenderer()->RegisterCurrentScene(world->Scene);
	else if (IsInGameThread())
		FSlateApplication::Get().GetRenderer()->RegisterCurrentScene(nullptr);

	// Update the number of retainers we've drawn this frame
	s_RetainerWorkThisFrame = s_RetainerWorkThisFrame.TryGetValue(0) + 1;

	m_LastTickedFrame = GFrameCounter;

	// Size must be a positive integer to allocate the RenderTarget
	// TODO: This is where we're setting the RT size with viewport scale already applied
	uint32 rtWidth  = FMath::RoundToInt(FMath::Abs(renderSize.X));
	uint32 rtHeight = FMath::RoundToInt(FMath::Abs(renderSize.Y));

	// Handle invalid states
	if (FMath::Max(rtWidth, rtHeight) > GetMax2DTextureDimension())
	{
		// TODO: Replace with a proper logger category
		UE_LOG(LogTemp, Error,
			TEXT("The requested size for SRWA_RetainerWidget is too large! "
				"Width: %i; Height: %i;"),
			rtWidth, rtHeight);

		return EPaintRetainedContentResult::TextureSizeTooBig;
	}
	if (rtWidth == 0 || rtHeight == 0)
	{
		// TODO: Replace with a proper logger category
		UE_LOG(LogTemp, Error,
			TEXT("The requested size of SRWA_RetainerWidget has a zero dimension! "
				"Width: %i; Height: %i;"),
			rtWidth, rtHeight);

		return EPaintRetainedContentResult::TextureSizeZero;
	}

	UTextureRenderTarget2D* rt = m_RenderResources->RenderTarget;
	FWidgetRenderer* wr = m_RenderResources->WidgetRenderer;

	// Handle size mismatch
	if ((int32)rt->GetSurfaceWidth() != (int32)rtWidth
		|| (int32)rt->GetSurfaceHeight() != (int32)rtHeight)
	{
		// If the render target resource already exists, just resize it. Calling
		// InitCustomFormat flushes render commands which could result in a huge
		// hitch.
		if (rt->GameThread_GetRenderTargetResource()
			&& rt->OverrideFormat == PF_B8G8R8A8)
		{
			rt->ResizeTarget(rtWidth, rtHeight);
		}
		else
		{
			bool forceLinearGamma = false;
			rt->InitCustomFormat(rtWidth, rtHeight, PF_B8G8R8A8, forceLinearGamma);
			rt->UpdateResourceImmediate();
		}
	}

	// Update the surface brush to match the latest size
	m_SurfaceBrush.ImageSize = FVector2D(rtWidth, rtHeight);

	wr->ViewOffset = -xform.GetTranslation();

	bool repainted = wr->DrawInvalidationRoot(m_VirtualWindow, rt, *this, ctx, false);

	m_RenderRequested = false;
	s_WaitingToRender.Remove(this);
	// FIXME: Should LastDrawTime be updated if `repainted` is false?
	m_LastDrawTime = FApp::GetCurrentTime();

	return repainted
		? EPaintRetainedContentResult::Painted
		: EPaintRetainedContentResult::NotPainted;
}

int32 SRWA_RetainerWidget::OnPaint(
	FPaintArgs const& args,
	FGeometry const& geo,
	FSlateRect const& cullingRect,
	FSlateWindowElementList& out_drawElements,
	int32 layerId,
	FWidgetStyle const& style,
	bool parentEnabled)
	const
{
	STAT(FScopeCycleCounter paintCycleCounter(m_StatId));

	if (!m_EnableRetainedRendering || !IsAnythingVisibleToRender())
		return Super::OnPaint(
			args,
			geo,
			cullingRect,
			out_drawElements,
			layerId,
			style,
			parentEnabled);

	SCOPE_CYCLE_COUNTER(STAT_RWA_RetainerWidgetPaint);
	
	Self* self_mut = const_cast<Self*>(this);

	// Copy hit-test grid settings from the root

	// FIXME: FPaintArgs declares SRetainerWidget as a friend class so that it's
	// able to access its private members directly. We don't have that luxury
	// here in userland, so we have to do this sketchy bullshit instead. If the
	// type or offset of FPaintArgs::RootGrid ever changes, this is going to be a
	// nightmare to debug.
	FHittestGrid const& rootGrid = **(FHittestGrid**)&args;

	if (m_HitTestGrid->SetHittestArea(
		rootGrid.GetGridOrigin(),
		rootGrid.GetGridSize(),
		rootGrid.GetGridWindowOrigin()))
	{
		self_mut->RequestRender();
	}

	m_HitTestGrid->SetOwner(this);
	m_HitTestGrid->SetCullingRect(cullingRect);

	FPaintArgs args_new = args.WithNewHitTestGrid(m_HitTestGrid.Get());
	// Copy the current user index into the new grid since nested hittest grids
	// should inherit their parent's user id
	args_new.GetHittestGrid().SetUserIndex(rootGrid.GetUserIndex());

	FSlateInvalidationContext ctx (out_drawElements, style);
	ctx.bParentEnabled = parentEnabled;
	ctx.bAllowFastPathUpdate = true;
	ctx.LayoutScaleMultiplier = GetPrepassLayoutScaleMultiplier();
	ctx.PaintArgs = &args_new;
	ctx.IncomingLayerId = layerId;
	ctx.CullingRect = cullingRect;

	EPaintRetainedContentResult paintResult = self_mut->PaintRetainedContentImpl(ctx, geo, layerId);

#if WITH_SLATE_DEBUGGING
	if (paintResult == EPaintRetainedContentResult::NotPainted
		|| paintResult == EPaintRetainedContentResult::TextureSizeZero
		|| paintResult == EPaintRetainedContentResult::TextureSizeTooBig)
	{
		self_mut->SetLastPaintType(ESlateInvalidationPaintType::None);
	}
#endif

	if (paintResult == EPaintRetainedContentResult::TextureSizeTooBig)
		return Super::OnPaint(
			args,
			geo,
			cullingRect,
			out_drawElements,
			layerId,
			style,
			parentEnabled);

	if (paintResult == EPaintRetainedContentResult::TextureSizeZero)
		return GetCachedMaxLayerId();

	UTextureRenderTarget2D* rt = m_RenderResources->RenderTarget;
	check(rt);

	if (rt->GetSurfaceWidth() >= 1.f && rt->GetSurfaceHeight() >= 1.f) {
		FLinearColor computedColorAndOpacity =
			ctx.WidgetStyle.GetColorAndOpacityTint()
				* GetColorAndOpacity()
				* m_SurfaceBrush.GetTint(ctx.WidgetStyle);

		FLinearColor premulColorAndOpacity = computedColorAndOpacity * computedColorAndOpacity.A;
		
		if (UMaterialInstanceDynamic* effect = m_RenderResources->DynamicEffect)
			effect->SetTextureParameterValue(m_DynamicEffectTextureParam, rt);

		FSlateDrawElement::MakeBox(
			*ctx.WindowElementList,
			ctx.IncomingLayerId,
			geo.ToPaintGeometry(),
			&m_SurfaceBrush,
			// We always write out the content in gamma space, so when we render the
			// final version we need to render without gamma correction enabled.
			ESlateDrawEffect::PreMultipliedAlpha | ESlateDrawEffect::NoGamma,
			premulColorAndOpacity);
	}

	// Add our widgets to the root hit-test grid
	args.GetHittestGrid().AddGrid(m_HitTestGrid);

	return GetCachedMaxLayerId();
}

FVector2D SRWA_RetainerWidget::ComputeDesiredSize(float scale) const
{
	if (!m_EnableRetainedRendering)
		return Super::ComputeDesiredSize(scale);

	return m_Widget->GetDesiredSize();
}

void SRWA_RetainerWidget::OnGlobalInvalidationToggled(bool value)
{
	InvalidateRootChildOrder();
	ClearAllFastPathData(true);
}

bool SRWA_RetainerWidget::CustomPrepass(float layoutScaleMultiplier)
{
	if (!m_EnableRetainedRendering)
		return true;

	ProcessInvalidation();

	if (NeedsSlowPath()) {
		FChildren* children = Super::GetChildren();
		Prepass_ChildLoop(layoutScaleMultiplier, children);
	}

	return false;
}

TSharedRef<SWidget> SRWA_RetainerWidget::GetRootWidget() 
{
	if (!m_EnableRetainedRendering)
		return SNullWidget::NullWidget;

	return Super::GetChildren()->GetChildAt(0);
}

int32 SRWA_RetainerWidget::PaintSlowPath(FSlateInvalidationContext const& ctx) 
{
	if (m_EnableRenderWithLocalTransform && g_SlateEnableRenderWithLocalTransform) {
		FGeometry geo = GetPaintSpaceGeometry();
		FSlateRenderTransform simplifiedXform {
			geo.GetAccumulatedRenderTransform().GetMatrix().GetScale(),
			geo.GetAccumulatedRenderTransform().GetTranslation(),
		};

		FGeometry const geo_new =
			FGeometry::MakeRoot(geo.GetLocalSize(), FSlateLayoutTransform())
				.MakeChild(simplifiedXform, FVector2D::ZeroVector);

		return Super::OnPaint(
			*ctx.PaintArgs,
			geo_new,
			ctx.CullingRect,
			*ctx.WindowElementList,
			ctx.IncomingLayerId,
			ctx.WidgetStyle,
			ctx.bParentEnabled);
	}

	return Super::OnPaint(
		*ctx.PaintArgs,
		GetPaintSpaceGeometry(),
		ctx.CullingRect,
		*ctx.WindowElementList,
		ctx.IncomingLayerId,
		ctx.WidgetStyle,
		ctx.bParentEnabled);
}

