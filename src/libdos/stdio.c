#include "stdio.h"

#include "conio.h"
#include "string.h"
#include "stdarg.h"

static char fpbuf[32];

typedef enum farglen
{
    FAL_HH,
    FAL_H,
    FAL_DEF,
    FAL_L
} farglen;

static size_t _fargnbuf(char **buf, const char **s, size_t n)
{
    while ((!n || n > 1) && **s)
    {
	*(*buf)++ = *(*s)++;
	if (n) --n;
    }
    **buf = 0;
    while (**s) ++(*s);
    return n;
}

static size_t _ffmtnbuf(char **buf, const char **s, size_t n)
{
    while ((!n || n > 1) && **s && **s != '%')
    {
	*(*buf)++ = *(*s)++;
	if (n) --n;
    }
    **buf = 0;
    while (**s && **s != '%') ++(*s);
    return n;
}

static void _fnumarg(long num, char pad, size_t minwidth, int sign)
{
    char tmp[11];
    int i = 0;
    char *s = fpbuf;

    if (minwidth > 31) minwidth = 31;

    if (sign)
    {
	if (num < 0)
	{
	    *s++ = '-';
	    --minwidth;
	    num = -num;
	}
	else if (pad == ' ')
	{
	    *s++ = ' ';
	    --minwidth;
	}
    }
    if (pad != '0') pad = ' ';
    do
    {
	tmp[i++] = (unsigned long)num % 10;
	num = (unsigned long)num / 10;
    } while (num > 0);
    while (minwidth-- > i) *s++ = pad;
    while (--i >= 0) *s++ = tmp[i] + '0';
    *s = 0;
}

static const char *_fftcharg(const char **s, va_list *ap)
{
    char pad = 0;
    size_t minwidth = 0;
    farglen len = FAL_DEF;
    unsigned long num;

    while (1)
    {
	(*s)++;
	switch (**s)
	{
	    case 'd':
		switch (len)
		{
		    case FAL_HH:
			num = (signed char) va_arg(*ap, int);
			break;
		    case FAL_H:
			num = (short) va_arg(*ap, int);
			break;
		    case FAL_DEF:
			num = va_arg(*ap, int);
			break;
		    case FAL_L:
			num = va_arg(*ap, unsigned long);
		}
		_fnumarg((long)num, pad, minwidth, 1);
		(*s)++;
		return fpbuf;

	    case 'u':
		switch (len)
		{
		    case FAL_HH:
			num = (unsigned char) va_arg(*ap, unsigned int);
			break;
		    case FAL_H:
			num = (unsigned short) va_arg(*ap, unsigned int);
			break;
		    case FAL_DEF:
			num = va_arg(*ap, unsigned int);
			break;
		    case FAL_L:
			num = va_arg(*ap, unsigned long);
		}
		_fnumarg((long)num, pad, minwidth, 0);
		(*s)++;
		return fpbuf;

	    case 's':
		(*s)++;
		return va_arg(*ap, const char*);

	    case '0':
		if (minwidth) minwidth *= 10;
		else pad = '0';
		break;

	    case ' ':
		pad = ' ';
		break;

	    case 'h':
		if (len > FAL_HH) --len;
		break;

	    case 'l':
		if (len < FAL_L) ++len;
		break;

	    default:
		if (!**s) return 0;
		if (**s >= '1' && **s <= '9')
		{
		    minwidth *= 10;
		    minwidth += **s - '0';
		}
	}
    }
}

static size_t _ffmtlen(const char *format)
{
    size_t len = 0;
    while (*format && *(format++) != '%') ++len;
    return len;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int n = 0;

    while (*format)
    {
	if (*format == '%')
	{
	    const char *arg = _fftcharg(&format, &ap);
	    n += strlen(arg);
	    size = _fargnbuf(&str, &arg, size);
	}
	else
	{
	    n += _ffmtlen(format);
	    size = _ffmtnbuf(&str, &format, size);
	}
    }

    return n;
}

int vsprintf(char *str, const char *format, va_list ap)
{
    return vsnprintf(str, 0, format, ap);
}

int vprintf(const char *format, va_list ap)
{
    int n = 0;
    char *buf;

    while (*format)
    {
	if (*format == '%') n += putstr(_fftcharg(&format, &ap));
	else
	{
	    buf = fpbuf;
	    _ffmtnbuf(&buf, &format, 32);
	    n += putstr(fpbuf);
	}
    }
    return n;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);

    return ret;
}

int sprintf(char *str, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    int ret = vsprintf(str, format, ap);
    va_end(ap);

    return ret;
}

int printf(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    int ret = vprintf(format, ap);
    va_end(ap);

    return ret;
}

