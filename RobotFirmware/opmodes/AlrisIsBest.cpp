#include "HexaDCMotor.h"
#include "NRL.h"
#include "RobotConfig.h"
#include <HexaOLED.h>
#include "TankDrive.h"

// Declare your hardware here (file-scope), e.g.:
static HexaDCMotor leftMotor {{ .dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM }};
static HexaDCMotor rightMotor {{ .dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM  }};
static TankDrive drive(leftMotor, rightMotor);
static HexaIMU imu{ hexaImuConfig()};

class AlrisIsBest : public NRLOpMode {

// oled variables
HexaOLED _oled{ hexaOledConfig() };
bool _oledOk = false;

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
        _oled.print("calib...");
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

    // system functions
    void init() override {
        _oledOk =_oled.begin();
        // capture success - don't ignore it
        // telemetry.addData("oled ok", _oledOk ? 1.0f : 0.0f);
        imu.begin();
        leftMotor.begin();
        rightMotor.begin();
        // Runs once when INIT is pressed  — begin() your hardware here.
    }

    void loop() override {
        // driving
        drive.setScale(gamepad1.pressed(BTN_LB) ? 0.35f : 1.0f); // slow while LB held
        drive.drive(gamepad1.leftY(), gamepad1.rightX());

        // giving data to controller
        telemetry.addData("Alris is the best programmer");
        telemetry.addData("left motor speed:", leftMotor.getSpeed());
        telemetry.addData("right motor speed:", rightMotor.getSpeed());

        batteryLow(); // Call battery low function to check if battery is low

        // Runs at 50 Hz until STOP — read gamepad1, drive motors here.
        //
        // Control each part in ONE place: loop() or an action, not
        // both. Actions run before loop() each tick, so loop() would
        // overwrite them. To share one part, step aside while a
        // sequence plays:  if (!isActionRunning()) { ... }
    }

    void stop() override {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
        // Runs once on STOP — stop your motors/servos here.
    }
};

REGISTER_OPMODE(AlrisIsBest, "AlrisIsBest", TELEOP);
