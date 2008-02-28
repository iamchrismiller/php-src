/* 
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2008 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Colin Viebrock <colin@easydns.com>                          |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "info.h"
#include "credits.h"
#include "css.h"
#include "SAPI.h"
#include <time.h>
#include "php_main.h"
#include "zend_globals.h"		/* needs ELS */
#include "zend_extensions.h"
#include "zend_highlight.h"
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#if HAVE_MBSTRING
#include "ext/mbstring/mbstring.h"
ZEND_EXTERN_MODULE_GLOBALS(mbstring)
#endif

#if HAVE_ICONV
#include "ext/iconv/php_iconv.h"
ZEND_EXTERN_MODULE_GLOBALS(iconv)
#endif

#include <unicode/uversion.h>

#define SECTION(name)	if (!sapi_module.phpinfo_as_text) { \
							php_info_print("<h2>" name "</h2>\n"); \
						} else { \
							php_info_print_table_start(); \
							php_info_print_table_header(1, name); \
							php_info_print_table_end(); \
						} \

PHPAPI extern char *php_ini_opened_path;
PHPAPI extern char *php_ini_scanned_files;

static int php_info_print_html_esc(const char *str, int len) /* {{{ */
{
	int new_len, written;
	char *new_str;
	TSRMLS_FETCH();
	
	new_str = php_escape_html_entities((char *) str, len, &new_len, 0, ENT_QUOTES, "utf-8" TSRMLS_CC);
	written = php_output_write_utf8(new_str, new_len TSRMLS_CC);
	efree(new_str);
	return written;
}
/* }}} */

static int php_info_uprint_html_esc(const UChar *str, int len) /* {{{ */
{
	UErrorCode status = U_ZERO_ERROR;
	char *new_str = NULL;
	int new_len, written;
	TSRMLS_FETCH();
	
	zend_unicode_to_string_ex(UG(utf8_conv), &new_str, &new_len, str, len, &status);
	if (U_FAILURE(status)) {
		return 0;
	}
	written = php_info_print_html_esc(new_str, new_len);
	efree(new_str);
	return written;
}
/* }}} */

static int php_info_printf(const char *fmt, ...) /* {{{ */
{
	char *buf;
	int len, written;
	va_list argv;
	TSRMLS_FETCH();
	
	va_start(argv, fmt);
	len = vspprintf(&buf, 0, fmt, argv);
	va_end(argv);
	
	written = php_output_write_utf8(buf, len TSRMLS_CC);
	efree(buf);
	return written;
}
/* }}} */

static void php_info_print_request_uri(TSRMLS_D) /* {{{ */
{
	if (SG(request_info).request_uri) {
		php_info_print_html_esc(SG(request_info).request_uri, strlen(SG(request_info).request_uri));
	}
}
/* }}} */

static int php_info_print(const char *str) /* {{{ */
{
	TSRMLS_FETCH();
	return php_output_write_utf8(str, strlen(str) TSRMLS_CC);
}
/* }}} */

static int php_info_uprint(const UChar *str, int len) /* {{{ */
{
	TSRMLS_FETCH();
	return php_output_write_unicode(str, len TSRMLS_CC);
}
/* }}} */

static void php_info_print_stream_hash(const char *name, HashTable *ht TSRMLS_DC) /* {{{ */
{
	zstr key;
	uint len;
	int type;
	
	if (ht) {
		if (zend_hash_num_elements(ht)) {
			if (!sapi_module.phpinfo_as_text) {
				php_info_printf("<tr class=\"v\"><td>Registered %s</td><td>", name);
			} else {
				php_info_printf("\nRegistered %s => ", name);
			}
			
			zend_hash_internal_pointer_reset(ht);
			type = zend_hash_get_current_key_ex(ht, &key, &len, NULL, 0, NULL);
			do {
				switch (type) {
					case IS_STRING:
						php_info_print(key.s);
						break;
					case IS_UNICODE:
						php_info_uprint(key.u, len);
						break;
				}
				
				zend_hash_move_forward(ht);
				type = zend_hash_get_current_key_ex(ht, &key, &len, NULL, 0, NULL);
				
				if (type == IS_STRING || type == IS_UNICODE) {
					php_info_print(", ");
				} else {
					break;
				}
			} while (1);
			
			if (!sapi_module.phpinfo_as_text) {
				php_info_print("</td></tr>\n");
			}
		} else {
			char reg_name[128];
			snprintf(reg_name, sizeof(reg_name), "Registered %s", name);
			php_info_print_table_row(2, reg_name, "none registered");
		}
	} else {
		php_info_print_table_row(2, name, "disabled");
	}
}
/* }}} */

PHPAPI void php_info_print_module(zend_module_entry *module TSRMLS_DC) /* {{{ */
{
	if (module->info_func) {
		if (!sapi_module.phpinfo_as_text) {
			php_info_printf("<h2><a name=\"module_%s\">%s</a></h2>\n", module->name, module->name);
		} else {
			php_info_print_table_start();
			php_info_print_table_header(1, module->name);
			php_info_print_table_end();
		}
		module->info_func(module TSRMLS_CC);
	} else {
		if (!sapi_module.phpinfo_as_text) {
			php_info_printf("<tr>");
			php_info_printf("<td>");
			php_info_printf("%s", module->name);
			php_info_printf("</td></tr>\n");
		} else {
			php_info_printf("%s", module->name);
			php_info_printf("\n");
		}	
	}
}
/* }}} */

static int _display_module_info_func(zend_module_entry *module TSRMLS_DC) /* {{{ */
{
	if (module->info_func) {
		php_info_print_module(module TSRMLS_CC);
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int _display_module_info_def(zend_module_entry *module TSRMLS_DC) /* {{{ */
{
	if (!module->info_func) {
		php_info_print_module(module TSRMLS_CC);
	}
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ php_print_gpcse_array
 */
static void php_print_gpcse_array(char *name, uint name_length TSRMLS_DC)
{
	zval **data, **tmp, tmp2;
	zstr string_key;
	uint string_len;
	ulong num_key;

	zend_is_auto_global(name, name_length TSRMLS_CC);

	if (zend_hash_find(&EG(symbol_table), name, name_length+1, (void **) &data)!=FAILURE
		&& (Z_TYPE_PP(data)==IS_ARRAY)) {
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(data));
		while (zend_hash_get_current_data(Z_ARRVAL_PP(data), (void **) &tmp) == SUCCESS) {
			if (!sapi_module.phpinfo_as_text) {
				php_info_print("<tr>");
				php_info_print("<td class=\"e\">");
			}

			php_info_print(name);
			php_info_print("[\"");
			
			switch (zend_hash_get_current_key_ex(Z_ARRVAL_PP(data), &string_key, &string_len, &num_key, 0, NULL)) {
				case HASH_KEY_IS_UNICODE:
					if (!sapi_module.phpinfo_as_text) {
						php_info_uprint_html_esc(string_key.u, string_len-1);
					} else {
						php_info_uprint(string_key.u, string_len);
					}
					break;
				case HASH_KEY_IS_STRING:
					if (!sapi_module.phpinfo_as_text) {
						php_info_print_html_esc(string_key.s, string_len-1);
					} else {
						php_info_print(string_key.s);
					}
					break;
				case HASH_KEY_IS_LONG:
					php_info_printf("%ld", num_key);
					break;
			}
			php_info_print("\"]");
			if (!sapi_module.phpinfo_as_text) {
				php_info_print("</td><td class=\"v\">");
			} else {
				php_info_print(" => ");
			}
			if (Z_TYPE_PP(tmp) == IS_ARRAY) {
				if (!sapi_module.phpinfo_as_text) {
					php_info_print("<pre>");
					zend_print_zval_r_ex((zend_write_func_t) php_info_print_html_esc, *tmp, 0 TSRMLS_CC);
					php_info_print("</pre>");
				} else {
					zend_print_zval_r(*tmp, 0 TSRMLS_CC);
				}
			} else {
				tmp2 = **tmp;
				switch (Z_TYPE_PP(tmp)) {
					default:
						tmp = NULL;
						zval_copy_ctor(&tmp2);
						convert_to_string(&tmp2);
					case IS_STRING:
					case IS_UNICODE:
						if (!sapi_module.phpinfo_as_text) {
							if (Z_UNILEN(tmp2) == 0) {
								php_info_print("<i>no value</i>");
							} else if (Z_TYPE(tmp2) == IS_UNICODE){
								php_info_uprint_html_esc(Z_USTRVAL(tmp2), Z_USTRLEN(tmp2));
							} else {
								php_info_print_html_esc(Z_STRVAL(tmp2), Z_STRLEN(tmp2));
							}
						} else if(Z_TYPE(tmp2) == IS_UNICODE) {
							php_info_uprint(Z_USTRVAL(tmp2), Z_USTRLEN(tmp2));
						} else {
							php_info_print(Z_STRVAL(tmp2));
						}
				}
				if (!tmp) {
					zval_dtor(&tmp2);
				}
			}
			if (!sapi_module.phpinfo_as_text) {
				php_info_print("</td></tr>\n");
			} else {
				php_info_print("\n");
			}
			zend_hash_move_forward(Z_ARRVAL_PP(data));
		}
	}
}
/* }}} */

/* {{{ php_info_print_style
 */
void php_info_print_style(TSRMLS_D)
{
	php_info_printf("<style type=\"text/css\">\n");
	php_info_print_css(TSRMLS_C);
	php_info_printf("</style>\n");
}
/* }}} */

/* {{{ php_info_html_esc
 */
PHPAPI char *php_info_html_esc(char *string TSRMLS_DC)
{
	int new_len;
	return php_escape_html_entities(string, strlen(string), &new_len, 0, ENT_QUOTES, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ php_get_uname
 */
PHPAPI char *php_get_uname(char mode)
{
	char *php_uname;
	char tmp_uname[256];
#ifdef PHP_WIN32
	DWORD dwBuild=0;
	DWORD dwVersion = GetVersion();
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
	char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	SYSTEM_INFO SysInfo;

	GetComputerName(ComputerName, &dwSize);
	GetSystemInfo(&SysInfo);

	if (mode == 's') {
		if (dwVersion < 0x80000000) {
			php_uname = "Windows NT";
		} else {
			php_uname = "Windows 9x";
		}
	} else if (mode == 'r') {
		snprintf(tmp_uname, sizeof(tmp_uname), "%d.%d", dwWindowsMajorVersion, dwWindowsMinorVersion);
		php_uname = tmp_uname;
	} else if (mode == 'n') {
		php_uname = ComputerName;
	} else if (mode == 'v') {
		dwBuild = (DWORD)(HIWORD(dwVersion));
		snprintf(tmp_uname, sizeof(tmp_uname), "build %d", dwBuild);
		php_uname = tmp_uname;
	} else if (mode == 'm') {
		switch (SysInfo.wProcessorArchitecture) {
			case PROCESSOR_ARCHITECTURE_INTEL :
				snprintf(tmp_uname, sizeof(tmp_uname), "i%d", SysInfo.dwProcessorType);
				php_uname = tmp_uname;
				break;
			case PROCESSOR_ARCHITECTURE_MIPS :
				php_uname = "MIPS R4000";
				php_uname = tmp_uname;
				break;
			case PROCESSOR_ARCHITECTURE_ALPHA :
				snprintf(tmp_uname, sizeof(tmp_uname), "Alpha %d", SysInfo.wProcessorLevel);
				php_uname = tmp_uname;
				break;
			case PROCESSOR_ARCHITECTURE_PPC :
				snprintf(tmp_uname, sizeof(tmp_uname), "PPC 6%02d", SysInfo.wProcessorLevel);
				php_uname = tmp_uname;
				break;
			case PROCESSOR_ARCHITECTURE_IA64 :
				php_uname = "IA64";
				break;
#if defined(PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)
			case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 :
				php_uname = "IA32";
				break;
#endif
#if defined(PROCESSOR_ARCHITECTURE_AMD64)
			case PROCESSOR_ARCHITECTURE_AMD64 :
				php_uname = "AMD64";
				break;
#endif
			case PROCESSOR_ARCHITECTURE_UNKNOWN :
			default :
				php_uname = "Unknown";
				break;
		}
	} else { /* assume mode == 'a' */
		/* Get build numbers for Windows NT or Win95 */
		if (dwVersion < 0x80000000){
			dwBuild = (DWORD)(HIWORD(dwVersion));
			snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %d.%d build %d",
					 "Windows NT", ComputerName,
					 dwWindowsMajorVersion, dwWindowsMinorVersion, dwBuild);
		} else {
			snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %d.%d",
					 "Windows 9x", ComputerName,
					 dwWindowsMajorVersion, dwWindowsMinorVersion);
		}
		php_uname = tmp_uname;
	}
#else
#ifdef HAVE_SYS_UTSNAME_H
	struct utsname buf;
	if (uname((struct utsname *)&buf) == -1) {
		php_uname = PHP_UNAME;
	} else {
		if (mode == 's') {
			php_uname = buf.sysname;
		} else if (mode == 'r') {
			php_uname = buf.release;
		} else if (mode == 'n') {
			php_uname = buf.nodename;
		} else if (mode == 'v') {
			php_uname = buf.version;
		} else if (mode == 'm') {
			php_uname = buf.machine;
		} else { /* assume mode == 'a' */
			snprintf(tmp_uname, sizeof(tmp_uname), "%s %s %s %s %s",
					 buf.sysname, buf.nodename, buf.release, buf.version,
					 buf.machine);
			php_uname = tmp_uname;
		}
	}
#else
	php_uname = PHP_UNAME;
#endif
#endif
	return estrdup(php_uname);
}
/* }}} */

/* {{{ php_print_info_htmlhead
 */
PHPAPI void php_print_info_htmlhead(TSRMLS_D)
{
	php_info_print("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-transitional.dtd\">\n");
	php_info_print("<html>");
	php_info_print("<head>\n");
	php_info_print_style(TSRMLS_C);
	php_info_print("<title>phpinfo()</title>");
	php_info_print("<meta name=\"ROBOTS\" content=\"NOINDEX,NOFOLLOW,NOARCHIVE\" />");
	php_info_print("</head>\n");
	php_info_print("<body><div class=\"center\">\n");
}
/* }}} */

/* {{{ module_name_cmp */
static int module_name_cmp(const void *a, const void *b TSRMLS_DC)
{
	Bucket *f = *((Bucket **) a);
	Bucket *s = *((Bucket **) b);

	return strcasecmp(((zend_module_entry *)f->pData)->name,
				  ((zend_module_entry *)s->pData)->name);
}
/* }}} */

/* {{{ php_print_info
 */
PHPAPI void php_print_info(int flag TSRMLS_DC)
{
	char **env, *tmp1, *tmp2;
	char *php_uname;
	int expose_php = INI_INT("expose_php");

	if (!sapi_module.phpinfo_as_text) {
		php_print_info_htmlhead(TSRMLS_C);
	} else {
		php_info_print("phpinfo()\n");
	}

	if (flag & PHP_INFO_GENERAL) {
		char *zend_version = get_zend_version();
		char temp_api[10];
		char *logo_guid;

		php_uname = php_get_uname('a');
		
		if (!sapi_module.phpinfo_as_text) {
			php_info_print_box_start(1);
		}

		if (expose_php && !sapi_module.phpinfo_as_text) {
			php_info_print("<a href=\"http://www.php.net/\"><img border=\"0\" src=\"");
			php_info_print_request_uri(TSRMLS_C);
			php_info_print("?=");
			logo_guid = php_logo_guid();
			php_info_print(logo_guid);
			efree(logo_guid);
			php_info_print("\" alt=\"PHP Logo\" /></a>");
		}

		if (!sapi_module.phpinfo_as_text) {
			php_info_printf("<h1 class=\"p\">PHP Version %s</h1>\n", PHP_VERSION);
		} else {
			php_info_print_table_row(2, "PHP Version", PHP_VERSION);
		}	
		php_info_print_box_end();
		php_info_print_table_start();
		php_info_print_table_row(2, "System", php_uname );
		php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__ );
#ifdef CONFIGURE_COMMAND
		php_info_print_table_row(2, "Configure Command", CONFIGURE_COMMAND );
#endif
		if (sapi_module.pretty_name) {
			php_info_print_table_row(2, "Server API", sapi_module.pretty_name );
		}

#ifdef VIRTUAL_DIR
		php_info_print_table_row(2, "Virtual Directory Support", "enabled" );
#else
		php_info_print_table_row(2, "Virtual Directory Support", "disabled" );
#endif

		php_info_print_table_row(2, "Configuration File (php.ini) Path", PHP_CONFIG_FILE_PATH);
		php_info_print_table_row(2, "Loaded Configuration File", php_ini_opened_path ? php_ini_opened_path : "(none)");

		if (strlen(PHP_CONFIG_FILE_SCAN_DIR)) {
			php_info_print_table_row(2, "Scan this dir for additional .ini files", PHP_CONFIG_FILE_SCAN_DIR);
			if (php_ini_scanned_files) {
				php_info_print_table_row(2, "additional .ini files parsed", php_ini_scanned_files);
			}
		}
		
		snprintf(temp_api, sizeof(temp_api), "%d", PHP_API_VERSION);
		php_info_print_table_row(2, "PHP API", temp_api);

		snprintf(temp_api, sizeof(temp_api), "%d", ZEND_MODULE_API_NO);
		php_info_print_table_row(2, "PHP Extension", temp_api);

		snprintf(temp_api, sizeof(temp_api), "%d", ZEND_EXTENSION_API_NO);
		php_info_print_table_row(2, "Zend Extension", temp_api);

#if ZEND_DEBUG
		php_info_print_table_row(2, "Debug Build", "yes" );
#else
		php_info_print_table_row(2, "Debug Build", "no" );
#endif

#ifdef ZTS
		php_info_print_table_row(2, "Thread Safety", "enabled" );
#else
		php_info_print_table_row(2, "Thread Safety", "disabled" );
#endif

		php_info_print_table_row(2, "Zend Memory Manager", is_zend_mm(TSRMLS_C) ? "enabled" : "disabled" );

		{
			char buf[1024];
			snprintf(buf, sizeof(buf), "Based on%s. ICU Version %s.", U_COPYRIGHT_STRING, U_ICU_VERSION);
			php_info_print_table_row(2, "Unicode Support", buf);
		}
#if HAVE_IPV6
		php_info_print_table_row(2, "IPv6 Support", "enabled" );
#else
		php_info_print_table_row(2, "IPv6 Support", "disabled" );
#endif
		
		php_info_print_stream_hash("PHP Streams",  php_stream_get_url_stream_wrappers_hash() TSRMLS_CC);
		php_info_print_stream_hash("Stream Socket Transports", php_stream_xport_get_hash() TSRMLS_CC);
		php_info_print_stream_hash("Stream Filters", php_get_stream_filters_hash() TSRMLS_CC);

		php_info_print_table_end();

		/* Zend Engine */
		php_info_print_box_start(0);
		if (expose_php && !sapi_module.phpinfo_as_text) {
			php_info_print("<a href=\"http://www.zend.com/\"><img border=\"0\" src=\"");
			php_info_print_request_uri(TSRMLS_C);
			php_info_print("?="ZEND_LOGO_GUID"\" alt=\"Zend logo\" /></a>\n");
		}
		php_info_print("This program makes use of the Zend Scripting Language Engine:");
		php_info_print(!sapi_module.phpinfo_as_text?"<br />":"\n");
		if (sapi_module.phpinfo_as_text) {
			php_info_print(zend_version);
		} else {
			zend_html_puts(zend_version, strlen(zend_version) TSRMLS_CC);
		}
		php_info_print_box_end();
		efree(php_uname);
	}

	if ((flag & PHP_INFO_CREDITS) && expose_php && !sapi_module.phpinfo_as_text) {	
		php_info_print_hr();
		php_info_print("<h1><a href=\"");
		php_info_print_request_uri(TSRMLS_C);
		php_info_print("?=PHPB8B5F2A0-3C92-11d3-A3A9-4C7B08C10000\">");
		php_info_print("PHP Credits");
		php_info_print("</a></h1>\n");
	}

	zend_ini_sort_entries(TSRMLS_C);

	if (flag & PHP_INFO_CONFIGURATION) {
		php_info_print_hr();
		if (!sapi_module.phpinfo_as_text) {
			php_info_print("<h1>Configuration</h1>\n");
		} else {
			SECTION("Configuration");
		}	
		SECTION("PHP Core");
		display_ini_entries(NULL);
	}

	if (flag & PHP_INFO_MODULES) {
		HashTable sorted_registry;
		zend_module_entry tmp;

		zend_hash_init(&sorted_registry, zend_hash_num_elements(&module_registry), NULL, NULL, 1);
		zend_hash_copy(&sorted_registry, &module_registry, NULL, &tmp, sizeof(zend_module_entry));
		zend_hash_sort(&sorted_registry, zend_qsort, module_name_cmp, 0 TSRMLS_CC);

		zend_hash_apply(&sorted_registry, (apply_func_t) _display_module_info_func TSRMLS_CC);

		SECTION("Additional Modules");
		php_info_print_table_start();
		php_info_print_table_header(1, "Module Name");
		zend_hash_apply(&sorted_registry, (apply_func_t) _display_module_info_def TSRMLS_CC);
		php_info_print_table_end();

		zend_hash_destroy(&sorted_registry);
	}

	if (flag & PHP_INFO_ENVIRONMENT) {
		SECTION("Environment");
		php_info_print_table_start();
		php_info_print_table_header(2, "Variable", "Value");
		for (env=environ; env!=NULL && *env !=NULL; env++) {
			tmp1 = estrdup(*env);
			if (!(tmp2=strchr(tmp1,'='))) { /* malformed entry? */
				efree(tmp1);
				continue;
			}
			*tmp2 = 0;
			tmp2++;
			php_info_print_table_row(2, tmp1, tmp2);
			efree(tmp1);
		}
		php_info_print_table_end();
	}

	if (flag & PHP_INFO_VARIABLES) {
		zval **data;

		SECTION("PHP Variables");

		php_info_print_table_start();
		php_info_print_table_header(2, "Variable", "Value");
		if (zend_ascii_hash_find(&EG(symbol_table), "PHP_SELF", sizeof("PHP_SELF"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_SELF", Z_STRVAL_PP(data));
		}
		if (zend_ascii_hash_find(&EG(symbol_table), "PHP_AUTH_TYPE", sizeof("PHP_AUTH_TYPE"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_TYPE", Z_STRVAL_PP(data));
		}
		if (zend_ascii_hash_find(&EG(symbol_table), "PHP_AUTH_USER", sizeof("PHP_AUTH_USER"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_USER", Z_STRVAL_PP(data));
		}
		if (zend_ascii_hash_find(&EG(symbol_table), "PHP_AUTH_PW", sizeof("PHP_AUTH_PW"), (void **) &data) != FAILURE) {
			php_info_print_table_row(2, "PHP_AUTH_PW", Z_STRVAL_PP(data));
		}
		php_print_gpcse_array("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
		php_print_gpcse_array("_GET", sizeof("_GET")-1 TSRMLS_CC);
		php_print_gpcse_array("_POST", sizeof("_POST")-1 TSRMLS_CC);
		php_print_gpcse_array("_FILES", sizeof("_FILES")-1 TSRMLS_CC);
		php_print_gpcse_array("_COOKIE", sizeof("_COOKIE")-1 TSRMLS_CC);
		php_print_gpcse_array("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
		php_print_gpcse_array("_ENV", sizeof("_ENV")-1 TSRMLS_CC);
		php_info_print_table_end();
	}

	if (flag & PHP_INFO_LICENSE) {
		if (!sapi_module.phpinfo_as_text) {
			SECTION("PHP License");
			php_info_print_box_start(0);
			php_info_print("<p>\n");
			php_info_print("This program is free software; you can redistribute it and/or modify ");
			php_info_print("it under the terms of the PHP License as published by the PHP Group ");
			php_info_print("and included in the distribution in the file:  LICENSE\n");
			php_info_print("</p>\n");
			php_info_print("<p>");
			php_info_print("This program is distributed in the hope that it will be useful, ");
			php_info_print("but WITHOUT ANY WARRANTY; without even the implied warranty of ");
			php_info_print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
			php_info_print("</p>\n");
			php_info_print("<p>");
			php_info_print("If you did not receive a copy of the PHP license, or have any questions about ");
			php_info_print("PHP licensing, please contact license@php.net.\n");
			php_info_print("</p>\n");
			php_info_print_box_end();
		} else {
			php_info_print("\nPHP License\n");
			php_info_print("This program is free software; you can redistribute it and/or modify\n");
			php_info_print("it under the terms of the PHP License as published by the PHP Group\n");
			php_info_print("and included in the distribution in the file:  LICENSE\n");
			php_info_print("\n");
			php_info_print("This program is distributed in the hope that it will be useful,\n");
			php_info_print("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
			php_info_print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
			php_info_print("\n");
			php_info_print("If you did not receive a copy of the PHP license, or have any\n");
			php_info_print("questions about PHP licensing, please contact license@php.net.\n");
		}
	}
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("</div></body></html>");
	}	
}
/* }}} */

PHPAPI void php_info_print_table_start(void) /* {{{ */
{
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("<table border=\"0\" cellpadding=\"3\" width=\"600\">\n");
	} else {
		php_info_print("\n");
	}	
}
/* }}} */

PHPAPI void php_info_print_table_end(void) /* {{{ */
{
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("</table><br />\n");
	}

}
/* }}} */

PHPAPI void php_info_print_box_start(int flag) /* {{{ */
{
	php_info_print_table_start();
	if (flag) {
		if (!sapi_module.phpinfo_as_text) {
			php_info_print("<tr class=\"h\"><td>\n");
		}
	} else {
		if (!sapi_module.phpinfo_as_text) {
			php_info_print("<tr class=\"v\"><td>\n");
		} else {
			php_info_print("\n");
		}	
	}
}
/* }}} */

PHPAPI void php_info_print_box_end(void) /* {{{ */
{
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("</td></tr>\n");
	}
	php_info_print_table_end();
}
/* }}} */

PHPAPI void php_info_print_hr(void) /* {{{ */
{
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("<hr />\n");
	} else {
		php_info_print("\n\n _______________________________________________________________________\n\n");
	}
}
/* }}} */

PHPAPI void php_info_print_table_colspan_header(int num_cols, char *header) /* {{{ */
{
	int spaces;

	if (!sapi_module.phpinfo_as_text) {
		php_info_printf("<tr class=\"h\"><th colspan=\"%d\">%s</th></tr>\n", num_cols, header );
	} else {
		spaces = (74 - strlen(header));
		php_info_printf("%*s%s%*s\n", (int)(spaces/2), " ", header, (int)(spaces/2), " ");
	}	
}
/* }}} */

/* {{{ php_info_print_table_header
 */
PHPAPI void php_info_print_table_header(int num_cols, ...)
{
	int i;
	va_list row_elements;
	char *row_element;

	va_start(row_elements, num_cols);
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("<tr class=\"h\">");
	}	
	for (i=0; i<num_cols; i++) {
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			row_element = " ";
		}
		if (!sapi_module.phpinfo_as_text) {
			php_info_print("<th>");
			php_info_print(row_element);
			php_info_print("</th>");
		} else {
			php_info_print(row_element);
			if (i < num_cols-1) {
				php_info_print(" => ");
			} else {
				php_info_print("\n");
			}
		}
	}
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("</tr>\n");
	}

	va_end(row_elements);
}
/* }}} */

/* {{{ php_info_print_table_row_internal
 */
static void php_info_print_table_row_internal(int num_cols, 
		const char *value_class, va_list row_elements)
{
	int i;
	char *row_element;

	if (!sapi_module.phpinfo_as_text) {
		php_info_print("<tr>");
	}	
	for (i=0; i<num_cols; i++) {
		if (!sapi_module.phpinfo_as_text) {
			php_info_printf("<td class=\"%s\">",
			   (i==0 ? "e" : value_class )
			);
		}	
		row_element = va_arg(row_elements, char *);
		if (!row_element || !*row_element) {
			if (!sapi_module.phpinfo_as_text) {
				php_info_print( "<i>no value</i>" );
			} else {
				php_info_print( " " );
			}
		} else {
			if (!sapi_module.phpinfo_as_text) {
				php_info_print_html_esc(row_element, strlen(row_element));
			} else {
				php_info_print(row_element);
				if (i < num_cols-1) {
					php_info_print(" => ");
				}	
			}
		}
		if (!sapi_module.phpinfo_as_text) {
			php_info_print(" </td>");
		} else if (i == (num_cols - 1)) {
			php_info_print("\n");
		}
	}
	if (!sapi_module.phpinfo_as_text) {
		php_info_print("</tr>\n");
	}
}
/* }}} */

/* {{{ php_info_print_table_row
 */
PHPAPI void php_info_print_table_row(int num_cols, ...)
{
	va_list row_elements;
	
	va_start(row_elements, num_cols);
	php_info_print_table_row_internal(num_cols, "v", row_elements);
	va_end(row_elements);
}
/* }}} */

/* {{{ php_info_print_table_row_ex
 */
PHPAPI void php_info_print_table_row_ex(int num_cols, const char *value_class, 
		...)
{
	va_list row_elements;
	
	va_start(row_elements, value_class);
	php_info_print_table_row_internal(num_cols, value_class, row_elements);
	va_end(row_elements);
}
/* }}} */

/* {{{ register_phpinfo_constants
 */
void register_phpinfo_constants(INIT_FUNC_ARGS)
{
	REGISTER_LONG_CONSTANT("INFO_GENERAL", PHP_INFO_GENERAL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_CREDITS", PHP_INFO_CREDITS, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_CONFIGURATION", PHP_INFO_CONFIGURATION, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_MODULES", PHP_INFO_MODULES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_ENVIRONMENT", PHP_INFO_ENVIRONMENT, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_VARIABLES", PHP_INFO_VARIABLES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_LICENSE", PHP_INFO_LICENSE, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("INFO_ALL", PHP_INFO_ALL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_GROUP",	PHP_CREDITS_GROUP, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_GENERAL",	PHP_CREDITS_GENERAL, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_SAPI",	PHP_CREDITS_SAPI, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_MODULES",	PHP_CREDITS_MODULES, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_DOCS",	PHP_CREDITS_DOCS, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_FULLPAGE",	PHP_CREDITS_FULLPAGE, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_QA",	PHP_CREDITS_QA, CONST_PERSISTENT|CONST_CS);
	REGISTER_LONG_CONSTANT("CREDITS_ALL",	PHP_CREDITS_ALL, CONST_PERSISTENT|CONST_CS);
}
/* }}} */

/* {{{ proto void phpinfo([int what]) U
   Output a page of useful information about PHP and the current request */
PHP_FUNCTION(phpinfo)
{
	int argc = ZEND_NUM_ARGS();
	long flag;

	if (zend_parse_parameters(argc TSRMLS_CC, "|l", &flag) == FAILURE) {
		return;
	}

	if(!argc) {
		flag = PHP_INFO_ALL;
	}

	/* Andale!  Andale!  Yee-Hah! */
	php_output_start_default(TSRMLS_C);
	php_print_info(flag TSRMLS_CC);
	php_output_end(TSRMLS_C);

	RETURN_TRUE;
}

/* }}} */

/* {{{ proto string phpversion([string extension]) U
   Return the current PHP version */
PHP_FUNCTION(phpversion)
{
	char *ext_name = NULL;
	int ext_name_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &ext_name, &ext_name_len) == FAILURE) {
		return;
	}

	if (!ext_name) {
		RETURN_ASCII_STRING(PHP_VERSION, 1);
	} else {
		const char *version;
		version = zend_get_module_version(ext_name);
		if (version == NULL) {
			RETURN_FALSE;
		}
		RETURN_ASCII_STRING(version, 1);
	}
}
/* }}} */

/* {{{ proto void phpcredits([int flag]) U
   Prints the list of people who've contributed to the PHP project */
PHP_FUNCTION(phpcredits)
{
	int argc = ZEND_NUM_ARGS();
	long flag;

	if (zend_parse_parameters(argc TSRMLS_CC, "|l", &flag) == FAILURE) {
		return;
	}

	if(!argc) {
		flag = PHP_CREDITS_ALL;
	} 

	php_print_credits(flag TSRMLS_CC);
	RETURN_TRUE;
}
/* }}} */

/* {{{ php_logo_guid
 */
PHPAPI char *php_logo_guid(void)
{
	char *logo_guid;

	time_t the_time;
	struct tm *ta, tmbuf;

	the_time = time(NULL);
	ta = php_localtime_r(&the_time, &tmbuf);

	if (ta && (ta->tm_mon==3) && (ta->tm_mday==1)) {
		logo_guid = PHP_EGG_LOGO_GUID;
	} else {
		logo_guid = PHP_LOGO_GUID;
	}

	return estrdup(logo_guid);

}
/* }}} */

/* {{{ proto string php_logo_guid(void) U
   Return the special ID used to request the PHP logo in phpinfo screens*/
PHP_FUNCTION(php_logo_guid)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ASCII_STRING(php_logo_guid(), ZSTR_AUTOFREE);
}
/* }}} */

/* {{{ proto string php_real_logo_guid(void) U
   Return the special ID used to request the PHP logo in phpinfo screens*/
PHP_FUNCTION(php_real_logo_guid)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ASCII_STRINGL(PHP_LOGO_GUID, sizeof(PHP_LOGO_GUID)-1, 1);
}
/* }}} */

/* {{{ proto string php_egg_logo_guid(void) U
   Return the special ID used to request the PHP logo in phpinfo screens*/
PHP_FUNCTION(php_egg_logo_guid)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ASCII_STRINGL(PHP_EGG_LOGO_GUID, sizeof(PHP_EGG_LOGO_GUID)-1, 1);
}
/* }}} */

/* {{{ proto string zend_logo_guid(void) U
   Return the special ID used to request the Zend logo in phpinfo screens*/
PHP_FUNCTION(zend_logo_guid)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_ASCII_STRINGL(ZEND_LOGO_GUID, sizeof(ZEND_LOGO_GUID)-1, 1);
}
/* }}} */

/* {{{ proto string php_sapi_name(void) U
   Return the current SAPI module name */
PHP_FUNCTION(php_sapi_name)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (sapi_module.name) {
		RETURN_ASCII_STRING(sapi_module.name, 1);
	} else {
		RETURN_FALSE;
	}
}

/* }}} */

/* {{{ proto string php_uname(void) U
   Return information about the system PHP was built on */
PHP_FUNCTION(php_uname)
{
	char *mode = "a";
	int modelen;
	char *tmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &mode, &modelen) == FAILURE) {
		return;
	}
	tmp = php_get_uname(*mode);
	RETVAL_RT_STRING(tmp, ZSTR_AUTOFREE);
}

/* }}} */

/* {{{ proto string php_ini_scanned_files(void) U
   Return comma-separated string of .ini files parsed from the additional ini dir */
PHP_FUNCTION(php_ini_scanned_files)
{
	if (strlen(PHP_CONFIG_FILE_SCAN_DIR) && php_ini_scanned_files) {
		RETURN_RT_STRING(php_ini_scanned_files, ZSTR_DUPLICATE);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string php_ini_loaded_file(void)
   Return the actual loaded ini filename */
PHP_FUNCTION(php_ini_loaded_file)
{
	if (php_ini_opened_path) {
		RETURN_STRING(php_ini_opened_path, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
