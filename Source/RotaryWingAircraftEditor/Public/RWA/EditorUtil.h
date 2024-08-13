#pragma once

#include "CoreMinimal.h"


namespace RWA::Editor::Util {

template <typename T>
T* LoadAsset(FName const& referenceName)
{
	return Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *referenceName.ToString()));
}

} // namespace RWA::Editor::Util
