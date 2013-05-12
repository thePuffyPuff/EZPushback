/* ezpushback.h
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

#ifndef EXPUSHBACK_H
#define EXPUSHBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <XPLMDefs.h>
#include <XPLMPlugin.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <XPLMProcessing.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(IBM_X64)
    #define PLATFORM "win64"
#elif defined(IBM_X86)
    #define PLATFORM "win32"
#elif defined(MACX)
    #define PLATFORM "macx"
#elif defined(LINUX_X64)
    #define PLATFORM "lin64"
#elif defined(LINUX_X86)
    #define PLATFORM "lin32"
#endif

#define PLUGIN_VERSION      "0.3.2"
#define PLUGIN_NAME         "EZPushback " PLUGIN_VERSION " (" PLATFORM ")"
#define PLUGIN_SIGNATURE    "tpp.ezpushback"
#define COPYRIGHT           "Copyright (C) 2013 Rich Lucas (thePuffyPuff)"
#define PLUGIN_DESCRIPTION  "X-Plane pushback utility by Rich Lucas (thePuffyPuff)."
#define DEBUG_PREFIX        "EZPushback: "

#define DEG2RAD(deg)  ((deg)/57.296f);

#define MSG_ADD_DATAREF      0x1000000

#define EZPB_CMD_STOP        0
#define EZPB_CMD_PUSH        1
#define EZPB_CMD_TUG         2

#define EZPB_STATE_IDLE      0
#define EZPB_STATE_WAITING   1
#define EZPB_STATE_STARTING  2
#define EZPB_STATE_ACTIVE    3
#define EZPB_STATE_STOPPING  4

#define EZPB_FLAG_OK                     0x0000
#define EZPB_FLAG_AIRCRAFT_NOT_LOADED    0x0001
#define EZPB_FLAG_PARKING_BRAKE_ENGAGED  0x0002
#define EZPB_FLAG_AIRCRAFT_MOVING        0x0004

#define EZPB_REVERSE  -1
#define EZPB_STOPPED   0
#define EZPB_FORWARD   1

#define UI_MENU_ITEM_START_PUSH  1
#define UI_MENU_ITEM_START_TUG   2
#define UI_MENU_ITEM_STOP        3

#define UI_DIALOG_TEXT_PUSHBACK_IN_PROGRESS  0
#define UI_DIALOG_TEXT_TUG_IN_PROGRESS       1
#define UI_DIALOG_TEXT_OPERATION_COMPLETE    2
#define UI_DIALOG_TEXT_AIRCRAFT_NOT_LOADED   3
#define UI_DIALOG_TEXT_PARKING_BRAKE         4
#define UI_DIALOG_TEXT_AIRCRAFT_MOVING       5

#define UI_POSITION_DEFAULT  -1

#define UI_DIALOG_NO_DELAY   0.0f
#define UI_DIALOG_STD_DELAY  5.0f

#define UI_BUTTON_NONE       0
#define UI_BUTTON_TEXT_STOP  1

#define UNUSED(x) (void)(x)

// *********************************************************************************************
// debug.c
// *********************************************************************************************

void debug(const char *format, ...);


// *********************************************************************************************
// datarefs.c
// *********************************************************************************************

XPLMDataRef find_dataref         (const char* name);
XPLMDataRef create_dataref_int   (const char* name, XPLMGetDatai_f read_fn, XPLMSetDatai_f write_fn);
XPLMDataRef create_dataref_float (const char* name, XPLMGetDataf_f read_fn, XPLMSetDataf_f write_fn);


// *********************************************************************************************
// ui.c
// *********************************************************************************************

void  create_ui();
void  destroy_ui();
void  ui_dialog(int dialog_text_id, int button_text_id, float hide_after);
void  set_dialog_position(int x, int y);
int   get_dialog_x();
int   get_dialog_y();
int   get_dialog_default_x();
int   get_dialog_default_y();
void  show_dialog();
void  hide_dialog();
void  hide_dialog_later(float seconds);
void  set_dialog_text(int dialog_text_id);
void  show_button();
void  hide_button();
void  set_button_text(int button_text_id);
void  set_menu_active_state();
void  set_menu_idle_state();


// *********************************************************************************************
// ezpushback.c
// *********************************************************************************************

int   get_ezpb_flags        (void* refcon);
int   get_ezpb_state        (void* refcon);
int   get_ezpb_direction    (void* refcon);
int   get_ezpb_callbacks    (void* refcon);
float get_ezpb_target_speed (void* refcon);
float get_ezpb_actual_speed (void* refcon);
float get_ezpb_acceleration (void* refcon);
float get_ezpb_deceleration (void* refcon);
float get_ezpb_local_vx     (void* refcon);
float get_ezpb_local_vz     (void* refcon);

void  set_ezpb_acceleration (void* refcon, float value);
void  set_ezpb_deceleration (void* refcon, float value);
void  set_ezpb_target_speed (void* refcon, float value);

void  reset_state();
void  cancel_operation();
void  schedule_pushback_flcb();
void  unable_command(int cmd, int flags);
void  start_push();
void  start_tug();
void  stop_any();

float pushback_flcb        (float elapsed_sec_last_call, float elapsed_sec_since_last_loop, int counter, void* refcon);
float declare_dataref_flcb (float elapsed_sec_last_call, float elapsed_sec_since_last_loop, int counter, void* refcon);


// *********************************************************************************************
// prefs.c
// *********************************************************************************************

char* get_ini_path();
void  read_prefs();
void  write_prefs();
int   ConvertPath(const char * inPath, char * outPath, int outPathMaxLen);



#ifdef __cplusplus
}
#endif

#endif // EXPUSHBACK_H
