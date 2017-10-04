#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include "math.h"

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)


#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, int type, char *str)
{
    int i = 0;
    int len = strlen(str);
    for (i = 0;i < len;i++) {
        if (c->json[i] != str[i]) {
            return LEPT_PARSE_INVALID_VALUE; 
        }
    }
    c->json += len;
    v->type = type;
    return LEPT_PARSE_OK;
}


static void check_number_int(char **number)
{
    while (**number != '\0' && (ISDIGIT(**number))) {
        *number += 1;
    }
}

static int check_number_exp(char **number)
{
    *number += 1;
    if (**number == '+' || **number == '-') {
        *number += 1;  
    }
    if (!ISDIGIT(**number)) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    check_number_int(number);
    if (**number != '\0') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    return LEPT_PARSE_OK;
}


static int lept_parse_number(lept_context* c, lept_value* v) {
    char *json = c->json;
    if (*json == '-') {
        json += 1;
    }
    if (!ISDIGIT(*json)) {
        return LEPT_PARSE_INVALID_VALUE;
    }

    if (ISDIGIT1TO9(*json)) {
        json += 1;
        check_number_int(&json);    
    } else {
        json += 1;
        if (*json != '.' && *json != '\0') {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }

    if (*json == '.') {
        json += 1;
        if (!ISDIGIT(*json)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        check_number_int(&json);
        if (*(json) == 'e' || *(json) == 'E') {
            if (check_number_exp(&json) == LEPT_PARSE_INVALID_VALUE) {
                return LEPT_PARSE_INVALID_VALUE;
            }
        } else if (*(json) == '\0') {

        } else {
            return LEPT_PARSE_INVALID_VALUE;
        }
    } else if (*json == 'e' || *json == 'E') {
            if (check_number_exp(&json) == LEPT_PARSE_INVALID_VALUE) {
                return LEPT_PARSE_INVALID_VALUE;
            }
    } else if (*json == '\0') {
        
    } else {
        return LEPT_PARSE_INVALID_VALUE;
    }
    
    /* \TODO validate number */
    v->n = strtod(c->json, NULL);
    if (v->n == HUGE_VAL || v->n == -HUGE_VAL) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = json;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, LEPT_TRUE, "true");
        case 'f':  return lept_parse_literal(c, v, LEPT_FALSE, "false");
        case 'n':  return lept_parse_literal(c, v, LEPT_NULL, "null");
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
