# CAA Software Development Kit (SDK) Manual  
Version: v1.0  
Date: 2024-12-30  

This manual describes the CAA Software Development Kit (SDK), which provides functions to control CAA devices using C, C++, C#, and other development tools. It supports x86/x64 Windows, Linux, and macOS.

---

## 1. Introduction

The SDK contains a collection of APIs used to operate the CAA device.

### 1.1 Header Files and Libraries

**Header file:**  
- `CAA_API.h`

**Windows:**  
- Import/Dynamic libraries: `CAA_SRC.lib`, `CAA_SRC.dll`

**Linux:**  
- Dynamic/Static libraries: `libCAA.so`, `libCAA.a`

**macOS:**  
- Dynamic/Static libraries: `libCAA.dylib`, `libCAA.a`

### 1.2 Installation

**Windows:**  
- Extract the ZIP file to any directory.  
- Add the DLL folder path to your system environment variables.  
- You may also place the DLL in the same folder as the application executable.  
- Re-login may be required.

**Linux / macOS:**  
- Ensure the udev rules file exists: `/etc/udev/rules.d/caa.rules`.  
- If missing, copy `caa.rules` there and reboot.  
- Extract the `.tar.gz` file anywhere.  
- Add the `.so` or `.a` path to your project or system library paths.

---

## 2. API Reference

### 2.1 Enums and Structs

#### 2.1.1 `CAA_ERROR_CODE`
Enumeration describing CAA-related error codes including invalid index, invalid ID, device removed, moving state, timeout, and more.

#### 2.1.2 `CAA_ID`
Represents an 8‑byte device serial number or alias.

#### 2.1.3 `CAA_TYPE`
Represents the CAA device type (e.g., `"CAA-M54"`).

#### 2.1.4 `CAA_INFO`
Contains device ID, name, and maximum degrees.

---

### 2.2 Function Definitions

Below is a summary of all SDK functions.

#### 2.2.1 `CAAGetNum()`
Returns the number of connected CAA devices.

#### 2.2.2 `CAAGetProductIDs(int* pPIDs)`
Retrieves the CAA product IDs.

#### 2.2.3 `CAACheck(int VID, int PID)`
Checks if a device is a CAA.  
VID = `0x03C3`, PID obtained via `CAAGetProductIDs()`.

#### 2.2.4 `CAAGetID(int index, int* ID)`
Gets the ID of a connected device by index.

#### 2.2.5 `CAAOpen(int ID)`
Opens a CAA device.

#### 2.2.6 `CAAGetProperty(int ID, CAA_INFO* pInfo)`
Retrieves device properties.

#### 2.2.7 `CAAMove(int ID, float angle)`
Moves the device by a specified angle (0–360°).

#### 2.2.8 `CAAMoveTo(int ID, float angle)`
Moves the device to an absolute angle (0–360°).

#### 2.2.9 `CAAStop(int ID)`
Stops movement.

#### 2.2.10 `CAAIsMoving(int ID, bool* isMoving, bool* handControl)`
Checks if the device is currently moving.

#### 2.2.11 `CAAGetDegree(int ID, float* angle)`
Returns the current angle.

#### 2.2.12 `CAACurDegree(int ID, float angle)`
Sets the current angle.

#### 2.2.13 `CAAMinDegree(...)`
Deprecated.

#### 2.2.14 `CAASetMaxDegree(int ID, float angle)`
Sets maximum movement angle.

#### 2.2.15 `CAAGetMaxDegree(int ID, float* angle)`
Gets maximum movement angle.

#### 2.2.16 `CAAGetTemp(...)`
Deprecated.

#### 2.2.17 `CAASetBeep(int ID, bool enabled)`
Enables or disables the buzzer.

#### 2.2.18 `CAAGetBeep(int ID, bool* enabled)`
Returns buzzer state.

#### 2.2.19 `CAASetReverse(int ID, bool reversed)`
Sets rotation direction.

#### 2.2.20 `CAAGetReverse(int ID, bool* reversed)`
Gets rotation direction.

#### 2.2.21 `CAAClose(int ID)`
Closes the device connection.

#### 2.2.22 `CAAGetSDKVersion()`
Returns SDK version string.

#### 2.2.23 `CAAGetFirmwareVersion(int ID, ...)`
Gets device firmware version.

#### 2.2.24 `CAAGetSerialNumber(int ID, CAA_SN* sn)`
Gets device serial number.

#### 2.2.25 `CAASetID(int ID, CAA_ID alias)`
Sets a device alias.

#### 2.2.26 `CAAGetType(int ID, CAA_TYPE* type)`
Gets device type.

#### 2.2.27 `CAASendCMD(...)`
Deprecated.

#### 2.2.28 `CAASetSerialNumber(...)`
Deprecated.

---

### 2.3 Recommended Call Flow

1. Get number of devices: `CAAGetNum()`  
2. Get device ID: `CAAGetID()`  
3. Get device properties: `CAAGetProperty()`  
4. Open device: `CAAOpen()`  
5. Move device: `CAAMove()`  
6. Close device: `CAAClose()`

---

## 3. Demo Usage

Extract `caa_demo.zip`.  
See the included `README.md` for usage instructions.

---

© ZWO Optical Corp. All rights reserved.
