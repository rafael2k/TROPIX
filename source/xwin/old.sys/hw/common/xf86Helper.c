/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Helper.c,v 1.156 2006/03/17 02:25:02 dawes Exp $ */

/*
 * Copyright (c) 1997-2006 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright � 2004, 2005 X-Oz Technologies.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *  3. The end-user documentation included with the redistribution,
 *     if any, must include the following acknowledgment: "This product
 *     includes software developed by X-Oz Technologies
 *     (http://www.x-oz.com/)."  Alternately, this acknowledgment may
 *     appear in the software itself, if and wherever such third-party
 *     acknowledgments normally appear.
 *
 *  4. Except as contained in this notice, the name of X-Oz
 *     Technologies shall not be used in advertising or otherwise to
 *     promote the sale, use or other dealings in this Software without
 *     prior written authorization from X-Oz Technologies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL X-OZ TECHNOLOGIES OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Authors: Dirk Hohndel <hohndel@XFree86.Org>
 *          David Dawes <dawes@XFree86.Org>
 *          ... and others
 *
 * This file includes the helper functions that the server provides for
 * different drivers.
 */

#include <X11/X.h>
#include "os.h"
#include "servermd.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "propertyst.h"
#include "gcstruct.h"
#ifndef	TROPIX
#include "loaderProcs.h"
#endif	TROPIX
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "micmap.h"
#include "xf86PciInfo.h"
#include "xf86DDC.h"
#include "xf86Xinput.h"
#include "xf86InPriv.h"
#include "mivalidate.h"
#include "xf86RAC.h"
#include "xf86Bus.h"
#include "xf86Version.h"

/* For xf86GetClocks */
#ifndef	TROPIX
#if defined(CSRG_BASED) || defined(__GNU__)
#define HAS_SETPRIORITY
#include <sys/resource.h>
#endif
#endif	TROPIX

static int xf86ScrnInfoPrivateCount = 0;

#ifdef XFree86LOADER

/* Add a pointer to a new DriverRec to xf86DriverList */

void
xf86AddDriver(DriverPtr driver, ModuleDescPtr module, int flags)
{
    /* Don't add null entries */
    if (!driver)
	return;

    if (xf86DriverList == NULL)
	xf86NumDrivers = 0;

    xf86NumDrivers++;
    xf86DriverList = xnfrealloc(xf86DriverList,
				xf86NumDrivers * sizeof(DriverPtr));
    xf86DriverList[xf86NumDrivers - 1] = xnfalloc(sizeof(DriverRec));
    *xf86DriverList[xf86NumDrivers - 1] = *driver;
    xf86DriverList[xf86NumDrivers - 1]->module = module;
    xf86DriverList[xf86NumDrivers - 1]->refCount = 0;
}
#endif

void
xf86DeleteDriver(int drvIndex)
{
    if (xf86DriverList[drvIndex] &&
	!xf86DriverHasEntities(xf86DriverList[drvIndex])) {
	xf86ClearDriverEntities(xf86DriverList[drvIndex]);
#ifdef XFree86LOADER
	if (xf86DriverList[drvIndex]->module) {
	    UnloadModule(xf86DriverList[drvIndex]->module);
	    xfree(xf86DriverList[drvIndex]);
	}
#endif
	xf86DriverList[drvIndex] = NULL;
    }
}

#ifdef XFree86LOADER
/* Add a pointer to a new InputDriverRec to xf86InputDriverList */

void
xf86AddInputDriver(InputDriverPtr driver, ModuleDescPtr module, int flags)
{
    /* Don't add null entries */
    if (!driver)
	return;

    if (xf86InputDriverList == NULL)
	xf86NumInputDrivers = 0;

    xf86NumInputDrivers++;
    xf86InputDriverList = xnfrealloc(xf86InputDriverList,
				xf86NumInputDrivers * sizeof(InputDriverPtr));
    xf86InputDriverList[xf86NumInputDrivers - 1] =
				xnfalloc(sizeof(InputDriverRec));
    *xf86InputDriverList[xf86NumInputDrivers - 1] = *driver;
    xf86InputDriverList[xf86NumInputDrivers - 1]->module = module;
    xf86InputDriverList[xf86NumInputDrivers - 1]->refCount = 0;
}

void
xf86DeleteInputDriver(int drvIndex)
{
    if (xf86InputDriverList[drvIndex] && xf86InputDriverList[drvIndex]->module)
	UnloadModule(xf86InputDriverList[drvIndex]->module);
    xfree(xf86InputDriverList[drvIndex]);
    xf86InputDriverList[drvIndex] = NULL;
}

void
xf86AddModuleInfo(ModuleInfoPtr info, ModuleDescPtr module)
{
    /* Don't add null entries */
    if (!module)
	return;

    if (xf86ModuleInfoList == NULL)
	xf86NumModuleInfos = 0;

    xf86NumModuleInfos++;
    xf86ModuleInfoList = xnfrealloc(xf86ModuleInfoList,
				    xf86NumModuleInfos * sizeof(ModuleInfoPtr));
    xf86ModuleInfoList[xf86NumModuleInfos - 1] = xnfalloc(sizeof(ModuleInfoRec));
    *xf86ModuleInfoList[xf86NumModuleInfos - 1] = *info;
    xf86ModuleInfoList[xf86NumModuleInfos - 1]->module = module;
    xf86ModuleInfoList[xf86NumModuleInfos - 1]->refCount = 0;
}

void
xf86DeleteModuleInfo(int idx)
{
    if (xf86ModuleInfoList[idx]) {
	if (xf86ModuleInfoList[idx]->module)
	    UnloadModule(xf86ModuleInfoList[idx]->module);
	xfree(xf86ModuleInfoList[idx]);
	xf86ModuleInfoList[idx] = NULL;
    }
}
#endif


/* Allocate a new ScrnInfoRec in xf86Screens */

ScrnInfoPtr
xf86AllocateScreen(DriverPtr drv, int flags)
{
    int i;

    if (xf86Screens == NULL)
	xf86NumScreens = 0;

    i = xf86NumScreens++;
    xf86Screens = xnfrealloc(xf86Screens, xf86NumScreens * sizeof(ScrnInfoPtr));
    xf86Screens[i] = xnfcalloc(sizeof(ScrnInfoRec), 1);
    xf86Screens[i]->scrnIndex = i;	/* Changes when a screen is removed */
    xf86Screens[i]->origIndex = i;	/* This never changes */
    xf86Screens[i]->privates = xnfcalloc(sizeof(DevUnion),
					 xf86ScrnInfoPrivateCount);
    /*
     * EnableDisableFBAccess now gets initialized in InitOutput()
     * xf86Screens[i]->EnableDisableFBAccess = xf86EnableDisableFBAccess;
     */

    xf86Screens[i]->drv = drv;
    drv->refCount++;
#ifdef XFree86LOADER
    xf86Screens[i]->module = DuplicateModule(drv->module, NULL);
#else
    xf86Screens[i]->module = NULL;
#endif
    /*
     * set the initial access state. This will be modified after PreInit.
     * XXX Or should we do it some other place?
     */
    xf86Screens[i]->CurrentAccess = &xf86CurrentAccess;
    xf86Screens[i]->resourceType = MEM_IO;

#ifdef DEBUG
    /* OOps -- What's this ? */
    ErrorF("xf86AllocateScreen - xf86Screens[%d]->pScreen = %p\n",
	   i, xf86Screens[i]->pScreen );
    if ( NULL != xf86Screens[i]->pScreen ) {
      ErrorF("xf86Screens[%d]->pScreen->CreateWindow = %p\n",
	     i, xf86Screens[i]->pScreen->CreateWindow );
    }
#endif

    return xf86Screens[i];
}

/*
 * Remove an entry from xf86Screens.  Ideally it should free all allocated
 * data.  To do this properly may require a driver hook.
 */

void
xf86DeleteScreen(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn;
    int i;

    /* First check if the screen is valid */
    if (xf86NumScreens == 0 || xf86Screens == NULL)
	return;

    if (scrnIndex > xf86NumScreens - 1)
	return;

    if (!(pScrn = xf86Screens[scrnIndex]))
	return;

    /* If a FreeScreen function is defined, call it here */
    if (pScrn->FreeScreen != NULL)
	pScrn->FreeScreen(scrnIndex, 0);

    while (pScrn->modes)
	xf86DeleteMode(&pScrn->modes, pScrn->modes);

    while (pScrn->modePool)
	xf86DeleteMode(&pScrn->modePool, pScrn->modePool);

    xf86OptionListFree(pScrn->options);

#ifdef XFree86LOADER
    if (pScrn->module)
	UnloadModule(pScrn->module);
#endif

    if (pScrn->drv)
	pScrn->drv->refCount--;

    if (pScrn->privates)
	xfree(pScrn->privates);

    xf86ClearEntityListForScreen(scrnIndex);

    xfree(pScrn);

    /* Move the other entries down, updating their scrnIndex fields */

    xf86NumScreens--;

    for (i = scrnIndex; i < xf86NumScreens; i++) {
	xf86Screens[i] = xf86Screens[i + 1];
	xf86Screens[i]->scrnIndex = i;
	/* Also need to take care of the screen layout settings */
    }
}

/*
 * Allocate a private in ScrnInfoRec.
 */

int
xf86AllocateScrnInfoPrivateIndex(void)
{
    int idx, i;
    ScrnInfoPtr pScr;
    DevUnion *nprivs;

    idx = xf86ScrnInfoPrivateCount++;
    for (i = 0; i < xf86NumScreens; i++) {
	pScr = xf86Screens[i];
	nprivs = xnfrealloc(pScr->privates,
			    xf86ScrnInfoPrivateCount * sizeof(DevUnion));
	/* Zero the new private */
	bzero(&nprivs[idx], sizeof(DevUnion));
	pScr->privates = nprivs;
    }
    return idx;
}

/* Allocate a new InputInfoRec and add it to the head xf86InputDevs. */

InputInfoPtr
xf86AllocateInput(InputDriverPtr drv, int flags)
{
    InputInfoPtr new;

    if (!(new = xcalloc(sizeof(InputInfoRec), 1)))
	return NULL;

    new->drv = drv;
    drv->refCount++;
#ifdef XFree86LOADER
    new->module = DuplicateModule(drv->module, NULL);
#else
    new->module = NULL;
#endif
    new->next = xf86InputDevs;
    xf86InputDevs = new;
    return new;
}


/*
 * Remove an entry from xf86InputDevs.  Ideally it should free all allocated
 * data.  To do this properly may require a driver hook.
 */

void
xf86DeleteInput(InputInfoPtr pInp, int flags)
{
    InputInfoPtr p;

    /* First check if the inputdev is valid. */
    if (pInp == NULL)
	return;

#if 0
    /* If a free function is defined, call it here. */
    if (pInp->free)
	pInp->free(pInp, 0);
#endif

#ifdef XFree86LOADER
    if (pInp->module)
	UnloadModule(pInp->module);
#endif

    if (pInp->drv)
	pInp->drv->refCount--;

    if (pInp->private)
	xfree(pInp->private);

    /* Remove the entry from the list. */
    if (pInp == xf86InputDevs)
	xf86InputDevs = pInp->next;
    else {
	p = xf86InputDevs;
	while (p && p->next != pInp)
	    p = p->next;
	if (p)
	    p->next = pInp->next;
	/* Else the entry wasn't in the xf86InputDevs list (ignore this). */
    }
    xfree(pInp);
}

Bool
xf86AddPixFormat(ScrnInfoPtr pScrn, int depth, int bpp, int pad)
{
    int i;

    if (pScrn->numFormats >= MAXFORMATS)
	return FALSE;

    if (bpp <= 0) {
	if (depth == 1)
	    bpp = 1;
	else if (depth <= 8)
	    bpp = 8;
	else if (depth <= 16)
	    bpp = 16;
	else if (depth <= 32)
	    bpp = 32;
	else
	    return FALSE;
    }
    if (pad <= 0)
	pad = BITMAP_SCANLINE_PAD;

    i = pScrn->numFormats++;
    pScrn->formats[i].depth = depth;
    pScrn->formats[i].bitsPerPixel = bpp;
    pScrn->formats[i].scanlinePad = pad;
    return TRUE;
}

/*
 * Set the depth we are using based on (in the following order of preference):
 *  - values given on the command line
 *  - values given in the config file
 *  - values provided by the driver
 *  - an overall default when nothing else is given
 *
 * Also find a Display subsection matching the depth/bpp found.
 *
 * Sets the following ScrnInfoRec fields:
 *     bitsPerPixel, pixmap24, depth, display, imageByteOrder,
 *     bitmapScanlinePad, bitmapScanlineUnit, bitmapBitOrder, numFormats,
 *     formats, fbFormat.
 */

/* Can the screen handle 24 bpp pixmaps */
#define DO_PIX24(f) ((f & Support24bppFb) || \
		     ((f & Support32bppFb) && (f & SupportConvert24to32)))

/* Can the screen handle 32 bpp pixmaps */
#define DO_PIX32(f) ((f & Support32bppFb) || \
		     ((f & Support24bppFb) && (f & SupportConvert32to24)))

/* Does the screen prefer 32bpp fb for 24bpp pixmaps */
#define CHOOSE32FOR24(f) ((f & Support32bppFb) && (f & SupportConvert24to32) \
			  && (f & PreferConvert24to32))

/* Does the screen prefer 24bpp fb for 32bpp pixmaps */
#define CHOOSE24FOR32(f) ((f & Support24bppFb) && (f & SupportConvert32to24) \
			  && (f & PreferConvert32to24))

/* Can the screen handle 32bpp pixmaps for 24bpp fb */
#define DO_PIX32FOR24(f) ((f & Support24bppFb) && (f & SupportConvert32to24))

/* Can the screen handle 24bpp pixmaps for 32bpp fb */
#define DO_PIX24FOR32(f) ((f & Support32bppFb) && (f & SupportConvert24to32))

#ifndef GLOBAL_DEFAULT_DEPTH
#define GLOBAL_DEFAULT_DEPTH 16
#endif

#ifndef GLOBAL_DEFAULT_FBBPP
#define GLOBAL_DEFAULT_FBBPP 16
#endif

Bool
xf86SetDepthBpp(ScrnInfoPtr scrp, int depth, int dummy, int fbbpp,
		int depth24flags)
{
    Pix24Flags pix24 = xf86Info.pixmap24;
    Bool nomatch = FALSE;

    scrp->bitsPerPixel = -1;
    scrp->depth = -1;
    scrp->pixmap24 = Pix24DontCare;
    scrp->bitsPerPixelFrom = X_DEFAULT;
    scrp->depthFrom = X_DEFAULT;

    if (xf86FbBpp > 0) {
	scrp->bitsPerPixel = xf86FbBpp;
	scrp->bitsPerPixelFrom = X_CMDLINE;
    }

    if (xf86Depth > 0) {
	scrp->depth = xf86Depth;
	scrp->depthFrom = X_CMDLINE;
    }

    if (xf86FbBpp < 0 && xf86Depth < 0) {
	if (scrp->confScreen->defaultfbbpp > 0) {
	    scrp->bitsPerPixel = scrp->confScreen->defaultfbbpp;
	    scrp->bitsPerPixelFrom = X_CONFIG;
	}
	if (scrp->confScreen->defaultdepth > 0) {
	    scrp->depth = scrp->confScreen->defaultdepth;
	    scrp->depthFrom = X_CONFIG;
	}

	if (scrp->confScreen->defaultfbbpp <= 0 &&
	    scrp->confScreen->defaultdepth <= 0) {
	    /*
	     * Check for DefaultDepth and DefaultFbBpp options in the
	     * Device sections.
	     */
	    int i;
	    GDevPtr device;
	    Bool found = FALSE;

	    for (i = 0; i < scrp->numEntities; i++) {
		device = xf86GetDevFromEntity(scrp->entityList[i],
					      scrp->entityInstanceList[i]);
		if (device && device->options) {
		    if (xf86FindOption(device->options, "DefaultDepth")) {
			scrp->depth = xf86SetIntOption(device->options,
						       "DefaultDepth", -1);
			scrp->depthFrom = X_CONFIG;
			found = TRUE;
		    }
		    if (xf86FindOption(device->options, "DefaultFbBpp")) {
			scrp->bitsPerPixel = xf86SetIntOption(device->options,
							      "DefaultFbBpp",
							      -1);
			scrp->bitsPerPixelFrom = X_CONFIG;
			found = TRUE;
		    }
		}
		if (found)
		    break;
	    }
	}
    }

    /* If none of these is set, pick a default */
    if (scrp->bitsPerPixel < 0 && scrp->depth < 0) {
	if (fbbpp > 0 || depth > 0) {
	    if (fbbpp > 0)
		scrp->bitsPerPixel = fbbpp;
	    if (depth > 0)
		scrp->depth = depth;
	} else {
	    scrp->bitsPerPixel = GLOBAL_DEFAULT_FBBPP;
	    scrp->depth = GLOBAL_DEFAULT_DEPTH;
	}
    }

    /* If any are not given, determine a default for the others */

    if (scrp->bitsPerPixel < 0) {
	/* The depth must be set */
	if (scrp->depth > -1) {
	    if (scrp->depth == 1)
		scrp->bitsPerPixel = 1;
	    else if (scrp->depth <= 4)
		scrp->bitsPerPixel = 4;
	    else if (scrp->depth <= 8)
		scrp->bitsPerPixel = 8;
	    else if (scrp->depth <= 16)
		scrp->bitsPerPixel = 16;
	    else if (scrp->depth <= 24) {
		/*
		 * Figure out if a choice is possible based on the depth24
		 * and pix24 flags.
		 */
		/* Check pix24 first */
		if (pix24 != Pix24DontCare) {
		    if (pix24 == Pix24Use32) {
			if (DO_PIX32(depth24flags)) {
			    if (CHOOSE24FOR32(depth24flags))
				scrp->bitsPerPixel = 24;
			    else
				scrp->bitsPerPixel = 32;
			} else {
			    nomatch = TRUE;
			}
		    } else if (pix24 == Pix24Use24) {
			if (DO_PIX24(depth24flags)) {
			    if (CHOOSE32FOR24(depth24flags))
				scrp->bitsPerPixel = 32;
			    else
				scrp->bitsPerPixel = 24;
			} else {
			    nomatch = TRUE;
			}
		    }
		} else {
		    if (DO_PIX32(depth24flags)) {
			if (CHOOSE24FOR32(depth24flags))
			    scrp->bitsPerPixel = 24;
			else
			    scrp->bitsPerPixel = 32;
		    } else if (DO_PIX24(depth24flags)) {
			if (CHOOSE32FOR24(depth24flags))
			    scrp->bitsPerPixel = 32;
			else
			    scrp->bitsPerPixel = 24;
		    }
		}
	    } else if (scrp->depth <= 32)
		scrp->bitsPerPixel = 32;
	    else {
		xf86DrvMsg(scrp->scrnIndex, X_ERROR,
			   "Specified depth (%d) is greater than 32\n",
			   scrp->depth);
		return FALSE;
	    }
	} else {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
			"xf86SetDepthBpp: internal error: depth and fbbpp"
			" are both not set\n");
	    return FALSE;
	}
	if (scrp->bitsPerPixel < 0) {
	    if (nomatch)
		xf86DrvMsg(scrp->scrnIndex, X_ERROR,
			"Driver can't support depth 24 pixmap format (%d)\n",
			PIX24TOBPP(pix24));
	    else if ((depth24flags & (Support24bppFb | Support32bppFb)) ==
		     NoDepth24Support)
		xf86DrvMsg(scrp->scrnIndex, X_ERROR,
			"Driver can't support depth 24\n");
	    else
		xf86DrvMsg(scrp->scrnIndex, X_ERROR,
			"Can't find fbbpp for depth 24\n");
	    return FALSE;
	}
	scrp->bitsPerPixelFrom = X_PROBED;
    }

    if (scrp->depth <= 0) {
	/* bitsPerPixel is already set */
	switch (scrp->bitsPerPixel) {
	case 32:
	    scrp->depth = 24;
	    break;
	default:
	    /* 1, 4, 8, 16 and 24 */
	    scrp->depth = scrp->bitsPerPixel;
	    break;
	}
	scrp->depthFrom = X_PROBED;
    }

    /* Sanity checks */
    if (scrp->depth < 1 || scrp->depth > 32) {
	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Specified depth (%d) is not in the range 1-32\n",
		    scrp->depth);
	return FALSE;
    }
    switch (scrp->bitsPerPixel) {
    case 1:
    case 4:
    case 8:
    case 16:
    case 24:
    case 32:
	break;
    default:
	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Specified fbbpp (%d) is not a permitted value\n",
		   scrp->bitsPerPixel);
	return FALSE;
    }
    if (scrp->depth > scrp->bitsPerPixel) {
	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Specified depth (%d) is greater than the fbbpp (%d)\n",
		   scrp->depth, scrp->bitsPerPixel);
	return FALSE;
    }

    /* set scrp->pixmap24 if the driver isn't flexible */
    if (scrp->bitsPerPixel == 24 && !DO_PIX32FOR24(depth24flags)) {
	scrp->pixmap24 = Pix24Use24;
    }
    if (scrp->bitsPerPixel == 32 && !DO_PIX24FOR32(depth24flags)) {
	scrp->pixmap24 = Pix24Use32;
    }

    scrp->display = xf86GetDisplayByMonitorNum(scrp, -1);

    /*
     * Setup defaults for the display-wide attributes the framebuffer will
     * need.  These defaults should eventually be set globally, and not
     * dependent on the screens.
     */
    scrp->imageByteOrder = IMAGE_BYTE_ORDER;
    scrp->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    if (scrp->depth < 8) {
	/* Planar modes need these settings */
	scrp->bitmapScanlineUnit = 8;
	scrp->bitmapBitOrder = MSBFirst;
    } else {
	scrp->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	scrp->bitmapBitOrder = BITMAP_BIT_ORDER;
    }

    /*
     * If an unusual depth is required, add it to scrp->formats.  The formats
     * for the common depths are handled globally in InitOutput
     */
    switch (scrp->depth) {
    case 1:
    case 4:
    case 8:
    case 15:
    case 16:
    case 24:
	/* Common depths.  Nothing to do for them */
	break;
    default:
	if (!xf86AddPixFormat(scrp, scrp->depth, 0, 0)) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		       "Can't add pixmap format for depth %d\n", scrp->depth);
	    return FALSE;
	}
    }

    /* Initialise the framebuffer format for this screen */
    scrp->fbFormat.depth = scrp->depth;
    scrp->fbFormat.bitsPerPixel = scrp->bitsPerPixel;
    scrp->fbFormat.scanlinePad = BITMAP_SCANLINE_PAD;

    return TRUE;
}

/*
 * Print out the selected depth and bpp.
 */
void
xf86PrintDepthBpp(ScrnInfoPtr scrp)
{
    xf86DrvMsg(scrp->scrnIndex, scrp->depthFrom, "Depth %d, ", scrp->depth);
    xf86Msg(scrp->bitsPerPixelFrom, "framebuffer bpp %d\n", scrp->bitsPerPixel);
}

/*
 * xf86SetWeight sets scrp->weight, scrp->mask, scrp->offset, and for depths
 * greater than MAX_PSEUDO_DEPTH also scrp->rgbBits.
 */
Bool
xf86SetWeight(ScrnInfoPtr scrp, rgb weight, rgb mask)
{
    MessageType weightFrom = X_DEFAULT;

    scrp->weight.red = 0;
    scrp->weight.green = 0;
    scrp->weight.blue = 0;

    if (xf86Weight.red > 0 && xf86Weight.green > 0 && xf86Weight.blue > 0) {
	scrp->weight = xf86Weight;
	weightFrom = X_CMDLINE;
    } else if (scrp->display->weight.red > 0 && scrp->display->weight.green > 0
	       && scrp->display->weight.blue > 0) {
	scrp->weight = scrp->display->weight;
	weightFrom = X_CONFIG;
    } else if (weight.red > 0 && weight.green > 0 && weight.blue > 0) {
	scrp->weight = weight;
    } else {
	switch (scrp->depth) {
	case 1:
	case 4:
	case 8:
	    scrp->weight.red = scrp->weight.green =
		scrp->weight.blue = scrp->rgbBits;
	    break;
	case 15:
	    scrp->weight.red = scrp->weight.green = scrp->weight.blue = 5;
	    break;
	case 16:
	    scrp->weight.red = scrp->weight.blue = 5;
	    scrp->weight.green = 6;
	    break;
	case 24:
	    scrp->weight.red = scrp->weight.green = scrp->weight.blue = 8;
	    break;
	case 30:
	    scrp->weight.red = scrp->weight.green = scrp->weight.blue = 10;
	    break;
	}
    }

    if (scrp->weight.red)
	xf86DrvMsg(scrp->scrnIndex, weightFrom, "RGB weight %d%d%d\n",
		   (int)scrp->weight.red, (int)scrp->weight.green,
		   (int)scrp->weight.blue);

    if (scrp->depth > MAX_PSEUDO_DEPTH &&
	(scrp->depth != scrp->weight.red + scrp->weight.green +
			scrp->weight.blue)) {
	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Weight given (%d%d%d) is inconsistent with the "
		   "depth (%d)\n",
		   (int)scrp->weight.red, (int)scrp->weight.green,
		   (int)scrp->weight.blue, scrp->depth);
	return FALSE;
    }
    if (scrp->depth > MAX_PSEUDO_DEPTH && scrp->weight.red) {
	/*
	 * XXX Does this even mean anything for TrueColor visuals?
	 * If not, we shouldn't even be setting it here.  However, this
	 * matches the behaviour of 3.x versions of XFree86.
	 */
	scrp->rgbBits = scrp->weight.red;
	if (scrp->weight.green > scrp->rgbBits)
	    scrp->rgbBits = scrp->weight.green;
	if (scrp->weight.blue > scrp->rgbBits)
	    scrp->rgbBits = scrp->weight.blue;
    }

    /* Set the mask and offsets */
    if (mask.red == 0 || mask.green == 0 || mask.blue == 0) {
	/* Default to a setting common to PC hardware */
	scrp->offset.red = scrp->weight.green + scrp->weight.blue;
	scrp->offset.green = scrp->weight.blue;
	scrp->offset.blue = 0;
	scrp->mask.red = ((1 << scrp->weight.red) - 1) << scrp->offset.red;
	scrp->mask.green = ((1 << scrp->weight.green) - 1)
				<< scrp->offset.green;
	scrp->mask.blue = (1 << scrp->weight.blue) - 1;
    } else {
	/* Initialise to the values passed */
	scrp->mask.red = mask.red;
	scrp->mask.green = mask.green;
	scrp->mask.blue = mask.blue;
	scrp->offset.red = ffs(mask.red) - 1;
	scrp->offset.green = ffs(mask.green) - 1;
	scrp->offset.blue = ffs(mask.blue) - 1;
    }
    return TRUE;
}

Bool
xf86SetDefaultVisual(ScrnInfoPtr scrp, int visual)
{
    MessageType visualFrom = X_DEFAULT;
    Bool bad = FALSE;

    if (defaultColorVisualClass >= 0) {
	scrp->defaultVisual = defaultColorVisualClass;
	visualFrom = X_CMDLINE;
    } else if (scrp->display->defaultVisual >= 0) {
	scrp->defaultVisual = scrp->display->defaultVisual;
	visualFrom = X_CONFIG;
    } else if (visual >= 0) {
	scrp->defaultVisual = visual;
    } else {
	if (scrp->depth == 1)
	    scrp->defaultVisual = StaticGray;
	else if (scrp->depth == 4)
	    scrp->defaultVisual = StaticColor;
	else if (scrp->depth <= MAX_PSEUDO_DEPTH)
	    scrp->defaultVisual = PseudoColor;
	else
	    scrp->defaultVisual = TrueColor;
    }
    switch (scrp->defaultVisual) {
    case StaticGray:
    case GrayScale:
    case StaticColor:
    case PseudoColor:
    case TrueColor:
    case DirectColor:
	xf86DrvMsg(scrp->scrnIndex, visualFrom, "Default visual is %s\n",
		   xf86VisualNames[scrp->defaultVisual]);
	/* Check if the visual is valid for the depth */
	if (scrp->depth == 1 && scrp->defaultVisual != StaticGray)
	   bad = TRUE;
#if 0
	else if (scrp->depth == 4 &&
		 (scrp->defaultVisual == TrueColor ||
		  scrp->defaultVisual == DirectColor))
	   bad = TRUE;
#endif
	else if (scrp->depth > MAX_PSEUDO_DEPTH &&
		 scrp->defaultVisual != TrueColor &&
		 scrp->defaultVisual != DirectColor)
	   bad = TRUE;
	if (bad) {
	    xf86DrvMsg(scrp->scrnIndex, X_ERROR, "Selected default "
		       "visual (%s) is not valid for depth %d\n",
		       xf86VisualNames[scrp->defaultVisual], scrp->depth);
	    return FALSE;
	} else
	    return TRUE;
    default:

	xf86DrvMsg(scrp->scrnIndex, X_ERROR,
		   "Invalid default visual class (%d)\n", scrp->defaultVisual);
	return FALSE;
    }
}

#define TEST_GAMMA(g) \
	(g).red > GAMMA_ZERO || (g).green > GAMMA_ZERO || (g).blue > GAMMA_ZERO

#define SET_GAMMA(g) \
	(g) > GAMMA_ZERO ? (g) : 1.0

Bool
xf86SetGamma(ScrnInfoPtr scrp, Gamma gamma)
{
    MessageType from = X_DEFAULT;
#if 0
    xf86MonPtr DDC = (xf86MonPtr)(scrp->monitor->DDC);
#endif
    if (TEST_GAMMA(xf86Gamma)) {
	from = X_CMDLINE;
	scrp->gamma.red = SET_GAMMA(xf86Gamma.red);
	scrp->gamma.green = SET_GAMMA(xf86Gamma.green);
	scrp->gamma.blue = SET_GAMMA(xf86Gamma.blue);
    } else if (TEST_GAMMA(scrp->monitor->gamma)) {
	from = X_CONFIG;
	scrp->gamma.red = SET_GAMMA(scrp->monitor->gamma.red);
	scrp->gamma.green = SET_GAMMA(scrp->monitor->gamma.green);
	scrp->gamma.blue = SET_GAMMA(scrp->monitor->gamma.blue);
#if 0
    } else if ( DDC && DDC->features.gamma > GAMMA_ZERO ) {
	from = X_PROBED;
	scrp->gamma.red = SET_GAMMA(DDC->features.gamma);
	scrp->gamma.green = SET_GAMMA(DDC->features.gamma);
	scrp->gamma.blue = SET_GAMMA(DDC->features.gamma);
	/* EDID structure version 2 gives optional seperate red, green & blue gamma values
	 * in bytes 0x57-0x59 */
#endif
    } else if (TEST_GAMMA(gamma)) {
	scrp->gamma.red = SET_GAMMA(gamma.red);
	scrp->gamma.green = SET_GAMMA(gamma.green);
	scrp->gamma.blue = SET_GAMMA(gamma.blue);
    } else {
	scrp->gamma.red = 1.0;
	scrp->gamma.green = 1.0;
	scrp->gamma.blue = 1.0;
    }
    xf86DrvMsg(scrp->scrnIndex, from,
	       "Using gamma correction (%.1f, %.1f, %.1f)\n",
	       scrp->gamma.red, scrp->gamma.green, scrp->gamma.blue);

    return TRUE;
}

#undef TEST_GAMMA
#undef SET_GAMMA


/*
 * Set the DPI from the command line option.  XXX should allow it to be
 * calculated from the widthmm/heightmm values.
 */

#undef MMPERINCH
#define MMPERINCH 25.4

void
xf86SetDpi(ScrnInfoPtr pScrn, int x, int y)
{
    MessageType from = X_DEFAULT;
    xf86MonPtr DDC = (xf86MonPtr)(pScrn->monitor->DDC);
    int ddcWidthmm, ddcHeightmm;
    int widthErr, heightErr;

    /* XXX Maybe there is no need for widthmm/heightmm in ScrnInfoRec */
    pScrn->widthmm = pScrn->monitor->widthmm;
    pScrn->heightmm = pScrn->monitor->heightmm;

    if (DDC && (DDC->features.hsize > 0 && DDC->features.vsize > 0) ) {
      /* DDC gives display size in mm for individual modes,
       * but cm for monitor
       */
      ddcWidthmm = DDC->features.hsize * 10; /* 10mm in 1cm */
      ddcHeightmm = DDC->features.vsize * 10; /* 10mm in 1cm */
    } else {
      ddcWidthmm = ddcHeightmm = 0;
    }

    if (monitorResolution > 0) {
	pScrn->xDpi = monitorResolution;
	pScrn->yDpi = monitorResolution;
	from = X_CMDLINE;
    } else if (pScrn->widthmm > 0 || pScrn->heightmm > 0) {
	from = X_CONFIG;
	if (pScrn->widthmm > 0) {
	   pScrn->xDpi =
		(int)((double)pScrn->virtualX * MMPERINCH / pScrn->widthmm);
	}
	if (pScrn->heightmm > 0) {
	   pScrn->yDpi =
		(int)((double)pScrn->virtualY * MMPERINCH / pScrn->heightmm);
	}
	if (pScrn->xDpi > 0 && pScrn->yDpi <= 0)
	    pScrn->yDpi = pScrn->xDpi;
	if (pScrn->yDpi > 0 && pScrn->xDpi <= 0)
	    pScrn->xDpi = pScrn->yDpi;
	xf86DrvMsg(pScrn->scrnIndex, from, "Display dimensions: (%d, %d) mm\n",
		   pScrn->widthmm, pScrn->heightmm);

	/* Warn if config and probe disagree about display size */
	if ( ddcWidthmm && ddcHeightmm ) {
	  if (pScrn->widthmm > 0) {
	    widthErr  = abs(ddcWidthmm  - pScrn->widthmm);
	  } else {
	    widthErr  = 0;
	  }
	  if (pScrn->heightmm > 0) {
	    heightErr = abs(ddcHeightmm - pScrn->heightmm);
	  } else {
	    heightErr = 0;
	  }
	  if (widthErr>10 || heightErr>10) {
	    /* Should include config file name for monitor here */
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Probed monitor is %dx%d mm, using Displaysize %dx%d mm\n",
		       ddcWidthmm,ddcHeightmm, pScrn->widthmm,pScrn->heightmm);
	  }
	}
    } else if ( ddcWidthmm && ddcHeightmm ) {
	from = X_PROBED;
	xf86DrvMsg(pScrn->scrnIndex, from, "Display dimensions: (%d, %d) mm\n",
		   ddcWidthmm, ddcHeightmm );
	pScrn->widthmm = ddcWidthmm;
	pScrn->heightmm = ddcHeightmm;
	if (pScrn->widthmm > 0) {
	   pScrn->xDpi =
		(int)((double)pScrn->virtualX * MMPERINCH / pScrn->widthmm);
	}
	if (pScrn->heightmm > 0) {
	   pScrn->yDpi =
		(int)((double)pScrn->virtualY * MMPERINCH / pScrn->heightmm);
	}
	if (pScrn->xDpi > 0 && pScrn->yDpi <= 0)
	    pScrn->yDpi = pScrn->xDpi;
	if (pScrn->yDpi > 0 && pScrn->xDpi <= 0)
	    pScrn->xDpi = pScrn->yDpi;
    } else {
	if (x > 0)
	    pScrn->xDpi = x;
	else
	    pScrn->xDpi = DEFAULT_DPI;
	if (y > 0)
	    pScrn->yDpi = y;
	else
	    pScrn->yDpi = DEFAULT_DPI;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "DPI set to (%d, %d)\n",
	       pScrn->xDpi, pScrn->yDpi);
}

#undef MMPERINCH


void
xf86SetBlackWhitePixels(ScreenPtr pScreen)
{
    if (xf86FlipPixels) {
	pScreen->whitePixel = 0;
	pScreen->blackPixel = 1;
    } else {
	pScreen->whitePixel = 1;
	pScreen->blackPixel = 0;
    }
}

/*
 * xf86SetRootClip --
 *	Enable or disable rendering to the screen by
 *	setting the root clip list and revalidating
 *	all of the windows
 */

static void
xf86SetRootClip (ScreenPtr pScreen, Bool enable)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    WindowPtr	pChild;
    Bool	WasViewable = (Bool)(pWin->viewable);
    Bool	anyMarked = FALSE;
    RegionPtr	pOldClip = NULL, bsExposed;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr   pLayerWin;
    BoxRec	box;

    if (WasViewable)
    {
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    (void) (*pScreen->MarkOverlappedWindows)(pChild,
						     pChild,
						     &pLayerWin);
	}
	(*pScreen->MarkWindow) (pWin);
	anyMarked = TRUE;
	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr	borderVisible;

		borderVisible = REGION_CREATE(pScreen, NullBox, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				&pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    /*
     * Use REGION_BREAK to avoid optimizations in ValidateTree
     * that assume the root borderClip can't change well, normally
     * it doesn't...)
     */
    if (enable)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pScreen->width;
	box.y2 = pScreen->height;
	REGION_INIT (pScreen, &pWin->winSize, &box, 1);
	REGION_INIT (pScreen, &pWin->borderSize, &box, 1);
	if (WasViewable)
	    REGION_RESET(pScreen, &pWin->borderClip, &box);
	pWin->drawable.width = pScreen->width;
	pWin->drawable.height = pScreen->height;
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    else
    {
	REGION_EMPTY(pScreen, &pWin->borderClip);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }

    ResizeChildrenWinSize (pWin, 0, 0, 0, 0);

    if (WasViewable)
    {
	if (pWin->backStorage)
	{
	    pOldClip = REGION_CREATE(pScreen, NullBox, 1);
	    REGION_COPY(pScreen, pOldClip, &pWin->clipList);
	}

	if (pWin->firstChild)
	{
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
							   pWin->firstChild,
							   (WindowPtr *)NULL);
	}
	else
	{
	    (*pScreen->MarkWindow) (pWin);
	    anyMarked = TRUE;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
    }

    if (pWin->backStorage &&
	((pWin->backingStore == Always) || WasViewable))
    {
	if (!WasViewable)
	    pOldClip = &pWin->clipList; /* a convenient empty region */
	bsExposed = (*pScreen->TranslateBackingStore)
			     (pWin, 0, 0, pOldClip,
			      pWin->drawable.x, pWin->drawable.y);
	if (WasViewable)
	    REGION_DESTROY(pScreen, pOldClip);
	if (bsExposed)
	{
	    RegionPtr	valExposed = NullRegion;

	    if (pWin->valdata)
		valExposed = &pWin->valdata->after.exposed;
	    (*pScreen->WindowExposures) (pWin, valExposed, bsExposed);
	    if (valExposed)
		REGION_EMPTY(pScreen, valExposed);
	    REGION_DESTROY(pScreen, bsExposed);
	}
    }
    if (WasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures)(pWin);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
    FlushAllOutput ();
}

/*
 * Function to enable/disable access to the frame buffer
 *
 * This is used when VT switching and when entering/leaving DGA direct mode.
 *
 * This has been rewritten again to eliminate the saved pixmap.  The
 * devPrivate field in the screen pixmap is set to NULL to catch code
 * accidentally referencing the frame buffer while the X server is not
 * supposed to touch it.
 *
 * Here, we exchange the pixmap private data, rather than the pixmaps
 * themselves to avoid having to find and change any references to the screen
 * pixmap such as GC's, window privates etc.  This also means that this code
 * does not need to know exactly how the pixmap pixels are accessed.  Further,
 * this exchange is >not< done through the screen's ModifyPixmapHeader()
 * vector.  This means the called frame buffer code layers can determine
 * whether they are switched in or out by keeping track of the root pixmap's
 * private data, and therefore don't need to access pScrnInfo->vtSema.
 */
void
xf86EnableDisableFBAccess(int scrnIndex, Bool enable)
{
    ScrnInfoPtr pScrnInfo = xf86Screens[scrnIndex];
    ScreenPtr pScreen = pScrnInfo->pScreen;
    PixmapPtr pspix;

    pspix = (*pScreen->GetScreenPixmap) (pScreen);
    if (enable)
    {
	/*
	 * Restore the screen pixmap devPrivate field
	 */
	pspix->devPrivate = pScrnInfo->pixmapPrivate;
	/*
	 * Restore all of the clip lists on the screen
	 */
	if (!xf86Resetting)
	    xf86SetRootClip (pScreen, TRUE);

    }
    else
    {
	/*
	 * Empty all of the clip lists on the screen
	 */
	xf86SetRootClip (pScreen, FALSE);
	/*
	 * save the screen pixmap devPrivate field and
	 * replace it with NULL so accidental references
	 * to the frame buffer are caught
	 */
	pScrnInfo->pixmapPrivate = pspix->devPrivate;
	pspix->devPrivate.ptr = NULL;
    }
}

/* Print driver messages in the standard format */

void
xf86VDrvMsgVerb(int scrnIndex, MessageType type, int verb, const char *format,
		va_list args)
{
    char *tmpFormat = NULL;

    /* Prefix the scrnIndex name to the format string. */
    if (scrnIndex >= 0 && scrnIndex < xf86NumScreens &&
	xf86Screens[scrnIndex]->name) {
	xasprintf(&tmpFormat, "%s(%d): %s",
		  xf86Screens[scrnIndex]->name, scrnIndex, format);
	if (!tmpFormat)
	    return;

	LogVMessageVerb(type, verb, tmpFormat, args);
	xfree(tmpFormat);
    } else
	LogVMessageVerb(type, verb, format, args);
}

/* Print driver messages, with verbose level specified directly */
void
xf86DrvMsgVerb(int scrnIndex, MessageType type, int verb, const char *format,
	       ...)
{
    va_list ap;

    va_start(ap, format);
    xf86VDrvMsgVerb(scrnIndex, type, verb, format, ap);
    va_end(ap);
}

/* Print driver messages, with verbose level of X_LOG_DEFAULT_VERB. */
void
xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    xf86VDrvMsgVerb(scrnIndex, type, X_LOG_DEFAULT_VERB, format, ap);
    va_end(ap);
}

/* Print non-driver messages with verbose level specified directly */
void
xf86MsgVerb(MessageType type, int verb, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    xf86VDrvMsgVerb(-1, type, verb, format, ap);
    va_end(ap);
}

/* Print non-driver messages with verbose level of 1 (default) */
void
xf86Msg(MessageType type, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    xf86VDrvMsgVerb(-1, type, X_LOG_DEFAULT_VERB, format, ap);
    va_end(ap);
}

/* Just like ErrorF, but with the verbose level checked */
void
xf86ErrorFVerb(int verb, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    if (xf86Verbose >= verb || xf86LogVerbose >= verb)
	LogVWrite(verb, format, ap);
    va_end(ap);
}

/*
 * Like xf86ErrorFVerb, but with an implied verbose level
 * of X_LOG_DEFAULT_VERB.
 */
void
xf86ErrorF(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    if (xf86Verbose >= 1 || xf86LogVerbose >= 1)
	LogVWrite(X_LOG_DEFAULT_VERB, format, ap);
    va_end(ap);
}


void
xf86LogInit()
{
    char *lf = NULL;

#define LOGSUFFIX ".log"
#define LOGOLDSUFFIX ".old"

    /* Get the log file name */
    if (xf86FileCmdline.logFile) {
	lf = xf86FileCmdline.logFile;
	xf86FileCmdline.logFile = (char *)LogInit(lf, LOGOLDSUFFIX);
    } else {
	/* Append the display number and ".log" */
	lf = malloc(strlen(xf86FileDefaults.logFile) + strlen("%s") +
		    strlen(LOGSUFFIX) + 1);
	if (!lf)
	    FatalError("Cannot allocate space for the log file name\n");
	sprintf(lf, "%s%%s" LOGSUFFIX, xf86FileDefaults.logFile);
	xf86FileDefaults.logFile = (char *)LogInit(lf, LOGOLDSUFFIX);
    }
    free(lf);
    xf86LogFileWasOpened = TRUE;

    xf86SetVerbosity(xf86Verbose);
    xf86SetLogVerbosity(xf86LogVerbose);

#undef LOGSUFFIX
#undef LOGOLDSUFFIX
}

void
xf86CloseLog()
{
    LogClose();
}


/*
 * Drivers can use these for using their own SymTabRecs.
 */

const char *
xf86TokenToString(SymTabPtr table, int token)
{
    int i;

    for (i = 0; table[i].token >= 0 && table[i].token != token; i++)
	;

    if (table[i].token < 0)
	return NULL;
    else
	return(table[i].name);
}

int
xf86StringToToken(SymTabPtr table, const char *string)
{
    int i;

    if (string == NULL)
	return -1;

    for (i = 0; table[i].token >= 0 && xf86NameCmp(string, table[i].name); i++)
	;

    return(table[i].token);
}

/*
 * helper to display the clocks found on a card
 */
void
xf86ShowClocks(ScrnInfoPtr scrp, MessageType from)
{
    int j;

    xf86DrvMsg(scrp->scrnIndex, from, "Pixel clocks available:");
    for (j=0; j < scrp->numClocks; j++) {
	if ((j % 4) == 0) {
	    xf86ErrorF("\n");
	    xf86DrvMsg(scrp->scrnIndex, from, "pixel clocks:");
	}
	xf86ErrorF(" %7.3f", (double)scrp->clock[j] / 1000.0);
    }
    xf86ErrorF("\n");
}


/*
 * This prints out the driver identify message, including the names of
 * the supported chipsets.
 *
 * XXX This makes assumptions about the line width, etc.  Maybe we could
 * use a more general "pretty print" function for messages.
 */
void
xf86PrintChipsets(const char *drvname, const char *drvmsg, SymTabPtr chips)
{
    int len, i;

    len = 6 + strlen(drvname) + 2 + strlen(drvmsg) + 2;
    xf86Msg(X_INFO, "%s: %s:", drvname, drvmsg);
    for (i = 0; chips[i].name != NULL; i++) {
	if (i != 0) {
	    xf86ErrorF(",");
	    len++;
	}
	if (len + 2 + strlen(chips[i].name) < 78) {
	    xf86ErrorF(" ");
	    len++;
	} else {
	    xf86ErrorF("\n\t");
	    len = 8;
	}
	xf86ErrorF("%s", chips[i].name);
	len += strlen(chips[i].name);
    }
    xf86ErrorF("\n");
}


#define MAXDRIVERS 64	/* A >hack<, to be sure ... */


int
xf86MatchDevice(const char *drivername, GDevPtr **sectlist)
{
    GDevPtr       gdp, *pgdp = NULL;
    confScreenPtr screensecptr;
    int i,j;

    if (sectlist)
	*sectlist = NULL;

    if (xf86DoProbe) return 1;

    if (xf86DoConfigure && xf86DoConfigurePass1) return 1;

    /*
     * This is a very important function that matches the device sections
     * as they show up in the config file with the drivers that the server
     * loads at run time.
     *
     * ChipProbe can call
     * int xf86MatchDevice(char * drivername, GDevPtr ** sectlist)
     * with its driver name. The function allocates an array of GDevPtr and
     * returns this via sectlist and returns the number of elements in
     * this list as return value. 0 means none found, -1 means fatal error.
     *
     * It can figure out which of the Device sections to use for which card
     * (using things like the Card statement, etc). For single headed servers
     * there will of course be just one such Device section.
     */
    i = 0;

    /*
     * first we need to loop over all the Screens sections to get to all
     * 'active' device sections
     */
    for (j = 0; j < xf86Info.serverLayout->numScreens; j++) {
	if (!xf86Info.serverLayout->screenLayouts[j])
	    continue;
	screensecptr = xf86Info.serverLayout->screenLayouts[j]->screen;
	if (screensecptr && screensecptr->device &&
	    screensecptr->device->driver &&
	    xf86NameCmp(screensecptr->device->driver,drivername) == 0 &&
	    !screensecptr->device->claimed) {
	    /*
	     * we have a matching driver that wasn't claimed, yet
	     */
	    pgdp = xnfrealloc(pgdp, (i + 2) * sizeof(GDevPtr));
	    pgdp[i++] = screensecptr->device;
	}
    }

    /* Then handle the inactive devices */
    if (xf86Info.serverLayout->numInactives > 0 &&
	xf86Info.serverLayout->inactiveDevs) {
	for (j = 0; j < xf86Info.serverLayout->numInactives; j++) {
	    gdp = xf86Info.serverLayout->inactiveDevs[j];
	    if (gdp && gdp->driver && !gdp->claimed &&
		!xf86NameCmp(gdp->driver,drivername)) {
		/* we have a matching driver that wasn't claimed yet */
		pgdp = xnfrealloc(pgdp, (i + 2) * sizeof(GDevPtr));
		pgdp[i++] = gdp;
	    }
	    j++;
	}
    }

    /*
     * make the array NULL terminated and return its address
     */
    if (i)
	pgdp[i] = NULL;

    if (sectlist)
	*sectlist = pgdp;
    else
	xfree(pgdp);
    return i;
}

struct Inst {
    pciVideoPtr	pci;
    GDevPtr		dev;
    Bool		foundHW;  /* PCIid in list of supported chipsets */
    Bool		claimed;  /* BusID matches with a device section */
    int 		chip;
    int 		screen;
};

int
xf86MatchPciInstances(const char *driverName, int vendorID,
		      SymTabPtr chipsets, PciChipsets *PCIchipsets,
		      GDevPtr *devList, int numDevs, DriverPtr drvp,
		      int **foundEntities)
{
    int i,j;
    MessageType from;
    pciVideoPtr pPci, *ppPci;
    struct Inst {
	pciVideoPtr	pci;
	GDevPtr		dev;
	Bool		foundHW;  /* PCIid in list of supported chipsets */
	Bool		claimed;  /* BusID matches with a device section */
	int             chip;
	int		screen;
    } *instances = NULL;
    int numClaimedInstances = 0;
    int allocatedInstances = 0;
    int numFound = 0;
    SymTabRec *c;
    PciChipsets *id;
    GDevPtr devBus = NULL;
    GDevPtr dev = NULL;
    int *retEntities = NULL;

    *foundEntities = NULL;
    if (xf86PciVideoInfo == NULL)
	return 0;

    for (ppPci = xf86PciVideoInfo; (pPci = *ppPci); ppPci++) {
	/* Two or more different drivers may not share entities */
	if (!xf86CheckPciSlot(pPci->bus, pPci->device, pPci->func))
	    continue;

	if (vendorID == 0) {
	    Bool foundVendor = FALSE;
	    for (id = PCIchipsets; id->PCIid != -1; id++) {
		if ( (((id->PCIid & 0xFFFF0000) >> 16) == pPci->vendor)) {
		    if (!foundVendor) {
			++allocatedInstances;
			instances = xnfrealloc(instances,
				     allocatedInstances * sizeof(struct Inst));
			instances[allocatedInstances - 1].pci = pPci;
			instances[allocatedInstances - 1].dev = NULL;
			instances[allocatedInstances - 1].claimed = FALSE;
			instances[allocatedInstances - 1].foundHW = FALSE;
			instances[allocatedInstances - 1].screen = 0;
			foundVendor = TRUE;
		    }
		    if ((id->PCIid & 0x0000FFFF) == pPci->chipType) {
		       instances[allocatedInstances - 1].foundHW = TRUE;
		       instances[allocatedInstances - 1].chip = id->numChipset;
		       numFound++;
		    }
		}
	    }
	} else if (vendorID == PCI_VENDOR_GENERIC) {
	    for (id = PCIchipsets; id->PCIid != -1; id++) {
		if (id->PCIid == xf86CheckPciGAType(pPci)) {
		    ++allocatedInstances;
		    instances = xnfrealloc(instances,
				  allocatedInstances * sizeof(struct Inst));
		    instances[allocatedInstances - 1].pci = pPci;
		    instances[allocatedInstances - 1].dev = NULL;
		    instances[allocatedInstances - 1].claimed = FALSE;
		    instances[allocatedInstances - 1].foundHW = TRUE;
		    instances[allocatedInstances - 1].chip = id->numChipset;
		    instances[allocatedInstances - 1].screen = 0;
		    numFound++;
		}
	    }
	} else {
	    /* Find PCI devices that match the given vendor ID */
	    if (pPci->vendor == vendorID) {
		++allocatedInstances;
		instances = xnfrealloc(instances,
			      allocatedInstances * sizeof(struct Inst));
		instances[allocatedInstances - 1].pci = pPci;
		instances[allocatedInstances - 1].dev = NULL;
		instances[allocatedInstances - 1].claimed = FALSE;
		instances[allocatedInstances - 1].foundHW = FALSE;
		instances[allocatedInstances - 1].screen = 0;

		/* Check if the chip type is listed in the chipsets table */
		for (id = PCIchipsets; id->PCIid != -1; id++) {
		    if (id->PCIid == pPci->chipType) {
			instances[allocatedInstances - 1].chip
			    = id->numChipset;
			instances[allocatedInstances - 1].foundHW = TRUE;
			numFound++;
			break;
		    }
		}
	    }
	}
    }

    /*
     * This may be debatable, but if no PCI devices with a matching vendor
     * type is found, return zero now.  It is probably not desirable to
     * allow the config file to override this.
     */
    if (allocatedInstances <= 0) {
	xfree(instances);
	return 0;
    }

    if (xf86DoProbe) {
	xfree(instances);
	return numFound;
    }

    if (xf86DoConfigure && xf86DoConfigurePass1) {
	GDevPtr pGDev;
	int actualcards = 0;
	for (i = 0; i < allocatedInstances; i++) {
	    pPci = instances[i].pci;
	    if (instances[i].foundHW) {
		if (!xf86CheckPciSlot(pPci->bus, pPci->device, pPci->func))
		    continue;
		actualcards++;
		pGDev = xf86AddDeviceToConfigure(drvp->driverName,
						 instances[i].pci, -1);
		if (pGDev) {
		   /*
		    * XF86Match???Instances() treat chipID and chipRev as
		    * overrides, so clobber them here.
		    */
		   pGDev->chipID = pGDev->chipRev = -1;
		}
	    }
	}
	xfree(instances);
	return actualcards;
    }

#ifdef DEBUG
    ErrorF("%s instances found: %d\n", driverName, allocatedInstances);
#endif

   /*
    * Check for devices that need duplicated instances.  This is required
    * when there is more than one screen per entity.
    *
    * XXX This currently doesn't work for cases where the BusID isn't
    * specified explicitly in the config file.
    */

    for (j = 0; j < numDevs; j++) {
	if (devList[j]->screen > 0 && devList[j]->busID
	    && *devList[j]->busID) {
	    for (i = 0; i < allocatedInstances; i++) {
		pPci = instances[i].pci;
		if (xf86ComparePciBusString(devList[j]->busID, pPci->bus,
					    pPci->device,
					    pPci->func)) {
		    allocatedInstances++;
		    instances = xnfrealloc(instances,
					   allocatedInstances *
					   sizeof(struct Inst));
		    instances[allocatedInstances - 1] = instances[i];
		    instances[allocatedInstances - 1].screen =
						devList[j]->screen;
		    numFound++;
		    break;
		}
	    }
	}
    }

    for (i = 0; i < allocatedInstances; i++) {
	pPci = instances[i].pci;
	devBus = NULL;
	dev = NULL;
	for (j = 0; j < numDevs; j++) {
	    if (devList[j]->busID && *devList[j]->busID) {
		if (xf86ComparePciBusString(devList[j]->busID, pPci->bus,
					   pPci->device,
					   pPci->func) &&
		    devList[j]->screen == instances[i].screen) {

		    if (devBus)
			xf86MsgVerb(X_WARNING,0,
			    "%s: More than one matching Device section for "
			    "instances\n\t(BusID: %s) found: %s\n",
			    driverName, devList[j]->busID,
			    devList[j]->identifier);
		    else
			devBus = devList[j];
		}
	    } else {
		/*
		 * if device section without BusID is found
		 * only assign to it to the primary device.
		 */
		if (xf86IsPrimaryPci(pPci)) {
		    xf86Msg(X_PROBED, "Assigning device section with no busID"
			    " to primary device\n");
		    if (dev || devBus)
			xf86MsgVerb(X_WARNING, 0,
			    "%s: More than one matching Device section "
			    "found: %s\n", driverName, devList[j]->identifier);
		    else
			dev = devList[j];
		}
	    }
	}
	if (devBus) dev = devBus;  /* busID preferred */
	if (!dev) {
	    if (xf86CheckPciSlot(pPci->bus, pPci->device, pPci->func)) {
		xf86MsgVerb(X_WARNING, 0, "%s: No matching Device section "
			    "for instance (BusID PCI:%i:%i:%i) found\n",
			    driverName, pPci->bus, pPci->device, pPci->func);
	    }
	} else {
	    numClaimedInstances++;
	    instances[i].claimed = TRUE;
	    instances[i].dev = dev;
	}
    }
#ifdef DEBUG
    ErrorF("%s instances found: %d\n", driverName, numClaimedInstances);
#endif
    /*
     * Now check that a chipset or chipID override in the device section
     * is valid.  Chipset has precedence over chipID.
     * If chipset is not valid ignore BusSlot completely.
     */
    for (i = 0; i < allocatedInstances && numClaimedInstances > 0; i++) {
	if (!instances[i].claimed) {
	    continue;
	}
	from = X_PROBED;
	if (instances[i].dev->chipset) {
	    for (c = chipsets; c->token >= 0; c++) {
		if (xf86NameCmp(c->name, instances[i].dev->chipset) == 0)
		    break;
	    }
	    if (c->token == -1) {
		instances[i].claimed = FALSE;
		numClaimedInstances--;
		xf86MsgVerb(X_WARNING, 0, "%s: Chipset \"%s\" in Device "
			    "section \"%s\" isn't valid for this driver\n",
			    driverName, instances[i].dev->chipset,
			    instances[i].dev->identifier);
	    } else {
		instances[i].chip = c->token;

		for (id = PCIchipsets; id->numChipset >= 0; id++) {
		    if (id->numChipset == instances[i].chip)
			break;
		}
		if(id->numChipset >=0){
		    xf86Msg(X_CONFIG,"Chipset override: %s\n",
			     instances[i].dev->chipset);
		    from = X_CONFIG;
		} else {
		    instances[i].claimed = FALSE;
		    numClaimedInstances--;
		    xf86MsgVerb(X_WARNING, 0, "%s: Chipset \"%s\" in Device "
				"section \"%s\" isn't a valid PCI chipset\n",
				driverName, instances[i].dev->chipset,
				instances[i].dev->identifier);
		}
	    }
	} else if (instances[i].dev->chipID > 0) {
	    for (id = PCIchipsets; id->numChipset >= 0; id++) {
		if (id->PCIid == instances[i].dev->chipID)
		    break;
	    }
	    if (id->numChipset == -1) {
		instances[i].claimed = FALSE;
		numClaimedInstances--;
		xf86MsgVerb(X_WARNING, 0, "%s: ChipID 0x%04X in Device "
			    "section \"%s\" isn't valid for this driver\n",
			    driverName, instances[i].dev->chipID,
			    instances[i].dev->identifier);
	    } else {
		instances[i].chip = id->numChipset;

		xf86Msg( X_CONFIG,"ChipID override: 0x%04X\n",
			 instances[i].dev->chipID);
		from = X_CONFIG;
	    }
	} else if (!instances[i].foundHW) {
	    /*
	     * This means that there was no override and the PCI chipType
	     * doesn't match one that is supported
	     */
	    instances[i].claimed = FALSE;
	    numClaimedInstances--;
	}
	if (instances[i].claimed == TRUE){
	    for (c = chipsets; c->token >= 0; c++) {
		if (c->token == instances[i].chip)
		    break;
	    }
	    xf86Msg(from,"Chipset %s found\n",
		    c->name);
	}
    }

    /*
     * Of the claimed instances, check that another driver hasn't already
     * claimed its slot.
     */
    numFound = 0;
    for (i = 0; i < allocatedInstances && numClaimedInstances > 0; i++) {

	if (!instances[i].claimed)
	    continue;
	pPci = instances[i].pci;


	/*
	 * Allow the same entity to be used more than once for devices with
	 * multiple screens per entity.  This assumes implicitly that there
	 * will be a screen == 0 instance.
	 *
	 * XXX Need to make sure that two different drivers don't claim
	 * the same screen > 0 instance.
	 */
	if (instances[i].screen == 0 &&
	    !xf86CheckPciSlot(pPci->bus, pPci->device, pPci->func))
	    continue;

#ifdef DEBUG
	ErrorF("%s: card at %d:%d:%d is claimed by a Device section\n",
	       driverName, pPci->bus, pPci->device, pPci->func);
#endif

	/* Allocate an entry in the lists to be returned */
	numFound++;
	retEntities = xnfrealloc(retEntities, numFound * sizeof(int));
	retEntities[numFound - 1]
	    = xf86ClaimPciSlot(pPci->bus, pPci->device,
			       pPci->func,drvp,	instances[i].chip,
			       instances[i].dev,instances[i].dev->active ?
			       TRUE : FALSE);
	if (retEntities[numFound - 1] == -1 && instances[i].screen > 0) {
	    for (j = 0; j < xf86NumEntities; j++) {
		EntityPtr pEnt = xf86Entities[j];
		if (pEnt->busType != BUS_PCI)
		    continue;
		if (pEnt->pciBusId.bus == pPci->bus &&
		    pEnt->pciBusId.device == pPci->device &&
		    pEnt->pciBusId.func == pPci->func) {
		    retEntities[numFound - 1] = j;
		    xf86AddDevToEntity(j, instances[i].dev);
		    break;
		}
	    }
	}
    }
    xfree(instances);
    if (numFound > 0) {
	*foundEntities = retEntities;
    }

    return numFound;
}

int
xf86MatchIsaInstances(const char *driverName, SymTabPtr chipsets,
		      IsaChipsets *ISAchipsets, DriverPtr drvp,
		      FindIsaDevProc FindIsaDevice, GDevPtr *devList,
		      int numDevs, int **foundEntities)
{
    SymTabRec *c;
    IsaChipsets *Chips;
    int i;
    int numFound = 0;
    int foundChip = -1;
    int *retEntities = NULL;

    *foundEntities = NULL;

#if defined(__sparc__) || defined(__powerpc__)
    FindIsaDevice = NULL;	/* Temporary */
#endif

    if (xf86DoProbe || (xf86DoConfigure && xf86DoConfigurePass1)) {
	if (FindIsaDevice &&
	    ((foundChip = (*FindIsaDevice)(NULL)) != -1)) {
	    xf86AddDeviceToConfigure(drvp->driverName, NULL, foundChip);
	    return 1;
	}
	return 0;
    }

    for (i = 0; i < numDevs; i++) {
	MessageType from = X_CONFIG;
	GDevPtr dev = NULL;
	GDevPtr devBus = NULL;

	if (devList[i]->busID && *devList[i]->busID) {
	    if (xf86ParseIsaBusString(devList[i]->busID)) {
		if (devBus) xf86MsgVerb(X_WARNING,0,
					"%s: More than one matching Device "
					"section for ISA-Bus found: %s\n",
					driverName,devList[i]->identifier);
		else devBus = devList[i];
	    }
	} else {
	    if (xf86IsPrimaryIsa()) {
		if (dev) xf86MsgVerb(X_WARNING,0,
				     "%s: More than one matching "
				     "Device section found: %s\n",
				     driverName,devList[i]->identifier);
		else dev = devList[i];
	    }
	}
	if (devBus) dev = devBus;
	if (dev) {
	    if (dev->chipset) {
		for (c = chipsets; c->token >= 0; c++) {
		    if (xf86NameCmp(c->name, dev->chipset) == 0)
			break;
		}
		if (c->token == -1) {
		    xf86MsgVerb(X_WARNING, 0, "%s: Chipset \"%s\" in Device "
				"section \"%s\" isn't valid for this driver\n",
				driverName, dev->chipset,
				dev->identifier);
		} else
		    foundChip = c->token;
	    } else {
		if (FindIsaDevice) foundChip = (*FindIsaDevice)(dev);
							/* Probe it */
		from = X_PROBED;
	    }
	}

	/* Check if the chip type is listed in the chipset table - for sanity*/

	if (foundChip >= 0){
	    for (Chips = ISAchipsets; Chips->numChipset >= 0; Chips++) {
		if (Chips->numChipset == foundChip)
		    break;
	    }
	    if (Chips->numChipset == -1){
		foundChip = -1;
		xf86MsgVerb(X_WARNING,0,
			    "%s: Driver detected unknown ISA-Bus Chipset\n",
			    driverName);
	    }
	}
	if (foundChip != -1) {
	    numFound++;
	    retEntities = xnfrealloc(retEntities,numFound * sizeof(int));
	    retEntities[numFound - 1] =
	    xf86ClaimIsaSlot(drvp,foundChip,dev, dev->active ? TRUE : FALSE);
	    for (c = chipsets; c->token >= 0; c++) {
		if (c->token == foundChip)
		    break;
	    }
	    xf86Msg(from, "Chipset %s found\n", c->name);
	}
    }
    *foundEntities = retEntities;

    return numFound;
}

/*
 * xf86GetClocks -- get the dot-clocks via a BIG BAD hack ...
 */
void
xf86GetClocks(ScrnInfoPtr pScrn, int num, Bool (*ClockFunc)(ScrnInfoPtr, int),
	      void (*ProtectRegs)(ScrnInfoPtr, Bool),
	      void (*BlankScreen)(ScrnInfoPtr, Bool), IOADDRESS vertsyncreg,
	      int maskval, int knownclkindex, int knownclkvalue)
{
    register int status = vertsyncreg;
    unsigned long i, cnt, rcnt, sync;

    /* First save registers that get written on */
    (*ClockFunc)(pScrn, CLK_REG_SAVE);

    xf86SetPriority(TRUE);

    if (num > MAXCLOCKS)
	num = MAXCLOCKS;

    for (i = 0; i < num; i++)
    {
	if (ProtectRegs)
	    (*ProtectRegs)(pScrn, TRUE);
	if (!(*ClockFunc)(pScrn, i))
	{
	    pScrn->clock[i] = -1;
	    continue;
	}
	if (ProtectRegs)
	    (*ProtectRegs)(pScrn, FALSE);
	if (BlankScreen)
	    (*BlankScreen)(pScrn, FALSE);

	usleep(50000);     /* let VCO stabilise */

	cnt  = 0;
	sync = 200000;

	/* XXX How critical is this? */
	if (!xf86DisableInterrupts())
	{
	    (*ClockFunc)(pScrn, CLK_REG_RESTORE);
	    ErrorF("Failed to disable interrupts during clock probe.  If\n");
	    ErrorF("your OS does not support disabling interrupts, then you\n");
	    FatalError("must specify a Clocks line in the XF86Config file.\n");
	}
	while ((inb(status) & maskval) == 0x00)
	    if (sync-- == 0) goto finish;
	/* Something appears to be happening, so reset sync count */
	sync = 200000;
	while ((inb(status) & maskval) == maskval)
	    if (sync-- == 0) goto finish;
	/* Something appears to be happening, so reset sync count */
	sync = 200000;
	while ((inb(status) & maskval) == 0x00)
	    if (sync-- == 0) goto finish;

	for (rcnt = 0; rcnt < 5; rcnt++)
	{
	    while (!(inb(status) & maskval))
		cnt++;
	    while ((inb(status) & maskval))
		cnt++;
	}

finish:
	xf86EnableInterrupts();

	pScrn->clock[i] = cnt ? cnt : -1;
	if (BlankScreen)
	    (*BlankScreen)(pScrn, TRUE);
    }

    xf86SetPriority(FALSE);

    for (i = 0; i < num; i++)
    {
	if (i != knownclkindex)
	{
	    if (pScrn->clock[i] == -1)
	    {
		pScrn->clock[i] = 0;
	    }
	    else
	    {
		pScrn->clock[i] = (int)(0.5 +
		    (((float)knownclkvalue) * pScrn->clock[knownclkindex]) /
		    (pScrn->clock[i]));
		/* Round to nearest 10KHz */
		pScrn->clock[i] += 5;
		pScrn->clock[i] /= 10;
		pScrn->clock[i] *= 10;
	    }
	}
    }

    pScrn->clock[knownclkindex] = knownclkvalue;
    pScrn->numClocks = num;

    /* Restore registers that were written on */
    (*ClockFunc)(pScrn, CLK_REG_RESTORE);
}

void
xf86SetPriority(Bool up)
{
    static int saved_nice;

    if (up) {
#ifdef HAS_SETPRIORITY
	saved_nice = getpriority(PRIO_PROCESS, 0);
	setpriority(PRIO_PROCESS, 0, -20);
#endif
#if defined(SYSV) || defined(SVR4) || defined(linux)
	saved_nice = nice(0);
	nice(-20 - saved_nice);
#endif
    } else {
#ifdef HAS_SETPRIORITY
	setpriority(PRIO_PROCESS, 0, saved_nice);
#endif
#if defined(SYSV) || defined(SVR4) || defined(linux)
	nice(20 + saved_nice);
#endif
    }
}

const char *
xf86GetVisualName(int visual)
{
    if (visual < 0 || visual > DirectColor)
	return NULL;

    return xf86VisualNames[visual];
}


int
xf86GetVerbosity()
{
    return max(xf86Verbose, xf86LogVerbose);
}

Pix24Flags
xf86GetPix24()
{
    return xf86Info.pixmap24;
}


int
xf86GetDepth()
{
    return xf86Depth;
}


rgb
xf86GetWeight()
{
    return xf86Weight;
}


Gamma
xf86GetGamma()
{
    return xf86Gamma;
}


Bool
xf86GetFlipPixels()
{
    return xf86FlipPixels;
}


const char *
xf86GetServerName()
{
    return xf86ServerName;
}


Bool
xf86ServerIsExiting()
{
    return (dispatchException & DE_TERMINATE) == DE_TERMINATE;
}


Bool
xf86ServerIsResetting()
{
    return xf86Resetting;
}


Bool
xf86ServerIsInitialising()
{
    return xf86Initialising;
}


Bool
xf86ServerIsOnlyDetecting(void)
{
    return xf86DoProbe || xf86DoConfigure;
}


Bool
xf86ServerIsOnlyProbing(void)
{
    return xf86ProbeOnly;
}


Bool
xf86CaughtSignal()
{
    return xf86Info.caughtSignal;
}


Bool
xf86GetVidModeAllowNonLocal()
{
    return xf86Info.vidModeAllowNonLocal;
}


Bool
xf86GetVidModeEnabled()
{
    return xf86Info.vidModeEnabled;
}

Bool
xf86GetModInDevAllowNonLocal()
{
    return xf86Info.miscModInDevAllowNonLocal;
}


Bool
xf86GetModInDevEnabled()
{
    return xf86Info.miscModInDevEnabled;
}


Bool
xf86GetAllowMouseOpenFail()
{
    return xf86Info.allowMouseOpenFail;
}


Bool
xf86IsPc98()
{
#if defined(i386) || defined(__i386__)
    return xf86Info.pc98;
#else
    return FALSE;
#endif
}

void
xf86DisableRandR()
{
    xf86Info.disableRandR = TRUE;
    xf86Info.randRFrom = X_PROBED;
}

CARD32
xf86GetVersion()
{
    return XF86_VERSION_CURRENT;
}

CARD32
xf86GetModuleVersion(ModuleDescPtr module)
{
#ifdef XFree86LOADER
    return (CARD32)LoaderGetModuleVersion(module);
#else
    return 0;
#endif
}

ModuleDescPtr
xf86GetSubModuleByName(ModuleDescPtr mod, const char *name)
{
#ifdef XFree86LOADER
    return LoaderGetSubModuleByName(mod, name);
#else
    return NULL;
#endif
}

void
xf86SetParentModuleRequirements(ModuleDescPtr module, XF86ModReqInfo *req)
{
#ifdef XFree86LOADER
    LoaderSetParentModuleRequirements(module, req);
#endif
}

ModuleDescPtr
xf86LoadDrvSubModule(DriverPtr drv, const char *name)
{
#ifdef XFree86LOADER
    ModuleDescPtr ret;
    int errmaj = 0, errmin = 0;

    ret = LoadSubModule(drv->module, name, NULL, NULL, NULL, NULL,
			&errmaj, &errmin);
    if (!ret)
	LoaderErrorMsg(NULL, name, errmaj, errmin);
    return ret;
#else
    return (pointer)1;
#endif
}

ModuleDescPtr
xf86LoadDrvSubModuleWithRequirements(DriverPtr drv, const char *name,
				     XF86ModReqInfo *req)
{
#ifdef XFree86LOADER
    ModuleDescPtr ret;
    int errmaj = 0, errmin = 0;

    ret = LoadSubModule(drv->module, name, NULL, NULL, NULL, req,
			&errmaj, &errmin);
    if (!ret)
	LoaderErrorMsg(NULL, name, errmaj, errmin);
    return ret;
#else
    return (pointer)1;
#endif
}

ModuleDescPtr
xf86LoadSubModule(ScrnInfoPtr pScrn, const char *name)
{
#ifdef XFree86LOADER
    ModuleDescPtr ret;
    int errmaj = 0, errmin = 0;

    ret = LoadSubModule(pScrn->module, name, NULL, NULL, NULL, NULL,
			&errmaj, &errmin);
    if (!ret)
	LoaderErrorMsg(pScrn->name, name, errmaj, errmin);
    return ret;
#else
    return (pointer)1;
#endif
}

ModuleDescPtr
xf86LoadSubModuleWithRequirements(ScrnInfoPtr pScrn, const char *name,
				  XF86ModReqInfo *req)
{
#ifdef XFree86LOADER
    ModuleDescPtr ret;
    int errmaj = 0, errmin = 0;

    ret = LoadSubModule(pScrn->module, name, NULL, NULL, NULL, req,
			&errmaj, &errmin);
    if (!ret)
	LoaderErrorMsg(pScrn->name, name, errmaj, errmin);
    return ret;
#else
    return (pointer)1;
#endif
}

/*
 * xf86LoadOneModule loads a single module.
 */
ModuleDescPtr
xf86LoadOneModule(char *name, pointer opt)
{
#ifdef XFree86LOADER
    int errmaj, errmin;
#endif
    char *Name;
    ModuleDescPtr mod;

    if (!name)
	return NULL;

#ifndef NORMALISE_MODULE_NAME
    Name = xstrdup(name);
#else
    /* Normalise the module name */
    Name = xf86NormalizeName(name);
#endif

    /* Skip empty names */
    if (Name == NULL)
	return NULL;
    if (*Name == '\0') {
	xfree(Name);
	return NULL;
    }

#ifdef XFree86LOADER
    mod = LoadModule(Name, NULL, NULL, NULL, opt, NULL, &errmaj, &errmin);
    if (!mod)
	LoaderErrorMsg(NULL, Name, errmaj, errmin);
#else
    mod = (pointer)1;
#endif
    xfree(Name);
    return mod;
}

void
xf86UnloadSubModule(ModuleDescPtr mod)
{
#if defined(XFree86LOADER)
    UnloadSubModule(mod);
#endif
}

Bool
xf86LoaderCheckSymbol(const char *name)
{
#ifdef XFree86LOADER
    return LoaderSymbol(name) != NULL;
#else
    return TRUE;
#endif
}

void
xf86LoaderReqSymLists(const char **list0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, list0);
    LoaderVReqSymLists(NULL, list0, ap);
    va_end(ap);
#endif
}

void
xf86LoaderReqSymbols(const char *sym0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, sym0);
    LoaderVReqSymbols(NULL, sym0, ap);
    va_end(ap);
#endif
}

void
xf86LoaderRefSymLists(const char **list0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, list0);
    LoaderVRefSymLists(NULL, list0, ap);
    va_end(ap);
#endif
}

void
xf86LoaderRefSymbols(const char *sym0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, sym0);
    LoaderVRefSymbols(NULL, sym0, ap);
    va_end(ap);
#endif
}

int
xf86LoaderModReqSymLists(ModuleDescPtr module, const char **list0, ...)
{
    int ret = 0;
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, list0);
    ret = LoaderVReqSymLists(module, list0, ap);
    va_end(ap);
#endif
    return ret;
}

int
xf86LoaderModReqSymbols(ModuleDescPtr module, const char *sym0, ...)
{
    int ret = 0;
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, sym0);
    ret = LoaderVReqSymbols(module, sym0, ap);
    va_end(ap);
#endif
    return ret;
}

void
xf86LoaderModRefSymLists(ModuleDescPtr module, const char **list0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, list0);
    LoaderVRefSymLists(module, list0, ap);
    va_end(ap);
#endif
}

void
xf86LoaderModRefSymbols(ModuleDescPtr module, const char *sym0, ...)
{
#ifdef XFree86LOADER
    va_list ap;

    va_start(ap, sym0);
    LoaderVRefSymbols(module, sym0, ap);
    va_end(ap);
#endif
}


typedef enum {
   OPTION_BACKING_STORE
} BSOpts;

static const OptionInfoRec BSOptions[] = {
   { OPTION_BACKING_STORE, "BackingStore", OPTV_BOOLEAN, {0}, FALSE },
   { -1,                   NULL,           OPTV_NONE,    {0}, FALSE }
};

void
xf86SetBackingStore(ScreenPtr pScreen)
{
    Bool useBS = FALSE;
    MessageType from = X_DEFAULT;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    OptionInfoPtr options;

    options = xnfalloc(sizeof(BSOptions));
    (void)memcpy(options, BSOptions, sizeof(BSOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

    /* check for commandline option here */
    if (xf86bsEnableFlag) {
	from = X_CMDLINE;
	useBS = TRUE;
    } else if (xf86bsDisableFlag) {
	from = X_CMDLINE;
	useBS = FALSE;
    } else {
	if (xf86GetOptValBool(options, OPTION_BACKING_STORE, &useBS))
	    from = X_CONFIG;
    }
    xfree(options);
    pScreen->backingStoreSupport = useBS ? Always : NotUseful;
    if (serverGeneration == 1)
	xf86DrvMsg(pScreen->myNum, from, "Backing store %s\n",
		   useBS ? "enabled" : "disabled");
}


typedef enum {
   OPTION_SILKEN_MOUSE
} SMOpts;

static const OptionInfoRec SMOptions[] = {
   { OPTION_SILKEN_MOUSE, "SilkenMouse",   OPTV_BOOLEAN, {0}, FALSE },
   { -1,                   NULL,           OPTV_NONE,    {0}, FALSE }
};

void
xf86SetSilkenMouse (ScreenPtr pScreen)
{
    Bool useSM = TRUE;
    MessageType from = X_DEFAULT;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    OptionInfoPtr options;

    options = xnfalloc(sizeof(SMOptions));
    (void)memcpy(options, SMOptions, sizeof(SMOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

    /* check for commandline option here */
    /* disable if screen shares resources */
    if (((pScrn->racMemFlags & RAC_CURSOR) &&
	 !xf86NoSharedResources(pScrn->scrnIndex,MEM)) ||
	((pScrn->racIoFlags & RAC_CURSOR) &&
	 !xf86NoSharedResources(pScrn->scrnIndex,IO))) {
	useSM = FALSE;
	from = X_PROBED;
    } else if (xf86silkenMouseDisableFlag) {
	from = X_CMDLINE;
	useSM = FALSE;
    } else {
	if (xf86GetOptValBool(options, OPTION_SILKEN_MOUSE, &useSM))
	    from = X_CONFIG;
    }
    xfree(options);
    /*
     * XXX quick hack to report correctly for OSs that can't do SilkenMouse
     * yet.  Should handle this differently so that alternate async methods
     * like Xqueue work correctly with this too.
     */
    pScrn->silkenMouse = useSM && xf86SIGIOSupported();
    if (serverGeneration == 1)
	xf86DrvMsg(pScreen->myNum, from, "Silken mouse %s\n",
		   pScrn->silkenMouse ? "enabled" : "disabled");
}

/* Wrote this function for the PM2 Xv driver, preliminary. */

pointer
xf86FindXvOptions(int scrnIndex, int adaptor_index, char *port_name,
		  char **adaptor_name, pointer *adaptor_options)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    confXvAdaptorPtr adaptor;
    int i;

    if (adaptor_index >= pScrn->confScreen->numXvAdaptors) {
	if (adaptor_name) *adaptor_name = NULL;
	if (adaptor_options) *adaptor_options = NULL;
	return NULL;
    }

    adaptor = pScrn->confScreen->xvAdaptorList[adaptor_index];
    if (!adaptor) {
	if (adaptor_name) *adaptor_name = NULL;
	if (adaptor_options) *adaptor_options = NULL;
	return NULL;
    }
    if (adaptor_name) *adaptor_name = adaptor->identifier;
    if (adaptor_options) *adaptor_options = adaptor->options;

    for (i = 0; i < adaptor->numports; i++)
	if (adaptor->xvPorts && adaptor->xvPorts[i] &&
	    !xf86NameCmp(adaptor->xvPorts[i]->identifier, port_name))
	    return adaptor->xvPorts[i]->options;

    return NULL;
}

/* Rather than duplicate loader's get OS function, just include it directly */
#ifndef	TROPIX
#define LoaderGetOS xf86GetOS
#include "loader/os.c"
#endif	TROPIX

/* new RAC */
/*
 * xf86ConfigIsa/PciEntity() -- These helper functions assign an
 * active entity to a screen, registers its fixed resources, assign
 * special enter/leave functions and their private scratch area to
 * this entity, take the dog for a walk...
 */
ScrnInfoPtr
xf86ConfigIsaEntity(ScrnInfoPtr pScrn, int scrnFlag, int entityIndex,
			  IsaChipsets *i_chip, resList res, EntityProc init,
			  EntityProc enter, EntityProc leave, pointer private)
{
    IsaChipsets *i_id;
    EntityInfoPtr pEnt = xf86GetEntityInfo(entityIndex);
    if (!pEnt) return pScrn;

    if (!(pEnt->location.type == BUS_ISA)) {
	xfree(pEnt);
	return pScrn;
    }

    if (!pEnt->active) {
	xf86ConfigIsaEntityInactive(pEnt, i_chip, res, init,  enter,
				    leave,  private);
	return pScrn;
    }

    if (!pScrn)
	pScrn = xf86AllocateScreen(pEnt->driver,scrnFlag);
    xf86AddEntityToScreen(pScrn,entityIndex);

    if (i_chip) {
	for (i_id = i_chip; i_id->numChipset != -1; i_id++) {
	    if (pEnt->chipset == i_id->numChipset) break;
	}
	xf86ClaimFixedResources(i_id->resList,entityIndex);
    }
    xfree(pEnt);
    xf86ClaimFixedResources(res,entityIndex);
    xf86SetEntityFuncs(entityIndex,init,enter,leave,private);

    return pScrn;
}

ScrnInfoPtr
xf86ConfigPciEntity(ScrnInfoPtr pScrn, int scrnFlag, int entityIndex,
			  PciChipsets *p_chip, resList res, EntityProc init,
			  EntityProc enter, EntityProc leave, pointer private)
{
    PciChipsets *p_id;
    EntityInfoPtr pEnt = xf86GetEntityInfo(entityIndex);
    if (!pEnt) return pScrn;

    if (!(pEnt->location.type == BUS_PCI)
	|| !xf86GetPciInfoForEntity(entityIndex)) {
	xfree(pEnt);
	return pScrn;
    }
    if (!pEnt->active) {
	xf86ConfigPciEntityInactive(pEnt, p_chip, res, init,  enter,
				    leave,  private);
	return pScrn;
    }

    if (!pScrn)
	pScrn = xf86AllocateScreen(pEnt->driver,scrnFlag);
    if (xf86IsEntitySharable(entityIndex)) {
	xf86SetEntityShared(entityIndex);
    }
    xf86AddEntityToScreen(pScrn,entityIndex);
    if (xf86IsEntityShared(entityIndex)) {
	return pScrn;
    }
    if (p_chip) {
	for (p_id = p_chip; p_id->numChipset != -1; p_id++) {
	    if (pEnt->chipset == p_id->numChipset) break;
	}
	xf86ClaimFixedResources(p_id->resList,entityIndex);
    }
    xfree(pEnt);

    xf86ClaimFixedResources(res,entityIndex);
    xf86SetEntityFuncs(entityIndex,init,enter,leave,private);

    return pScrn;
}

ScrnInfoPtr
xf86ConfigFbEntity(ScrnInfoPtr pScrn, int scrnFlag, int entityIndex,
		   EntityProc init, EntityProc enter, EntityProc leave,
		   pointer private)
{
    EntityInfoPtr pEnt = xf86GetEntityInfo(entityIndex);
    if (!pEnt) return pScrn;

    if (!(pEnt->location.type == BUS_NONE)) {
	xfree(pEnt);
	return pScrn;
    }

    if (!pEnt->active) {
	xf86ConfigFbEntityInactive(pEnt, init,  enter, leave,  private);
	return pScrn;
    }

    if (!pScrn)
	pScrn = xf86AllocateScreen(pEnt->driver,scrnFlag);
    xf86AddEntityToScreen(pScrn,entityIndex);

    xf86SetEntityFuncs(entityIndex,init,enter,leave,private);

    return pScrn;
}

/*
 *
 *  OBSOLETE ! xf86ConfigActiveIsaEntity() and xf86ConfigActivePciEntity()
 *             are obsolete functions. They the are likely to be removed
 *             Don't use!
 */
Bool
xf86ConfigActiveIsaEntity(ScrnInfoPtr pScrn, int entityIndex,
			  IsaChipsets *i_chip, resList res, EntityProc init,
			  EntityProc enter, EntityProc leave, pointer private)
{
    IsaChipsets *i_id;
    EntityInfoPtr pEnt = xf86GetEntityInfo(entityIndex);
    if (!pEnt) return FALSE;

    if (!pEnt->active || !(pEnt->location.type == BUS_ISA)) {
	xfree(pEnt);
	return FALSE;
    }

    xf86AddEntityToScreen(pScrn,entityIndex);

    if (i_chip) {
	for (i_id = i_chip; i_id->numChipset != -1; i_id++) {
	    if (pEnt->chipset == i_id->numChipset) break;
	}
	xf86ClaimFixedResources(i_id->resList,entityIndex);
    }
    xfree(pEnt);
    xf86ClaimFixedResources(res,entityIndex);
    if (!xf86SetEntityFuncs(entityIndex,init,enter,leave,private))
	return FALSE;

    return TRUE;
}

Bool
xf86ConfigActivePciEntity(ScrnInfoPtr pScrn, int entityIndex,
			  PciChipsets *p_chip, resList res, EntityProc init,
			  EntityProc enter, EntityProc leave, pointer private)
{
    PciChipsets *p_id;
    EntityInfoPtr pEnt = xf86GetEntityInfo(entityIndex);
    if (!pEnt) return FALSE;

    if (!pEnt->active || !(pEnt->location.type == BUS_PCI)) {
	xfree(pEnt);
	return FALSE;
    }
    xf86AddEntityToScreen(pScrn,entityIndex);

    if (p_chip) {
	for (p_id = p_chip; p_id->numChipset != -1; p_id++) {
	    if (pEnt->chipset == p_id->numChipset) break;
	}
	xf86ClaimFixedResources(p_id->resList,entityIndex);
    }
    xfree(pEnt);

    xf86ClaimFixedResources(res,entityIndex);
    if (!xf86SetEntityFuncs(entityIndex,init,enter,leave,private))
	return FALSE;

    return TRUE;
}

/*
 * xf86ConfigPci/IsaEntityInactive() -- These functions can be used
 * to configure an inactive entity as well as to reconfigure an
 * previously active entity inactive. If the entity has been
 * assigned to a screen before it will be removed. If p_pci(p_isa) is
 * non-NULL all static resources listed there will be registered.
 */
void
xf86ConfigPciEntityInactive(EntityInfoPtr pEnt, PciChipsets *p_chip,
			    resList res, EntityProc init, EntityProc enter,
			    EntityProc leave, pointer private)
{
    PciChipsets *p_id;
    ScrnInfoPtr pScrn;

    if ((pScrn = xf86FindScreenForEntity(pEnt->index)))
	xf86RemoveEntityFromScreen(pScrn,pEnt->index);
    else if (p_chip) {
	for (p_id = p_chip; p_id->numChipset != -1; p_id++) {
	    if (pEnt->chipset == p_id->numChipset) break;
	}
	xf86ClaimFixedResources(p_id->resList,pEnt->index);
    }
    xf86ClaimFixedResources(res,pEnt->index);
    /* shared resources are only needed when entity is active: remove */
    xf86DeallocateResourcesForEntity(pEnt->index, ResShared);
    xf86SetEntityFuncs(pEnt->index,init,enter,leave,private);
}

void
xf86ConfigIsaEntityInactive(EntityInfoPtr pEnt, IsaChipsets *i_chip,
			    resList res, EntityProc init, EntityProc enter,
			    EntityProc leave, pointer private)
{
    IsaChipsets *i_id;
    ScrnInfoPtr pScrn;

    if ((pScrn = xf86FindScreenForEntity(pEnt->index)))
	xf86RemoveEntityFromScreen(pScrn,pEnt->index);
    else if (i_chip) {
	for (i_id = i_chip; i_id->numChipset != -1; i_id++) {
	    if (pEnt->chipset == i_id->numChipset) break;
	}
	xf86ClaimFixedResources(i_id->resList,pEnt->index);
    }
    xf86ClaimFixedResources(res,pEnt->index);
    /* shared resources are only needed when entity is active: remove */
    xf86DeallocateResourcesForEntity(pEnt->index, ResShared);
    xf86SetEntityFuncs(pEnt->index,init,enter,leave,private);
}

void
xf86ConfigFbEntityInactive(EntityInfoPtr pEnt, EntityProc init,
			   EntityProc enter, EntityProc leave, pointer private)
{
    ScrnInfoPtr pScrn;

    if ((pScrn = xf86FindScreenForEntity(pEnt->index)))
	xf86RemoveEntityFromScreen(pScrn,pEnt->index);
    xf86SetEntityFuncs(pEnt->index,init,enter,leave,private);
}

Bool
xf86IsScreenPrimary(int scrnIndex)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    int i;

    for (i=0 ; i < pScrn->numEntities; i++) {
	if (xf86IsEntityPrimary(i))
	    return TRUE;
    }
    return FALSE;
}

int
xf86RegisterRootWindowProperty(int ScrnIndex, Atom property, Atom type,
			       int format, unsigned long len, pointer value )
{
    RootWinPropPtr pNewProp = NULL, pRegProp;
    int i;
    Bool existing = FALSE;

#ifdef DEBUG
    ErrorF("xf86RegisterRootWindowProperty(%d, %ld, %ld, %d, %ld, %p)\n",
	   ScrnIndex, property, type, format, len, value);
#endif

    if (ScrnIndex<0 || ScrnIndex>=xf86NumScreens) {
      return(BadMatch);
    }

    if (xf86RegisteredPropertiesTable &&
	xf86RegisteredPropertiesTable[ScrnIndex]) {
      for (pNewProp = xf86RegisteredPropertiesTable[ScrnIndex];
	   pNewProp; pNewProp = pNewProp->next) {
	if (strcmp(pNewProp->name, NameForAtom(property)) == 0)
	  break;
      }
    }

    if (!pNewProp) {
      if ((pNewProp = (RootWinPropPtr)xalloc(sizeof(RootWinProp))) == NULL) {
	return(BadAlloc);
      }
      /*
       * We will put this property at the end of the list so that
       * the changes are made in the order they were requested.
       */
      pNewProp->next = NULL;
    } else {
      if (pNewProp->name)
	xfree(pNewProp->name);
      existing = TRUE;
    }

    pNewProp->name = xnfstrdup(NameForAtom(property));
    pNewProp->type = type;
    pNewProp->format = format;
    pNewProp->size = len;
    pNewProp->data = value;

#ifdef DEBUG
    ErrorF("new property filled\n");
#endif

    if (NULL==xf86RegisteredPropertiesTable) {
#ifdef DEBUG
      ErrorF("creating xf86RegisteredPropertiesTable[] size %d\n",
	     xf86NumScreens);
#endif
      if ( NULL==(xf86RegisteredPropertiesTable=(RootWinPropPtr*)xnfcalloc(sizeof(RootWinProp),xf86NumScreens) )) {
	return(BadAlloc);
      }
      for (i=0; i<xf86NumScreens; i++) {
	xf86RegisteredPropertiesTable[i] = NULL;
      }
    }

#ifdef DEBUG
    ErrorF("xf86RegisteredPropertiesTable %p\n",
	   (void *)xf86RegisteredPropertiesTable);
    ErrorF("xf86RegisteredPropertiesTable[%d] %p\n",
	   ScrnIndex, (void *)xf86RegisteredPropertiesTable[ScrnIndex]);
#endif

    if (!existing) {
      if ( xf86RegisteredPropertiesTable[ScrnIndex] == NULL) {
	xf86RegisteredPropertiesTable[ScrnIndex] = pNewProp;
      } else {
	pRegProp = xf86RegisteredPropertiesTable[ScrnIndex];
	while (pRegProp->next != NULL) {
#ifdef DEBUG
	  ErrorF("- next %p\n", (void *)pRegProp);
#endif
	  pRegProp = pRegProp->next;
	}
	pRegProp->next = pNewProp;
      }
    }
#ifdef DEBUG
    ErrorF("xf86RegisterRootWindowProperty succeeded\n");
#endif
    return(Success);
}

Bool
xf86IsUnblank(int mode)
{
    switch(mode) {
    case SCREEN_SAVER_OFF:
    case SCREEN_SAVER_FORCER:
	return TRUE;
    case SCREEN_SAVER_ON:
    case SCREEN_SAVER_CYCLE:
	return FALSE;
    default:
	xf86MsgVerb(X_WARNING, 0, "Unexpected save screen mode: %d\n", mode);
	return TRUE;
    }
}

MonPtr
xf86GetMonitorByNumber(const ScrnInfoRec *pScrn, int monNum)
{
    int i;

    for (i = 0; i < pScrn->confScreen->numMonitors; i++) {
	if (pScrn->confScreen->monitors[i]->monitorNum == monNum) {
	    return pScrn->confScreen->monitors[i];
	}
    }
    return NULL;
}

DispPtr
xf86GetDisplayByMonitorNum(const ScrnInfoRec *pScrn, int monNum)
{
    int i;
    DispPtr pDisp, foundDisplay = NULL;

    /*
     * Find the Display subsection matching the depth/fbbpp and monitor number.
     */
    for (i = 0; i < pScrn->confScreen->numdisplays; i++) {
	pDisp = pScrn->confScreen->displayList[i];
	if (pDisp && pDisp->monitorNum == monNum &&
	    ((pDisp->depth == pScrn->depth &&
	      pDisp->fbbpp == pScrn->bitsPerPixel) ||
	     (pDisp->depth == pScrn->depth && pDisp->fbbpp <= 0) ||
	     (pDisp->fbbpp == pScrn->bitsPerPixel && pDisp->depth <= 0))) {
	    foundDisplay = pDisp;
	    break;
	}
    }

    /*
     * If an exact match cannot be found, see if there is one with a matching
     * monitor number and no depth or fbbpp specified.
     */
    if (!foundDisplay) {
	for (i = 0; i < pScrn->confScreen->numdisplays; i++) {
	    pDisp = pScrn->confScreen->displayList[i];
	    if (pDisp && pDisp->monitorNum == monNum &&
		pDisp->depth <= 0 && pDisp->fbbpp <= 0) {
		foundDisplay = pDisp;
		break;
	    }
	}
    }

    /*
     * If all else fails, create a default one.
     */
    if (!foundDisplay) {
	i = pScrn->confScreen->numdisplays++;
	pScrn->confScreen->displayList =
		xnfrealloc(pScrn->confScreen->displayList,
			   pScrn->confScreen->numdisplays * sizeof(DispPtr));
	pScrn->confScreen->displayList[i] = xf86ConfAllocDisplay();
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Creating default Display subsection in Screen section\n"
		   "\t\"%s\" ", pScrn->confScreen->id);
	if (monNum >= 0) {
	    xf86ErrorF("for monitor number %d, depth/fbbpp %d/%d\n",
		       monNum, pScrn->depth, pScrn->bitsPerPixel);
	} else {
	    xf86ErrorF("for depth/fbbpp %d/%d\n",
		       pScrn->depth, pScrn->bitsPerPixel);
	}
	foundDisplay = pScrn->confScreen->displayList[i];
    }

    pDisp = xnfalloc(sizeof(DispRec));
    *pDisp = *foundDisplay;
    return pDisp;
}

Bool
xf86GetNextMonitor(const ScrnInfoRec *pScrn, MonPtr *pMonitor, DispPtr *pDisplay)
{
    int i, prevNum, nextNum = 10000000;
    MonPtr next = NULL;

    if (!pMonitor)
	return FALSE;

    if (!*pMonitor)
	prevNum = -1;
    else
	prevNum = (*pMonitor)->monitorNum;

    for (i = 0; i < pScrn->confScreen->numMonitors; i++) {
	if (pScrn->confScreen->monitors[i]->monitorNum > prevNum &&
	    pScrn->confScreen->monitors[i]->monitorNum < nextNum) {
	    next = pScrn->confScreen->monitors[i];
	    nextNum = pScrn->confScreen->monitors[i]->monitorNum;
	}
    }

    if (!next)
	return FALSE;

    *pMonitor = next;

    if (pDisplay) {
	*pDisplay = xf86GetDisplayByMonitorNum(pScrn, nextNum);
    }

    return TRUE;
}

#ifdef	TROPIX
void
xf86GetOS(const char **name, int *major, int *minor, int *teeny)
{
	if (name)
		*name = OSNAME;
		
	/* reporting runtime versions isn't supported yet */
}

int
xf86getpagesize (void)
{
	return (4096);
}
#endif	TROPIX
