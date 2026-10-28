/*
#include "NRL.h"
static HexaDCMotor leftMotor  {{ .dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM, .flipped = true }}; 
static HexaDCMotor rightMotor {{ .dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM }};
static HexaServo  armServo   {{ .signalPin = SERVO_1, .startAngle = 60.0f, .settleMs = 0 }};
static HexaServo  gripperServo {{ .signalPin = SERVO_2, .startAngle = 165.0f, .settleMs = 0 }};
static TankDrive  drive      { leftMotor, rightMotor };


class fullcode : public NRLOpMode {
public:
    void init() override {
        leftMotor.begin();
        rightMotor.begin();
        armServo.begin();
        gripperServo.begin();
    }

    void loop() override {
        
        drive.drive(gamepad1.leftY() , gamepad1.rightX());

        if (gamepad1.pressed(BTN_DPAD_UP)) {  // arm up
            armServo.setPosition();
        }
        if (gamepad1.pressed(BTN_DPAD_DOWN)) {// arm down
            armServo.setPosition();
        }
        if (gamepad1.pressed(BTN_DPAD_LEFT)) { //gripper open
            gripperServo.setPosition();  
        }
        if (gamepad1.pressed(BTN_DPAD_RIGHT)) { // gripper close
            gripperServo.setPosition();  
        }
    }

    void stop() override {
        leftMotor.stop();
        rightMotor.stop();
        armServo.detach();
        gripperServo.detach();
    }
};
REGISTER_OPMODE(fullcode, "RUNBOT", TELEOP);
*/


// Code with full Explaination !!!

#include "NRL.h"  // Includes the NRL library containing motor, servo, gamepad, and OpMode definitions


// ============================================================
// PART 1: DEFINE THE ROBOT HARDWARE
// These objects represent the physical components of the robot.
// ============================================================

// Create the left DC motor object.
static HexaDCMotor leftMotor {
    {
        .dirPin = MOTOR_L_DIR,   // Pin used to control the motor's direction.
        .pwmPin = MOTOR_L_PWM,   // Pin used to control the motor's speed.
        .flipped = true          // Reverse the motor direction in the library.
    }
};

// Create the right DC motor object.
static HexaDCMotor rightMotor {
    {
        .dirPin = MOTOR_R_DIR,   // Pin used to control the motor's direction.
        .pwmPin = MOTOR_R_PWM    // Pin used to control the motor's speed.
    }
};

// Create the arm servo object.
static HexaServo armServo {
    {
        .signalPin = SERVO_1,    // Pin connected to the arm servo's signal wire.
        .startAngle = 60.0f,     // Initial configured angle of the arm servo.
        .settleMs = 0            // Configured settling delay in milliseconds.
    }
};

// Create the gripper servo object.
static HexaServo gripperServo {
    {
        .signalPin = SERVO_2,    // Pin connected to the gripper servo's signal wire.
        .startAngle = 165.0f,    // Initial configured angle of the gripper servo.
        .settleMs = 0            // Configured settling delay in milliseconds.
    }
};

// Create a tank-drive object using the left and right motors.
static TankDrive drive { leftMotor, rightMotor };


// ============================================================
// PART 2: DEFINE SERVO TARGET ANGLES
// These constants make it easier to adjust servo positions.
// The values are examples and must be calibrated on the robot.
// ============================================================

constexpr float ARM_UP_ANGLE = 90.0f;            // Example target angle for raising the arm.
constexpr float ARM_DOWN_ANGLE = 60.0f;          // Example target angle for lowering the arm.
constexpr float GRIPPER_OPEN_ANGLE = 120.0f;     // Example target angle for opening the gripper.
constexpr float GRIPPER_CLOSE_ANGLE = 165.0f;    // Example target angle for closing the gripper.


// ============================================================
// PART 3: CREATE THE OPMODE CLASS
// An OpMode defines how the robot behaves during a session.
// ============================================================

class fullcode : public NRLOpMode {  // Create a class named fullcode based on NRLOpMode.

public:  // The following functions can be accessed by the framework.

    // --------------------------------------------------------
    // PART 4: INITIALIZATION FUNCTION
    // This function prepares the hardware before control begins.
    // --------------------------------------------------------

    void init() override {  // Override the initialization function provided by NRLOpMode.

        leftMotor.begin();      // Initialize the left motor.
        rightMotor.begin();     // Initialize the right motor.

        armServo.begin();       // Initialize the arm servo.
        gripperServo.begin();   // Initialize the gripper servo.
    }


    // --------------------------------------------------------
    // PART 5: MAIN CONTROL LOOP
    // This function repeatedly checks inputs while the OpMode runs.
    // --------------------------------------------------------

    void loop() override {  // Override the repeated control-loop function.

        // Read the controller joysticks and send their values to the drive system.
        drive.drive(gamepad1.leftY(), gamepad1.rightX());
        // leftY() reads the vertical axis of the left joystick.
        // rightX() reads the horizontal axis of the right joystick.
        // The drive library determines how these values control the motors.


        // ----------------------------------------------------
        // PART 6: ARM CONTROL
        // ----------------------------------------------------

        // Check whether the controller's D-pad UP button was pressed.
        if (gamepad1.pressed(BTN_DPAD_UP)) {

            armServo.setPosition(ARM_UP_ANGLE);  // Command the arm to its raised target angle.
        }

        // Check whether the controller's D-pad DOWN button was pressed.
        if (gamepad1.pressed(BTN_DPAD_DOWN)) {

            armServo.setPosition(ARM_DOWN_ANGLE);  // Command the arm to its lowered target angle.
        }


        // ----------------------------------------------------
        // PART 7: GRIPPER CONTROL
        // ----------------------------------------------------

        // Check whether the controller's D-pad LEFT button was pressed.
        if (gamepad1.pressed(BTN_DPAD_LEFT)) {

            gripperServo.setPosition(GRIPPER_OPEN_ANGLE);  // Command the gripper to open.
        }

        // Check whether the controller's D-pad RIGHT button was pressed.
        if (gamepad1.pressed(BTN_DPAD_RIGHT)) {

            gripperServo.setPosition(GRIPPER_CLOSE_ANGLE);  // Command the gripper to close.
        }
    }


    // --------------------------------------------------------
    // PART 8: STOP FUNCTION
    // Called when the OpMode is stopped by the framework.
    // --------------------------------------------------------

    void stop() override {  // Override the stop function provided by NRLOpMode.

        leftMotor.stop();   // Stop the left motor.
        rightMotor.stop();  // Stop the right motor.

        armServo.detach();      // Release control of the arm servo.
        gripperServo.detach();  // Release control of the gripper servo.
    }
};


// ============================================================
// PART 9: REGISTER THE OPMODE
// This makes the program available in the NRL OpMode system.
// ============================================================

REGISTER_OPMODE(fullcode, "RUNBOT", TELEOP);
// fullcode = Name of the class containing the robot program.
// "RUNBOT" = Name displayed for this OpMode.
// TELEOP = Identifies the program as driver-controlled operation.

