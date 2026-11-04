#ifndef MIGRAVEN_MIGRAVEN_H
#define MIGRAVEN_MIGRAVEN_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { MIGRAVEN_PASS=0, MIGRAVEN_FAIL=1, MIGRAVEN_INDETERMINATE=2, MIGRAVEN_ERROR=3 } MigravenStatus;
typedef struct { char **cells; size_t cell_count; size_t source_row; } MigravenRow;
typedef struct { char **headers; size_t header_count; MigravenRow *rows; size_t row_count; } MigravenTable;
typedef struct { char *key; char *left_value; char *right_value; size_t left_row; size_t right_row; MigravenStatus status; } MigravenFinding;
typedef struct { MigravenFinding *findings; size_t finding_count; size_t finding_capacity; size_t matched_count; size_t left_only_count; size_t right_only_count; } MigravenReport;
int migraven_csv_read(const char *path, MigravenTable *out, char *error, size_t error_size);
int migraven_jsonl_read(const char *path, MigravenTable *out, char *error, size_t error_size);
void migraven_table_free(MigravenTable *table);
int migraven_compare(const MigravenTable *left, const MigravenTable *right, const char *key_column, const char *value_column, MigravenReport *out, char *error, size_t error_size);
void migraven_report_free(MigravenReport *report);
const char *migraven_status_name(MigravenStatus status);
void migraven_json_string(const char *value);
#ifdef __cplusplus
}
#endif
#endif
