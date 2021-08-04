//kassert.c
//Kernel assertion macro
//Bryan E. Topp <betopp@betopp.com> 2021

#include "kassert.h"
#include "hal_panic.h"
#include "libcstubs.h"
#include "con.h"

#include "hal_spl.h"

static hal_spl_t kassert_spl;
static char kassert_buf[256];
static int kassert_len;
static void kassert_append(const char *str)
{
	strncpy(kassert_buf + kassert_len, str, sizeof(kassert_buf)-(kassert_len+1));
	kassert_len = strlen(kassert_buf);
}

void kassert_failed(const char *file, const char *line, const char *func, const char *cond)
{
	hal_spl_lock(&kassert_spl);
	kassert_append("Fail:");
	kassert_append(func);
	kassert_append("(");
	kassert_append(file);
	kassert_append(":");
	kassert_append(line);
	kassert_append("):");
	kassert_append(cond);
	
	con_panic(kassert_buf);
	hal_panic(kassert_buf);
}


