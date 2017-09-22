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
$socket = socket_create(AF_INET,SOCK_STREAM,0) or die("socket_create fail\n");
$result = socket_bind($socket,$ip,$port) or die("socket_bind fail\n");
$result = socket_listen($socket,5) or die("socket_listen fail\n");
$client = socket_accept($socket) or die("accept error\n");

/* LOOP */
$cnt=0;

$q = "update Store set count=0";
$mysqli->query($q);

while(1){
	$counta=0;
	$countb=0;
	$countc=0;	
	echo "RECV START\n\n";

	/* READ DATA FROM SOCKET & UPDATE DATABASE */
	for($x=0;$x<8;$x++){
		for($y=0;$y<9;$y++){
			$input = socket_read($client,1) or die("read input error\n");
//			$input = 'E';

			if($input=='F'){
				echo "input==F\n";
				echo "x = ".$x."\n";
				switch($x){	
					case 2:
						$counta++;
						echo "set a ".$counta." update\n";
						break;
					case 4:
						$countb++;
						echo "set b ".$countb." update\n";
						break;
					case 6:
						$countc++;
						echo "set c ".$countc." update\n";
						break;
				}
			}

			$q = "update Map set data='".$input."' where x=".$x." && y=".$y.";";
			$data = $mysqli->query($q);
		//	echo "Map[".$x."][".$y."]\n";
		}
	}
	echo "RECV COMPLETE\n\n";

	$q = "update Store set count=".$counta." where StoreNo=1;";
	$rst = $mysqli->query($q);
	
	$q = "update Store set count=".$countb." where StoreNo=2;";
        $rst = $mysqli->query($q);

	$q = "update Store set count=".$countc." where StoreNo=3;";
        $rst = $mysqli->query($q);


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
	$cnt++;
	
	echo $cnt."\n\n";
}

/* CLOSE SOCKET */
socket_close($client);
socket_close($socket);

?>
