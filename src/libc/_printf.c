#include <xprintf/xprintf.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <env.h>

inline int putchar(int ch) {
    if (isascii(ch)) {
        trace_put_char(ch);
        return ch;
    } else {
        trace_put_char('?');
        return '?';
    }
}

int puts(const char *str) {
    int len = 0;
    char ch = *str;
    while (ch) {
        putchar(ch);
        str ++;
        len ++;
        ch = *str;
    }
    return len + 1;
}

static void __stdout_func(void *p,void const *src,int len) {
    char *buf = (char *) src;
    while (len) {
        putchar(*buf);
        buf ++;
        len --;
    }
}

static struct ostrm __stdout = { .p = NULL, .func = __stdout_func };

int printf(char const* fmt, ... ) {
	va_list va;
	va_start( va, fmt );
	int const rslt = xvprintf( &__stdout, fmt, va );
	va_end( va );
	return rslt;
}

static void __putbuff( void* p, void const* src, int len ) {
    char** buff = p;
    memcpy( *buff, src, len );
    *buff += len;
}

int sprintf(char* buff, char const* fmt, ... ) {
    char* tmp = buff;
    struct ostrm const ostrm = { .p = &tmp, .func = __putbuff };
	va_list va;
	va_start( va, fmt );
	int const rslt = xvprintf( &ostrm, fmt, va );
	va_end( va );
    buff[ rslt ] = '\0';
	return rslt;
}
