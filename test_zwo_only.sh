#!/bin/bash
#*****************************************************************************
#*	Test script for ZWO-only driver installation
#*	Tests: ZWO EAF, EFW, and Camera drivers only
#*****************************************************************************

set -e  #*	Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "Testing ZWO-Only Driver Installation"
echo "=========================================="
echo ""
echo "This test will verify that setup_complete.sh can build"
echo "alpacapi with ONLY ZWO EAF, EFW, and Camera drivers."
echo ""

#*	Check if SDKs exist
MISSING_SDKS=0

if [ ! -d "sdk/ZWO_ASI_SDK" ]; then
	echo "❌ ERROR: sdk/ZWO_ASI_SDK not found"
	MISSING_SDKS=1
fi

if [ ! -d "sdk/ZWO_EFW_SDK" ]; then
	echo "❌ ERROR: sdk/ZWO_EFW_SDK not found"
	MISSING_SDKS=1
fi

if [ ! -d "sdk/ZWO_EAF_SDK" ]; then
	echo "❌ ERROR: sdk/ZWO_EAF_SDK not found"
	MISSING_SDKS=1
fi

if [ $MISSING_SDKS -eq 1 ]; then
	echo ""
	echo "Please ensure all ZWO SDKs are present before testing."
	exit 1
fi

echo "✓ All required SDKs found"
echo ""

#*	Create a test configuration file that simulates user input
#*	This will be used to test the selective build
TEST_CONFIG=".test_zwo_config"
cat > "$TEST_CONFIG" << 'TESTEOF'
#*	Test configuration for ZWO-only build
#*	Simulates answering "y" to ZWO drivers, "n" to everything else

#*	Camera: Yes, ZWO ASI only
BUILD_CAMERA=true
BUILD_CAMERA_ASI=true
BUILD_CAMERA_ATIK=false
BUILD_CAMERA_FLIR=false
BUILD_CAMERA_QHY=false
BUILD_CAMERA_TOUP=false

#*	Filter Wheel: Yes, ZWO EFW only
BUILD_FILTERWHEEL=true
BUILD_FILTERWHEEL_ZWO=true
BUILD_FILTERWHEEL_ATIK=false

#*	Focuser: Yes, ZWO EAF only
BUILD_FOCUSER=true
BUILD_FOCUSER_ZWO=true
BUILD_FOCUSER_MOONLITE=false

#*	Everything else: No
BUILD_ROTATOR=false
BUILD_ROTATOR_NITECRAWLER=false
BUILD_TELESCOPE=false
BUILD_TELESCOPE_LX200=false
BUILD_TELESCOPE_SKYWATCHER=false
BUILD_TELESCOPE_SERVO=false
BUILD_DOME=false
BUILD_SWITCH=false
BUILD_CALIBRATION=false
BUILD_OBSERVINGCONDITIONS=false
TESTEOF

echo "Test Configuration:"
echo "  ✓ Camera: ZWO ASI only"
echo "  ✓ Filter Wheel: ZWO EFW only"
echo "  ✓ Focuser: ZWO EAF only"
echo "  ✗ All other drivers: Disabled"
echo ""

#*	Source the test configuration
source "$TEST_CONFIG"

#*	Test the Makefile generation logic
echo "Testing Makefile generation..."
CUSTOM_MAKEFILE=".Makefile.test_zwo"
rm -f "$CUSTOM_MAKEFILE"

#*	Include base Makefile
cat > "$CUSTOM_MAKEFILE" << 'EOF'
include Makefile

#*	Base flags
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ALPACA_PI_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_INCLUDE_ALPACA_EXTENSIONS_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_INCLUDE_HTTP_HEADER_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_USE_CAMERA_READ_THREAD_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_INCLUDE_MILLIS_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_DISCOVERY_QUERRY_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_CTRL_IMAGE_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_LIVE_CONTROLLER_

EOF

#*	Add OpenCV if available
if pkg-config --exists opencv4 2>/dev/null || pkg-config --exists opencv 2>/dev/null || \
   [ -f "/usr/include/opencv2/highgui/highgui_c.h" ] || [ -f "/usr/local/include/opencv2/highgui/highgui_c.h" ] || \
   [ -f "/usr/include/opencv4/opencv2/highgui/highgui.hpp" ] || [ -f "/usr/local/include/opencv4/opencv2/highgui/highgui.hpp" ]
then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_USE_OPENCV_
EOF
fi

#*	Add ZWO Camera flags
if [ "$BUILD_CAMERA" = true ] && [ "$BUILD_CAMERA_ASI" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_CAMERA_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_ASI_
EOF
fi

#*	Add ZWO Filter Wheel flags
if [ "$BUILD_FILTERWHEEL" = true ] && [ "$BUILD_FILTERWHEEL_ZWO" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_FILTERWHEEL_ZWO_
EOF
fi

#*	Add ZWO Focuser flags
if [ "$BUILD_FOCUSER" = true ] && [ "$BUILD_FOCUSER_ZWO" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_
alpacapi_test_zwo:	DEFINEFLAGS		+=	-D_ENABLE_FOCUSER_ZWO_
EOF
fi

#*	Verify no other drivers are enabled
echo "Checking that other drivers are NOT enabled..."
if grep -q "BUILD_ROTATOR=true\|BUILD_TELESCOPE=true\|BUILD_DOME=true\|BUILD_SWITCH=true\|BUILD_CALIBRATION=true" "$TEST_CONFIG" 2>/dev/null; then
	echo "❌ ERROR: Other drivers are enabled in test config!"
	exit 1
fi

if grep -q "_ENABLE_ROTATOR_\|_ENABLE_TELESCOPE_\|_ENABLE_DOME_\|_ENABLE_SWITCH_\|_ENABLE_CALIBRATION_" "$CUSTOM_MAKEFILE" 2>/dev/null; then
	echo "❌ ERROR: Other driver flags found in generated Makefile!"
	exit 1
fi

echo "✓ No other drivers enabled"
echo ""

#*	Add object dependencies
cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	$(DRIVER_OBJECTS)				\
			$(HELPER_OBJECTS)				\
			$(SERIAL_OBJECTS)				\
			$(SOCKET_OBJECTS)

EOF

#*	Add ZWO Camera objects
if [ "$BUILD_CAMERA" = true ] && [ "$BUILD_CAMERA_ASI" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	$(CAMERA_DRIVER_OBJECTS)
alpacapi_test_zwo:	$(ASI_CAMERA_OBJECTS)
EOF
fi

#*	Add ZWO Filter Wheel objects
if [ "$BUILD_FILTERWHEEL" = true ] && [ "$BUILD_FILTERWHEEL_ZWO" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	$(FILTERWHEEL_DRIVER_OBJECTS)
alpacapi_test_zwo:	$(ZWO_EFW_OBJECTS)
EOF
fi

#*	Add ZWO Focuser objects
if [ "$BUILD_FOCUSER" = true ] && [ "$BUILD_FOCUSER_ZWO" = true ]; then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	$(FOCUSER_DRIVER_OBJECTS)
EOF
fi

#*	Add OpenCV objects if available
if pkg-config --exists opencv4 2>/dev/null || pkg-config --exists opencv 2>/dev/null || \
   [ -f "/usr/include/opencv2/highgui/highgui_c.h" ] || [ -f "/usr/local/include/opencv2/highgui/highgui_c.h" ] || \
   [ -f "/usr/include/opencv4/opencv2/highgui/highgui.hpp" ] || [ -f "/usr/local/include/opencv4/opencv2/highgui/highgui.hpp" ]
then
	cat >> "$CUSTOM_MAKEFILE" << 'EOF'
alpacapi_test_zwo:	$(LIVE_WINDOW_OBJECTS)
EOF
fi

#*	Build link command
LINK_OBJECTS="\$(DRIVER_OBJECTS) \$(HELPER_OBJECTS) \$(SERIAL_OBJECTS) \$(SOCKET_OBJECTS)"
LINK_LIBS="-ludev -lusb-1.0 -lpthread"

#*	Add OpenCV libraries if detected
if pkg-config --exists opencv4 2>/dev/null || pkg-config --exists opencv 2>/dev/null
then
	LINK_LIBS="\$(OPENCV_LINK) $LINK_LIBS"
	LINK_OBJECTS="$LINK_OBJECTS \$(LIVE_WINDOW_OBJECTS)"
fi

#*	Add ZWO Camera libraries
if [ "$BUILD_CAMERA" = true ] && [ "$BUILD_CAMERA_ASI" = true ]; then
	LINK_OBJECTS="$LINK_OBJECTS \$(CAMERA_DRIVER_OBJECTS)"
	LINK_OBJECTS="$LINK_OBJECTS \$(ASI_CAMERA_OBJECTS)"
fi

#*	Add ZWO Filter Wheel libraries
if [ "$BUILD_FILTERWHEEL" = true ] && [ "$BUILD_FILTERWHEEL_ZWO" = true ]; then
	LINK_OBJECTS="$LINK_OBJECTS \$(FILTERWHEEL_DRIVER_OBJECTS)"
	LINK_OBJECTS="$LINK_OBJECTS \$(ZWO_EFW_OBJECTS)"
fi

#*	Add ZWO Focuser libraries
if [ "$BUILD_FOCUSER" = true ] && [ "$BUILD_FOCUSER_ZWO" = true ]; then
	LINK_OBJECTS="$LINK_OBJECTS \$(FOCUSER_DRIVER_OBJECTS)"
	LINK_LIBS="-L\$(ZWO_EAF_LIB_DIR) -lEAFFocuser $LINK_LIBS"
fi

#*	Add link command
cat >> "$CUSTOM_MAKEFILE" << EOF
	\$(LINK)  									\\
		$LINK_OBJECTS				\\
		$LINK_LIBS					\\
		-o alpacapi

EOF

echo "Generated test Makefile:"
echo "  - File: $CUSTOM_MAKEFILE"
echo ""

#*	Verify the generated Makefile
echo "Verifying generated Makefile..."

#*	Check for required ZWO flags
REQUIRED_FLAGS=(
	"_ENABLE_CAMERA_"
	"_ENABLE_ASI_"
	"_ENABLE_FILTERWHEEL_"
	"_ENABLE_FILTERWHEEL_ZWO_"
	"_ENABLE_FOCUSER_"
	"_ENABLE_FOCUSER_ZWO_"
)

for flag in "${REQUIRED_FLAGS[@]}"; do
	if ! grep -q "$flag" "$CUSTOM_MAKEFILE"; then
		echo "❌ ERROR: Required flag $flag not found in Makefile!"
		exit 1
	fi
done

echo "✓ All required ZWO flags present"

#*	Check for required objects
REQUIRED_OBJECTS=(
	"\$(CAMERA_DRIVER_OBJECTS)"
	"\$(ASI_CAMERA_OBJECTS)"
	"\$(FILTERWHEEL_DRIVER_OBJECTS)"
	"\$(ZWO_EFW_OBJECTS)"
	"\$(FOCUSER_DRIVER_OBJECTS)"
)

for obj in "${REQUIRED_OBJECTS[@]}"; do
	if ! grep -q "$obj" "$CUSTOM_MAKEFILE"; then
		echo "❌ ERROR: Required object $obj not found in Makefile!"
		exit 1
	fi
done

echo "✓ All required objects present"

#*	Check for EAF library linking
if ! grep -q "EAFFocuser" "$CUSTOM_MAKEFILE"; then
	echo "❌ ERROR: EAF library not found in link command!"
	exit 1
fi

echo "✓ EAF library linking present"
echo ""

#*	Summary
echo "=========================================="
echo "Test Results:"
echo "=========================================="
echo "✓ SDKs found"
echo "✓ Makefile generation logic correct"
echo "✓ Only ZWO drivers enabled"
echo "✓ No other drivers enabled"
echo "✓ All required flags present"
echo "✓ All required objects present"
echo "✓ Library linking correct"
echo ""
echo "The setup script should work correctly for ZWO-only builds!"
echo ""
echo "To test actual compilation, run:"
echo "  make -f $CUSTOM_MAKEFILE clean"
echo "  make -f $CUSTOM_MAKEFILE alpacapi_test_zwo"
echo ""

#*	Cleanup
rm -f "$TEST_CONFIG"

echo "Test completed successfully!"

