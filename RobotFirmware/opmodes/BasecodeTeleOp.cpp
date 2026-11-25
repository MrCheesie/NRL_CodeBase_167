// Code for prod
// upload this
#include "NRL.h"

// ---------- Hardware (file scope) ----------
static HexaDCMotor leftMotor{{ .dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM }};
static HexaDCMotor rightMotor{{ .dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM, .flipped = true }};
static TankDrive drive(leftMotor, rightMotor);

static HexaServo arm{{ .signalPin = SERVO_1, .startAngle = 150.0f, .settleMs = 0 }};
static HexaServo claw{{ .signalPin = SERVO_2, .startAngle = 110.0f, .settleMs = 0 }};

static HexaIMU imu{hexaImuConfig()}; // IMU on Wire / I2C_SDA(47), I2C_SCL(48)

// ---------- Tunable values (adjust on your robot) ----------
static const float ARM_DOWN  = 150.0f;   // arm "down" preset
static const float ARM_UP    = 100.0f;  // arm "up" preset
static const float CLAW_OPEN = 100.0f;   // claw open preset
static const float CLAW_SHUT = 115.0f;   // claw closed preset
static const int   DRIVE_MAX_SPEED = 165;   // out of 255 (limits top drive speed)
static const float FINE_STEP = 1.0f;    // degrees per loop pass while D-pad held (~50 deg/s at 50 Hz)
static const float MIN_ANGLE = 0.0f;
static const float MAX_ANGLE = 180.0f;

class ArmClawTeleOp : public NRLOpMode {
    bool  _armUp    = false;
    bool  _clawOpen = false;
    // Speed mode: 0 = normal (capped), 1 = full speed, 2 = slow
    int   _speedMode = 0;
    float _armAngle  = ARM_DOWN;
    float _clawAngle = CLAW_SHUT;

    // oled variables
    HexaOLED _oled{hexaOledConfig()}; // OLED on Wire1 / OLED_SDA(6), OLED_SCL(7)
    bool _oledOk = false;

    static float clampAngle(float a) {
        if (a < MIN_ANGLE) return MIN_ANGLE;
        if (a > MAX_ANGLE) return MAX_ANGLE;
        return a;
    }

public:

    void drawOnOLED() {
      imu.tick(millis());
      // send heading regardless of whether the OLED started
      telemetry.addData("heading", imu.getHeading());
      if (!_oledOk)
        return;
      // nothing to draw it the OLED didn't start
      _oled.clearDisplay();
      // wipe the buffer
      _oled.setTextSize(1);
      _oled.setTextColor(HexaOLED::WHITE);
      _oled.setCursor(0, 0);
      _oled.print("Alris is the best programmer.");
      _oled.setCursor(0, 12);
      _oled.print("TEAM MGIS");
      _oled.setCursor(0, 24);
      if (!imu.isHeadingReady()) {
        _oled.print("calibrating, hold still...");
        // hold still until bias calibration finishes
      } else {
        // print the numeric value to the OLED (1 decimal place)
        _oled.print(imu.getHeading(), 1);
      }
      // push buffer to the screen (always, even during calibration)
      _oled.display();
    }

    void init() override {
        // Claim hardware only. Nothing should move here.
        leftMotor.begin();
        rightMotor.begin();
        arm.begin();
        claw.begin();
        drive.setRampRate(0.5f); // gentler on the battery; tune as needed
        drive.setScale(DRIVE_MAX_SPEED / 255.0f); // 165/255 = ~0.65 speed cap
    }

    void loop() override {
        // ----- BTN_Y: toggle full speed (unused button) -----
        // ----- BTN_X: toggle slow mode (unused button) -----
        // Each press flips between its mode and normal; pressing the other mode
        // switches straight to that mode.
        if (gamepad1.justPressed(BTN_Y)) {
            _speedMode = (_speedMode == 1) ? 0 : 1; // 1 = full speed
            if (_speedMode == 1) drive.setScale(1.0f);
            else if (_speedMode == 0) drive.setScale(DRIVE_MAX_SPEED / 255.0f);
            else drive.setScale(0.35f);
        }
        if (gamepad1.justPressed(BTN_X)) {
            _speedMode = (_speedMode == 2) ? 0 : 2; // 2 = slow
            if (_speedMode == 2) drive.setScale(0.35f);
            else if (_speedMode == 0) drive.setScale(DRIVE_MAX_SPEED / 255.0f);
            else drive.setScale(1.0f);
        }

        // ----- Driving: left stick Y = forward/back, right stick X = turn -----
        // Turn is negated: stick right was turning the robot left on this build
        drive.drive(gamepad1.leftY(), -gamepad1.rightX());

        // ----- LB: arm up/down toggle (one toggle per press) -----
        if (gamepad1.justPressed(BTN_LB)) {
            _armUp = !_armUp;
            _armAngle = _armUp ? ARM_UP : ARM_DOWN;
        }

        // ----- RB: claw open/close toggle (one toggle per press) -----
        if (gamepad1.justPressed(BTN_RB)) {
            _clawOpen = !_clawOpen;
            _clawAngle = _clawOpen ? CLAW_OPEN : CLAW_SHUT;
        }

        // ----- D-pad up/down: fine-adjust arm while held -----
        // Arm UP = lower angle (100), arm DOWN = higher angle (150)
        if (gamepad1.pressed(BTN_DPAD_UP))   _armAngle = clampAngle(_armAngle - FINE_STEP);
        if (gamepad1.pressed(BTN_DPAD_DOWN)) _armAngle = clampAngle(_armAngle + FINE_STEP);

        // ----- D-pad right/left: fine-adjust claw while held -----
        // Right = open, Left = close (see note: your list had "right" twice)
        // Direction assumes a LOWER angle is more open (CLAW_OPEN < CLAW_SHUT).
        if (gamepad1.pressed(BTN_DPAD_RIGHT)) _clawAngle = clampAngle(_clawAngle - FINE_STEP);
        if (gamepad1.pressed(BTN_DPAD_LEFT))  _clawAngle = clampAngle(_clawAngle + FINE_STEP);

        // ----- Apply -----
        arm.setPosition(_armAngle);
        claw.setPosition(_clawAngle);

        // ----- Telemetry (keys <= 11 chars, values <= 15 chars) -----
        telemetry.addData("arm", _armAngle);
        telemetry.addData("claw", _clawAngle);
        telemetry.addData("speed", _speedMode == 1 ? "full" : (_speedMode == 2 ? "slow" : "normal"));
        telemetry.addData("battery", power.getBatteryVoltage());
    }

    void stop() override {
        // Motors do NOT stop automatically.
        drive.stop();
    }
};

REGISTER_OPMODE(ArmClawTeleOp, "Arm Claw TeleOp", TELEOP);
