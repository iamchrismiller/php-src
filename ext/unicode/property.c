/*
   +----------------------------------------------------------------------+
   | PHP Version 6                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andrei Zmievski <andrei@php.net>                            |
   +----------------------------------------------------------------------+
 */

/* $Id$ */ 

#include "php_unicode.h"

typedef UBool (*prop_check_func_t)(UChar32 ch);

static void check_property_impl(INTERNAL_FUNCTION_PARAMETERS, prop_check_func_t checker)
{
	UChar	   *str;
	int			str_len;
	zend_bool	result = 1;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}

	while (offset < str_len && result) {
		U16_NEXT(str, offset, str_len, ch);
		result = checker(ch);
	}

	RETURN_BOOL(result);
}


/* {{{ C/POSIX migration functions */

PHP_FUNCTION(char_is_lower)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_islower);
}

PHP_FUNCTION(char_is_upper)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isupper);
}

PHP_FUNCTION(char_is_digit)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isdigit);
}

PHP_FUNCTION(char_is_alpha)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isalpha);
}

PHP_FUNCTION(char_is_alnum)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isalnum);
}

PHP_FUNCTION(char_is_xdigit)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isxdigit);
}

PHP_FUNCTION(char_is_punct)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_ispunct);
}

PHP_FUNCTION(char_is_graph)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isgraph);
}

PHP_FUNCTION(char_is_blank)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isblank);
}

PHP_FUNCTION(char_is_space)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isspace);
}

PHP_FUNCTION(char_is_cntrl)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_iscntrl);
}

PHP_FUNCTION(char_is_print)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isprint);
}

/* }}} */

/* {{{ Additional binary property functions */

PHP_FUNCTION(char_is_defined)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isdefined);
}

PHP_FUNCTION(char_is_id_start)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isIDStart);
}

PHP_FUNCTION(char_is_id_part)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isIDPart);
}

PHP_FUNCTION(char_is_id_ignorable)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isIDIgnorable);
}

PHP_FUNCTION(char_is_iso_control)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isISOControl);
}

PHP_FUNCTION(char_is_mirrored)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isMirrored);
}

PHP_FUNCTION(char_is_base)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isbase);
}

PHP_FUNCTION(char_is_whitespace)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isUWhiteSpace);
}

PHP_FUNCTION(char_is_alphabetic)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isUAlphabetic);
}

PHP_FUNCTION(char_is_uppercase)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isUUppercase);
}

PHP_FUNCTION(char_is_lowercase)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_isULowercase);
}

PHP_FUNCTION(char_is_titlecase)
{
	check_property_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, u_istitle);
}


/* }}} */

/* {{{ Single character properties */

PHP_FUNCTION(char_get_numeric_value)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_DOUBLE(u_getNumericValue(ch));
}

PHP_FUNCTION(char_get_combining_class)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_LONG((long)u_getCombiningClass(ch));
}

PHP_FUNCTION(char_get_digit_value)
{
	UChar	   *str;
	int			str_len;
	int			radix = 0;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u|l", &str, &str_len, &radix) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	if (ZEND_NUM_ARGS() > 1) {
		if (radix < 2 || radix > 36) {
			php_error(E_WARNING, "Radix has to be in 2-36 range");
			return;
		}
		RETURN_LONG(u_digit(ch, radix));
	} else {
		RETURN_LONG(u_charDigitValue(ch));
	}
}

PHP_FUNCTION(char_get_mirrored)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_UCHAR32(u_charMirror(ch));
}

PHP_FUNCTION(char_get_direction)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_LONG((long)u_charDirection(ch));
}

PHP_FUNCTION(char_get_age)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;
	UVersionInfo version;
	char        buf[18] = { 0, };

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	u_charAge(ch, version);
	u_versionToString(version, buf);

	RETURN_ASCII_STRING(buf, ZSTR_DUPLICATE);
}

PHP_FUNCTION(char_get_type)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_LONG(u_charType(ch));
}

PHP_FUNCTION(char_is_valid)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u", &str, &str_len) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}
	U16_NEXT(str, offset, str_len, ch);

	RETURN_BOOL(U_IS_UNICODE_CHAR(ch));
}

PHP_FUNCTION(char_from_digit)
{
	int			digit;
	int			radix = 10;
	UChar32		ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &digit, &radix) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() > 1) {
		if (radix < 2 || radix > 36) {
			php_error(E_WARNING, "Radix has to be in 2-36 range");
			return;
		}
	}
	ch = u_forDigit(digit, radix);
	
	if (ch == (UChar32)0) {
		RETURN_FALSE;
	}

	RETURN_UCHAR32(ch);
}

PHP_FUNCTION(char_from_name)
{
	void	   *name;
	int			name_len;
	zend_uchar	name_type;
	UChar32		ch;
	UCharNameChoice choice = U_UNICODE_CHAR_NAME;
	zend_bool	extended = FALSE;
	char	   *buf;
	UErrorCode	status = U_ZERO_ERROR;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|b", &name, &name_len, &name_type, &extended) == FAILURE) {
		return;
	}

	if (extended) {
		choice = U_EXTENDED_CHAR_NAME;
	}

	if (name_type == IS_UNICODE) {
		buf = zend_unicode_to_ascii(name, name_len TSRMLS_CC);
		if (buf == NULL) {
			php_error(E_WARNING, "Character name has to consist only of ASCII characters");
			RETURN_FALSE;
		}
	} else {
		buf = (char *) name;
	}

	ch = u_charFromName(choice, buf, &status);

	if (name_type == IS_UNICODE) {
		efree(buf);
	}

	if (U_SUCCESS(status)) {
		RETURN_UCHAR32(ch);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(char_get_name)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;
	zend_bool	extended = FALSE;
	UCharNameChoice choice = U_UNICODE_CHAR_NAME;
	char	   *buf;
	int			buf_len = 128;
	UErrorCode  status = U_ZERO_ERROR;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "u|b", &str, &str_len, &extended) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}

	if (extended) {
		choice = U_EXTENDED_CHAR_NAME;
	}

	U16_NEXT(str, offset, str_len, ch);

	buf = emalloc(buf_len);
	buf_len = u_charName(ch, choice, buf, buf_len, &status);
	if (buf_len == 0) {
		efree(buf);
		RETURN_FALSE;
	} else if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		buf = erealloc(buf, buf_len+1);
		buf_len = u_charName(ch, choice, buf, buf_len+1, &status);
	}

	RETURN_ASCII_STRINGL(buf, buf_len, ZSTR_AUTOFREE);
}

/* }}} */

/* {{{ Other property functions */

PHP_FUNCTION(char_has_binary_property)
{
	UChar 		*str = NULL;
	int    		 str_len;
	long   		 prop;
	UProperty	 uprop; 
	int			 offset = 0;
	zend_bool	 result = 1;
	UChar32		 ch;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ul", &str, &str_len, &prop) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}

	uprop = (UProperty)prop;

	while (offset < str_len && result) {
		U16_NEXT(str, offset, str_len, ch);
		result = u_hasBinaryProperty(ch, uprop);
	}

	RETURN_BOOL(result);
}

PHP_FUNCTION(char_get_property_value)
{
	UChar	   *str;
	int			str_len;
	int			offset = 0;
	UChar32		ch;
	long		prop;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ul", &str, &str_len, &prop) == FAILURE) {
		return;
	}

	if (str_len == 0) {
		RETURN_FALSE;
	}

	U16_NEXT(str, offset, str_len, ch);

	if (prop >= UCHAR_BINARY_START && prop < UCHAR_BINARY_LIMIT) {
		RETURN_BOOL((zend_bool)u_getIntPropertyValue(ch, (UProperty)prop));
	} else {
		RETURN_LONG(u_getIntPropertyValue(ch, (UProperty)prop));
	}
}

PHP_FUNCTION(char_get_property_min_value)
{
	long prop;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prop) == FAILURE) {
		return;
	}
	
	RETURN_LONG(u_getIntPropertyMinValue((UProperty)prop));
}

PHP_FUNCTION(char_get_property_max_value)
{
	long prop;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prop) == FAILURE) {
		return;
	}
	
	RETURN_LONG(u_getIntPropertyMaxValue((UProperty)prop));
}

PHP_FUNCTION(char_get_property_name)
{
	long 		 prop;
	long		 name_choice = U_LONG_PROPERTY_NAME;
	const char	*name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &prop, &name_choice) == FAILURE) {
		return;
	}

	if (name_choice < 0) {
		name_choice = U_LONG_PROPERTY_NAME;
	}

	name = u_getPropertyName((UProperty) prop, (UPropertyNameChoice) name_choice);
	if (name) {
		RETURN_ASCII_STRING((char *)name, ZSTR_DUPLICATE);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(char_get_property_from_name)
{
	void	   *name;
	int			name_len;
	zend_uchar	name_type;
	char	   *buf;
	UProperty	prop;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	if (name_type == IS_UNICODE) {
		buf = zend_unicode_to_ascii(name, name_len TSRMLS_CC);
		if (buf == NULL) {
			php_error(E_WARNING, "Property name has to consist only of ASCII characters");
			RETURN_FALSE;
		}
	} else {
		buf = (char *) name;
	}

	prop = u_getPropertyEnum(buf);
	if (name_type == IS_UNICODE) {
		efree(buf);
	}

	RETURN_LONG(prop);
}

PHP_FUNCTION(char_get_property_value_name)
{
	long 		 prop;
	long		 value;
	long		 name_choice = U_LONG_PROPERTY_NAME;
	const char	*name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &prop, &value, &name_choice) == FAILURE) {
		return;
	}

	if (name_choice < 0) {
		name_choice = U_LONG_PROPERTY_NAME;
	}

	name = u_getPropertyValueName((UProperty) prop, (int32_t) value, (UPropertyNameChoice) name_choice);
	if (name) {
		RETURN_ASCII_STRING((char *)name, ZSTR_DUPLICATE);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(char_get_property_value_from_name)
{
	long		prop;
	void	   *name;
	int			name_len;
	zend_uchar	name_type;
	char	   *buf;
	long		value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lt", &prop, &name, &name_len, &name_type) == FAILURE) {
		return;
	}

	if (name_type == IS_UNICODE) {
		buf = zend_unicode_to_ascii(name, name_len TSRMLS_CC);
		if (buf == NULL) {
			php_error(E_WARNING, "Property value name has to consist only of ASCII characters");
			RETURN_FALSE;
		}
	} else {
		buf = (char *) name;
	}

	value = u_getPropertyValueEnum((UProperty)prop, buf);
	if (name_type == IS_UNICODE) {
		efree(buf);
	}

	RETURN_LONG(value);
}

/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
