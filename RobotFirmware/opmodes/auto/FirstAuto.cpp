#include "NRL.h"
#include <cmath>
#include <memory>
#include <cstdint>

// ---------- Drive and line sensor ----------
static HexaDCMotor leftMotor  {{ .dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM,
                                 .flipped = true, .minDrive = 29 }};
static HexaDCMotor rightMotor {{ .dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM,
                                 .minDrive = 32 }};
static TankDrive     drive(leftMotor, rightMotor);
static DriveActions  driveActions(drive);

// ---------- Arm and gripper (tune angles on SERVO TEST) ----------
constexpr float ARM_DOWN   = 115.0f;
constexpr float ARM_UP     =  45.0f;
constexpr float GRIP_OPEN  = 125.0f;
constexpr float GRIP_CLOSE = 155.0f;
constexpr float ARM_DEG_S  = 100.0f;
constexpr float GRIP_DEG_S = 200.0f;

static HexaServo arm    {{ .signalPin = SERVO_1, .startAngle = ARM_UP     }};
static HexaServo gripper{{ .signalPin = SERVO_2, .startAngle = GRIP_CLOSE }};

// ---------- Mission state ----------
static const char* stage = "ready";

static ActionPtr at(const char* name) {
    return instant([name] { stage = name; });
}

// Smooth servo move: keeps stepping setPosition() until it arrives, then dwells.
static ActionPtr moveServo(HexaServo& s, float to, unsigned long dwellMs) {
    auto arrivedMs = std::make_shared<uint32_t>(0);
    return sequential({
        instant([&s, arrivedMs] {
            s.setPosition(s.getPosition());   // restart ramp clock
            *arrivedMs = 0;
        }),
        wait_until([&s, to, dwellMs, arrivedMs] {
            if (*arrivedMs == 0) {
                s.setPosition(to);
                if (std::fabs(s.getPosition() - to) < 0.5f) *arrivedMs = millis() | 1;
                return false;
            }
            return millis() - *arrivedMs >= dwellMs;
        }),
    });
}

static ActionPtr pick() {
    return sequential({
        at("PICK"),
        moveServo(gripper, GRIP_OPEN,  200),
        moveServo(arm,     ARM_DOWN,   300),
        moveServo(gripper, GRIP_CLOSE, 300),
        moveServo(arm,     ARM_UP,     200),
    });
}

static ActionPtr place() {
    return sequential({
        at("PLACE"),
        moveServo(arm,     ARM_DOWN,   300),
        moveServo(gripper, GRIP_OPEN,  300),
        moveServo(arm,     ARM_UP,     200),
    });
}

// ---------- Autonomous mission ----------
// Collects all 6 COREs one at a time, drops each into the DROP ZONE, then Parks.
static ActionPtr skillRun() {
    return sequential({
        at("wait heading"),
        driveActions.waitForHeadingReady(),
        at("leave"),
        driveActions.driveInches(12),
        at("PICK"),
        pick(),
        at("PLACE"),
        place(),
        at("park"),
        driveActions.stopAll(),
        at("DONE"),
    });
}

class FirstAuto : public NRLOpMode {
public:
    void init() override {
        leftMotor.begin();
        rightMotor.begin();
        arm.setRampRate(ARM_DEG_S);
        gripper.setRampRate(GRIP_DEG_S);
        arm.begin();
        gripper.begin();
    }

    void start() override {
        stage = "ready";
        runAction(skillRun());
    }

    void loop() override {
        telemetry.addData("stage", stage);
    }

    void stop() override {
        drive.stop();
    }
};

REGISTER_OPMODE(FirstAuto, "Auto Mk-I", AUTO);
