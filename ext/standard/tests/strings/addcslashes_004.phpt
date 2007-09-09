--TEST--
Test addcslashes() function (errors)
--INI--
precision=14
--FILE--
<?php

echo "\n*** Testing error conditions ***\n";
/* zero argument */
var_dump( addcslashes() );

/* unexpected arguments */
var_dump( addcslashes(b"foo[]") );
var_dump( addcslashes("foo", "foo") );
var_dump( addcslashes(b'foo[]', b"o", b"foo") );

echo "Done\n"; 

?>
--EXPECTF--
*** Testing error conditions ***

Warning: addcslashes() expects exactly 2 parameters, 0 given in %s on line %d
NULL

Warning: addcslashes() expects exactly 2 parameters, 1 given in %s on line %d
NULL
string(6) "\f\o\o"

Warning: addcslashes() expects exactly 2 parameters, 3 given in %s on line %d
NULL
Done
--UEXPECTF--
*** Testing error conditions ***

Warning: addcslashes() expects exactly 2 parameters, 0 given in %s on line %d
NULL

Warning: addcslashes() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: addcslashes() expects parameter 1 to be strictly a binary string, Unicode string given in %s on line %d
NULL

Warning: addcslashes() expects exactly 2 parameters, 3 given in %s on line %d
NULL
Done
