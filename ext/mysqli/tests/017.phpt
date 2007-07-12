--TEST--
mysqli fetch functions 
--SKIPIF--
<?php require_once('skipif.inc'); ?>
<?php require_once('skipifemb.inc'); ?>
--FILE--
<?php
	include "connect.inc";
	
	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!$stmt = mysqli_prepare($link, "SELECT md5('bar'), database(), 'foo'"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_bind_result($stmt, $c0, $c1, $c2); 
	mysqli_execute($stmt);

	mysqli_fetch($stmt);
	mysqli_stmt_close($stmt);

	$test = array($c0, $c1, $c2);
	if ($c1 !== $db) {
		echo "Different data\n";
	}

	var_dump($test);
	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
array(3) {
  [0]=>
  string(32) "37b51d194a7513e45b56f6524f2d51f2"
  [1]=>
  string(%d) "%s"
  [2]=>
  string(3) "foo"
}
done!
--UEXPECTF--
array(3) {
  [0]=>
  string(32) "37b51d194a7513e45b56f6524f2d51f2"
  [1]=>
  unicode(%d) "%s"
  [2]=>
  unicode(3) "foo"
}
done!
