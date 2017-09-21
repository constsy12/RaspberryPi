#!/usr/bin/php -q
<?php

/* DATABASE SETUP */
$host='localhost';		// host name
$user='root';			// user name
$pw='1234';			// password
$db='HUB';			// DB name

$mysqli=new mysqli($host,$user,$pw,$db);	// DB connection

if($mysqli){					// configure connection state
	echo "MySQL success\n";
}else{
	echo "MySQL fail\n";
}

/* TCP SOCKET SETUP */
$ip="127.0.0.1";		// ip address
$port=5000;			// port number

set_time_limit(0);		// set connection time limitless


/* CREATE SOCKET */
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP) or die("socket_create fail\n");
$result = socket_bind($socket,$ip,$port) or die("socket_bind fail\n");
$result = socket_listen($socket,5) or die("socket_listen fail\n");
$client = socket_accept($socket) or die("accept error\n");

/* LOOP */
while(1){

	echo "RECV START\n\n";

	/* READ DATA FROM SOCKET & UPDATE DATABASE */
	for($x=0;$x<8;$x++){
		for($y=0;$y<9;$y++){
			$input = socket_read($client,1) or die("read input error\n");
			$q = "update Map set data='".$input."' where x=".$x." && y=".$y.";";
			$data = $mysqli->query($q);
		}
	}
	echo "RECV COMPLETE\n\n";

	/* PRINT DATA FROM DATABASE TO CONFIGURE */
/*	for($i=0;$i<9;$i++){
		for($j=0;$j<8;$j++){
			$q="select data from Map where x=".$j." && y=".$i.";";
			$data = $mysqli->query($q);
			$row = $data->fetch_array();
			echo $row['data']." ";		
		}
		echo "\n";
	}*/
	echo "\n\n";
}

/* CLOSE SOCKET */
socket_close($client);
socket_close($socket);

?>
