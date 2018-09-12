--[[
  Copyright (C) 2018 "IoT.bzh"
  Author Sebastien Douheret <sebastien@iot.bzh>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.


  NOTE: strict mode: every global variables should be prefixed by '_'

  Test commands:
    afb-client-demo 'localhost:2222/api?token=XXX&uuid=magic' hvac_mockup get-temperature
--]]

_tempe = {
    ["zone1"] = 30,
    ["zone2"] = 30
}

function _run_onload_(source)
    AFB:notice(source, "--InLua-- ENTER _run_onload_ HVAC mockup Application\n")

    return 0
end

function _evt_catcher_(source, action, event)
    AFB:notice(source, "RECV EVENT=%s", Dump_Table(event))

    if (event.count % 10) == 0 then
        AFB:notice(source, "Request can_emul status")
        local err, response = AFB:servsync(source, "can_emul", "status", {})
        if (err) then
            AFB:error(source, "--LUA:_start_app_ can_emul status response=%s", response)
            return 1
        end
    end

    if ((event.count % 10) == 0) and (math.random(5) == 1) then
        AFB:notice(source, "Request can_emul config with failure")
        local err, response = AFB:servsync(source, "can_emul", "config", {})
        AFB:debug(source, "--LUA:_start_app_ config status response=%s", response)
    end
end

function _get_temperature_(source, args, query)
    AFB:debug(source, "--InLua-- ENTER _get_temperature_ query=%s", Dump_Table(query))

    AFB:success(source, {
        { ["name"] = "zone_1", ["temperature"] = _tempe.zone1 },
        { ["name"] = "zone_2", ["temperature"] = _tempe.zone2 }
        }
    )
end

function _set_tempe1_(source, args, query)
    local qquery = query
    qquery.zone = "zone1"
    _set_temperature_(source, args, qquery)
end

function _set_tempe2_(source, args, query)
    local qquery = query
    qquery.zone = "zone2"
    _set_temperature_(source, args, qquery)
end

function _set_temperature_(source, args, query)
    AFB:debug(source, "--InLua-- ENTER _set_temperature_ query=%s", Dump_Table(query))

    if query == "null" then
        AFB:fail(source, "ERROR: invalid parameters")
        return 1
    end
    if query.zone == nil then
        AFB:fail(source, "ERROR: zone parameter not set")
        return 1
    end

    if query.temperature == nil then
        query.temperature = 30
    end

    if query.zone == "zone1" then
        AFB:debug(source, "--InLua-- Change temperature zone 1 to %s", query.temperature)
        _tempe.zone1 = query.temperature
    elseif query.zone == "zone2" then
        AFB:debug(source, "--InLua-- Change temperature zone 2 to %s", query.temperature)
        _tempe.zone2 = query.temperature
    else
        AFB:fail(source, "ERROR: unsupported zone")
        return 1
    end

    AFB:success(source, {
        { ["name"] = "zone_1", ["temperature"] = _tempe.zone1 },
        { ["name"] = "zone_2", ["temperature"] = _tempe.zone2 }
        }
    )
end
