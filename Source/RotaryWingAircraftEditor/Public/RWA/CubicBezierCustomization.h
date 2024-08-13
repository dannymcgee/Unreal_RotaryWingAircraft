#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "RWA/Input/CubicBezier.h"


class ROTARYWINGAIRCRAFTEDITOR_API FCubicBezierStructCustomization
	: public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

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

	bool ShouldInlineKey() const override { return true; }

private:
	TOptional<FCubicBezier> GetCurrentValue() const;

private:
	TSharedPtr<IPropertyHandle> m_Handle;
};
