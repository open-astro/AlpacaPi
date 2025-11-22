# AlpacaPi Logging and User Agent Recognition System

## Overview

AlpacaPi implements a comprehensive logging and user agent recognition system that:
- Recognizes official ASCOM Alpaca clients
- Logs unknown clients for self-learning
- Provides configurable client recognition without recompilation
- Maintains statistics on client usage

## Folder Structure

```
AlpacaPi/
├── assets/                    # Configuration files (no recompilation needed)
│   ├── user_agents.json       # User agent to client type mappings
│   └── README.md             # Assets folder documentation
│
└── logs/                      # Runtime log files
    ├── README.md             # This file - complete logging documentation
    ├── new_clients-YYYY-MM-DD-PORT.log    # Unknown user agents (self-learning)
    ├── requestlog-YYYY-MM-DD-PORT.txt     # HTTP request log
    └── [other log files]
```

## User Agent Recognition System

### Recognition Priority

The system checks user agents in this order:

1. **JSON Configuration** (`assets/user_agents.json`) - Configurable without recompilation
2. **Hardcoded List** - Backwards compatible fallback
3. **Unknown** - Logged to `logs/new_clients.log` for review

### Recognized Client Types

#### JSON-Configurable Clients (in `assets/user_agents.json`)
- `ASCOMAlpacaClient` → `kHTTPclient_ASCOM_AlpacaClient`
- `NINA` → `kHTTPclient_NINA`
- `SkySafari` → `kHTTPclient_SkySafari`

#### Hardcoded Clients (for backwards compatibility)
- `AlpacaPi` → `kHTTPclient_AlpacaPi`
- `ConformU` / `Conform` → `kHTTPclient_ConfomU`
- `RestSharp` → `kHTTPclient_ASCOM_RestSharp`
- `curl` → `kHTTPclient_Curl`
- `Mozilla` → `kHTTPclient_Mozilla`

### Adding New Clients

#### Method 1: JSON Configuration (No Recompilation)

1. Edit `assets/user_agents.json`:
```json
{
	"ASCOMAlpacaClient": "kHTTPclient_ASCOM_AlpacaClient",
	"NINA": "kHTTPclient_NINA",
	"SkySafari": "kHTTPclient_SkySafari",
	"YourNewClient": "kHTTPclient_YourNewClient"
}
```

2. **Note**: The enum value must already exist in the code. For new enum values, see Method 2.

3. Restart AlpacaPi - the new client will be recognized immediately.

#### Method 2: Adding New Enum Value (Requires Recompilation)

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

3. **Add mapping** to `assets/user_agents.json`:
```json
{
	"YourNewClient": "kHTTPclient_YourNewClient"
}
```

4. **Update `LoadUserAgentMappings()`** in `src/alpacadriver.cpp` to handle the new enum:
```cpp
else if (strcmp(enumName, "kHTTPclient_YourNewClient") == 0)
{
	clientType	=	kHTTPclient_YourNewClient;
	foundMapping	=	true;
}
```

5. Recompile and restart AlpacaPi.

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
- Add frequently-seen clients to `assets/user_agents.json`
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
- Unknown clients still logged to `logs/new_clients.log`

### Statistics

All user agents (recognized and unrecognized) are tracked in statistics:

- **Web Interface**: Visit `/stats` to see request counts by user agent
- **Counters**: Stored in `gUserAgentCounters[]` array
- **Reset**: Counters reset on server restart

## Implementation Details

### Code Flow

1. **Startup** (`main()` function):
   - `LoadUserAgentMappings()` reads `assets/user_agents.json`
   - Parses JSON using `SJP_ParseData()` (simple JSON parser)
   - Populates `gUserAgentMappings[]` array

2. **Request Processing** (`ParseHTMLdataIntoReqStruct()` function):
   - Extracts User-Agent header from HTTP request
   - Calls `CheckUserAgentFromJSON()` to check JSON mappings first
   - Falls back to hardcoded string comparisons
   - If still unrecognized, calls `LogUnknownUserAgent()`

3. **Logging** (`LogUnknownUserAgent()` function):
   - Opens/creates `logs/new_clients-YYYY-MM-DD-PORT.log`
   - Writes timestamp, client IP, and user agent string
   - Flushes immediately for reliability

### Key Functions

#### `LoadUserAgentMappings()`
- **Location**: `src/alpacadriver.cpp`
- **Purpose**: Load user agent mappings from JSON file at startup
- **Returns**: Number of mappings loaded
- **Called**: Once during `main()` initialization

#### `CheckUserAgentFromJSON()`
- **Location**: `src/alpacadriver.cpp`
- **Purpose**: Check if user agent matches a JSON-loaded mapping
- **Returns**: `TYPE_Client` enum value or `kHTTPclient_NotRecognized`
- **Called**: For every HTTP request with a User-Agent header

#### `LogUnknownUserAgent()`
- **Location**: `src/alpacadriver.cpp`
- **Purpose**: Log unknown user agents to `logs/new_clients.log`
- **Parameters**: User agent string, client IP address
- **Called**: When user agent is not recognized in JSON or hardcoded list

### Data Structures

#### `TYPE_UserAgentMapping`
```cpp
typedef struct
{
	char	userAgentPrefix[64];
	TYPE_Client	clientType;
} TYPE_UserAgentMapping;
```

- **Storage**: `gUserAgentMappings[kMaxUserAgentMappings]`
- **Limit**: 32 mappings (defined by `kMaxUserAgentMappings`)
- **Populated**: At startup from JSON file

## Troubleshooting

### Issue: User agent not being recognized

**Check**:
1. Is it in `assets/user_agents.json`?
2. Does the enum value exist in `RequestData.h`?
3. Is the name in `gUserAgentNames[]`?
4. Check `logs/new_clients.log` - is it being logged there?

**Solution**:
- Follow "Adding New Clients" steps above
- Verify JSON syntax is correct
- Check console for parsing errors (verbose mode)

### Issue: Too many unknown clients in log

**Solution**:
- Review `logs/new_clients.log` periodically
- Add frequently-seen clients to JSON configuration
- Consider adding common clients to hardcoded list if they're very common

### Issue: JSON file not loading

**Check**:
1. File exists at `assets/user_agents.json`
2. File is readable
3. JSON syntax is valid
4. Check verbose mode for error messages

**Solution**:
- System will fall back to hardcoded list if JSON doesn't load
- Fix JSON syntax and restart
- Check file permissions

## Best Practices

1. **Regular Review**: Check `logs/new_clients.log` weekly to identify new clients
2. **JSON Updates**: Prefer JSON configuration over code changes when possible
3. **Enum Management**: Keep enum values in sync with `gUserAgentNames[]` array
4. **Documentation**: Document any custom clients you add
5. **Log Rotation**: Consider implementing log rotation for long-running servers

## Future Enhancements

Potential improvements:
- Automatic log rotation
- Web interface to view/add user agents
- Statistics on unknown client frequency
- Auto-suggest adding clients that appear frequently
- Support for wildcard matching in JSON

## Related Files

- `src/alpacadriver.cpp` - Main implementation
- `src/RequestData.h` - Enum definitions
- `assets/user_agents.json` - Configuration file
- `libs/src_mlsLib/json_parse.c` - JSON parser library

