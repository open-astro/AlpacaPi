# AlpacaPi Logging and User Agent Recognition System

## Overview

AlpacaPi implements a comprehensive logging and user agent recognition system that:
- Recognizes official ASCOM Alpaca clients using hardcoded string matching
- Logs unknown clients for self-learning
- Maintains statistics on client usage

## Folder Structure

```
AlpacaPi/
└── logs/                      # Runtime log files
    ├── README.md             # This file - complete logging documentation
    ├── new_clients-YYYY-MM-DD-PORT.log    # Unknown user agents (self-learning)
    ├── requestlog-YYYY-MM-DD-PORT.txt     # HTTP request log
    └── [other log files]
```

## User Agent Recognition System

### Recognition Method

The system uses hardcoded string matching to recognize user agents:
1. **Hardcoded List** - Direct string comparison against known clients
2. **Unknown** - Logged to `logs/new_clients-YYYY-MM-DD-PORT.log` for review

### Recognized Client Types

All client recognition is performed using hardcoded string comparisons in `ParseHTMLdataIntoReqStruct()`:

- `AlpacaPi` → `kHTTPclient_AlpacaPi`
- `ConformU` / `Conform` → `kHTTPclient_ConfomU`
- `RestSharp` → `kHTTPclient_ASCOM_RestSharp`
- `curl` → `kHTTPclient_Curl`
- `Mozilla` → `kHTTPclient_Mozilla`
- `NINA` → `kHTTPclient_NINA`
- `ASCOMAlpacaClient` → `kHTTPclient_ASCOM_AlpacaClient`
- `SkySafari` → `kHTTPclient_SkySafari`

### Adding New Clients

To add a new client type, you must modify the source code and recompile:

1. **Add enum value** to `src/RequestData.h`:
```cpp
typedef enum
{
	kHTTPclient_NotSpecified		=	0,
	kHTTPclient_AlpacaPi,
	// ... existing values ...
	kHTTPclient_YourNewClient,      // <-- Add here
	kHTTPclient_NotRecognized,
	kHTTPclient_last
} TYPE_Client;
```

2. **Add name** to `gUserAgentNames[]` in `src/alpacadriver.cpp`:
```cpp
const char	*gUserAgentNames[]	=
{
	"NotSpecified",
	"AlpacaPi",
	// ... existing names ...
	"YourNewClient",                // <-- Add here (same order as enum)
	"NotRecognized"
};
```

3. **Add string comparison** in `ParseHTMLdataIntoReqStruct()` function in `src/alpacadriver.cpp`:
```cpp
else if (strncasecmp(reqData->httpUserAgent, "YourNewClient", 13) == 0)
{
	reqData->cHTTPclientType	=   kHTTPclient_YourNewClient;
}
```

4. Recompile and restart AlpacaPi.

## Logging System

### Log Files

#### `new_clients-YYYY-MM-DD-PORT.log`

**Purpose**: Self-learning log for unknown user agents

**Format**:
```
YYYY/MM/DD HH:MM:SS    ClientIP              User-Agent-String
```

**Example**:
```
2025/01/15 14:23:45    192.168.1.100        SomeNewClient/1.0.0
2025/01/15 14:24:12    192.168.1.100        SomeNewClient/1.0.0
```

**Usage**:
- Review this log periodically to identify new clients
- Add frequently-seen clients to the hardcoded detection list
- Helps track which clients are connecting to your server

**Location**: Created automatically in `logs/` folder
**Rotation**: New file created daily (based on date in filename)

#### `requestlog-YYYY-MM-DD-PORT.txt`

**Purpose**: HTTP request log (existing functionality)

**Format**:
```
YYYY/MM/DD HH:MM:SS    ClientIP              User-Agent    GET/PUT    Request-URL    [PUT-data]
```

**Example**:
```
2025/01/15 14:23:45    192.168.1.100        ASCOMAlpacaClient/2.1.0.0    GET /api/v1/camera/0/connected
2025/01/15 14:23:46    192.168.1.100        ASCOMAlpacaClient/2.1.0.0    PUT /api/v1/camera/0/connected    Connected=true
```

### Console Logging

**Verbose Mode** (`-v` flag):
- Logs unrecognized user agents to console
- Useful for debugging and troubleshooting

**Normal Mode** (default):
- Unrecognized user agents are **NOT** logged to console
- Reduces log noise
- Unknown clients still logged to `logs/new_clients-YYYY-MM-DD-PORT.log`

### Statistics

All user agents (recognized and unrecognized) are tracked in statistics:

- **Web Interface**: Visit `/stats` to see request counts by user agent
- **Counters**: Stored in `gUserAgentCounters[]` array
- **Reset**: Counters reset on server restart

## Implementation Details

### Code Flow

1. **Startup** (`main()` function):
   - Initializes `gUserAgentCounters[]` array to zero
   - No configuration files are loaded

2. **Request Processing** (`ParseHTMLdataIntoReqStruct()` function):
   - Extracts User-Agent header from HTTP request
   - Performs hardcoded string comparisons using `strncasecmp()`
   - If recognized, sets `reqData->cHTTPclientType` to appropriate enum value
   - If unrecognized, sets to `kHTTPclient_NotRecognized` and calls `LogUnknownUserAgent()`
   - Increments appropriate counter in `gUserAgentCounters[]`

3. **Logging** (`LogUnknownUserAgent()` function):
   - Opens/creates `logs/new_clients-YYYY-MM-DD-PORT.log`
   - Writes timestamp, client IP, and user agent string
   - Implements deduplication to avoid logging the same user agent repeatedly
   - Flushes immediately for reliability

### Key Functions

#### `ParseHTMLdataIntoReqStruct()`
- **Location**: `src/alpacadriver.cpp`
- **Purpose**: Parse HTTP request and identify user agent
- **User Agent Detection**: Performs hardcoded string comparisons
- **Called**: For every HTTP request

#### `LogUnknownUserAgent()`
- **Location**: `src/alpacadriver.cpp`
- **Purpose**: Log unknown user agents to `logs/new_clients-YYYY-MM-DD-PORT.log`
- **Parameters**: User agent string, client IP address
- **Called**: When user agent is not recognized in hardcoded list

## Troubleshooting

### Issue: User agent not being recognized

**Check**:
1. Does the enum value exist in `RequestData.h`?
2. Is the name in `gUserAgentNames[]`?
3. Is the string comparison added in `ParseHTMLdataIntoReqStruct()`?
4. Check `logs/new_clients-YYYY-MM-DD-PORT.log` - is it being logged there?

**Solution**:
- Follow "Adding New Clients" steps above
- Verify the string comparison matches the user agent prefix correctly
- Check console for errors (verbose mode)

### Issue: Too many unknown clients in log

**Solution**:
- Review `logs/new_clients-YYYY-MM-DD-PORT.log` periodically
- Add frequently-seen clients to the hardcoded detection list
- Consider case sensitivity - user agent matching is case-insensitive

### Issue: Client recognized but wrong type

**Check**:
1. Verify the string comparison order in `ParseHTMLdataIntoReqStruct()`
2. More specific matches should come before less specific ones
3. Check that enum value matches the intended client type

**Solution**:
- Reorder string comparisons if needed
- Verify enum value assignment

## Best Practices

1. **Regular Review**: Check `logs/new_clients-YYYY-MM-DD-PORT.log` weekly to identify new clients
2. **String Matching**: Use appropriate prefix length in `strncasecmp()` to avoid false matches
3. **Enum Management**: Keep enum values in sync with `gUserAgentNames[]` array
4. **Documentation**: Document any custom clients you add
5. **Log Rotation**: Consider implementing log rotation for long-running servers

## Future Enhancements

Potential improvements:
- Automatic log rotation
- Web interface to view/add user agents
- Statistics on unknown client frequency
- Auto-suggest adding clients that appear frequently
- Configuration file support for easier client addition

## Related Files

- `src/alpacadriver.cpp` - Main implementation (user agent detection and logging)
- `src/RequestData.h` - Enum definitions (`TYPE_Client`)
