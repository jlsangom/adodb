
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include "php_ext.h"
#include "adodb.h"

#include <ext/standard/info.h>

#include <Zend/zend_operators.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>

#include "kernel/main.h"
#include "kernel/memory.h"

zend_class_entry *adodb_datasource_ce;
zend_class_entry *adodb_driver_drivermanager_ce;

ZEND_DECLARE_MODULE_GLOBALS(adodb)

PHP_MINIT_FUNCTION(adodb){

	ZEPHIR_INIT(ADOdb_DataSource);
	ZEPHIR_INIT(ADOdb_Driver_DriverManager);
	return SUCCESS;
}

#ifndef ZEPHIR_RELEASE
static PHP_MSHUTDOWN_FUNCTION(adodb){

	assert(ZEPHIR_GLOBAL(function_cache) == NULL);
	//assert(ZEPHIR_GLOBAL(orm).parser_cache == NULL);
	//assert(ZEPHIR_GLOBAL(orm).ast_cache == NULL);

	return SUCCESS;
}
#endif

/**
 * Initialize globals on each request or each thread started
 */
static void php_zephir_init_globals(zend_zephir_globals *zephir_globals TSRMLS_DC) {

	/* Memory options */
	zephir_globals->active_memory = NULL;

	/* Virtual Symbol Tables */
	zephir_globals->active_symbol_table = NULL;

	/* Cache options */
	zephir_globals->function_cache = NULL;

	/* Recursive Lock */
	zephir_globals->recursive_lock = 0;

}

static PHP_RINIT_FUNCTION(adodb){

	php_zephir_init_globals(ZEPHIR_VGLOBAL TSRMLS_CC);
	//adodb_init_interned_strings(TSRMLS_C);

	return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(adodb){

	if (ZEPHIR_GLOBAL(start_memory) != NULL) {
		zephir_clean_restore_stack(TSRMLS_C);
	}

	if (ZEPHIR_GLOBAL(function_cache) != NULL) {
		zend_hash_destroy(ZEPHIR_GLOBAL(function_cache));
		FREE_HASHTABLE(ZEPHIR_GLOBAL(function_cache));
		ZEPHIR_GLOBAL(function_cache) = NULL;
	}

	return SUCCESS;
}

static PHP_MINFO_FUNCTION(adodb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, PHP_ADODB_NAME, "enabled");
	php_info_print_table_row(2, "Version", PHP_ADODB_VERSION);
	php_info_print_table_end();
}

static PHP_GINIT_FUNCTION(adodb)
{
	zephir_memory_entry *start;

	php_zephir_init_globals(adodb_globals TSRMLS_CC);

	/* Start Memory Frame */
	start = (zephir_memory_entry *) pecalloc(1, sizeof(zephir_memory_entry), 1);
	start->addresses       = pecalloc(16, sizeof(zval*), 1);
	start->capacity        = 16;
	start->hash_addresses  = pecalloc(4, sizeof(zval*), 1);
	start->hash_capacity   = 4;

	adodb_globals->start_memory = start;

	/* Global Constants */
	ALLOC_PERMANENT_ZVAL(adodb_globals->global_false);
	INIT_PZVAL(adodb_globals->global_false);
	ZVAL_FALSE(adodb_globals->global_false);
	Z_ADDREF_P(adodb_globals->global_false);

	ALLOC_PERMANENT_ZVAL(adodb_globals->global_true);
	INIT_PZVAL(adodb_globals->global_true);
	ZVAL_TRUE(adodb_globals->global_true);
	Z_ADDREF_P(adodb_globals->global_true);

	ALLOC_PERMANENT_ZVAL(adodb_globals->global_null);
	INIT_PZVAL(adodb_globals->global_null);
	ZVAL_NULL(adodb_globals->global_null);
	Z_ADDREF_P(adodb_globals->global_null);

}

static PHP_GSHUTDOWN_FUNCTION(adodb)
{
	assert(adodb_globals->start_memory != NULL);

	pefree(adodb_globals->start_memory->hash_addresses, 1);
	pefree(adodb_globals->start_memory->addresses, 1);
	pefree(adodb_globals->start_memory, 1);
	adodb_globals->start_memory = NULL;
}

zend_module_entry adodb_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	NULL,
	PHP_ADODB_EXTNAME,
	NULL,
	PHP_MINIT(adodb),
#ifndef ZEPHIR_RELEASE
	PHP_MSHUTDOWN(adodb),
#else
	NULL,
#endif
	PHP_RINIT(adodb),
	PHP_RSHUTDOWN(adodb),
	PHP_MINFO(adodb),
	PHP_ADODB_VERSION,
	ZEND_MODULE_GLOBALS(adodb),
	PHP_GINIT(adodb),
	PHP_GSHUTDOWN(adodb),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_ADODB
ZEND_GET_MODULE(adodb)
#endif
