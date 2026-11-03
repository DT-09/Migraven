#include "verity/verity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define V_MAX_FILE_BYTES ((size_t)268435456)
#define V_MAX_FIELD_BYTES ((size_t)16777216)
static void set_error(char *d,size_t n,const char *s){if(d&&n)(void)snprintf(d,n,"%s",s);}
static char *dup_n(const char *s,size_t n){char *p=(char*)malloc(n+1);if(!p)return NULL;memcpy(p,s,n);p[n]='\\0';return p;}
static int append_cell(VerityRow *r,const char *s,size_t n){
    char **p;if(n>V_MAX_FIELD_BYTES)return 0;p=(char**)realloc(r->cells,(r->cell_count+1)*sizeof(*p));if(!p)return 0;
    r->cells=p;r->cells[r->cell_count]=dup_n(s,n);if(!r->cells[r->cell_count])return 0;r->cell_count++;return 1;
}
static int append_row(VerityTable *t,VerityRow *r){VerityRow *p=(VerityRow*)realloc(t->rows,(t->row_count+1)*sizeof(*p));if(!p)return 0;t->rows=p;t->rows[t->row_count++]=*r;memset(r,0,sizeof(*r));return 1;}
static void row_free(VerityRow *r){size_t i;for(i=0;i<r->cell_count;i++)free(r->cells[i]);free(r->cells);memset(r,0,sizeof(*r));}
void verity_table_free(VerityTable *t){size_t i;if(!t)return;for(i=0;i<t->header_count;i++)free(t->headers[i]);free(t->headers);for(i=0;i<t->row_count;i++)row_free(&t->rows[i]);free(t->rows);memset(t,0,sizeof(*t));}
int verity_csv_read(const char *path,VerityTable *out,char *error,size_t error_size){
    FILE *fp;long end;size_t len,i=0,row_no=1;char *buf,*field;size_t flen=0,fcap=128;VerityRow row={0};int quoted=0,after_quote=0;
    if(!path||!out){set_error(error,error_size,"Invalid argument");return 0;}memset(out,0,sizeof(*out));
    fp=fopen(path,"rb");if(!fp){set_error(error,error_size,"Cannot open input file");return 0;}
    if(fseek(fp,0,SEEK_END)!=0||(end=ftell(fp))<0||(unsigned long)end>V_MAX_FILE_BYTES||fseek(fp,0,SEEK_SET)!=0){fclose(fp);set_error(error,error_size,"Input exceeds 256 MiB or cannot be read");return 0;}
    len=(size_t)end;buf=(char*)malloc(len+1);if(!buf){fclose(fp);set_error(error,error_size,"Out of memory");return 0;}
    if(fread(buf,1,len,fp)!=len){free(buf);fclose(fp);set_error(error,error_size,"Failed to read input");return 0;}fclose(fp);buf[len]='\\0';
    field=(char*)malloc(fcap);if(!field){free(buf);set_error(error,error_size,"Out of memory");return 0;}
    while(i<=len){
        char c=(i==len)?'\\n':buf[i];
        if(quoted){
            if(c=='"'){if(i+1<len&&buf[i+1]=='"'){c='"';i++;}else{quoted=0;after_quote=1;i++;continue;}}
            if(flen+2>fcap){size_t nc=fcap*2;char *p;if(nc>V_MAX_FIELD_BYTES+1)goto malformed;p=(char*)realloc(field,nc);if(!p)goto oom;field=p;fcap=nc;}
            field[flen++]=c;i++;continue;
        }
        if(after_quote&&c!=','&&c!='\\r'&&c!='\\n'&&!isspace((unsigned char)c))goto malformed;
        if(c=='"'&&flen==0&&!after_quote){quoted=1;i++;continue;}
        if(c==','||c=='\\n'||c=='\\r'){
            if(!append_cell(&row,field,flen))goto oom;flen=0;after_quote=0;
            if(c=='\\r'&&i+1<len&&buf[i+1]=='\\n')i++;
            if(c=='\\n'||c=='\\r'){
                if(row.cell_count>1||(row.cell_count==1&&row.cells[0][0]!='\\0')){
                    row.source_row=row_no++;
                    if(out->header_count==0){out->headers=row.cells;out->header_count=row.cell_count;memset(&row,0,sizeof(row));}
                    else if(!append_row(out,&row))goto oom;
                }
            }
            i++;continue;
        }
        if(flen+2>fcap){size_t nc=fcap*2;char *p;if(nc>V_MAX_FIELD_BYTES+1)goto malformed;p=(char*)realloc(field,nc);if(!p)goto oom;field=p;fcap=nc;}
        field[flen++]=c;i++;
    }
    if(quoted){set_error(error,error_size,"Unterminated quoted CSV field");goto fail;}
    if(out->header_count==0){set_error(error,error_size,"CSV has no header row");goto fail;}
    for(i=0;i<out->row_count;i++)if(out->rows[i].cell_count!=out->header_count){set_error(error,error_size,"CSV row width differs from header");goto fail;}
    free(field);free(buf);return 1;
malformed:set_error(error,error_size,"Malformed CSV quoting or characters after closing quote");goto fail;
oom:set_error(error,error_size,"Out of memory or field limit exceeded");
fail:row_free(&row);free(field);free(buf);verity_table_free(out);return 0;
}
