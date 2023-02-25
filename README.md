# Rotary-Wing Aircraft Plugin

This plugin provides a Chaos-powered helicopter flight model and modular
aircraft HUD for Unreal Engine 5.

[
	![
		WIP Helicopter Flight Model for Unreal Engine 5.1
	](https://img.youtube.com/vi/ig98iGbf2Bw/maxresdefault.jpg)
](https://www.youtube.com/watch?v=ig98iGbf2Bw "Watch on YouTube")

## Flight Model

The included flight model could best be described as "SimCade". It's intended to
be easy to learn and controllable with basic hardware like a standard console
gamepad, while still feeling plausibly realistic and providing some depth and
challenge for players who want to invest the time to "master" it.

### Features

* Drag simulation with dynamic cross-sectional area measurement
* Simplistic simulation of helicopter-specific phenomena like ground effect
* Designer-tunable parameters for power, agility, and aerodynamic coefficients
* Modular architecture inspired by Epic's Chaos Vehicles plugin

## HUD

The current iteration of the HUD is implemented as a post-process material. This
gives it a nice emissive quality with bloom at high brightness, but carries a
substantial down-side in that it doesn't play nicely with TAA, TSR, or excessive
motion blur. If you use this HUD as-is, you'll probably want to enable FXAA and
set your Motion Blur > Target FPS to 60 or higher.

It's also kind of convoluted to set up. You can take a look at the included
`BP_HUDController` and `BP_SampleHeli` blueprints to see how it's done and
adjust to your needs.

A UMG version (and potentially a more ergonomic workflow for the post-process
version) is in the works.

## Getting Started

### Installing the plugin

If you're already using Git in your project, it's recommended to add this repo
as a submodule. From the root directory:

```sh
git submodule add https://github.com/dannymcgee/Unreal_RotaryWingAircraft.git ./Plugins/Gameplay/RotaryWingAircraft
```

Otherwise, you can clone it into your project in much the same way:

```sh
git clone https://github.com/dannymcgee/Unreal_RotaryWingAircraft.git ./Plugins/Gameplay/RotaryWingAircraft
```

> **IMPORTANT:** Downloading as a ZIP file is **not recommended** unless you only need the C++ source
files. This repo uses Git LFS for binaries (e.g., blueprints), which will not be
included in the ZIP archive.

Finally, add the plugin to your `*.uproject` definition:

```json
{
	...
	"Plugins": [
		...
		{
			"Name": "RotaryWingAircraft",
			"Enabled": true
		}
	]
}
```

### Taking it for a spin

You can do the following to give the plugin a quick test drive before committing
to use it in your project:

1. Install the plugin.
1. Navigate to **Project Settings** > **Project** > **Maps & Modes**.
1. Set **Default GameMode** to `BP_SampleHeliGameMode`, and under
   **Selected GameMode**, set **Default Pawn Class** to `BP_SampleHeli`.
1. If you'd like to try out the included HUD, ensure that there's an enabled
   PostProcessVolume actor in your scene with **Post Process Volume Settings |
	Infinite Extent (Unbounded)** enabled.
1. If you find that the HUD looks like a blurry mess, see the HUD notes above.

### Setting up a new helicopter from scratch

The workflow here is very similar to Epic's Chaos Vehicles.

#### Preparing art assets

* Ensure that your skeletal mesh has bones for all of the rotors that you'd like
  to spin in sync with the flight model's engine state.
* Ensure that your mesh faces the +X axis.
* Ensure that your mesh has an assigned Physics Asset with a reasonable physics
  body attached to the root bone.
* You may need to manually override the Mass of your physics body to a
  reasonable value. For example, the included `SK_SampleHeli_Phys` is set to 900
  kg, which is roughly the weight of a laden MH-6 Little Bird.

#### Animation Blueprint

* Create an animation blueprint for your skeletal mesh, and set its parent class
  to `HeliAnimInstance`.
* Setup the following nodes in your `AnimGraph`:
  ```
  Mesh Space Ref Pose -> Rotor Controller -> Component to Local -> Output Pose
  ```

#### Pawn Blueprint

* Create a new Blueprint Class derived from `Heli`.
* Assign your skeletal mesh to `VehicleMesh` and set its **Animation** > **Anim
  Class** to the animation blueprint created in the previous step.
* Under **Input**, assign the included `IA_Heli_Cyclic`, `IA_Heli_Collective`,
  and `IA_Heli_AntiTorque` actions. You can assign the included
  `IMC_Heli_Default` as the Default Mapping Context, or you can create your own
  bindings to suit your preferences.
* Add a camera to your own preferences. You can take a look at the included
  `BP_SampleHeli` for an example.
* Under **Vehicle Movement** > **Vehicle Setup** > **Rotors**, create a rotor
  setup for each of your mesh's rigged rotors. Set **Bone Name** to the name of
  the skeleton bone that controls the rotor, and **Torque Normal** to a
  normalized vector that corresponds to the axis the rotor blades should spin
  around (in component space).
* Create CurveFloat assets for **Altitude Penalty Curve**, **Drag Coefficient
  Curve**, and **Aerodynamic Torque Influence**. Check the tooltips for
  explanations and axis descriptions, and configure to taste.

Alternatively, if the included `Heli` pawn doesn't meet your needs for some
reason, feel free to create your own Pawn class from scratch and configure it to
use the `HeliMovementComponent`. The movement component class has a very
straightforward, blueprint-friendly API allowing you to set inputs and query for
information about the vehicle's state.
