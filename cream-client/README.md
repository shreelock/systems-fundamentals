# CREAM Client

Interactive C client that can be used to test a `cream` server.

## Running the client

First compile the client with `make clean all`.
To connect to a `cream` server, run the `cream_client` executable with the correct arguments.

```
./cream_client HOSTNAME PORT
HOSTNAME           Valid hostname or IPv4 or IPv6 address to connect to.
PORT_NUMBER        Port number to connect to.
```

## Commands

The commands that this client supports are listed below.
Each command and its arguments are delimited by spaces.

`put KEY VALUE`  - Inserts a key-value pair into the cache.

`get KEY`        - Retrieves the value from the cache corresponding to `KEY` and dumps it to `stdout`.

`evict KEY`      - Evicts the value from the cache corresponding to `KEY`.

`clear`          - Clears all cache entries.

`test KEY VALUE` - Tests inserting and retrieving a value from the cache.

`quit`           - Exits the client.
