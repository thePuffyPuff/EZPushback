/* prefs.c
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


#include "ezpushback.h"

#include <XPLMMenus.h>
#include <XPLMDisplay.h>
#include <XPWidgets.h>
#include <XPStandardWidgets.h>


#define UI_MENU_ITEM_START_PUSH  1
#define UI_MENU_ITEM_START_TUG   2
#define UI_MENU_ITEM_STOP        3


XPLMMenuID  ui_menu_id      = NULL;  // id of menu
int         ui_menu_main_ix = 0;     // main menu index
int         ui_menu_push_ix = 0;     // menu item index for push
int         ui_menu_tug_ix  = 0;     // menu item index for tug
int         ui_menu_stop_ix = 0;     // menu item index for stop

int         ui_dialog_x     = UI_POSITION_DEFAULT;   // left x
int         ui_dialog_y     = UI_POSITION_DEFAULT;   // top y
int         ui_dialog_w     = 400;  // width
int         ui_dialog_h     = 100;  // height
int         ui_dialog_open  = 0;    // 1 = dialog window is visible, 0 = not visible
XPWidgetID  ui_dialog_wid   = NULL; // root dialog widget ID
XPWidgetID  ui_dlg_text_wid = NULL; // dialog text widget ID
XPWidgetID  ui_button_wid   = NULL; // dialog button


const char* ui_dialog_text[] = {
    "Pushback in progress...",
    "Tug in progress...",
    "Operation complete -- have a good flight!",
    "Unable to push / tug -- aircraft is not loaded.",
    "Unable to push / tug -- parking brake is set.",
    "Unable to push / tug -- aircraft is already moving.",
    "Parking brakes deployed -- stopping push / tug."
};


const char* ui_button_text[] = {
    NULL, // no button
    "STOP"
};


int ui_dialog_handler( XPWidgetMessage		inMessage,
                       XPWidgetID			inWidget,
                       intptr_t				inParam1,
                       intptr_t 			inParam2)
{
    UNUSED(inWidget);
    UNUSED(inParam2);

    if (inMessage == xpMsg_PushButtonPressed) {
        if (inParam1 == (intptr_t)ui_button_wid) {
            switch (get_ezpb_state(NULL)) {
                case EZPB_STATE_STARTING:
                case EZPB_STATE_ACTIVE:
                    debug("stop button pressed from UI");
                    stop_any();
                    return 1;
            }
        }
    }

    return 0;
}


void ui_dialog(int dialog_text_id, int button_text_id, float hide_after)
{
    set_dialog_text(dialog_text_id);

    if (button_text_id != UI_BUTTON_NONE) {
        set_button_text(button_text_id);
        show_button();
    }
    else {
        hide_button();
    }

    show_dialog();

    if (hide_after > 0) {
        hide_dialog_later(hide_after);
    }
}


int dialog_is_on_screen()
{
    int w, h;
    XPLMGetScreenSize(&w, &h);
    int x = get_dialog_x();
    int y = get_dialog_y();
    if (x < 0 || x > (w - (ui_dialog_w / 2))) return 0;
    if (y < 0 || y > (h - ui_dialog_h)) return 0;
    return 1;
}



void set_dialog_default_position()
{
    int w, h;
    XPLMGetScreenSize(&w, &h);
    set_dialog_position(get_dialog_default_x(), get_dialog_default_y());
    debug("moved dialog to default position");
}


void set_dialog_position(int x, int y)
{
    ui_dialog_x = x;
    ui_dialog_y = y;

    // (x0, y0) top left dialog
    int x1 = ui_dialog_x;
    int y1 = ui_dialog_y + ui_dialog_h - 1;
    // (x2, y2) bottom right dialog
    int x2 = x1 + ui_dialog_w - 1;
    int y2 = ui_dialog_y;

    if (ui_dialog_wid) {
        XPSetWidgetGeometry(ui_dialog_wid, x1, y1, x2, y2);
    }
}


int get_dialog_x()
{
    int x = 0;
    if (ui_dialog_wid) {
        XPGetWidgetGeometry(ui_dialog_wid, &x, NULL, NULL, NULL);
    }
    return x;
}


int get_dialog_y()
{
    int y = 0;
    if (ui_dialog_wid) {
        XPGetWidgetGeometry(ui_dialog_wid, NULL, &y, NULL, NULL);
    }
    return y;
}


int get_dialog_default_x()
{
    int w, h;
    XPLMGetScreenSize(&w, &h);
    return (w - ui_dialog_w) / 2;
}


int get_dialog_default_y()
{
    int w, h;
    XPLMGetScreenSize(&w, &h);
    return h - ((h - ui_dialog_h) / 4);
}


void show_dialog()
{
    if (ui_dialog_wid) {
        if (ui_dialog_open) {
            hide_dialog();
        }

        if (ui_dialog_x == UI_POSITION_DEFAULT || ui_dialog_y == UI_POSITION_DEFAULT) {
            set_dialog_default_position();
        }
        else if (!dialog_is_on_screen()) {
            debug("dialog is off screen");
            set_dialog_default_position();
        }

        XPShowWidget(ui_dialog_wid);
        XPBringRootWidgetToFront(ui_dialog_wid);
        // debug("showing dialog");
        ui_dialog_open = 1;
    }
}


float hide_dialog_flcb(float elapsed_sec_last_call,
                       float elapsed_sec_since_last_loop,
                       int counter,
                       void* refcon)
{
    UNUSED(elapsed_sec_last_call);
    UNUSED(elapsed_sec_since_last_loop);
    UNUSED(counter);
    UNUSED(refcon);

    hide_dialog();
    return 0;  // one shot
}


void hide_dialog()
{
    if (ui_dialog_wid && ui_dialog_open) {
        XPHideWidget(ui_dialog_wid);
        // debug("hiding dialog");
        ui_dialog_open = 0;

        // cancel any hide that may have been scheduled
        XPLMSetFlightLoopCallbackInterval(hide_dialog_flcb, 0, 0, NULL);
    }
}


void hide_dialog_later(float seconds)
{
    if (ui_dialog_wid && ui_dialog_open) {
        XPLMSetFlightLoopCallbackInterval(hide_dialog_flcb, seconds, 1, NULL);
    }
}


void set_dialog_text(int dialog_text_id)
{
    if (ui_dlg_text_wid) {
        XPSetWidgetDescriptor(ui_dlg_text_wid, ui_dialog_text[dialog_text_id]);
    }
}


void show_button()
{
    if (ui_button_wid) {
        XPShowWidget(ui_button_wid);
        // debug("show button");
    }
}


void hide_button()
{
    if (ui_button_wid) {
        XPHideWidget(ui_button_wid);
        // debug("hide button");
    }
}


void set_button_text(int button_text_id)
{
    if (ui_button_wid) {
        XPSetWidgetDescriptor(ui_button_wid, ui_button_text[button_text_id]);
    }
}


void create_dialog()
{
    int x1, y1, x2, y2;

    if (ui_dialog_x == UI_POSITION_DEFAULT || ui_dialog_y == UI_POSITION_DEFAULT) {
        x1 = 0;
        y1 = ui_dialog_h - 1;
        x2 = ui_dialog_w - 1;
        y2 = 0;
    }
    else {
        // (x0, y0) top left dialog
        x1 = ui_dialog_x;
        y1 = ui_dialog_y + ui_dialog_h - 1;
        // (x2, y2) bottom right dialog
        x2 = x1 + ui_dialog_w - 1;
        y2 = ui_dialog_y;
    }

    ui_dialog_wid = XPCreateWidget(x1, y1, x2, y2,  // dialog coordinates
                                   0,               // initially hide
                                   "EZPushback",    // window title
                                   1,               // root
                                   NULL,            // no container
                                   xpWidgetClass_MainWindow);

    if (ui_dialog_wid == NULL) {
        debug("could not create UI dialog");
        return;
    }

    XPSetWidgetProperty(ui_dialog_wid, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    XPSetWidgetProperty(ui_dialog_wid, xpProperty_MainWindowHasCloseBoxes, 0);

    // dialog text subwindow
    ui_dlg_text_wid = XPCreateWidget(x1, y1, x2, y2,
                                     1,  // show
                                     "", // text (set by set_dialog_text)
                                     0,  // not root
                                     ui_dialog_wid,  // parent is dialog window
                                     xpWidgetClass_Caption);

    if (ui_dlg_text_wid == NULL) {
        debug("could not create UI text widget");
        return;
    }

    XPSetWidgetProperty(ui_dlg_text_wid, xpProperty_CaptionLit, 1);

    int button_width = 100;
    int bx1 = x1 + ((ui_dialog_w - button_width) / 2);
    int bx2 = bx1 + button_width;
    ui_button_wid = XPCreateWidget(bx1, y2+20, bx2, y2+10,
                                   0,   // initially hide
                                   "",  // button text (set by set_button_text)
                                   0,   // not root
                                   ui_dialog_wid,  // parent is dialog window
                                   xpWidgetClass_Button);

    if (ui_button_wid == NULL) {
        debug("could not create UI button widget");
        return;
    }

    XPSetWidgetProperty(ui_button_wid, xpProperty_ButtonType, xpPushButton);

    XPAddWidgetCallback(ui_dialog_wid, ui_dialog_handler);
}


void destroy_dialog()
{
    if (ui_dialog_wid) {
        hide_dialog();
        XPDestroyWidget(ui_dialog_wid, 1); // 1 = destroy child widgets
        ui_dialog_wid   = NULL;
        ui_dlg_text_wid = NULL;
    }
}


void set_menu_active_state()
{
    XPLMEnableMenuItem(ui_menu_id, ui_menu_push_ix, 0);
    XPLMEnableMenuItem(ui_menu_id, ui_menu_tug_ix,  0);
    XPLMEnableMenuItem(ui_menu_id, ui_menu_stop_ix, 1);
}


void set_menu_idle_state()
{
    XPLMEnableMenuItem(ui_menu_id, ui_menu_push_ix, 1);
    XPLMEnableMenuItem(ui_menu_id, ui_menu_tug_ix,  1);
    XPLMEnableMenuItem(ui_menu_id, ui_menu_stop_ix, 0);
}


void menu_handler(void* menu_ref, void* item_ref)
{
    UNUSED(menu_ref);

    intptr_t item_num = (intptr_t)item_ref;

    switch (item_num) {
        case UI_MENU_ITEM_START_PUSH:
            debug("start pushback requested from UI menu");
            start_push();
            break;

        case UI_MENU_ITEM_START_TUG:
            debug("start tug requested from UI menu");
            start_tug();
            break;

        case UI_MENU_ITEM_STOP:
            debug("stop requested from UI menu");
            stop_any();
            break;
    }
}



void create_menu()
{
    ui_menu_main_ix = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "EZPushback", NULL, 1);
    ui_menu_id = XPLMCreateMenu("EZPushback", XPLMFindPluginsMenu(), ui_menu_main_ix, menu_handler, NULL);

    if (ui_menu_id == NULL) {
        debug("could not create UI menu!");
        return;
    }

    ui_menu_push_ix = XPLMAppendMenuItem(ui_menu_id, "Start Pushback (reverse)", (void*) UI_MENU_ITEM_START_PUSH, 1);
    ui_menu_tug_ix  = XPLMAppendMenuItem(ui_menu_id, "Start Tug (forward)",      (void*) UI_MENU_ITEM_START_TUG, 1);
                      XPLMAppendMenuSeparator(ui_menu_id);
    ui_menu_stop_ix = XPLMAppendMenuItem(ui_menu_id, "Stop",                     (void*) UI_MENU_ITEM_STOP, 1);

    set_menu_idle_state();
}


void destroy_menu()
{
    if (ui_menu_id) {
        XPLMDestroyMenu(ui_menu_id);
        ui_menu_id = NULL;
    }
}


void create_ui()
{
    create_dialog();
    create_menu();
    XPLMRegisterFlightLoopCallback(hide_dialog_flcb, 0, NULL);
}


void destroy_ui()
{
    destroy_menu();
    destroy_dialog();
    XPLMUnregisterFlightLoopCallback(hide_dialog_flcb, NULL);
}
