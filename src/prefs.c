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

#define PREF_NO_VALUE -99999

const char* ini_file = "ezpushback.ini";
char        ini_path[256];
int         ini_path_set = 0;


// these preferences should not be changed when we write the prefs file
float pref_acceleration = 0.0f;
float pref_deceleration = 0.0f;
float pref_target_speed = 0.0f;


char* get_ini_path()
{
    if (ini_path_set) {
        return ini_path;
    }

    XPLMGetPluginInfo(XPLMGetMyID(), NULL, ini_path, NULL, NULL);
    debug("plugin path is '%s'", ini_path);
    XPLMExtractFileAndPath(ini_path);
    debug("plugin folder is '%s'", ini_path);

    if (strstr(ini_path, "/64" ) || strstr(ini_path, "/32")  ||
        strstr(ini_path, "\\64") || strstr(ini_path, "\\32")
    ) {
        // store .ini in plugin base folder
        ini_path[strlen(ini_path) - 2] = 0;
        debug("base folder is '%s'", ini_path);
    }

    strcat(ini_path, XPLMGetDirectorySeparator());
    strcat(ini_path, ini_file);
    // debug("ini file '%s'", ini_path);

    #if APL && __MACH__
        debug("converting path from HFS to Unix");
        // convert HFS to Unix path
        char temp[256];
        if (ConvertPath(ini_path, temp, 256) == 0) {
            strcpy(ini_path, temp);
        }
        else {
            debug("could not convert HFS to unix path");
        }
    #endif

    debug("ini file is '%s'", ini_path);
    ini_path_set = 1;
    return ini_path;
}


void read_prefs()
{
    char *path;
    FILE* fp;
    char line[80];
    char *token, *nl;
    int x = PREF_NO_VALUE;
    int y = PREF_NO_VALUE;
    float f;
    int i;
    int prefs = 0;
    int errors = 0;

    pref_acceleration = get_ezpb_acceleration(NULL);
    pref_deceleration = get_ezpb_deceleration(NULL);
    pref_target_speed = get_ezpb_target_speed(NULL);

    path = get_ini_path();
    fp = fopen(path, "r");
    if (fp == NULL) {
        debug("no ini file found at %s (using defaults)", path);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        nl = strrchr(line, '\r');
        if (nl) *nl = '\0';
        nl = strrchr(line, '\n');
        if (nl) *nl = '\0';

        // debug("ini line '%s'", line);
        if (line[0] == '\0' || line[0] == '#') continue;

        token = strtok(line, "= \n");
        if (!token) continue;
        // debug("pref left token=%s", token);

        if (strcmp("x", token) == 0) {
            token = strtok(NULL, "= \n");
            i = atoi(token);
            if (i >= 0 && i < 99999) {
                debug("%s x=%d", ini_file, i);
                x = i;
                prefs++;
            }
            else {
                debug("invalid x='%s' in %s", token, ini_file);
                errors++;
            }
        }
        else if (strcmp("y", token) == 0) {
            token = strtok(NULL, "= \n");
            i = atoi(token);
            if (i >= 0 && i < 99999) {
                debug("%s y=%d", ini_file, i);
                y = i;
                prefs++;
            }
            else {
                debug("invalid y='%s' in %s", token, ini_file);
                errors++;
            }
        }
        else if (strcmp("acceleration", token) == 0) {
            token = strtok(NULL, "= \n");
            f = (float)atof(token);
            if (f > 0.0f) {
                debug("%s acceleration=%f", ini_file, f);
                set_ezpb_acceleration(NULL, f);
                pref_acceleration = f;
                prefs++;
            }
            else {
                debug("invalid acceleration='%s' in %s", token, ini_file);
                errors++;
            }
        }
        else if (strcmp("deceleration", token) == 0) {
            token = strtok(NULL, "= \n");
            f = (float)atof(token);
            if (f > 0.0f) {
                debug("%s deceleration=%f", ini_file, f);
                set_ezpb_deceleration(NULL, f);
                pref_deceleration = f;
                prefs++;
            }
            else {
                debug("invalid acceleration='%s' in %s", token, ini_file);
                errors++;
            }
        }
        else if (strcmp("target_speed", token) == 0) {
            token = strtok(NULL, "= \n");
            f = (float)atof(token);
            if (f > 0.0f) {
                debug("%s target_speed=%f", ini_file, f);
                set_ezpb_target_speed(NULL, f);
                pref_target_speed = f;
                prefs++;
            }
            else {
                debug("invalid target_speed='%s'' in %s", token, ini_file);
                errors++;
            }
        }
        else {
            debug("%s unrecognized setting '%s'", ini_file, token);
            errors++;
        }
    }

    if (x != PREF_NO_VALUE && y != PREF_NO_VALUE) {
        set_dialog_position(x, y);
    }
    else {
        debug("window position not set by %s", ini_file);
    }

    fclose(fp);

    if (errors) {
        debug("%s had %d errors", ini_file, errors);
    }

    debug("%s contained %d usable settings", ini_file, prefs);
    return;
}


void write_prefs()
{
    char *path;
    FILE* fp;

    path = get_ini_path();
    fp = fopen(path, "w");
    if (fp == NULL) {
        debug("could not write %s", path);
        return;
    }

    fprintf(fp,

            "# ezpushback.ini\n"
            "# You can change the values in this file.\n"
            "# This file will be written and reformatted when the plugin saves state.\n"
            "\n"
            "# dialog window position (x=left, y=top)\n"
            "# moving the window in the sim will change x and y upon save\n"
            "x=%d\n"
            "y=%d\n"
            "\n"
            "# acceleration/deceleration in meters per second per second\n"
            "acceleration=%.2f\n"
            "deceleration=%.2f\n"
            "\n"
            "# target speed in meters per second\n"
            "target_speed=%.2f\n",

            get_dialog_x(),
            get_dialog_y(),
            pref_acceleration,
            pref_deceleration,
            pref_target_speed
    );

    fclose(fp);
    debug("prefs written to %s", path);
}


#if APL && __MACH__
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
//  from http://www.xsquawkbox.net/xpsdk/mediawiki/FilePathsAndMacho
int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen)
{
    CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath ,kCFStringEncodingMacRoman);
    if (inStr == NULL)
        return -1;
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0);
    CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
        return -1;
    CFRelease(outStr);
    CFRelease(url);
    CFRelease(inStr);
    return 0;
}
#endif
