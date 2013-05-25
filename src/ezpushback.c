/* ezpushback.c
 * EZPushback (X-Plane pushback utility)
 * Copyright (c) 2013 Rich Lucas (thePuffyPuff)

 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "ezpushback.h"


// *********************************************************************************************
// Our Global Vars
// *********************************************************************************************

XPLMPluginID plugin_id  = XPLM_NO_PLUGIN_ID;

int   aircraft_loaded   = 0;
int   ezpb_state        = EZPB_STATE_IDLE;
int   ezpb_direction    = EZPB_STOPPED;
float ezpb_actual_speed = 0.0f;     // calculated/actual speed
float ezpb_target_speed = 1.0f;     // target speed meters/sec
float ezpb_acceleration = 0.2f;     // rate acceleration meters/sec/sec
float ezpb_deceleration = 0.5f;     // rate deceleration meters/sec/sec
float ezpb_local_vx     = 0.0f;     // velocity we are applying in X direction
float ezpb_local_vz     = 0.0f;     // velocity we are applying in Z direction
int   ezpb_callbacks    = 0;        // flight loop callbacks


// *********************************************************************************************
// X-Plane Datarefs
// *********************************************************************************************

// sim/flightmodel/position
XPLMDataRef dr_local_vx    = NULL;  // The X velocity in OGL coordinates (RW).
XPLMDataRef dr_local_vz    = NULL;  // The Z velocity in OGL coordinates (RW).
XPLMDataRef dr_psi         = NULL;  // Aircraft true heading (RW).
XPLMDataRef dr_groundspeed = NULL;  // Groundspeed of aircraf in meters/sec (R).
XPLMDataRef dr_parkbrake   = NULL;  // parking brake deployed (1.0 == full)


// *********************************************************************************************
// Our Published Datarefs
// *********************************************************************************************

const char* ezpb_published_datarefs[]= {
    "tpp/ezpushback/state",
    "tpp/ezpushback/direction",
    "tpp/ezpushback/flags",
    "tpp/ezpushback/target_speed",
    "tpp/ezpushback/actual_speed",
    "tpp/ezpushback/acceleration",
    "tpp/ezpushback/deceleration",
    "tpp/ezpushback/local_vx",
    "tpp/ezpushback/local_vz",
    "tpp/ezpushback/callbacks",
    NULL // end of list
};

XPLMDataRef dr_ezpb_state           = NULL;
XPLMDataRef dr_ezpb_direction       = NULL;
XPLMDataRef dr_ezpb_target_speed    = NULL;
XPLMDataRef dr_ezpb_actual_speed    = NULL;
XPLMDataRef dr_ezpb_flags           = NULL;
XPLMDataRef dr_ezpb_acceleration    = NULL;
XPLMDataRef dr_ezpb_deceleration    = NULL;
XPLMDataRef dr_ezpb_local_vx        = NULL;
XPLMDataRef dr_ezpb_local_vz        = NULL;
XPLMDataRef dr_ezpb_callbacks       = NULL;


// *********************************************************************************************
// Our Published Commands
// *********************************************************************************************

XPLMCommandRef cr_ezpb_start_push   = NULL;   // start pushback
XPLMCommandRef cr_ezpb_start_tug    = NULL;   // start tug (pull forward)
XPLMCommandRef cr_ezpb_stop         = NULL;   // stop (push or tug)
XPLMCommandRef cr_ezpb_toggle_push  = NULL;   // toggle pushback
XPLMCommandRef cr_ezpb_toggle_tug   = NULL;   // toggle tug (pull forward)


// *********************************************************************************************
// Misc. Functions
// *********************************************************************************************

void schedule_pushback_flcb()
{
    XPLMSetFlightLoopCallbackInterval(pushback_flcb, -1, 0, NULL);
}


void unschedule_pushback_flcb()
{
    XPLMSetFlightLoopCallbackInterval(pushback_flcb, 0, 0, NULL);
}


void set_state(int new_state)
{
    ezpb_state = new_state;
}


void reset_state()
{
    debug("reset state");
    set_state(EZPB_STATE_IDLE);

    ezpb_direction    = EZPB_STOPPED;
    ezpb_actual_speed = 0.0f;
    ezpb_local_vx     = 0.0f;
    ezpb_local_vz     = 0.0f;
    ezpb_callbacks    = 0;

    unschedule_pushback_flcb();
    set_menu_idle_state();
}


// *********************************************************************************************
// Our Dataref Read/Write Callbacks
// *********************************************************************************************

int get_ezpb_flags(void* refcon)
{
    UNUSED(refcon);
    int flags = 0;

    if (aircraft_loaded == 0) {
        flags |= EZPB_FLAG_AIRCRAFT_NOT_LOADED;
    }

    if (XPLMGetDataf(dr_parkbrake) > 0.0f) {
        flags |= EZPB_FLAG_PARKING_BRAKE_ENGAGED;
    }

    if (XPLMGetDataf(dr_groundspeed) > 0.5f) {
        flags |= EZPB_FLAG_AIRCRAFT_MOVING;
    }

    return flags;
}

int   get_ezpb_state        (void* refcon) { UNUSED(refcon); return ezpb_state; }
int   get_ezpb_direction    (void* refcon) { UNUSED(refcon); return ezpb_direction; }
int   get_ezpb_callbacks    (void* refcon) { UNUSED(refcon); return ezpb_callbacks; }
float get_ezpb_target_speed (void* refcon) { UNUSED(refcon); return ezpb_target_speed; }
float get_ezpb_actual_speed (void* refcon) { UNUSED(refcon); return ezpb_actual_speed; }
float get_ezpb_acceleration (void* refcon) { UNUSED(refcon); return ezpb_acceleration; }
float get_ezpb_deceleration (void* refcon) { UNUSED(refcon); return ezpb_deceleration; }
float get_ezpb_local_vx     (void* refcon) { UNUSED(refcon); return ezpb_local_vx; }
float get_ezpb_local_vz     (void* refcon) { UNUSED(refcon); return ezpb_local_vz; }


void set_ezpb_acceleration (void* refcon, float value)
{
    UNUSED(refcon);

    if (value < 0.1f || value > 4.5f) {
        debug("set acceleration data ref %.1f out of range (0.1-4.5)", value);
        return;
    }

    ezpb_acceleration = value;
}


void set_ezpb_deceleration (void* refcon, float value)
{
    UNUSED(refcon);

    if (value < 0.1f || value > 4.5f) {
        debug("set deceleration data ref %.1f out of range (0.1-4.5)", value);
        return;
    }

    ezpb_deceleration = value;
}


void set_ezpb_target_speed (void* refcon, float value)
{
    UNUSED(refcon);

    if (value < 0.1f || value > 9.0f) {
        debug("set target_speed data ref %.1f out of range (0.1-9.0)", value);
        return;
    }

    ezpb_target_speed = value;
}


// *********************************************************************************************
// Flight Loop Callbacks
// *********************************************************************************************

float pushback_flcb(float    elapsed_sec_since_last_call,
                    float    elapsed_sec_since_last_loop,
                    int      counter,
                    void*    refcon)
{
    UNUSED(elapsed_sec_since_last_call);
    UNUSED(elapsed_sec_since_last_loop);
    UNUSED(counter);
    UNUSED(refcon);

    // do nothing if state is idle, and don't callback
    if (ezpb_state == EZPB_STATE_IDLE) return 0.0f;

    float true_hdg = XPLMGetDataf(dr_psi);

    if (ezpb_callbacks == 0) {
        // a push/tug operation is starting
        // zero time since last call for proper acceleration
        elapsed_sec_since_last_call = 0;
        ezpb_actual_speed = 0.0f;
    }

    ezpb_callbacks++;

    int flags = get_ezpb_flags(NULL);
    if (flags & EZPB_FLAG_AIRCRAFT_NOT_LOADED) {
        reset_state();
        return 0.0f;
    }

    if (ezpb_state != EZPB_STATE_STOPPING && flags & EZPB_FLAG_PARKING_BRAKE_ENGAGED) {
        debug("parking was brake engaged during operation -- stopping push/tug");
        ui_dialog(UI_DIALOG_TEXT_BRAKES_STOPPING, UI_BUTTON_NONE, UI_DIALOG_NO_DELAY);
        set_state(EZPB_STATE_STOPPING);
    }

    switch (ezpb_state) {
        case EZPB_STATE_ACTIVE:
            ezpb_actual_speed = ezpb_target_speed;
            break;

        case EZPB_STATE_STARTING:
            ezpb_actual_speed += ezpb_acceleration * elapsed_sec_since_last_call;
            if (ezpb_actual_speed >= ezpb_target_speed) {
                ezpb_actual_speed = ezpb_target_speed;
                set_state(EZPB_STATE_ACTIVE);
            }
            break;

        case EZPB_STATE_STOPPING:
            ezpb_actual_speed -= ezpb_deceleration * elapsed_sec_since_last_call;
            if (ezpb_actual_speed <= 0.2f) {
                ui_dialog(UI_DIALOG_TEXT_OPERATION_COMPLETE, UI_BUTTON_NONE, UI_DIALOG_STD_DELAY);
                reset_state();
                return 0.0f; // do not callback again in idle state
            }
            break;
    }

    if (flags & EZPB_FLAG_PARKING_BRAKE_ENGAGED) {
        // do not apply force when parking brake is engaged
        // but keep calling back until aircraft has stopped
        return -1.0f;
    }

    // compute velocity in X and Z directions
    float ax = DEG2RAD(true_hdg);
    float az = DEG2RAD(true_hdg - 90.0f);
    ezpb_local_vx = sinf(ax) * ezpb_actual_speed * ezpb_direction;
    ezpb_local_vz = sinf(az) * ezpb_actual_speed * ezpb_direction;

    // apply force to aircraft
    XPLMSetDataf(dr_local_vx, ezpb_local_vx);
    XPLMSetDataf(dr_local_vz, ezpb_local_vz);

    // these datarefs are only for monitoring the plugin
    XPLMSetDataf(dr_ezpb_local_vx, ezpb_local_vx);
    XPLMSetDataf(dr_ezpb_local_vz, ezpb_local_vz);
    XPLMSetDatai(dr_ezpb_callbacks, ezpb_callbacks);

    // callback again as soon as possible
    return -1.0f;
}


float declare_dataref_flcb(float    elapsed_sec_since_last_call,
                           float    elapsed_sec_since_last_loop,
                           int      counter,
                           void*    refcon)
{
    UNUSED(elapsed_sec_since_last_call);
    UNUSED(elapsed_sec_since_last_loop);
    UNUSED(counter);
    UNUSED(refcon);

    int i;
    // tell DataRefEditor plugin about our custom datarefs
    XPLMPluginID pid = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
    if (pid != XPLM_NO_PLUGIN_ID) {
        for (i=0; ezpb_published_datarefs[i] != NULL; i++) {
            XPLMSendMessageToPlugin(pid, MSG_ADD_DATAREF, (void*)ezpb_published_datarefs[i]);
        }
        debug("added tpp/ezpushback/* refs to DataRefEditor plugin");
    }
    else {
        debug("DataRefEditor plugin not found");
    }

    return 0; // called only once
}

// *********************************************************************************************
// Our Commands
// *********************************************************************************************

void start_push()
{
    int flags;

    if (ezpb_state == EZPB_STATE_IDLE) {
        flags = get_ezpb_flags(NULL);
        if (flags != 0) {
            unable_command(EZPB_CMD_PUSH, flags);
            return;
        }

        debug("start push");
        ezpb_direction = EZPB_REVERSE;
        set_state(EZPB_STATE_STARTING);
        ui_dialog(UI_DIALOG_TEXT_PUSHBACK_IN_PROGRESS, UI_BUTTON_TEXT_STOP, UI_DIALOG_NO_DELAY);
        set_menu_active_state();
        schedule_pushback_flcb();
    }
    else {
        debug("start push command ignored, operation already in progress");
    }

    return;
}


void start_tug()
{
    int flags;

    if (ezpb_state == EZPB_STATE_IDLE) {
        flags = get_ezpb_flags(NULL);
        if (flags != 0) {
            unable_command(EZPB_CMD_TUG, flags);
            return;
        }

        debug("start tug");
        ezpb_direction = EZPB_FORWARD;
        set_state(EZPB_STATE_STARTING);
        ui_dialog(UI_DIALOG_TEXT_TUG_IN_PROGRESS, UI_BUTTON_TEXT_STOP, UI_DIALOG_NO_DELAY);
        set_menu_active_state();
        schedule_pushback_flcb();
    }
    else {
        debug("start tug command ignored, operation already in progress");
    }
}


void stop_any()
{
    if (ezpb_state != EZPB_STATE_IDLE) {
        set_state(EZPB_STATE_STOPPING);
        hide_button();  // hide the STOP button
    }
    else {
        debug("stop command ignored, idle");
    }
}


void unable_command(int cmd, int flags)
{
    debug("unable command (cmd=%d flags=0x%04X)", cmd, flags);

    if (flags & EZPB_FLAG_AIRCRAFT_NOT_LOADED) {
        ui_dialog(UI_DIALOG_TEXT_AIRCRAFT_NOT_LOADED, UI_BUTTON_NONE, UI_DIALOG_STD_DELAY);
    }
    else if (flags & EZPB_FLAG_PARKING_BRAKE_ENGAGED) {
        ui_dialog(UI_DIALOG_TEXT_PARKING_BRAKE, UI_BUTTON_NONE, UI_DIALOG_STD_DELAY);
    }
    else if (flags & EZPB_FLAG_AIRCRAFT_MOVING) {
        ui_dialog(UI_DIALOG_TEXT_AIRCRAFT_MOVING, UI_BUTTON_NONE, UI_DIALOG_STD_DELAY);
    }
}


// *********************************************************************************************
// Custom XP Command Handlers
// *********************************************************************************************

int cch_start_tug(XPLMCommandRef cref, XPLMCommandPhase phase, void* refcon)
{
    UNUSED(cref);
    UNUSED(refcon);

    if (phase == xplm_CommandBegin) {
        start_tug();
    }
    return 0;
}


int cch_start_push(XPLMCommandRef cref, XPLMCommandPhase phase, void* refcon)
{
    UNUSED(cref);
    UNUSED(refcon);

    if (phase == xplm_CommandBegin) {
        start_push();
    }
    return 0;
}


int cch_stop_any(XPLMCommandRef cref, XPLMCommandPhase phase, void* refcon)
{
    UNUSED(cref);
    UNUSED(refcon);

    if (phase == xplm_CommandBegin) {
        stop_any();
    }
    return 0;
}


int cch_toggle_tug(XPLMCommandRef cref, XPLMCommandPhase phase, void* refcon)
{
    UNUSED(cref);
    UNUSED(refcon);

    if (phase == xplm_CommandBegin) {
        if (ezpb_state == EZPB_STATE_IDLE) {
            start_tug();
        }
        else {
            stop_any();
        }
    }
    return 0;
}


int cch_toggle_push(XPLMCommandRef cref, XPLMCommandPhase phase, void* refcon)
{
    UNUSED(cref);
    UNUSED(refcon);

    if (phase == xplm_CommandBegin) {
        if (ezpb_state == EZPB_STATE_IDLE) {
            start_push();
        }
        else {
            stop_any();
        }
    }
    return 0;
}


// *********************************************************************************************
// Plugin
// *********************************************************************************************

PLUGIN_API int XPluginStart( char *out_name,
                             char *out_sig,
                             char *out_desc)
{
    strcpy(out_name, PLUGIN_NAME);
    strcpy(out_sig,  PLUGIN_SIGNATURE);
    strcpy(out_desc, PLUGIN_DESCRIPTION);

    debug("Version %s (%s), %s", PLUGIN_VERSION, PLATFORM, COPYRIGHT);
    plugin_id = XPLMGetMyID();

    read_prefs();

    dr_local_vx    = find_dataref("sim/flightmodel/position/local_vx");
    dr_local_vz    = find_dataref("sim/flightmodel/position/local_vz");
    dr_psi         = find_dataref("sim/flightmodel/position/psi");
    dr_groundspeed = find_dataref("sim/flightmodel/position/groundspeed");
    dr_parkbrake   = find_dataref("sim/flightmodel/controls/parkbrake");

    if (dr_local_vx    == NULL ||
        dr_local_vz    == NULL ||
        dr_psi         == NULL ||
        dr_groundspeed == NULL ||
        dr_parkbrake   == NULL
    ) {
        debug("could not start plugin: failed to find one or more XP core datarefs");
        return 0;
    }

    dr_ezpb_state        = create_dataref_int   ("tpp/ezpushback/state",        get_ezpb_state,        NULL);
    dr_ezpb_direction    = create_dataref_int   ("tpp/ezpushback/direction",    get_ezpb_direction,    NULL);
    dr_ezpb_flags        = create_dataref_int   ("tpp/ezpushback/flags",        get_ezpb_flags,        NULL);
    dr_ezpb_target_speed = create_dataref_float ("tpp/ezpushback/target_speed", get_ezpb_target_speed, set_ezpb_target_speed);
    dr_ezpb_actual_speed = create_dataref_float ("tpp/ezpushback/actual_speed", get_ezpb_actual_speed, NULL);
    dr_ezpb_acceleration = create_dataref_float ("tpp/ezpushback/acceleration", get_ezpb_acceleration, set_ezpb_acceleration);
    dr_ezpb_deceleration = create_dataref_float ("tpp/ezpushback/deceleration", get_ezpb_deceleration, set_ezpb_deceleration);
    dr_ezpb_local_vx     = create_dataref_float ("tpp/ezpushback/local_vx",     get_ezpb_local_vx,     NULL);
    dr_ezpb_local_vz     = create_dataref_float ("tpp/ezpushback/local_vz",     get_ezpb_local_vz,     NULL);
    dr_ezpb_callbacks    = create_dataref_int   ("tpp/ezpushback/callbacks",    get_ezpb_callbacks,    NULL);

    cr_ezpb_start_push  = XPLMCreateCommand("tpp/ezpushback/start_push",  "Start pushback (reverse)");
    cr_ezpb_start_tug   = XPLMCreateCommand("tpp/ezpushback/start_tug",   "Start tug (forward)");
    cr_ezpb_stop        = XPLMCreateCommand("tpp/ezpushback/stop",        "Stop push or pull");
    cr_ezpb_toggle_push = XPLMCreateCommand("tpp/ezpushback/toggle_push", "Toggle pushback (reverse)");
    cr_ezpb_toggle_tug  = XPLMCreateCommand("tpp/ezpushback/toggle_tug",  "Toggle tug (forward)");

    XPLMRegisterCommandHandler(cr_ezpb_start_push,  cch_start_push,  1, NULL);
    XPLMRegisterCommandHandler(cr_ezpb_start_tug,   cch_start_tug,   1, NULL);
    XPLMRegisterCommandHandler(cr_ezpb_stop,        cch_stop_any,    1, NULL);
    XPLMRegisterCommandHandler(cr_ezpb_toggle_push, cch_toggle_push, 1, NULL);
    XPLMRegisterCommandHandler(cr_ezpb_toggle_tug,  cch_toggle_tug,  1, NULL);

    create_ui();

    debug("started plugin_id=%d", plugin_id);
    return 1; // OK
}


PLUGIN_API int XPluginEnable(void)
{
    reset_state();
    hide_dialog();
    XPLMRegisterFlightLoopCallback(declare_dataref_flcb, 1, NULL);
    XPLMRegisterFlightLoopCallback(pushback_flcb, 0, NULL);
    debug("enabled");
    return 1; // OK
}


PLUGIN_API void XPluginDisable(void)
{
    reset_state();
    hide_dialog();
    XPLMUnregisterFlightLoopCallback(pushback_flcb, NULL);
    XPLMUnregisterFlightLoopCallback(declare_dataref_flcb, NULL);
    debug("disabled");
}



PLUGIN_API void	XPluginStop(void)
{
    write_prefs();
    destroy_ui();
    debug("stopped");
}



PLUGIN_API void XPluginReceiveMessage(XPLMPluginID  in_from_who,
                                      long          in_message,
                                      void*         in_param)
{
    intptr_t plane;

    if (in_from_who == XPLM_PLUGIN_XPLANE) {
        switch (in_message) {
            case XPLM_MSG_PLANE_LOADED:
                plane = (intptr_t)in_param;
                if (plane == 0) {
                    reset_state();
                    aircraft_loaded = 1;
                    debug("aircraft loaded");
                }
                break;

            case XPLM_MSG_PLANE_UNLOADED:
                plane = (intptr_t)in_param;
                if (plane == 0) {
                    reset_state();
                    aircraft_loaded = 0;
                    debug("aircraft unloaded");
                }
                break;
        }
    }
}

#ifdef __cplusplus
}
#endif
