#include "RWA/CubicBezierCustomization.h"

#include "DetailWidgetRow.h"
#include "RWA/SCubicBezierViewer.h"

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

	row.OverrideResetToDefault(FResetToDefaultOverride::Hide())
		.NameContent()
		[
			propHandle->CreatePropertyNameWidget()
		]
		// TODO:
		// Make this expandable instead of showing it in the value content slot
		.ValueContent()
		[
			SNew(SCubicBezierViewer)
				.CubicBezier(this, &Self::GetCurrentValue)
		];
}

auto Self::GetCurrentValue() const -> TOptional<FCubicBezier>
{
	TArray<void*> structPtrs;
	m_Handle->AccessRawData(structPtrs);

	if (structPtrs.Num() == 0)
		return {};

	if (auto* result = (FCubicBezier*)structPtrs[0])
		return *result;

	return {};
}


#undef Self
#undef LOCTEXT_NAMESPACE
