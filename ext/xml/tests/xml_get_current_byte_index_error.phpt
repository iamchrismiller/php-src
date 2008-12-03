--TEST--
Test xml_get_current_byte_index() function : error conditions 
--SKIPIF--
<?php 
if (!extension_loaded("xml")) {
	print "skip - XML extension not loaded"; 
}	 
?>
--FILE--
<?php
/* Prototype  : proto int xml_get_current_byte_index(resource parser)
 * Description: Get current byte index for an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

/*
 * add a comment here to say what the test is supposed to do
 */

echo "*** Testing xml_get_current_byte_index() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_current_byte_index() function with Zero arguments --\n";
var_dump( xml_get_current_byte_index() );

//Test xml_get_current_byte_index with one more than the expected number of arguments
echo "\n-- Testing xml_get_current_byte_index() function with more than expected no. of arguments --\n";

$extra_arg = 10;
var_dump( xml_get_current_byte_index(null, $extra_arg) );

echo "Done";
?>
--EXPECTF--
*** Testing xml_get_current_byte_index() : error conditions ***

-- Testing xml_get_current_byte_index() function with Zero arguments --

Warning: Wrong parameter count for xml_get_current_byte_index() in %s on line %d
NULL

-- Testing xml_get_current_byte_index() function with more than expected no. of arguments --

Warning: Wrong parameter count for xml_get_current_byte_index() in %s on line %d
NULL
Done