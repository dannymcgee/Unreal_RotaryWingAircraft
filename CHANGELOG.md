# [2.2.0] - Upgrade to UE 5.4

* Migrated raw pointer properties to `TObjectPtr`
* Fixed an issue with thrust computation that made aircraft tuning difficult
* Refactored C++ to move closer to Epic's style conventions

# [2.1.1] - Bug fixes

* Fixed an issue that prevented the game from being packaged
* Fixed a framerate-independence issue in the Virtual Throttle input modifier
* <details><summary>Fixed some issues with the HUD appearance when the game is running outside of a PIE viewport</summary>
  Previously, the Viewport Scale was being set once, when the SampleHeli was first possessed by a player or spectator. A timing difference between the PIE viewport vs independent windows (like a packaged game) caused the resulting viewport scale to be incorrect for the latter.</details>

# [2.1.0] - Input Improvements

This update brings a couple of handy Input Modifiers and a new Input Trigger to make it easier to design intuitive control schemes for players with a limited number of analog axes (for example, mouse & keyboard users).

## Input Modifiers
### Virtual Joystick

This modifier transforms inputs coming from digital button/key presses to emulate the feel of a springy analog joystick. Basically, inputs are mapped to outputs by following a cubic bezier curve, where the `x` parameter is the duration of the keypress, and the corresponding `y` value is the output.

It comes with a set of tunable parameters to dial in exactly the feel you're looking for:

- **Resistance** - Emulates a linear tension/friction on the joystick, modulating how quickly the output value reaches the target value. Higher values _increase_ the overall duration of the curve.
- **Attack** - Modulates the initial slope of the curve, i.e. how quickly the output starts accelerating toward the target on the initial keypress. Higher values _decrease_ the overall duration of the curve.
- **Spring Tension** - Applies additional resistance proportional to how far the current output is from the axis's resting/centered position. This has the effect of "easing out" movement from 0 to +/-1, and the opposite effect on movement back to 0 (increasing the initial acceleration towards 0 once the input is released).
- **Spring Balance** - Distributes the "weight" of the curvature toward the leading (from 0) or trailing (to +/-1) edge of the curve. A value of 0.5 results in a smoother overall curve, meaning the change in acceleration is more gradual over its duration.
- **Damping** - This is basically the counter to **Spring Tension**, having the effect of "easing out" the transition from +/-1 back to 0 once the input is released.

The total impact of these parameters can be a little bit tricky to reason about (or explain!), so as a design aid, the modifier displays a small visual representation of the Rising (from 0 to +/-1) and Falling (from +/-1 to 0) curves that are produced by the chosen settings:

![Input Modifier | Virtual Joystick](./Resources/InputModifier_VirtualJoystick_v2_1.png)

### Virtual Throttle

Similar to the Virtual Joystick modifier (but much simpler), the Virtual Throttle transforms digital button/key presses into simple linear motion along an analog axis. Unlike the Virtual Joystick, the Virtual Throttle is "sticky," meaning that when the input is released, the axis value stays put until it's told to move again. This one has much fewer (and again, simpler) parameters:

- **Axis ID** - This is an unfortunate necessity due to a limitation of the APIs provided by Unreal (or perhaps the unorthodox way I'm trying to use them). This _must be provided_ or the modifier will not work (and you'll see an error in the Output Log). All Virtual Throttle modifiers that act upon a common axis (i.e., one key for pushing the axis up and another for pulling it down) must be set to use the same Axis ID, or else the two key-bindings will "fight" one another, leading to very unexpected results.
- **Sensitivity** - Determines how quickly the axis value moves while the input is pressed.
- **Detent** - Adds a subtle virtual [detent](https://reference.wolfram.com/applications/mechsystems/MultistageMechanisms/TimeSwitchingConstraints/HTMLImages/Mech.5.2.2.en/Mech.5.2.2.en_17.gif) around the 0 value of the axis, providing a smidge of extra resistance when first moving away from 0, and a tendency to settle back to 0 when the input is released within the detent's window. For example, if the detent is set to 0.05, and the throttle value is sitting at 4% when all throttle inputs are released, the throttle value will settle back to exactly 0% over a short duration.

## Input Triggers

### Decayed Release

This is a (probably poorly named) Input Trigger that enables both of the Input Modifiers above. With the default triggers provided by the Enhanced Input plugin, all input values immediately cease to be sent once the bound key is released, and the input value returns to zero no matter what Input Modifiers are present. This is not great if you want to interpolate back to 0 over time when a key is released. This trigger enables our desired behavior by continuing to emit the "Triggered" event until both the raw input _and_ the modified output both reach zero.

However, **do be warned** that this may have performance implications, as each instance of the Decayed Release trigger needs to tick every frame to determine what state it should emit. I didn't notice any difference in my own (informal) testing, but a comment in Epic's source code warns against using this option if it can be avoided, so I thought it was worth mentioning here as a disclaimer.

## What's next for input?

These modifiers are a good first step toward solid mouse & keyboard support, but the helicopter is frankly still pretty awkward to control (particularly the collective) even with these in place. The next step is to add an assist layer to the input in a (near) future update.

The idea behind this is that players constrained by less-than-ideal input hardware (or a simple lack of dexterity) could enable a simplified control scheme that attempts to interpret the player's _intention_ (i.e., holding Shift because they want to gain altitude, or holding W because they want to move forward), and translates that to produce inputs for the simulation that will achieve that objective.

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
