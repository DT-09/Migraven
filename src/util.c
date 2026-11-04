#include "migraven/migraven.h"
#include <stdio.h>
const char *migraven_status_name(MigravenStatus s) {
    switch(s) { case MIGRAVEN_PASS:return "PASS"; case MIGRAVEN_FAIL:return "FAIL"; case MIGRAVEN_INDETERMINATE:return "INDETERMINATE"; case MIGRAVEN_ERROR:return "ERROR"; default:return "ERROR"; }
}
void migraven_json_string(const char *value) {
    const unsigned char *p=(const unsigned char *)(value?value:"");
    putchar('"');
    while(*p) {
        switch(*p) {
        case '"': fputs("\\\"",stdout); break;
        case '\\': fputs("\\\\",stdout); break;
        case '\b': fputs("\\b",stdout); break;
        case '\f': fputs("\\f",stdout); break;
        case '\n': fputs("\\n",stdout); break;
        case '\r': fputs("\\r",stdout); break;
        case '\t': fputs("\\t",stdout); break;
        default: if(*p<0x20) printf("\\u%04x",(unsigned int)*p); else putchar((int)*p);
        }
        ++p;
    }
    putchar('"');
}
