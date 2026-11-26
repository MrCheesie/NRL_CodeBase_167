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
static HexaLineArray line{ hexaLineConfig() };
static LineActions   lineActions(drive, line);

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
    return lineActions.auto_()
        // Leave the black START box onto the main line.
        .then(at("leave")).leaveBox()

        // ---- CORE 1: right side, bottom pick-up -> drop at D1 (forward) ----
        .then(at("to P1")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn R")).right()          // continue north
        .then(at("to D1")).followToJunction(LineSide::LEFT, 1)
        .then(at("turn L")).left()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue north

        // ---- CORE 2: right side, top pick-up -> backtrack to D1 ----
        .then(at("to P2")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // head south to D1
        .then(at("to D1")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue north, skip P2
        .then(at("to P3")).followToJunction(LineSide::RIGHT, 2)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())

        // ---- CORE 3: top side, right pick-up -> drop at D2 (forward) ----
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn R")).right()          // continue west
        .then(at("to D2")).followToJunction(LineSide::LEFT, 1)
        .then(at("turn L")).left()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue west

        // ---- CORE 4: top side, left pick-up -> backtrack to D2 ----
        .then(at("to P4")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // head east to D2
        .then(at("to D2")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue west, skip P4
        .then(at("to P5")).followToJunction(LineSide::RIGHT, 2)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())

        // ---- CORE 5: left side, top pick-up -> drop at D3 (forward) ----
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn R")).right()          // continue south
        .then(at("to D3")).followToJunction(LineSide::LEFT, 1)
        .then(at("turn L")).left()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue south

        // ---- CORE 6: left side, bottom pick-up -> backtrack to D3 ----
        .then(at("to P6")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to end")).followToEnd()
        .then(pick())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // head north to D3
        .then(at("to D3")).followToJunction(LineSide::RIGHT, 1)
        .then(at("turn R")).right()
        .then(at("to drop")).followToEnd()
        .then(place())
        .then(at("U-turn")).uTurn()
        .then(at("to main")).followToEnd()
        .then(at("turn L")).left()           // continue south to PARK

        // Park in the black PARK zone.
        .then(at("to park")).followToBox()
        .then(at("park")).park(400)
        .then(at("DONE"))
        .build();
}

class FirstAuto : public NRLOpMode {
public:
    void init() override {
        leftMotor.begin();
        rightMotor.begin();
        line.begin();
        arm.setRampRate(ARM_DEG_S);
        gripper.setRampRate(GRIP_DEG_S);
        arm.begin();
        gripper.begin();
    }

    void start() override {
        lineActions.clearFault();
        lineActions.resetCounts();
        lineActions.clearRoute();
        stage = "ready";
        runAction(skillRun());
    }

    void loop() override {
        const LineJunction& j = lineActions.lastJunction();
        char lrs[4] = { j.left ? 'L' : '-', j.right ? 'R' : '-', j.straight ? 'S' : '-', 0 };
        telemetry.addData("stage", stage);
        telemetry.addData("fault", lineActions.faultName());
        telemetry.addData("jx",    (float)lineActions.junctionCount());
        telemetry.addData("cnr",   (float)lineActions.cornerCount());
        telemetry.addData("turn",  lineActions.lastTurnDegrees());
        telemetry.addData("hdg",   lineActions.gridHeading());
        telemetry.addData("last",  lrs);
        telemetry.addData("depth", (float)lineActions.routeDepth());
    }

    void stop() override {
        drive.stop();
    }
};

REGISTER_OPMODE(FirstAuto, "First Auto", AUTO);