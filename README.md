# agl_services_hvac_mockup

This code has been developed and used during LG Workshop hackathon to emulate
HVAC and demoed webOS/Enact HTML5 application.

## Native build

```bash
mkdir build && cd build
cmake ..
make
```

## Manual tests

```bash
# Native binder
afb-daemon --port=1111 --name=afb-hvac_mockup --workdir=/home/seb/Work/webos-lge/git/agl-services-hvac-mockup/build/package --ldpaths=lib --roothttp=htdocs --token= -vvv
```

```bash
curl -s http://localhost:1111/api/hvac_mockup/get-temperature?token=HELLO&uuid=magic |jq .

curl -s http://localhost:1111/api/hvac_mockup/set-temperature-zone1?token=HELLO&uuid=magic |jq .

curl -s -H  "Content-Type: application/json" -d '{"temperature":33}' http://localhost:1111/api/hvac_mockup/set-temperature-zone1?token=HELLO&uuid=magic |jq .

curl -s -H  "Content-Type: application/json" -d '{"temperature":10}' http://localhost:1111/api/hvac_mockup/set-temperature-zone2?token=HELLO&uuid=magic |jq .
```

You can also use the basic html app
```
chromium http://localhost:1111
```
