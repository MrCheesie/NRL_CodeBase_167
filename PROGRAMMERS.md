# PROGRAMMERS.md

Context from programmers, for programmers.

## By Agents

- Created `RobotFirmware/opmodes/auto/FirstAuto.cpp`: autonomous opmode for the NRL 2026 Autonomous Skill Challenge. Uses the line-following SDK (`HexaLineArray`, `LineActions`) to leave the black START zone, collect all six COREs one at a time, drop each into the DROP ZONE (backtracking to the previous Drop terminal where the next Pick-Up occurs first), and Park in the black PARK zone. Servo angles are placeholders to be tuned on the robot.