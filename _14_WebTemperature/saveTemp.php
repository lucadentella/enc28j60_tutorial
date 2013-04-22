<?php
	
	define("PASSWORD", "password");
	define("LOG_FILE", "./temperatures.csv");
	
	if(!isset($_GET["temp"])) die("KO - no value present");
	if(!isset($_GET["pwd"])) die("KO - no password present");
	
	$pwd = $_GET["pwd"];
	if($pwd != PASSWORD) die("KO - invalid password");	

	$temp = $_GET["temp"];
	if(!is_numeric($temp)) die("KO - Value is not a number");
		
	$file_handler = fopen(LOG_FILE, "a+");
	fwrite($file_handler, time() . "," . $temp . "\n");
	fflush($file_handler);
	echo "OK";

?>