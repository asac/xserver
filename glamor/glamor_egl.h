/*
 * Copyright © 2016 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Adam Jackson <ajax@redhat.com>
 */

#ifndef GLAMOR_EGL_H
#define GLAMOR_EGL_H

#define MESA_EGL_NO_X11_HEADERS
#include <epoxy/gl.h>
#include <epoxy/egl.h>

/*
 * Create an EGLDisplay from a native display type. This is a little quirky
 * for a few reasons.
 *
 * 1: GetPlatformDisplayEXT and GetPlatformDisplay are the API you want to
 * use, but have different function signatures in the third argument; this
 * happens not to matter for us, at the moment, but it means epoxy won't alias
 * them together.
 *
 * 2: epoxy 1.3 and earlier don't understand EGL client extensions, which
 * means you can't call "eglGetPlatformDisplayEXT" directly, as the resolver
 * will crash.
 *
 * 3: You can't tell whether you have EGL 1.5 at this point, because
 * eglQueryString(EGL_VERSION) is a property of the display, which we don't
 * have yet. So you have to query for extensions no matter what. Fortunately
 * epoxy_has_egl_extension _does_ let you query for client extensions, so
 * we don't have to write our own extension string parsing.
 */
static inline EGLDisplay
glamor_egl_get_display(EGLint type, void *native)
{
    EGLDisplay dpy = NULL;

    /* If we're EGL 1.5, the KHR extension will be there, try that */
    if (epoxy_has_egl_extension(NULL, "EGL_KHR_platform_base")) {
        PFNEGLGETPLATFORMDISPLAYPROC getPlatformDisplay =
            (void *) eglGetProcAddress("eglGetPlatformDisplay");
        if (getPlatformDisplay)
            dpy = getPlatformDisplay(type, native, NULL);
        if (dpy)
            return dpy;
    }

    /* Okay, maybe the EXT extension works */
    if (epoxy_has_egl_extension(NULL, "EGL_EXT_platform_base")) {
        PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplayEXT =
            (void *) eglGetProcAddress("eglGetPlatformDisplayEXT");
        if (getPlatformDisplayEXT)
            dpy = getPlatformDisplayEXT(type, native, NULL);
        if (dpy)
            return dpy;
    }

    /* Welp, everything is awful. */
    return eglGetDisplay(native);
}

#endif
