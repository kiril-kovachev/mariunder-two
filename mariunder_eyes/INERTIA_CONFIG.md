# Eye Inertia Effect Configuration

## Overview
The eye inertia effect uses the MPU6050 gyroscope to create realistic eye movement based on device rotation. When you rotate the device, the eyes lag behind with a physics-based inertia effect, making them feel more lifelike.

## How It Works
- The gyroscope measures rotation rate in 3D space
- Rotation is converted to acceleration on the eye position
- Eyes have velocity and damping, creating a lag effect
- A return force pulls the eyes back to center when rotation stops
- The effect is additive - it works on top of other eye movements like random looking

## Configuration Parameters

All parameters can be adjusted in `mariunder_eyes.ino` after creating the Face object:

### `Enabled` (bool)
- **Default:** `true`
- **Description:** Enable or disable the inertia effect entirely
- **Example:**
  ```cpp
  face->Inertia.Enabled = false;  // Disable inertia
  ```

### `Sensitivity` (float)
- **Default:** `1.0`
- **Range:** `0.1` to `5.0`
- **Description:** Multiplier for how much rotation affects eye movement
  - Lower values (0.5): Subtle, gentle movement
  - Default (1.0): Natural, responsive movement
  - Higher values (2.0-3.0): Dramatic, exaggerated movement
- **Example:**
  ```cpp
  face->Inertia.Sensitivity = 1.5f;  // More pronounced effect
  ```

### `Damping` (float)
- **Default:** `0.85`
- **Range:** `0.0` to `1.0`
- **Description:** Controls how quickly the eyes slow down (friction/drag)
  - Lower values (0.5): Eyes stop quickly, snappy movement
  - Default (0.85): Natural, smooth deceleration
  - Higher values (0.95): Very sluggish, slow to stop
- **Example:**
  ```cpp
  face->Inertia.Damping = 0.9f;  // More sluggish movement
  ```

### `MaxDisplacement` (float)
- **Default:** `8.0` pixels
- **Range:** `2.0` to `15.0`
- **Description:** Maximum distance eyes can move from center due to inertia
  - Lower values (4.0): Constrained, subtle movement
  - Default (8.0): Noticeable but natural range
  - Higher values (12.0): Wide range, more dramatic
- **Example:**
  ```cpp
  face->Inertia.MaxDisplacement = 10.0f;  // Allow more movement
  ```

### `ReturnSpeed` (float)
- **Default:** `0.05`
- **Range:** `0.01` to `0.2`
- **Description:** How quickly eyes return to center when no rotation
  - Lower values (0.02): Slow, drift back gradually
  - Default (0.05): Moderate return speed
  - Higher values (0.15): Quick snap back to center
- **Example:**
  ```cpp
  face->Inertia.ReturnSpeed = 0.08f;  // Faster return
  ```

## Example Configurations

### Subtle, Gentle Effect
```cpp
face->Inertia.Sensitivity = 0.7f;
face->Inertia.Damping = 0.8f;
face->Inertia.MaxDisplacement = 5.0f;
face->Inertia.ReturnSpeed = 0.04f;
```

### Dramatic, Bouncy Effect
```cpp
face->Inertia.Sensitivity = 2.5f;
face->Inertia.Damping = 0.9f;
face->Inertia.MaxDisplacement = 12.0f;
face->Inertia.ReturnSpeed = 0.03f;
```

### Quick, Responsive Effect
```cpp
face->Inertia.Sensitivity = 1.2f;
face->Inertia.Damping = 0.75f;
face->Inertia.MaxDisplacement = 8.0f;
face->Inertia.ReturnSpeed = 0.1f;
```

### Floating, Slow-Motion Effect
```cpp
face->Inertia.Sensitivity = 1.5f;
face->Inertia.Damping = 0.95f;
face->Inertia.MaxDisplacement = 10.0f;
face->Inertia.ReturnSpeed = 0.02f;
```

## Technical Details

### Transformation Chain
The inertia effect is implemented as a separate transformation in the eye rendering pipeline:
```
Config → Transformation (look) → InertiaTransformation → Variation → Blink → Final
```

This allows the inertia effect to combine with:
- Random look movements
- Emotional expressions
- Blink animations
- Eye variations

### Physics Model
The inertia system uses a simple physics simulation:
1. Gyroscope rotation → Acceleration
2. Acceleration → Velocity (with damping)
3. Spring force pulls eyes toward center
4. Velocity → Position (clamped to MaxDisplacement)
5. Position applied as eye offset

### Performance
- Updates every frame (≈10ms)
- Very lightweight calculation
- No noticeable performance impact

## Troubleshooting

**Eyes don't move with rotation:**
- Check that `Enabled = true`
- Verify MPU6050 is working (check serial output)
- Try increasing `Sensitivity`

**Eyes move too much/wildly:**
- Decrease `Sensitivity`
- Decrease `MaxDisplacement`
- Increase `Damping`

**Eyes feel sluggish:**
- Decrease `Damping`
- Increase `ReturnSpeed`

**Eyes don't return to center:**
- Increase `ReturnSpeed`
- Decrease `Damping`
