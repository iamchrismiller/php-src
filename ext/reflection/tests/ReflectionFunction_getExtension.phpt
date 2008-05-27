--TEST--
ReflectionFunction::getExtension()
--FILE--
<?php
function foo () {}

$function = new ReflectionFunction('sort');
var_dump($function->getExtension());

$function = new ReflectionFunction('foo');
var_dump($function->getExtension());
?>
--EXPECTF--
object(ReflectionExtension)#%i (1) {
  [u"name"]=>
  unicode(8) "standard"
}
NULL

