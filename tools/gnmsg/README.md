## Message parsing tool for geode-native (gnmsg)
Given a debug-level log file from a geode-native application, this script will decode all of the Geode protocol messages it can between client and server, and print them out as a list of JSON objects.

```
usage: gnmsg.py [-h] [--file [F]] [--handshake] [--messages]

Parse a Gemfire NativeClient log file.

optional arguments:
  -h, --help   show this help message and exit
  --file [F]   Data file path/name
  --handshake  (optionally) print out handshake message details
  --messages   (optionally) print out regular message details
```

The `handshake` argument should be considered experimental at the time of this writing, since there doesn't yet exist a public version of geode-native that actually logs the handshake bytes to parse.
