/*
* Copyright (C) 2018 "IoT.bzh"
* Author Fulup Ar Foll <fulup@iot.bzh>
* Author Romain Forlot <romain@iot.bzh>
* Author Sebastien Douheret <sebastien@iot.bzh>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "agl_services_hvac_mockup-binding.h"

AFB_ApiT AFB_default;

// Config Section definition (note: controls section index should match handle
// retrieval in HalConfigExec)
static CtlSectionT ctrlSections[] = {
    { .key = "plugins", .loadCB = PluginConfig },
    { .key = "onload", .loadCB = OnloadConfig },
    { .key = "controls", .loadCB = ControlConfig },
    { .key = "events", .loadCB = EventConfig },
    { .key = NULL }
};

static void ctrlapi_ping(AFB_ReqT request)
{
    static int count = 0;

    count++;
    AFB_ReqNotice(request, "Controller:ping count=%d", count);
    AFB_ReqSuccess(request, json_object_new_int(count), NULL);

    return;
}

void ctrlapi_auth(AFB_ReqT request)
{
    AFB_ReqSetLOA(request, 1);
    AFB_ReqSuccess(request, NULL, NULL);
}

static AFB_ApiVerbs CtrlApiVerbs[] = {
    /* VERB'S NAME         FUNCTION TO CALL         SHORT DESCRIPTION */
    { .verb = "ping-global", .callback = ctrlapi_ping, .info = "ping test for API" },
    { .verb = "auth", .callback = ctrlapi_auth, .info = "Authenticate session to raise Level Of Assurance of the session" },
    { .verb = NULL } /* marker for end of the array */
};

static int CtrlLoadStaticVerbs(AFB_ApiT apiHandle, AFB_ApiVerbs* verbs)
{
    int errcount = 0;

    for (int idx = 0; verbs[idx].verb; idx++) {
        errcount += AFB_ApiAddVerb(
            apiHandle, CtrlApiVerbs[idx].verb, NULL, CtrlApiVerbs[idx].callback,
            (void*)&CtrlApiVerbs[idx], CtrlApiVerbs[idx].auth, 0, 0);
    }

    return errcount;
};

static int CtrlLoadOneApi(void* cbdata, AFB_ApiT apiHandle)
{
    CtlConfigT* ctrlConfig = (CtlConfigT*)cbdata;

    // save closure as api's data context
    AFB_ApiSetUserData(apiHandle, ctrlConfig);

    // add static controls verbs
    int err = CtrlLoadStaticVerbs(apiHandle, CtrlApiVerbs);
    if (err) {
        AFB_ApiError(apiHandle, "CtrlLoadSection fail to register static V2 verbs");
        return ERROR;
    }

    // load section for corresponding API
    err = CtlLoadSections(apiHandle, ctrlConfig, ctrlSections);

    // declare an event event manager for this API;
    AFB_ApiOnEvent(apiHandle, CtrlDispatchApiEvent);

    AFB_ApiSeal(apiHandle);
    return err;
}

int afbBindingEntry(AFB_ApiT apiHandle)
{
    AFB_default = apiHandle;
    AFB_ApiNotice(apiHandle, "Controller in afbBindingEntry");

    const char* dirList = getenv("CONTROL_CONFIG_PATH");
    if (!dirList)
        dirList = CONTROL_CONFIG_PATH;

    const char* configPath = CtlConfigSearch(apiHandle, dirList, "");
    if (!configPath) {
        AFB_ApiError(apiHandle, "CtlPreInit: No %s* config found in %s ", GetBinderName(), dirList);
        return ERROR;
    }

    // load config file and create API
    CtlConfigT* ctrlConfig = CtlLoadMetaData(apiHandle, configPath);
    if (!ctrlConfig) {
        AFB_ApiError(apiHandle,
            "No valid control config file in:\n-- %s", configPath);
        return ERROR;
    }

    if (!ctrlConfig->api) {
        AFB_ApiError(apiHandle,
            "API Missing from metadata in:\n-- %s", configPath);
        return ERROR;
    }

    AFB_ApiNotice(apiHandle, "Controller API='%s' info='%s'", ctrlConfig->api,
        ctrlConfig->info);

    // create one API per config file
    if (! AFB_NewApi(apiHandle, ctrlConfig->api, ctrlConfig->info, 1, CtrlLoadOneApi, ctrlConfig)) {
        AFB_ApiError(apiHandle, "API creation failed");
        return ERROR;
    }

    // config exec should be done after api init in order to enable onload to use newly defined ctl API.
    return CtlConfigExec(apiHandle, ctrlConfig);
}
