#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "RWA/Input/CubicBezier.h"


class ROTARYWINGAIRCRAFTEDITOR_API FCubicBezierStructCustomization
	: public IPropertyTypeCustomization
{
public:
	static auto MakeInstance() -> TSharedRef<IPropertyTypeCustomization>;

	void CustomizeHeader(
		TSharedRef<IPropertyHandle> propHandle,
		FDetailWidgetRow& row,
		IPropertyTypeCustomizationUtils& utils)
		override;

	void CustomizeChildren(
		TSharedRef<IPropertyHandle> propHandle,
		IDetailChildrenBuilder& childBuilder,
		IPropertyTypeCustomizationUtils& utils)
		override
	{}

	auto ShouldInlineKey() const -> bool override { return true; }

private:
	auto GetCurrentValue() const -> TOptional<FCubicBezier>;

private:
	TSharedPtr<IPropertyHandle> m_Handle;
};
