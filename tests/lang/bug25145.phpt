--TEST--
Bug #25145 (SEGV on recpt of form input with name like "123[]")
--GET--
123[]=SEGV
--FILE--
<?php

var_dump($_REQUEST);
echo "Done\n";

?>
--EXPECT--
array(1) {
  [123]=>
  array(1) {
    [0]=>
    unicode(4) "SEGV"
  }
}
Done
