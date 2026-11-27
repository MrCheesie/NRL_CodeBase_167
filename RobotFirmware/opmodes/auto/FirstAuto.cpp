#include "NRL.h"
#include <cmath>
#include <memory>
#include <cstdint>

// ---------- Drive and line sensor ----------
static HexaDCMotor leftMotor  {{ .dirPin = MOTOR_L_DIR, .pwmPin = MOTOR_L_PWM,
                                 .flipped = true, .minDrive = 29 }};
static HexaDCMotor rightMotor {{ .dirPin = MOTOR_R_DIR, .pwmPin = MOTOR_R_PWM,
                                 .minDrive = 32 }};
static TankDrive       drive(leftMotor, rightMotor);
static HexaLineArray   line{ hexaLineConfig() };
static LineActions     lineActions(drive, line);

// ---------- Arm and gripper (tune angles on SERVO TEST) ----------
constexpr float ARM_DOWN   = 115.0f;
constexpr float ARM_UP     =  45.0f;
constexpr float GRIP_OPEN  = 125.0f;
constexpr float GRIP_CLOSE = 155.0f;
constexpr float ARM_DEG_S  = 100.0f;
constexpr float GRIP_DEG_S = 200.0f;

static HexaServo arm    {{ .signalPin = SERVO_1, .startAngle = ARM_UP     }};
static HexaServo gripper{{ .signalPin = SERVO_2, .startAngle = GRIP_CLOSE }};

// ---------- Route memory (must be static: read later during the run) ----------
static int markP3 = -1;   // Gold Pick-Up junction
static int markD2 = -1;   // Drop terminal junction

// ---------- Mission state (shown on screen) ----------
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

// Gripper opens, arm goes down over the CORE, gripper closes, arm lifts.
static ActionPtr pick() {
    return sequential({
        at("PICK"),
        moveServo(gripper, GRIP_OPEN,  200),
        moveServo(arm,     ARM_DOWN,   300),
        moveServo(gripper, GRIP_CLOSE, 300),
        moveServo(arm,     ARM_UP,     200),
    });
}

// Arm goes down with the CORE, gripper opens so it comes to rest in the DROP ZONE, arm lifts.
static ActionPtr place() {
    return sequential({
        at("PLACE"),
        moveServo(arm,     ARM_DOWN,   300),
        moveServo(gripper, GRIP_OPEN,  300),
        moveServo(arm,     ARM_UP,     200),
    });
}

// ---------- Autonomous mission: Gold CORE (P3) -> DROP ZONE via D2 -> PARK ----------
// Assumes the robot starts square to the line in the START box, facing the line that leaves it.
// Junction counts are from the robot's view while travelling:
//   Out, heading north up the right side: P1 (R), P2 (R)  [D1 is LEFT, not counted]
//   Along the top heading west: P3 (R), D2 (L), P4 (R)
//   Down the left side heading south: P5 (R), D3 (L), P6 (R)
static ActionPtr skillRun() {
    return lineActions.auto_()
        .then(at("leave START")).leaveBox()
        .then(at("to P3")).followToJunction(LineSide::RIGHT, 3)   // past P1, P2; stop at P3
        .markHere(&markP3)
        .right()                                                   // onto the P3 terminal
        .then(at("to P3 end")).followToEnd()                       // stop at terminal end bar
        .then(pick())                                              // secure the Gold CORE
        .uTurn()
        .retraceTo(&markP3)                                        // back to P3 junction
        .right()                                                   // heading back south, west is right
        .then(instant([] { lineActions.resetCounts(); }))          // D2 is the 1st left junction under either counting rule
        .then(at("to D2")).followToJunction(LineSide::LEFT, 1)     // stop at D2
        .markHere(&markD2)
        .left()                                                    // onto D2 terminal
        .then(at("to D2 end")).followToEnd()                       // stop at D2 end bar
        .then(instant([] { drive.drive(0.25f, 0.0f); }))           // short nudge so CORE lands in DROP ZONE
        .sleepMs(250)
        .then(instant([] { drive.stop(); }))
        .then(place())                                             // release Gold into DROP ZONE
        .uTurn()
        .retraceTo(&markD2)                                        // back to D2 junction
        .left()                                                    // face west along the top
        .then(at("to PARK")).followToBox()                         // around the loop to the PARK box
        .then(at("park")).park(300)                                // drive in, stop, stay still
        .then(at("DONE"))
        .build();
}

class FirstAuto : public NRLOpMode {
public:
    void init() override {
        leftMotor.begin();
        rightMotor.begin();
        line.begin();                       // loads saved calibration
        arm.setRampRate(ARM_DEG_S);         // before begin(): ramp from the start
        gripper.setRampRate(GRIP_DEG_S);
        arm.begin();
        gripper.begin();
    }

    void start() override {
        lineActions.clearFault();           // fresh run: forget old faults,
        lineActions.resetCounts();          // counts,
        lineActions.clearRoute();           // and route memory (grid 0 = facing now)
        markP3 = -1;
        markD2 = -1;
        stage = "ready";
        runAction(skillRun());
    }

    void loop() override {
        const LineJunction& j = lineActions.lastJunction();
        char lrs[4] = { j.left ? 'L' : '-', j.right ? 'R' : '-', j.straight ? 'S' : '-', 0 };
        telemetry.addData("stage", stage);
        telemetry.addData("fault", lineActions.faultName());
        telemetry.addData("jx", (float)lineActions.junctionCount());
        telemetry.addData("cnr", (float)lineActions.cornerCount());
        telemetry.addData("turn", lineActions.lastTurnDegrees());
        telemetry.addData("hdg", lineActions.gridHeading());
        telemetry.addData("last", lrs);
        telemetry.addData("depth", (float)lineActions.routeDepth());
    }

    void stop() override {
        cancelActions();
        drive.stop();
    }
};

REGISTER_OPMODE(FirstAuto, "Auto Mk-I", AUTO);
