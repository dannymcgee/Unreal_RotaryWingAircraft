#include "RWA/CubicBezierCustomization.h"

#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "RotaryWingAircraftEditor"
#define Self FCubicBezierStructCustomization


auto Self::MakeInstance() -> TSharedRef<IPropertyTypeCustomization>
{
	return MakeShareable(new Self);
}

void Self::CustomizeHeader(
	TSharedRef<IPropertyHandle> propHandle,
	FDetailWidgetRow& row,
	IPropertyTypeCustomizationUtils& utils)
{
	m_Handle = propHandle;

	auto value = GetCurrentValue();
	auto text = value
		? FText::FromString(FString::Printf(
				TEXT("P0[ %.3f, %.3f ], P1[ %.3f, %.3f ], P2[ %.3f, %.3f ], P3[ %.3f, %.3f ]"),
				value->P0.X, value->P0.Y,
				value->P1.X, value->P1.Y,
				value->P2.X, value->P2.Y,
				value->P3.X, value->P3.Y))
		: LOCTEXT("CubicBezier_NoneValue", "None");

	row.OverrideResetToDefault(FResetToDefaultOverride::Hide())
		.NameContent()
		[
			propHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(text)
		];
}

auto Self::GetCurrentValue() const -> TOptional<FCubicBezier>
{
	TArray<void*> structPtrs;
	m_Handle->AccessRawData(structPtrs);

	if (structPtrs.Num() == 0)
		return {};

	auto* result = (FCubicBezier*)structPtrs[0];
	if (result)
		return *result;

	return {};
}


#undef Self
#undef LOCTEXT_NAMESPACE
