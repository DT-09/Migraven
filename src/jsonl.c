#include "verity/verity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define JSONL_MAX_FILE_BYTES ((size_t)268435456)
#define JSONL_MAX_FIELD_BYTES ((size_t)16777216)

typedef struct { const char *p; const char *end; char *error; size_t error_size; } Parser;

static void fail(Parser *p, const char *message) {
    if (p->error && p->error_size) (void)snprintf(p->error, p->error_size, "%s", message);
}
static void ws(Parser *p) { while (p->p < p->end && (*p->p == ' ' || *p->p == '\t' || *p->p == '\r')) p->p++; }
static int hex4(Parser *p, unsigned *value) {
    unsigned v = 0; int i;
    if ((size_t)(p->end - p->p) < 4) return 0;
    for (i = 0; i < 4; ++i) {
        unsigned char c = (unsigned char)*p->p++; unsigned d;
        if (c >= '0' && c <= '9') d = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') d = (unsigned)(c - 'a') + 10U;
        else if (c >= 'A' && c <= 'F') d = (unsigned)(c - 'A') + 10U;
        else return 0;
        v = (v << 4) | d;
    }
    *value = v; return 1;
}
static int put_utf8(char **out, size_t *len, size_t *cap, unsigned cp) {
    unsigned char b[4]; size_t n, i; char *next;
    if (cp <= 0x7fU) { b[0]=(unsigned char)cp; n=1; }
    else if (cp <= 0x7ffU) { b[0]=(unsigned char)(0xc0U|(cp>>6)); b[1]=(unsigned char)(0x80U|(cp&0x3fU)); n=2; }
    else if (cp <= 0xffffU) { b[0]=(unsigned char)(0xe0U|(cp>>12)); b[1]=(unsigned char)(0x80U|((cp>>6)&0x3fU)); b[2]=(unsigned char)(0x80U|(cp&0x3fU)); n=3; }
    else if (cp <= 0x10ffffU) { b[0]=(unsigned char)(0xf0U|(cp>>18)); b[1]=(unsigned char)(0x80U|((cp>>12)&0x3fU)); b[2]=(unsigned char)(0x80U|((cp>>6)&0x3fU)); b[3]=(unsigned char)(0x80U|(cp&0x3fU)); n=4; }
    else return 0;
    if (*len+n+1 > *cap) { size_t nc=*cap; while(nc<*len+n+1) nc*=2; next=(char*)realloc(*out,nc); if(!next)return 0; *out=next; *cap=nc; }
    for(i=0;i<n;i++) (*out)[(*len)++]=(char)b[i];
    return 1;
}
static char *string_value(Parser *p) {
    char *out=(char*)malloc(64); size_t len=0,cap=64;
    if(!out)return NULL;
    if(p->p>=p->end || *p->p!='"') { free(out); return NULL; }
    p->p++;
    while(p->p<p->end && *p->p!='"') {
        unsigned cp=(unsigned char)*p->p++;
        if(cp=='\\') {
            unsigned char e;
            if(p->p>=p->end) goto bad;
            e=(unsigned char)*p->p++;
            if(e=='"'||e=='\\'||e=='/') cp=e;
            else if(e=='b') cp='\b'; else if(e=='f') cp='\f'; else if(e=='n') cp='\n'; else if(e=='r') cp='\r'; else if(e=='t') cp='\t';
            else if(e=='u') {
                unsigned u;
                if(!hex4(p,&u)) goto bad;
                if(u>=0xd800U && u<=0xdbffU) {
                    unsigned lo;
                    if((size_t)(p->end-p->p)<6 || p->p[0]!='\\' || p->p[1]!='u') goto bad;
                    p->p+=2; if(!hex4(p,&lo)||lo<0xdc00U||lo>0xdfffU) goto bad;
                    cp=0x10000U+((u-0xd800U)<<10)+(lo-0xdc00U);
                } else if(u>=0xdc00U && u<=0xdfffU) goto bad;
                else cp=u;
            } else goto bad;
        }
        if(cp<0x20U || !put_utf8(&out,&len,&cap,cp)) goto bad;
        if(len>JSONL_MAX_FIELD_BYTES) goto bad;
    }
    if(p->p>=p->end) goto bad;
    p->p++; out[len]='\0'; return out;
bad:
    free(out); fail(p,"Invalid JSON string or unsupported escape"); return NULL;
}
static char *scalar_value(Parser *p) {
    const char *start=p->p;
    size_t n; char *v;
    if(p->p<p->end && *p->p=='"') return string_value(p);
    while(p->p<p->end && *p->p!=',' && *p->p!='}' && *p->p!=' ' && *p->p!='\t' && *p->p!='\r') p->p++;
    n=(size_t)(p->p-start);
    if(n==0 || !((n==4 && memcmp(start,"true",4)==0)||(n==5 && memcmp(start,"false",5)==0)||(n==4 && memcmp(start,"null",4)==0)||(*start=='-'||(*start>='0'&&*start<='9')))) {
        fail(p,"JSONL values must be strings, numbers, booleans, or null"); return NULL;
    }
    if(n>128) { fail(p,"JSON scalar is too long"); return NULL; }
    if(n==4 && memcmp(start,"null",4)==0) return (char*)calloc(1,1);
    v=(char*)malloc(n+1); if(!v)return NULL; memcpy(v,start,n); v[n]='\0'; return v;
}
static void free_row(VerityRow *r) {
    size_t i; for(i=0;i<r->cell_count;i++) free(r->cells[i]); free(r->cells); memset(r,0,sizeof(*r));
}
static int push_pair(char ***keys, char ***values, size_t *count, char *key, char *value) {
    char **nk=(char**)realloc(*keys,(*count+1)*sizeof(**keys));
    char **nv;
    if(!nk)return 0; *keys=nk;
    nv=(char**)realloc(*values,(*count+1)*sizeof(**values)); if(!nv)return 0; *values=nv;
    (*keys)[*count]=key; (*values)[*count]=value; (*count)++; return 1;
}
static int parse_object(const char *line,size_t length,char ***keys,char ***values,size_t *count,char *error,size_t error_size) {
    Parser p={line,line+length,error,error_size}; size_t i;
    *keys=NULL; *values=NULL; *count=0; ws(&p);
    if(p.p>=p.end||*p.p++!='{') { fail(&p,"Each JSONL record must be a JSON object"); return 0; }
    ws(&p);
    while(p.p<p.end && *p.p!='}') {
        char *k,*v; size_t j;
        k=string_value(&p); if(!k)goto bad;
        ws(&p); if(p.p>=p.end||*p.p++!=':') { free(k); fail(&p,"Expected ':' after JSON property"); goto bad; }
        ws(&p); v=scalar_value(&p); if(!v) { free(k); goto bad; }
        for(j=0;j<*count;j++) if(strcmp((*keys)[j],k)==0) { free(k); free(v); fail(&p,"Duplicate JSON property"); goto bad; }
        if(!push_pair(keys,values,count,k,v)) { free(k); free(v); fail(&p,"Out of memory"); goto bad; }
        ws(&p); if(p.p<p.end && *p.p==',') { p.p++; ws(&p); continue; }
        break;
    }
    if(p.p>=p.end||*p.p++!='}') { fail(&p,"Expected '}' at end of JSON object"); goto bad; }
    ws(&p); if(p.p!=p.end) { fail(&p,"Unexpected content after JSON object"); goto bad; }
    if(*count==0) { fail(&p,"JSONL objects must contain at least one property"); goto bad; }
    for(i=0;i<*count;i++) { (void)i; }
    return 1;
bad:
    for(i=0;i<*count;i++){free((*keys)[i]);free((*values)[i]);}
    free(*keys);free(*values);*keys=NULL;*values=NULL;*count=0;return 0;
}
int verity_jsonl_read(const char *path,VerityTable *out,char *error,size_t error_size) {
    FILE *fp; long end; size_t len,pos=0,line_no=1; char *buf; int ok=0;
    if(!path||!out){if(error&&error_size)snprintf(error,error_size,"Invalid argument");return 0;}
    memset(out,0,sizeof(*out)); fp=fopen(path,"rb");
    if(!fp){if(error&&error_size)snprintf(error,error_size,"Cannot open JSONL input");return 0;}
    if(fseek(fp,0,SEEK_END)!=0||(end=ftell(fp))<0||(unsigned long)end>JSONL_MAX_FILE_BYTES||fseek(fp,0,SEEK_SET)!=0){fclose(fp);if(error&&error_size)snprintf(error,error_size,"JSONL exceeds 256 MiB or cannot be read");return 0;}
    len=(size_t)end;buf=(char*)malloc(len+1);if(!buf){fclose(fp);return 0;}
    if(fread(buf,1,len,fp)!=len){fclose(fp);free(buf);return 0;}fclose(fp);buf[len]='\0';
    while(pos<len) {
        size_t start=pos,n,i; char **keys=NULL,**values=NULL; size_t count=0; VerityRow row={0};
        while(pos<len&&buf[pos]!='\n')pos++; n=pos-start; if(pos<len)pos++;
        if(n&&buf[start+n-1]=='\r')n--;
        if(n==0){line_no++;continue;}
        if(!parse_object(buf+start,n,&keys,&values,&count,error,error_size))goto done;
        if(out->header_count==0) {
            out->headers=keys;out->header_count=count;keys=NULL;
        } else {
            if(count!=out->header_count){if(error&&error_size)snprintf(error,error_size,"JSONL schema changed at line %zu",line_no);goto row_fail;}
            for(i=0;i<count;i++) if(strcmp(keys[i],out->headers[i])!=0){if(error&&error_size)snprintf(error,error_size,"JSONL property order or schema changed at line %zu",line_no);goto row_fail;}
        }
        row.cells=values;row.cell_count=count;row.source_row=line_no;values=NULL;
        { VerityRow *next=(VerityRow*)realloc(out->rows,(out->row_count+1)*sizeof(*next)); if(!next){if(error&&error_size)snprintf(error,error_size,"Out of memory");goto row_fail;}out->rows=next;out->rows[out->row_count++]=row;memset(&row,0,sizeof(row));}
        for(i=0;i<count;i++)free(keys?keys[i]:NULL);free(keys);free(values);line_no++;continue;
row_fail:
        free_row(&row);for(i=0;i<count;i++){free(keys?keys[i]:NULL);free(values?values[i]:NULL);}free(keys);free(values);goto done;
    }
    if(out->header_count==0){if(error&&error_size)snprintf(error,error_size,"JSONL contains no records");goto done;}
    ok=1;
done:
    free(buf);if(!ok)verity_table_free(out);return ok;
}
