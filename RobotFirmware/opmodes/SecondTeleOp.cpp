// THIS IS THE CODE TO USE IN PRODUCTION
// UPLOAD THIS TO THE BOT

#include "NRL.h"
#include <HexaOLED.h>
#include <iostream>

// my consts
constexpr float ARM_SERVO_START_ANGLE = 90.0f;
constexpr float GRIPPER_SERVO_START_ANGLE = 90.0f;

// Declare hardware here (file-scope)
static HexaDCMotor leftMotor {{
    .dirPin = MOTOR_L_DIR,
    .pwmPin = MOTOR_L_PWM
}};

static HexaDCMotor rightMotor {{
    .dirPin = MOTOR_R_DIR,
    .pwmPin = MOTOR_R_PWM
}};

static HexaServo armServo {{
    .signalPin = SERVO_1,
    .startAngle = ARM_SERVO_START_ANGLE,
    .settleMs = 0
}};

static HexaServo gripServo {{
    .signalPin = SERVO_1,
    .startAngle = GRIPPER_SERVO_START_ANGLE,
    .settleMs = 0
}};

static TankDrive drive { leftMotor, rightMotor };

constexpr float ARM_UP_ANGLE = ARM_SERVO_START_ANGLE + 90.0f;
constexpr float ARM_DOWN_ANGLE = ARM_SERVO_START_ANGLE - 90.0f;
constexpr float GRIPPER_OPEN_ANGLE = GRIPPER_SERVO_START_ANGLE + 40.0f;
constexpr float GRIPPER_CLOSE_ANGLE = GRIPPER_SERVO_START_ANGLE - 40.0f;


class SecondTeleOp : public NRLOpMode {
public:
    // own functions
    void drawOnOLED() {
        imu.tick(millis());
        if (!_oledOk) return;
        // nothing to draw it the OLED didn't start
        _oled.clearDisplay();
        // wipe the buffer
        _oled.setTextSize(1);
        _oled.setTextColor(HexaOLED::WHITE);
        _oled.setCursor(0, 0);
        _oled.print("Alris is the best programmer.");
        _oled.setCursor(0, 12);
        if (!imu.isHeadingReady()) {
        _oled.print("calibrating, hold still...");
        // hold still until bias calibration finishes
        } else {
        // send heading to telemetry and also print the numeric value to the OLED
        telemetry.addData("heading", imu.getHeading());
        _oled.print(imu.getHeading(), 1); // 1 decimal place
        // push buffer to the screen
        _oled.display();
        }
    }

    void batteryLow() { // checks if battery is low and turns LED red, and sends message to controller
        if (power.isBatteryLow()) {
            userLed.setSolid(255, 0, 0);
            telemetry.addData("Battery Low:", "true");
        } else {
            userLed.setSolid(0, 0, 255);
            telemetry.addData("Battery Low:", "false");
        }
    }

    void sendTelemetry() {
        try {
            telemetry.addData("Alris is the best programmer"); // true
            telemetry.addData("left motor speed:", leftMotor.getSpeed());
            telemetry.addData("right motor speed:", rightMotor.getSpeed());
            telemetry.addData("arm servo position:", armServo.getPosition());
            telemetry.addData("gripper servo position:", gripServo.getPosition());
        } catch (std::exception e) {
            telemetry.addData("Error:", e);
            _oled.print("Error:", e);
        }
    }

    // sys functions
    void init() override {
        // oled stuff
        _oledOk =_oled.begin();
        // capture success - don't ignore it
        telemetry.addData("oled ok", _oledOk ? 1.0f : 0.0f);
        imu.begin();
        // motors
        leftMotor.begin();
        rightMotor.begin();

        // servos
        armServo.begin();
        gripServo.begin();
    }

    // Runs at 100 Hz until STOP — read gamepad1, drive motors here.
    //
    // Control each part in ONE place: loop() or an action, not
    // both. Actions run before loop() each tick, so loop() would
    // overwrite them. To share one part, step aside while a
    // sequence plays:  if (!isActionRunning()) { ... }
    void loop() override {

        /*
        ==============
        DRIVE CONTROL
        ==============
        */

        drive.drive(gamepad1.leftY(), gamepad1.rightX());
        drive.setScale(gamepad1.pressed(BTN_LB) ? 0.35f : 1.0f); // slow while LB held


        // Check whether the controller's D-pad UP button was pressed.
        if (gamepad1.pressed(BTN_DPAD_UP)) {

            armServo.setPosition(ARM_UP_ANGLE);  // Command the arm to its raised target angle.
        }

        // Check whether the controller's D-pad DOWN button was pressed.
        if (gamepad1.pressed(BTN_DPAD_DOWN)) {

            armServo.setPosition(ARM_DOWN_ANGLE);  // Command the arm to its lowered target angle.
        }

        // Check whether the controller's D-pad LEFT button was pressed.
        if (gamepad1.pressed(BTN_DPAD_LEFT)) {

            gripperServo.setPosition();  // Command the gripper to open.
        }

        // Check whether the controller's D-pad RIGHT button was pressed.
        if (gamepad1.pressed(BTN_DPAD_RIGHT)) {

            gripperServo.setPosition(GRIPPER_CLOSE_ANGLE);  // Command the gripper to close.
        }

        // draw stuff on the OLED
        drawOnOLED();
        // send data to controller
        sendTelemetry();
        // check if battery is low
        batteryLow();
    }

    }

    void stop() override {
        // Runs once on STOP — stop your motors/servos here.

        // stop motors
        leftMotor.stop();
        rightMotor.stop();

        // stop/detach servos
        armServo.detach();
        gripServo.detach();
    }
};

REGISTER_OPMODE(SecondTeleOp, "SecondTeleOp", TELEOP);
