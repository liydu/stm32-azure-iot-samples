#ifndef SYSTEM_INFORMATION_JSON_PARSER_H
#define SYSTEM_INFORMATION_JSON_PARSER_H

/* Generated by flatcc 0.6.1-dev FlatBuffers schema compiler for C by dvide.com */

#include "flatcc/flatcc_json_parser.h"
#include "flatcc/flatcc_prologue.h"

static const char *AzureIoTSecurity_SystemInformation_parse_json_table(flatcc_json_parser_t *ctx, const char *buf, const char *end, flatcc_builder_ref_t *result);
static const char *system_information_local_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
int *value_type, uint64_t *value, int *aggregate);
static const char *system_information_local_AzureIoTSecurity_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
int *value_type, uint64_t *value, int *aggregate);
static const char *system_information_global_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
        int *value_type, uint64_t *value, int *aggregate);

static const char *AzureIoTSecurity_SystemInformation_parse_json_table(flatcc_json_parser_t *ctx, const char *buf, const char *end, flatcc_builder_ref_t *result)
{
    int more;
    void *pval;
    flatcc_builder_ref_t ref, *pref;
    const char *mark;
    uint64_t w;

    *result = 0;
    if (flatcc_builder_start_table(ctx->ctx, 3)) goto failed;
    buf = flatcc_json_parser_object_start(ctx, buf, end, &more);
    while (more) {
        buf = flatcc_json_parser_symbol_start(ctx, buf, end);
        w = flatcc_json_parser_symbol_part(buf, end);
        if (w < 0x6b65726e656c5f69) { /* branch "kernel_i" */
            if ((w & 0xffffffffffffff00) == 0x68775f696e666f00) { /* "hw_info" */
                buf = flatcc_json_parser_match_symbol(ctx, (mark = buf), end, 7);
                if (mark != buf) {
                    buf = flatcc_json_parser_build_string(ctx, buf, end, &ref);
                    if (!ref || !(pref = flatcc_builder_table_add_offset(ctx->ctx, 2))) goto failed;
                    *pref = ref;
                } else {
                    buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
                }
            } else { /* "hw_info" */
                buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
            } /* "hw_info" */
        } else { /* branch "kernel_i" */
            if (w == 0x6b65726e656c5f69) { /* descend "kernel_i" */
                buf += 8;
                w = flatcc_json_parser_symbol_part(buf, end);
                if ((w & 0xffffff0000000000) == 0x6e666f0000000000) { /* "nfo" */
                    buf = flatcc_json_parser_match_symbol(ctx, (mark = buf), end, 3);
                    if (mark != buf) {
                        buf = flatcc_json_parser_build_string(ctx, buf, end, &ref);
                        if (!ref || !(pref = flatcc_builder_table_add_offset(ctx->ctx, 1))) goto failed;
                        *pref = ref;
                    } else {
                        buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
                    }
                } else { /* "nfo" */
                    buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
                } /* "nfo" */
            } else { /* descend "kernel_i" */
                if ((w & 0xffffffffffffff00) == 0x6f735f696e666f00) { /* "os_info" */
                    buf = flatcc_json_parser_match_symbol(ctx, (mark = buf), end, 7);
                    if (mark != buf) {
                        buf = flatcc_json_parser_build_string(ctx, buf, end, &ref);
                        if (!ref || !(pref = flatcc_builder_table_add_offset(ctx->ctx, 0))) goto failed;
                        *pref = ref;
                    } else {
                        buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
                    }
                } else { /* "os_info" */
                    buf = flatcc_json_parser_unmatched_symbol(ctx, buf, end);
                } /* "os_info" */
            } /* descend "kernel_i" */
        } /* branch "kernel_i" */
        buf = flatcc_json_parser_object_end(ctx, buf, end, &more);
    }
    if (ctx->error) goto failed;
    if (!(*result = flatcc_builder_end_table(ctx->ctx))) goto failed;
    return buf;
failed:
    return flatcc_json_parser_set_error(ctx, buf, end, flatcc_json_parser_error_runtime);
}

static inline int AzureIoTSecurity_SystemInformation_parse_json_as_root(flatcc_builder_t *B, flatcc_json_parser_t *ctx, const char *buf, size_t bufsiz, int flags, const char *fid)
{
    return flatcc_json_parser_table_as_root(B, ctx, buf, bufsiz, flags, fid, AzureIoTSecurity_SystemInformation_parse_json_table);
}

static const char *system_information_local_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
        int *value_type, uint64_t *value, int *aggregate)
{
    /* Scope has no enum / union types to look up. */
    return buf; /* unmatched; */
}

static const char *system_information_local_AzureIoTSecurity_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
        int *value_type, uint64_t *value, int *aggregate)
{
    /* Scope has no enum / union types to look up. */
    return buf; /* unmatched; */
}

static const char *system_information_global_json_parser_enum(flatcc_json_parser_t *ctx, const char *buf, const char *end,
        int *value_type, uint64_t *value, int *aggregate)
{
    /* Global scope has no enum / union types to look up. */
    return buf; /* unmatched; */
}

#include "flatcc/flatcc_epilogue.h"
#endif /* SYSTEM_INFORMATION_JSON_PARSER_H */
