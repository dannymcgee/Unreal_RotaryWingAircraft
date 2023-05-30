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
			.MaxDesiredWidth({})
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0,0,8,0)
			[
				propHandle->CreatePropertyNameWidget()
			]
			+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
					.ColorAndOpacity(FColor{ 0xFF, 0xFF, 0xFF, 0x40 })
					.Text(LOCTEXT("RWA_ReadOnly", "read-only"))
			]
		]
		// TODO:
		// Make this expandable instead of showing it in the value content slot
		.ValueContent()
			.MinDesiredWidth(0)
			.MaxDesiredWidth({})
		[
			SNew(SCubicBezierViewer)
				.CubicBezier(this, &Self::GetCurrentValue)
				.ColorAndOpacity(FColor{ 0xFF, 0xFF, 0xFF, 0x14 })
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
