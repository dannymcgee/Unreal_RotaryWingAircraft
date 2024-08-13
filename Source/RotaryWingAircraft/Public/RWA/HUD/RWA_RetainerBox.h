#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"

#include "RWA_RetainerBox.generated.h"

class SRWA_RetainerWidget;


UCLASS(DisplayName="RWA Retainer Box")
class ROTARYWINGAIRCRAFT_API URWA_RetainerBox : public UContentWidget
{
	GENERATED_BODY()

public:
	URWA_RetainerBox(FObjectInitializer const& init);

protected:
	UPROPERTY(EditAnywhere, Category="Render Rules")
	bool RetainedRendering = true;

public:
	/**
	 * Should this widget redraw the contents it has every time it receives an
	 * invalidation request from its children? (Similar to invalidation panel)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Render Rules", meta=(EditCondition="RetainedRendering"))
	bool RenderOnInvalidation = false;

	/**
	 * Should this widget redraw the contents it has every time the phase occurs?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Render Rules", meta=(EditCondition="RetainedRendering"))
	bool RenderOnPhase = true;

	/**
	 * The Phase this widget will draw on.
	 *
	 * If the Phase is 0, and the PhaseCount is 1, the widget will be drawn fresh
	 * every frame. If the Phase were 0, and the PhaseCount were 2, this retainer
	 * would draw a fresh frame every other frame. So in a 60Hz game, the UI
	 * would render at 30Hz.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Render Rules", meta=(UIMin=0, ClampMin=0))
	int32 Phase = 0;

	/**
	 * The PhaseCount controls how many phases are possible know what to modulus
	 * the current frame count by to determine if this is the current frame to
	 * draw the widget on.
	 * 
	 * If the Phase is 0, and the PhaseCount is 1, the widget will be drawn fresh
	 * every frame. If the Phase were 0, and the PhaseCount were 2, this retainer
	 * would draw a fresh frame every other frame.  So in a 60Hz game, the UI
	 * would render at 30Hz.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Render Rules", meta=(UIMin=1, ClampMin=1))
	int32 PhaseCount = 1;

public:
	UFUNCTION(BlueprintCallable, Category="Retainer")
	void SetRetainedRendering(bool value);

	UFUNCTION(BlueprintCallable, Category="Retainer")
	void SetRenderingPhase(int32 in_phase, int32 in_phaseCount);

	UFUNCTION(BlueprintCallable, Category="Retainer")
	void RequestRender();

	UFUNCTION(BlueprintPure, Category="Retainer|Effect")
	UMaterialInstanceDynamic* GetEffectMaterial() const;

	UFUNCTION(BlueprintCallable, Category="Retainer|Effect")
	void SetEffectMaterial(UMaterialInterface* value);

	/**
	 * Set the name of the @EffectMaterial's texture sampler parameter.
	 */
	UFUNCTION(BlueprintCallable, Category="Retainer|Effect")
	void SetTextureParameter(FName value);

	void ReleaseSlateResources(bool releaseChildren) override;

#if WITH_EDITOR
	FText const GetPaletteCategory() override;
#endif

	FGeometry GetCachedAllottedGeometry() const;

protected:
	/**
	 * The effect to optionally apply to the render target. We will set the
	 * texture sampler based on the name set in the @TextureParameter property.
	 * 
	 * If you want to adjust transparency of the final image, make sure you set
	 * Blend Mode to AlphaComposite (Pre-Multiplied Alpha) and make sure to
	 * multiply the alpha you're apply across the surface to the color and the
	 * alpha of the render target, otherwise you won't see the expected color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	TObjectPtr<UMaterialInterface> EffectMaterial;

	/**
	 * The texture sampler parameter of the @EffectMaterial.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effect")
	FName TextureParameter;

	// UPanelWidget interface
	void OnSlotAdded(UPanelSlot* slot) override;
	void OnSlotRemoved(UPanelSlot* slot) override;

	// UWidget interface
	TSharedRef<SWidget> RebuildWidget() override;
	void SynchronizeProperties() override;

	// UObject interface
#if WITH_EDITOR
	bool CanEditChange(FProperty const* property) const override;
#endif

protected:
	TSharedPtr<SRWA_RetainerWidget> m_Widget;
};
