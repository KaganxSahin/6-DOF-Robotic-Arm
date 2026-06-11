# 6-DOF Robotic Arm

A 6-servo robotic arm controlled by an Arduino Uno and a PCA9685 PWM driver. Commands are sent from the Arduino IDE Serial Monitor over USB.

![Robot Arm](robot_arm.jpg)

---

## Hardware

### Arduino Uno R3
- Microcontroller: ATmega328P
- Operating voltage: 5V
- Digital I/O pins: 14 (6 PWM)
- Analog input pins: 6
- Flash memory: 32 KB
- Clock speed: 16 MHz
- Communication: UART, I2C (A4 = SDA, A5 = SCL), SPI
- Role: Receives commands from the Serial Monitor and forwards PWM instructions to the PCA9685 over I2C.

### PCA9685 16-Channel PWM/Servo Driver
- 16 channels, 12-bit (4096 steps) PWM resolution
- I2C interface (default address `0x40`)
- 24 Hz – 1526 Hz output frequency (50 Hz used in this project)
- Dedicated V+ terminal for external servo power
- Logic voltage: 3.3V / 5V compatible
- Role: Drives all 6 servos simultaneously with stable PWM signals.

### Servo Motors

| Slot | Channel | Model | Type | Torque | Role |
|------|---------|-------|------|--------|------|
| **S1** | 1 | MG996R | Continuous rotation | ~10 kg·cm @ 6V | Base |
| **S0** | 0 | DS Servo 60KG | Continuous rotation, digital | ~60 kg·cm @ 6.8V | Shoulder |
| **S2** | 2 | MG996R | Positional (slow stepped) | ~10 kg·cm @ 6V | Upper arm |
| **S3** | 3 | MG90S | Positional (4 fixed angles) | ~1.8 kg·cm @ 4.8V | Elbow |
| **S4** | 4 | MG90S | Positional (4 fixed, slow) | ~1.8 kg·cm @ 4.8V | Wrist |
| **S5** | 5 | MG90S | Positional (3 fixed) | ~1.8 kg·cm @ 4.8V | Gripper |

### Power Supply
- Adjustable bench DC supply set to **9V** and connected to the PCA9685 V+ terminal.
- Under load the displayed voltage dropped to about **7V**, which is the actual voltage the system pulls when the servos are active.
- The Arduino is powered by USB (5V) from the computer.
- **Common ground**: the PSU GND, PCA9685 GND and Arduino GND are all tied together on the breadboard. Common ground is required for the PWM signals to be valid.

---

## Wiring

### Arduino → PCA9685
| Arduino | PCA9685 |
|---------|---------|
| 5V      | VCC     |
| GND     | GND     |
| A4      | SDA     |
| A5      | SCL     |

### Power Supply → PCA9685
| PSU | PCA9685 (green terminal) |
|-----|--------------------------|
| +9V (≈7V under load) | V+ |
| GND | GND (shared with Arduino GND) |

### Servos → PCA9685
Each servo is plugged into its own 3-pin header (GND / V+ / PWM) on the PCA9685.

- S0 → Channel 0
- S1 → Channel 1
- S2 → Channel 2
- S3 → Channel 3
- S4 → Channel 4
- S5 → Channel 5

---

## Commands

All commands are typed into the Arduino IDE Serial Monitor at **115200 baud** (line ending: Newline).

### S1 – Base (MG996R, continuous)
| Key | Action |
|-----|--------|
| `1` | Turn left |
| `2` | Turn right |
| `3` | Stop |

### S0 – Shoulder (DS 60KG, continuous)
| Key | Action |
|-----|--------|
| `4` | Left |
| `5` | Right |
| `6` | Stop |

### S2 – Upper Arm (MG996R, slow positional)
| Key | Position | Pulse |
|-----|----------|-------|
| `7` | Pos 1 | 150 |
| `8` | Pos 2 (middle) | 375 |
| `9` | Pos 3 | 600 |

### S3 – Elbow (MG90S, 4 positions)
| Key | Pulse |
|-----|-------|
| `Q` | 100 |
| `W` | 267 |
| `E` | 433 |
| `R` | 600 |

### S4 – Wrist (MG90S, 4 positions, slow)
| Key | Pulse |
|-----|-------|
| `A` | 100 |
| `S` | 267 |
| `D` | 433 |
| `F` | 600 |

### S5 – Gripper (MG90S, 3 positions)
| Key | Pulse |
|-----|-------|
| `G` | 100 |
| `H` | 350 |
| `J` | 600 |

### Other
| Key | Action |
|-----|--------|
| `STOP` | Cut signals to all servos |

---

## Software

### Required Arduino libraries
- `Wire.h` (built-in)
- `Adafruit_PWMServoDriver.h` — install via *Library Manager → "Adafruit PWM Servo Driver Library"*

### Files
- `robot_kol/robot_kol.ino` — Arduino firmware (the only thing that needs to be uploaded)

### Usage
1. Wire everything as shown above and turn on the external power supply.
2. Open `robot_kol/robot_kol.ino` in the Arduino IDE.
3. Install the Adafruit PWM Servo Driver Library.
4. Upload the sketch to the Arduino Uno.
5. Open the Serial Monitor at **115200 baud** with line ending set to **Newline**.
6. Type a command (for example `2`, `Q`, or `STOP`) and press Enter.

---

## Notes

- Continuous-rotation servos (S0, S1) do not move to angles; they spin in a direction. A pulse near 350 means stop, smaller / larger values rotate in opposite directions.
- Positional servos (S2, S3, S4, S5) map a pulse value to a fixed angle.
- After a positional move (S3, S4, S5), the PWM signal is cut with `pwm.setPWM(channel, 0, 4096)` so the small MG90S servos do not overheat while holding position.
- S2 and S4 use a stepped soft-move function so that motion is smooth instead of instant.
