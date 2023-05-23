# [2.0.0] - UMG HUD, C++ reorganization

## HUD

The main feature of this update is a new HUD that's been rebuilt from scratch as a UMG widget blueprint.

![New UMG HUD](https://media.githubusercontent.com/media/dannymcgee/Unreal_RotaryWingAircraft/d39e80bdea0254fc4f8b3df15b19a93b6f2def82/Resources/UMG_HUD_v2_0.jpg)

Functionally, it has feature-parity with the old post-process material, plus some unobtrusive readouts of the Collective and Anti-Torque input axes.

The design is mostly the same, with some tweaks to the pitch ladder and cross-hairs, and a new font for the numeric values (necessary because the old font's license doesn't allow for embedding in applications).

More importantly, converting the HUD to UMG comes with some big advantages:

* No more temporal smearing and ghosting, since the UI isn't rendered into the main scene color buffer
* It is now possible for mere mortals (like myself) to make adjustments to the layout, add or remove elements, etc., without needing to wade through a labyrinth of inscrutable material graphs.

Under the hood, most of the individual HUD elements are still implemented as dynamic UI materials, but they each live in a UV space that's much easier to reason about, so they should be much more understandable with some minimal knowledge of shader programming.

The one major downside (and the main reason it wasn't implemented this way to begin with) is the loss of the bloom effect. I plan to investigate reimplementing that with a custom Slate widget and a compute shader, but that will have to happen at some later date.

The old post-process HUD lives on as an alternative (mostly for backward compatibility), but it will not receive any updates going forward. To switch back to that version, you can change the `BP_SampleHeli`'s `HUD Class` property from `BP_HUD_UMG` to `BP_HUD_PostProcess`. (Incidentally, you can also now write your own HUD from scratch if you'd like, by implementing the `BPI_AircraftHUD` interface in a new blueprint, and using that for the `HUD Class`.)

## Housekeeping

Beyond the new HUD, some minor reorganization has been done to the C++ source code. This won't affect anyone interfacing with the plugin solely through the Blueprint API, but if you're consuming the C++ API, you'll need to update some header `#include`s and type names — hence the semver-major version bump. Here's a summary of the changes:

* All headers and source files have been moved into an `RWA` subdirectory
  - e.g.: `#include "Heli.h"` -> `#include "RWA/Heli.h"`
* Classes have been renamed with an `RWA_` prefix to avoid name collisions
  - e.g.: `AHeli` -> `ARWA_Heli`


# [1.0.0] - Initial Release
