--TEST--
Phar front controller mime type unknown tar-based
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller17.phar.php/fronk.gronk
REQUEST_URI=/frontcontroller17.phar.php/fronk.gronk
--FILE_EXTERNAL--
frontcontroller8.phar.tar
--EXPECTHEADERS--
Content-type: application/octet-stream
Content-length: 4
--EXPECT--
hio3

