/*
 * $Source$
 * $Author$
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <krb5/copyright.h>.
 *
 * krb5_unparse_name() routine
 *
 * Rewritten by Theodore Ts'o to propoerly unparse principal names
 * which have the component or realm separator as part of one of their
 * components.
 */

#if !defined(lint) && !defined(SABER)
static char rcsid_unparse_c[] =
"$Id$";
#endif	/* !lint & !SABER */

#include <stdio.h>
#include <krb5/copyright.h>
#include <krb5/krb5.h>
#include <krb5/ext-proto.h>

/*
 * converts the multi-part principal format used in the protocols to a
 * single-string representation of the name. 
 *  
 * The name returned is in allocated storage and should be freed by
 * the caller when finished.
 *
 * Conventions: / is used to separate components; @ is used to
 * separate the realm from the rest of the name.  If '/', '@', or '\0'
 * appear in any the component, they will be representing using
 * backslash encoding.  ("\/", "\@", or '\0', respectively)
 *
 * returns system errors XXX
 */

#define REALM_SEP	'@'
#define	COMPONENT_SEP	'/'

krb5_error_code
krb5_unparse_name_ext(principal, name, size)
const krb5_principal principal;
register char **name;
int	*size;
{
	register char *cp, *q;
	register int i,j;
	register int totalsize = 0;
	int	length;

	if (!principal[0] || !principal[1])
		return KRB5_PARSE_MALFORMED;
	for (i = 0; principal[i]; i++) {
		cp = principal[i]->data;
		length = principal[i]->length;
		for (j=0; j < length; j++,cp++)
			if (*cp == REALM_SEP || *cp == COMPONENT_SEP ||
			    *cp == '\0' || *cp == '\\' || *cp == '\t')
				totalsize += 2;
			else
				totalsize++;
		totalsize++;	/* This is for the separator */
	}

	/*
	 *  we need only n-1 seps for n components, but we need an
	 * extra byte for the NULL at the end
	 */
	if (*name) {
		if (*size < (totalsize)) {
			*size = totalsize;
			*name = realloc(*name, totalsize);
		}
	} else {
		*name = malloc(totalsize);	/* room for null */
		*size = totalsize;
	}
	
	if (!*name)
		return ENOMEM;

	q = *name;
	
	for (i = 1; principal[i]; i++) {
		cp = principal[i]->data;
		length = principal[i]->length;
		for (j=0; j < length; j++,cp++) {
			switch (*cp) {
			case COMPONENT_SEP:
			case REALM_SEP:
			case '\t':
			case '\\':
				*q++ = '\\';
				*q++ = *cp;
				break;
			case '\0':
				*q++ = '\\';
				*q++ = '0';
				break;
			default:
				*q++ = *cp;
			}
		}
		*q++ = COMPONENT_SEP;
	}

	q--;			/* Back up last component separator */
	*q++ = REALM_SEP;
	
	cp = principal[0]->data;
	length = principal[0]->length;
	for (j=0; j < length; j++,cp++) {
		switch (*cp) {
		case COMPONENT_SEP:
		case REALM_SEP:
			*q++ = '\\';
			*q++ = *cp;
			break;
		case '\0':
			*q++ = '\\';
			*q++ = '0';
			break;
		default:
			*q++ = *cp;
		}
	}
	*q++ = '\0';
	
    return 0;
}

krb5_error_code
krb5_unparse_name(principal, name)
const krb5_principal principal;
register char **name;
{
	return(krb5_unparse_name_ext(principal, name, NULL));
}


