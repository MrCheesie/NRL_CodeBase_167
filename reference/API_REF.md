


# A one-page cheat-sheet for everything covered in Phase 3 and Phase 4. See each module for full explanations and worked examples.



## OpMode skeleton




```cpp
class Name : public NRLOpMode {
 void init() override { /* begin() hardware */ }
 void loop() override { /* every pass */ }
 void stop() override { /* stop motors */ }
}; 

REGISTER_OPMODE(Name, "Menu Label", TELEOP); // or AUTO
```



## Motors — HexaDCMotor

| Member                    | Purpose                 |
| ------------------------- | ----------------------- |
| `dirPin, pwmPin, flipDir` | Declaration config.     |
| `begin()`                 | Init (`init()`).        |
| `setSpeed(255, 255)`      | Set speed; `0 = coast`. |
| `getSpeed() / stop()`     | Read speed / stop.      |

## Drive — TankDrive

| Member                            | Purpose                |
| --------------------------------- | ---------------------- |
| `TankDrive(left, right)`          | Build from two motors. |
| `drive(forward, turn)`            | Each `-1.0..+1.0`.     |
| `stop()`                          | Stop both.             |
| `setScale(0..1) / setRampRate(x)` | Speed cap / smoothing. |

## Servo — HexaServo

| Member                            | Purpose                    |
| --------------------------------- | -------------------------- |
| `...`                             | Declaration config.        |
| `begin()`                         | Init + go to start angle.  |
| `setPosition(0..180) / moveBy(d)` | Absolute / relative angle. |
| `getPosition()`                   | Read angle.                |
| `detach() / attach()`             | Release / re-engage.       |

## Gamepad — gamepad1

| Member                                         | Purpose                                                                      |
| ---------------------------------------------- | ---------------------------------------------------------------------------- |
| `leftY() leftX() rightX() rightY()`            | Axes, `-1.0..+1.0`.                                                          |
| `pressButton(BTN_...)`                         | Held right now.                                                              |
| `justPressed(BTN_...) / justReleased(BTN_...)` | Edge events.                                                                 |
| `onPress(BTN_..., cb)`                         | Bind a callback (in `init()`).                                               |
| `Buttons`                                      | `BTN_X A B · BTN_DPAD_UP/DOWN/LEFT/RIGHT · BTN_LB RB LT RT · (BTN Y = STOP)` |

## Heading & IMU

| Member                                             | Purpose                 |
| -------------------------------------------------- | ----------------------- |
| `NRLComms.getHeading()`                            | Heading in degrees.     |
| `NRLComms.zeroHeading()`                           | Call current heading 0. |
| `NRLComms.isHeadingReady()`                        | Calibration done?       |
| `HexaIMU.tick() / read() / getAccel() / getGyro()` | Raw motion access.      |
| ```                                                |                         |

## Telemetry, LED, Power

| Member                                                             | Purpose                                  |
| ------------------------------------------------------------------ | ---------------------------------------- |
| `telemetry.addData(key, value)`                                    | Send a number or text to the Controller. |
| `userLed.setRed() / setOff() / setBlink() / setPulse() / setOff()` | Drive the status LED.                    |
| `power.getBatteryVoltage() / getBatteryLevel()`                    | Battery state.                           |
| `power.getMotorCurrent() / getServoCurrent() / getMainCurrent()`   | Rail currents.                           |

## Actions & DriveActions

| Member                                                         | Purpose                     |
| -------------------------------------------------------------- | --------------------------- |
| `runAction(a) / isActionRunning()`                             | Queue a routine / check it. |
| `instant, sleep, ms, wait_until, sequential, parallel, repeat` | Action factories.           |
| `driveActions.turn(deg) / turnTo(t)`                           | Closed-loop IMU turns.      |
| `driveActions.driveInches(in) / driveForMs(ms)`                | Straight moves.             |
| `driveActions.auto { ...build}`                                | Chain a whole routine.      |
