/* datarefs.c
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


XPLMDataRef find_dataref(const char* name)
{
    XPLMDataRef dr = XPLMFindDataRef(name);
    if (dr == NULL) {
        debug("could not register dataref %s", name);
    }
    return dr;
}


XPLMDataRef create_dataref_int(const char* name, XPLMGetDatai_f read_fn, XPLMSetDatai_f write_fn)
{
    return XPLMRegisterDataAccessor(name, xplmType_Int, write_fn ? 1 : 0,
                                    read_fn, write_fn,  // int
                                    NULL, NULL,         // float
                                    NULL, NULL,         // double
                                    NULL, NULL,         // int array
                                    NULL, NULL,         // float array
                                    NULL, NULL,         // data
                                    NULL, NULL);        // refcons
}


XPLMDataRef create_dataref_float(const char* name, XPLMGetDataf_f read_fn, XPLMSetDataf_f write_fn)
{
    return XPLMRegisterDataAccessor(name, xplmType_Float, write_fn ? 1 : 0,
                                    NULL, NULL,         // int
                                    read_fn, write_fn,  // float
                                    NULL, NULL,         // double
                                    NULL, NULL,         // int array
                                    NULL, NULL,         // float array
                                    NULL, NULL,         // data
                                    NULL, NULL);        // refcons
}
