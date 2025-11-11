# Alpaca API Compliance Analysis

This document analyzes which features are **required by the Alpaca API** versus vendor SDK extras, and tracks implementation status.

## Reference Links
- [Alpaca Camera API](https://ascom-standards.org/api/#/Camera%20Specific%20Methods)
- [Alpaca Filter Wheel API](https://ascom-standards.org/api/#/FilterWheel%20Specific%20Methods)
- [Alpaca Focuser API](https://ascom-standards.org/api/#/Focuser%20Specific%20Methods)

---

## Camera Features

### ✅ REQUIRED by Alpaca API - **ALL IMPLEMENTED**

#### 1. **Pulse Guiding** - ✅ **IMPLEMENTED**
- **Alpaca API Method:** `PUT /api/v1/camera/{device_number}/pulseguide`
- **Status:** ✅ Fully implemented using ZWO SDK functions
- **Implementation:** Uses `ASIPulseGuideOn()` / `ASIPulseGuideOff()` from ZWO SDK
- **Location:** `drivers/ZWO/Camera/cameradriver_ASI.cpp`
- **Notes:** Properly handles direction conversion and duration tracking

### ❌ NOT Required by Alpaca API (Vendor SDK Extras)

These features are **NOT** part of the Alpaca API standard and are vendor-specific enhancements:

1. **GPS Support** - Vendor-specific feature (not in Alpaca API)
2. **Dark Frame Subtraction** - Vendor-specific optimization (not in Alpaca API)
3. **Trigger Output Configuration** - Vendor-specific hardware feature (not in Alpaca API)
4. **Soft Trigger** - Vendor-specific feature (not in Alpaca API)
5. **Dropped Frames Detection** - Diagnostic tool (not in Alpaca API)
6. **Gain/Offset Optimization** - Vendor-specific helper (not in Alpaca API)
7. **Start Position Control** - Vendor-specific ROI control (not in Alpaca API)
8. **Camera ID Management** - Vendor-specific device management (not in Alpaca API)
9. **Camera Check Utilities** - Vendor-specific enumeration (not in Alpaca API)
10. **New Control Types** (Fan, LED, USB Hub) - Vendor-specific hardware controls (not in Alpaca API)

---

## Filter Wheel Features

### ✅ REQUIRED by Alpaca API - **ALL IMPLEMENTED**

All required Filter Wheel methods are **fully implemented**:
- ✅ `position` - GET/PUT current filter position
- ✅ `names` - GET filter names
- ✅ `focusoffsets` - GET focus offsets

### ❌ NOT Required by Alpaca API (Vendor SDK Extras)

These features are **NOT** part of the Alpaca API standard:

1. **Filter Wheel Calibration** - Vendor-specific maintenance (not in Alpaca API)
2. **Direction Control** - Vendor-specific optimization (not in Alpaca API)
3. **Firmware Version Detection** - Diagnostic info (not in Alpaca API)
4. **Hardware Error Code Detection** - Diagnostic tool (not in Alpaca API)
5. **Serial Number Reading** - Device info (not in Alpaca API)
6. **Filter Wheel ID Management** - Vendor-specific device management (not in Alpaca API)
7. **Product ID Enumeration** - Vendor-specific enumeration (not in Alpaca API)

**Note:** Hardware error detection could be useful for troubleshooting but is not required by the API.

---

## Focuser Features

### ✅ REQUIRED by Alpaca API - **ALL IMPLEMENTED**

#### 1. **Halt** - ✅ **IMPLEMENTED**
- **Alpaca API Method:** `PUT /api/v1/focuser/{device_number}/halt`
- **Status:** ✅ Fully implemented using ZWO SDK function
- **Implementation:** Uses `EAFStop()` from ZWO SDK
- **Location:** `drivers/ZWO/Focuser/focuserdriver_ZWO.cpp`
- **Notes:** Properly handles connection state and error reporting

#### 2. **MaxStep** - ✅ **IMPLEMENTED**
- **Alpaca API Method:** `GET /api/v1/focuser/{device_number}/maxstep`
- **Status:** ✅ Fully implemented using ZWO SDK functions
- **Implementation:** Uses `EAFGetMaxStep()` / `EAFSetMaxStep()` from ZWO SDK
- **Location:** `drivers/ZWO/Focuser/focuserdriver_ZWO.cpp`
- **Notes:** Supports both GET and PUT operations

### ⚠️ RECOMMENDED (Not Required but Improves Functionality) - ✅ **IMPLEMENTED**

These features are **NOT** in the Alpaca API but improve functionality:

1. **Backlash Compensation** - ✅ **IMPLEMENTED**
   - **Status:** ✅ Fully implemented using ZWO SDK functions
   - **Implementation:** Uses `EAFGetBacklash()` / `EAFSetBacklash()` from ZWO SDK
   - **Location:** `drivers/ZWO/Focuser/focuserdriver_ZWO.cpp`
   - **API Endpoints:** `GET/PUT /api/v1/focuser/{device_number}/backlash` (vendor extension)
   - **Notes:** Improves focusing accuracy by compensating for mechanical backlash

2. **Reverse Direction** - ✅ **IMPLEMENTED**
   - **Status:** ✅ Fully implemented using ZWO SDK functions
   - **Implementation:** Uses `EAFGetReverse()` / `EAFSetReverse()` from ZWO SDK
   - **Location:** `drivers/ZWO/Focuser/focuserdriver_ZWO.cpp`
   - **API Endpoints:** `GET/PUT /api/v1/focuser/{device_number}/reverse` (vendor extension)
   - **Notes:** Useful for different setups where focuser direction needs to be reversed

### ❌ NOT Required by Alpaca API (Vendor SDK Extras)

These features are **NOT** part of the Alpaca API standard:

1. **Focuser Close Function** - Resource management (not in Alpaca API)
2. **Beep Control** - Vendor-specific hardware feature (not in Alpaca API)
3. **Step Range Query** - Vendor-specific helper (not in Alpaca API)
4. **Reset Position** - Vendor-specific calibration (not in Alpaca API)
5. **Firmware Version Detection** - Diagnostic info (not in Alpaca API)
6. **Serial Number Reading** - Device info (not in Alpaca API)
7. **Focuser ID Management** - Vendor-specific device management (not in Alpaca API)
8. **Product ID Enumeration** - Vendor-specific enumeration (not in Alpaca API)

---

## Summary

### ✅ COMPLETED - All Critical Features Implemented

1. **Camera Pulse Guiding** (`PUT /api/v1/camera/{device_number}/pulseguide`) ✅
   - ✅ Implemented using `ASIPulseGuideOn()` / `ASIPulseGuideOff()` from ZWO SDK
   - ✅ Proper direction conversion and duration tracking

2. **Focuser Halt** (`PUT /api/v1/focuser/{device_number}/halt`) ✅
   - ✅ Implemented using `EAFStop()` from ZWO SDK
   - ✅ Proper error handling and connection state management

3. **Focuser MaxStep** (`GET /api/v1/focuser/{device_number}/maxstep`) ✅
   - ✅ Implemented using `EAFGetMaxStep()` / `EAFSetMaxStep()` from ZWO SDK
   - ✅ Supports both GET and PUT operations

4. **Focuser Backlash** (`GET/PUT /api/v1/focuser/{device_number}/backlash`) ✅
   - ✅ Implemented using `EAFGetBacklash()` / `EAFSetBacklash()` from ZWO SDK
   - ✅ Vendor extension - improves focusing accuracy

5. **Focuser Reverse** (`GET/PUT /api/v1/focuser/{device_number}/reverse`) ✅
   - ✅ Implemented using `EAFGetReverse()` / `EAFSetReverse()` from ZWO SDK
   - ✅ Vendor extension - useful for different setups

### 🟢 OPTIONAL - Vendor SDK Extras (Not Required)

- All other vendor-specific features are **nice-to-have** but **NOT required** for Alpaca API compliance
- Implementing them would improve functionality but won't affect API compliance

---

## Conclusion

**All required Alpaca API features have been implemented** ✅

The following features are now fully compliant with the Alpaca API standard:
- ✅ Camera Pulse Guiding (using SDK functions)
- ✅ Focuser Halt (using SDK function)
- ✅ Focuser MaxStep (using SDK functions)

Additionally, the following recommended vendor extensions have been implemented:
- ✅ Focuser Backlash Compensation (improves focusing accuracy)
- ✅ Focuser Reverse Direction (useful for different setups)

All other vendor SDK features are optional enhancements that improve functionality but are **not required** by the Alpaca API standard.

**Last Updated:** 2024-12-19

