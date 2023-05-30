#pragma once

#include "CoreMinimal.h"


namespace RWA::Editor::Util {

template <typename T>
auto LoadAsset(FName const& referenceName) -> T*
{
	return Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *referenceName.ToString()));
}

} // namespace RWA::Editor::Util
