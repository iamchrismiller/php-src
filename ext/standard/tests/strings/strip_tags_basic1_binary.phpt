--TEST--
Test strip_tags() function : basic functionality - with default arguments, binary variant
--INI--
short_open_tag = on
--FILE--
<?php
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string 
 * Source code: ext/standard/string.c
*/

echo "*** Testing strip_tags() : basic functionality ***\n";

// array of arguments 
$string_array = array (
  b"<html>hello</html>",
  b'<html>hello</html>',
  b"<?php echo hello ?>",
  b'<?php echo hello ?>',
  b"<? echo hello ?>",
  b'<? echo hello ?>',
  b"<% echo hello %>",
  b'<% echo hello %>',
  b"<script language=\"PHP\"> echo hello </script>",
  b'<script language=\"PHP\"> echo hello </script>',
  b"<html><b>hello</b><p>world</p></html>",
  b'<html><b>hello</b><p>world</p></html>',
  b"<html><!-- COMMENT --></html>",
  b'<html><!-- COMMENT --></html>'
);
  
  		
// Calling strip_tags() with default arguments
// loop through the $string_array to test strip_tags on various inputs
$iteration = 1;
foreach($string_array as $string)
{
  echo "-- Iteration $iteration --\n";
  var_dump( strip_tags($string) );
  $iteration++;
}

echo "Done";
?>
--EXPECT--
*** Testing strip_tags() : basic functionality ***
-- Iteration 1 --
string(5) "hello"
-- Iteration 2 --
string(5) "hello"
-- Iteration 3 --
string(0) ""
-- Iteration 4 --
string(0) ""
-- Iteration 5 --
string(0) ""
-- Iteration 6 --
string(0) ""
-- Iteration 7 --
string(0) ""
-- Iteration 8 --
string(0) ""
-- Iteration 9 --
string(12) " echo hello "
-- Iteration 10 --
string(12) " echo hello "
-- Iteration 11 --
string(10) "helloworld"
-- Iteration 12 --
string(10) "helloworld"
-- Iteration 13 --
string(0) ""
-- Iteration 14 --
string(0) ""
Done
