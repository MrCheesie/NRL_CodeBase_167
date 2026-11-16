// THIS IS THE CODE TO USE IN PRODUCTION
// UPLOAD THIS TO THE BOT

#include "NRL.h"
#include "NRLGamepad.h"

// my consts
constexpr float ARM_SERVO_START_ANGLE = 90.0f;
constexpr float GRIPPER_SERVO_START_ANGLE = 90.0f;

static HexaIMU imu{hexaImuConfig()}; // IMU on Wire / I2C_SDA(47), I2C_SCL(48)

// Declare hardware here (file-scope)
static HexaDCMotor leftMotor{{.dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM}};

static HexaDCMotor rightMotor{{.dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM}};

static HexaServo armServo{
    {.signalPin = SERVO_1, .startAngle = ARM_SERVO_START_ANGLE, .settleMs = 0}};

static HexaServo gripServo{{.signalPin = SERVO_2,
                            .startAngle = GRIPPER_SERVO_START_ANGLE,
                            .settleMs = 0}};

static TankDrive drive{leftMotor, rightMotor};

constexpr float ARM_UP_ANGLE = ARM_SERVO_START_ANGLE - 90.0f;
constexpr float ARM_DOWN_ANGLE = ARM_SERVO_START_ANGLE + 90.0f;
constexpr float GRIPPER_OPEN_ANGLE = GRIPPER_SERVO_START_ANGLE - 40.0f;
constexpr float GRIPPER_CLOSE_ANGLE = GRIPPER_SERVO_START_ANGLE + 40.0f;

class SecondTeleOp : public NRLOpMode {
  // oled variables
  HexaOLED _oled{hexaOledConfig()}; // OLED on Wire1 / OLED_SDA(6), OLED_SCL(7)
  bool _oledOk = false;

public:
  // own functions
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

  void batteryLow() { // checks if battery is low and turns LED red, and sends
                      // message to controller
    if (power.isBatteryLow()) {
      userLed.setSolid(255, 0, 0);
      telemetry.addData("Battery Low", "true");
    } else {
      userLed.setSolid(0, 0, 255);
      telemetry.addData("Battery Low", "false");
    }
  }

  void sendTelemetry() {

    // removed try/catch block
    telemetry.addData("Alris is the ", "best programmer"); // true
    telemetry.addData("l motor spd",
                      static_cast<float>(leftMotor.getSpeed()));
    telemetry.addData("r motor spd",
                      static_cast<float>(rightMotor.getSpeed()));
    telemetry.addData("arm servo", armServo.getPosition());
    telemetry.addData("grip servo", gripServo.getPosition());
  }

  // sys functions
  void init() override {
    // oled stuff
    _oledOk = _oled.begin();
    // capture success - don't ignore it
    telemetry.addData("oled ok", _oledOk ? 1.0f : 0.0f);
    if (!imu.begin())
      telemetry.addData("imu begin", "failed");
    // motors
    leftMotor.begin();
    leftMotor.setFlipped(true);
    rightMotor.begin();
    rightMotor.setFlipped(true);

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

    // slow while DPAD LEFT held (set scale BEFORE drive so it applies this tick)
    drive.setScale(gamepad1.pressed(BTN_DPAD_LEFT) ? 0.35f : 1.0f);
    drive.drive(gamepad1.rightX(), gamepad1.leftY());

    // Check whether the controller's D-pad UP button was pressed.
    if (gamepad1.pressed(BTN_DPAD_UP)) {

      armServo.setPosition(
          ARM_UP_ANGLE); // Command the arm to its raised target angle.
    }

    // Check whether the controller's D-pad DOWN button was pressed.
    if (gamepad1.pressed(BTN_DPAD_DOWN)) {

      armServo.setPosition(
          ARM_DOWN_ANGLE); // Command the arm to its lowered target angle.
    }

    // Check whether the controller's D-pad LEFT button was pressed.
    if (gamepad1.pressed(BTN_LB)) {

      gripServo.setPosition(GRIPPER_OPEN_ANGLE); // Command the gripper to open.
    }

    // Check whether the controller's D-pad RIGHT button was pressed.
    if (gamepad1.pressed(BTN_LB)) {

      gripServo.setPosition(
          GRIPPER_CLOSE_ANGLE); // Command the gripper to close.
    }

    // draw stuff on the OLED
    drawOnOLED();
    // send data to controller
    sendTelemetry();
    // check if battery is low
    batteryLow();
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
