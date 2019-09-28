# vcv-httpserver

An HTTP server for VCV Rack.

Server binds to port `8331`.

Document root lives in `res/htdocs`

## API Routes

### /api/info

Returns JSON document with `sampleRate` and whether the engine is paused or not.
