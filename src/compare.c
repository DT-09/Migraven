#include "verity/verity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void err(char *d,size_t n,const char *s){if(d&&n)(void)snprintf(d,n,"%s",s);}
static size_t col_index(const VerityTable *t,const char *name){size_t i;for(i=0;i<t->header_count;i++)if(strcmp(t->headers[i],name)==0)return i;return (size_t)-1;}
static const char *cell(const VerityRow *r,size_t i){return i<r->cell_count?r->cells[i]:"";}
static char *copy_string(const char *s){size_t n=strlen(s);char *p=(char*)malloc(n+1);if(p)memcpy(p,s,n+1);return p;}
static int add_finding(VerityReport *r,const char *key,const char *lv,const char *rv,size_t lr,size_t rr,VerityStatus st){
    VerityFinding *p=(VerityFinding*)realloc(r->findings,(r->finding_count+1)*sizeof(*p));VerityFinding *f;
    if(!p)return 0;r->findings=p;f=&r->findings[r->finding_count];memset(f,0,sizeof(*f));
    f->key=copy_string(key);f->left_value=copy_string(lv);f->right_value=copy_string(rv);
    if(!f->key||!f->left_value||!f->right_value){free(f->key);free(f->left_value);free(f->right_value);return 0;}
    f->left_row=lr;f->right_row=rr;f->status=st;r->finding_count++;return 1;
}
void verity_report_free(VerityReport *r){size_t i;if(!r)return;for(i=0;i<r->finding_count;i++){free(r->findings[i].key);free(r->findings[i].left_value);free(r->findings[i].right_value);}free(r->findings);memset(r,0,sizeof(*r));}
int verity_compare(const VerityTable *l,const VerityTable *r,const char *kn,const char *vn,VerityReport *out,char *error,size_t error_size){
    size_t lk,lv,rk,rv,i,j;unsigned char *seen;
    if(!l||!r||!out||!kn||!vn){err(error,error_size,"Invalid comparison arguments");return 0;}memset(out,0,sizeof(*out));
    lk=col_index(l,kn);lv=col_index(l,vn);rk=col_index(r,kn);rv=col_index(r,vn);
    if(lk==(size_t)-1||lv==(size_t)-1||rk==(size_t)-1||rv==(size_t)-1){err(error,error_size,"Key or value column not found");return 0;}
    seen=(unsigned char*)calloc(r->row_count?r->row_count:1,1);if(!seen){err(error,error_size,"Out of memory");return 0;}
    for(i=0;i<r->row_count;i++){if(!*cell(&r->rows[i],rk)){free(seen);err(error,error_size,"Empty key in right dataset");goto fail;}for(j=0;j<i;j++)if(strcmp(cell(&r->rows[j],rk),cell(&r->rows[i],rk))==0){free(seen);err(error,error_size,"Duplicate key in right dataset");goto fail;}}
    for(i=0;i<l->row_count;i++){
        const char *key=cell(&l->rows[i],lk);size_t match=(size_t)-1,count=0;
        if(!*key){free(seen);err(error,error_size,"Empty key in left dataset");goto fail;}
        for(j=0;j<i;j++)if(strcmp(cell(&l->rows[j],lk),key)==0){free(seen);err(error,error_size,"Duplicate key in left dataset");goto fail;}
        for(j=0;j<r->row_count;j++)if(strcmp(cell(&r->rows[j],rk),key)==0){match=j;count++;}
        if(count>1){free(seen);err(error,error_size,"Duplicate key in right dataset");goto fail;}
        if(match==(size_t)-1){out->left_only_count++;if(!add_finding(out,key,cell(&l->rows[i],lv),"",l->rows[i].source_row,0,VERITY_INDETERMINATE)){free(seen);err(error,error_size,"Out of memory");goto fail;}}
        else{seen[match]=1;out->matched_count++;if(strcmp(cell(&l->rows[i],lv),cell(&r->rows[match],rv))!=0&&!add_finding(out,key,cell(&l->rows[i],lv),cell(&r->rows[match],rv),l->rows[i].source_row,r->rows[match].source_row,VERITY_FAIL)){free(seen);err(error,error_size,"Out of memory");goto fail;}}
    }
    for(j=0;j<r->row_count;j++)if(!seen[j]){out->right_only_count++;if(!add_finding(out,cell(&r->rows[j],rk),"",cell(&r->rows[j],rv),0,r->rows[j].source_row,VERITY_INDETERMINATE)){free(seen);err(error,error_size,"Out of memory");goto fail;}}
    free(seen);return 1;
fail:verity_report_free(out);return 0;
}
