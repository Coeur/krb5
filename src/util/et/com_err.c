/*
 * Copyright 1997 by Massachusetts Institute of Technology
 * 
 * Copyright 1987, 1988 by MIT Student Information Processing Board
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the names of M.I.T. and the M.I.T. S.I.P.B. not be
 * used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * M.I.T. and the M.I.T. S.I.P.B. make no representations about
 * the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 */

#include <stdio.h>
#include <string.h>

#include "com_err.h"
#include "error_table.h"

#ifdef _MACINTOSH
#include "icons.h"
static void MacMessageBox(errbuf);
#endif

#if !(defined(_MSDOS) || defined(_WIN32) || defined(_MACINTOSH))

static void default_com_err_proc
ET_P((et_ctx ectx, void FAR *priv,
	const char FAR *whoami, errcode_t code,
	const char FAR *fmt, va_list ap));

/* The version for Unix */
static void default_com_err_proc(ectx, priv, whoami, code, fmt, ap)
	et_ctx ectx;
	void FAR *priv;
	const char FAR *whoami;
	errcode_t code;
	const char FAR *fmt;
	va_list ap;
{
	if (whoami) {
		fputs(whoami, stderr);
		fputs(": ", stderr);
	}
	if (code) {
		fputs(error_message(code), stderr);
		fputs(" ", stderr);
	}
	if (fmt) {
		vfprintf(stderr, fmt, ap);
	}
	/* should do this only on a tty in raw mode */
	putc('\r', stderr);
	putc('\n', stderr);
	fflush(stderr);
}

#else

/*
 * For Macintosh and Windows
 */
void KRB5_CALLCONV default_com_err_proc
ET_P((et_ctx ectx, void FAR *priv,
	const char FAR *whoami, errcode_t code,
	const char FAR *fmt, va_list ap));

void KRB5_CALLCONV default_com_err_proc(ectx, priv, whoami, code, fmt, ap)
	et_ctx ectx;
	void FAR *priv;
	const char FAR *whoami;
	errcode_t code;
	const char FAR *fmt;
	va_list ap;
{
	static char errbuf[1024];                   /* For those w/o stdio */

	*errbuf = '\0';
	if (whoami) {
		strcat (errbuf, whoami);
		strcat (errbuf, ": ");
	}
	if (code) {
		strcat (errbuf, error_message(code));
		strcat (errbuf, " ");
	}
	if (fmt) {
		vsprintf (errbuf + strlen (errbuf), fmt, ap);
	}
#ifdef _MACINTOSH
	MacMessageBox(errbuf);
#else
	/* For Windows */
	MessageBox ((HWND)NULL, errbuf, "Kerberos", MB_ICONEXCLAMATION);
#endif
}
#endif

et_old_error_hook_func com_err_hook = 0;

void et_com_err_va(ectx, whoami, code, fmt, ap)
	et_ctx ectx;
	const char FAR *whoami;
	errcode_t code;
	const char FAR *fmt;
	va_list ap;
{
	if (ectx) {
		if (ectx->hook_func == 0)
			ectx->hook_func = default_com_err_proc;
		(*ectx->hook_func)(ectx, ectx->hook_func_data, whoami,
				   code, fmt, ap);
	} else {
		if (!com_err_hook)
			default_com_err_proc(0, 0, whoami,
					     code, fmt, ap);
		else
			(*com_err_hook)(whoami, code, fmt, ap);
	}
}

#ifndef ET_VARARGS
KRB5_DLLIMP void KRB5_CALLCONV_C et_com_err(et_ctx ectx,
					    const char FAR *whoami,
					    errcode_t code,
					    const char FAR *fmt, ...)
#else
KRB5_DLLIMP void KRB5_CALLCONV_C et_com_err(ectx, whoami,
					    code, fmt, va_alist)
	et_ctx ectx;
	const char FAR *whoami;
	errcode_t code;
	const char FAR *fmt;
	va_dcl
#endif
{
	va_list ap;

#ifdef ET_VARARGS
	va_start(ap);
#else
	va_start(ap, fmt);
#endif
	et_com_err_va(ectx, whoami, code, fmt, ap);
	va_end(ap);
}


/*
 * The following are the old-style com_err interfaces....
 */
KRB5_DLLIMP void KRB5_CALLCONV com_err_va(whoami, code, fmt, args)
    const char FAR * whoami;
    errcode_t code;
    const char FAR * fmt;
    va_list args;
{
    et_com_err_va(0, whoami, code, fmt, args);
}

#ifndef ET_VARARGS
KRB5_DLLIMP void KRB5_CALLCONV_C com_err(const char FAR *whoami,
					 errcode_t code,
					 const char FAR *fmt, ...)
#else
KRB5_DLLIMP void KRB5_CALLCONV_C com_err(whoami, code, fmt, va_alist)
	const char FAR *whoami;
	errcode_t code;
	const char FAR *fmt;
	va_dcl
#endif
{
	va_list ap;

#ifdef ET_VARARGS
	va_start(ap);
#else
	va_start(ap, fmt);
#endif
	com_err_va(whoami, code, fmt, ap);
	va_end(ap);
}

KRB5_DLLIMP et_old_error_hook_func KRB5_CALLCONV set_com_err_hook (new_proc)
	et_old_error_hook_func new_proc;
{
	et_old_error_hook_func x = com_err_hook;

	com_err_hook = new_proc;
	return x;
}

KRB5_DLLIMP et_old_error_hook_func KRB5_CALLCONV reset_com_err_hook ()
{
	et_old_error_hook_func x = com_err_hook;
    
	com_err_hook = 0;
	return x;
}

KRB5_DLLIMP errcode_t KRB5_CALLCONV et_set_hook(ectx, new, old)
	et_ctx ectx;
	struct et_hook FAR *new;
	struct et_hook FAR *old;
{
	if (old) {
		old->func = ectx->hook_func;
		old->data = ectx->hook_func_data;
	}
	if (new) {
		ectx->hook_func = new->func;
		ectx->hook_func_data = new->data;
	} else {
		ectx->hook_func = 0;
		ectx->hook_func_data = 0;
	}
	return 0;
}

#ifdef _MACINTOSH
static void MacMessageBox(errbuf)
	char *errbuf;
{
	WindowPtr	errWindow;
	ControlHandle	errOkButton;
	Rect		errOkButtonRect = { 120, 220, 140, 280 };
	Rect		errRect = { 0, 0, 150, 300 };
	GDHandle	mainDevice = GetMainDevice();
	Rect		mainRect = (**mainDevice).gdRect;
	Rect		tmpRect;
	Rect		errTextRect = { 10, 70, 110, 290 };
	Rect		errIconRect = { 10, 10, 10 + 32, 10 + 32 };
	EventRecord	theEvent;
	Point		localPt;
	Boolean		done;

	/* Find Centered rect for window */
	tmpRect.top	= ((mainRect.bottom + mainRect.top)/2 -
			   (errRect.bottom + errRect.top)/2);
	tmpRect.bottom = tmpRect.top + (errRect.bottom - errRect.top);
	tmpRect.left = ((mainRect.right + mainRect.left)/2 -
			(errRect.right + errRect.left)/2);
	tmpRect.right = tmpRect.left + (errRect.right - errRect.left);

	/* Create the error window - as a dialog window */
	errWindow = NewWindow(NULL, &tmpRect, "\p", TRUE,
			      dBoxProc, (WindowPtr) -1, FALSE, 0L);

	SetPort(errWindow);
	TextFont(systemFont);
	TextSize(12);

	errOkButton = NewControl(errWindow, &errOkButtonRect,
				 "\pOk", TRUE, 0, 0, 1, pushButProc, 0L);
      DrawControls(errWindow);
	InsetRect(&errOkButtonRect, -4, -4);
	PenSize(3,3);
	FrameRoundRect(&errOkButtonRect, 15,15);
	PenSize(1,1);
	InsetRect(&errOkButtonRect, 4, 4);

	/* Draw the error text */
	TextBox(errbuf, strlen(errbuf), &errTextRect, teForceLeft);

	/* Draw the Stop icon */
	PlotIcon(&errIconRect, GetResource('ICON', 0));

	/* mini event loop here */
	done = FALSE;
	while(!done) {
		GetNextEvent(mDownMask | mUpMask | keyDownMask, &theEvent);
		if (theEvent.what == mouseDown) {
			localPt = theEvent.where;
			GlobalToLocal(&localPt);
			if (TestControl(errOkButton, localPt) &&
			    TrackControl(errOkButton, localPt, NULL)) {
				done = TRUE;
			}
		} else if (theEvent.what == keyDown &&
			   (theEvent.message & 0xff) == 0x0d ||	/* CR */
			   (theEvent.message & 0xff) == 0x03 ||	/* Enter */
			   (theEvent.message & 0xff) == 0x1b) {	/* Escape */
			long t;
			/* Hilite the button for a bit */
			HiliteControl(errOkButton, 1);	
			Delay(5, &t);
			/* Dehilite the button */
			HiliteControl(errOkButton, 0);
			done = TRUE;
		}
	}

	/* Dispose of the Window, disposes of controls */
	DisposeWindow(errWindow);
}
#endif
