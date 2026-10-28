#include "NRL.h"

// my consts
static float ARM_SERVO_START_ANGLE = 90.0f;
static float GRIPPER_SERVO_START_ANGLE = 90.0f;

// Declare your hardware here (file-scope), e.g.:
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

constexpr float ARM_UP_ANGLE = SERVO_START_ANGLE + 90.0f;
constexpr float ARM_DOWN_ANGLE = SERVO_START_ANGLE - 90.0f;


class SecondTeleOp : public NRLOpMode {
public:
    void init() override {
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

        drive.drive(gamepad1.)
        
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
