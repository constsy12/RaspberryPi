#!/usr/bin/php -q
<?php

$host='localhost';
$user='root';
$pw='1234';
$db='HUB';
$mysqli=new mysqli($host,$user,$pw,$db);

if($mysqli){
	echo "MySQL success\n";
}else{
	echo "MySQL fail\n";
}

##$q="update Map set data=2 where x=0 && y=0;";
$x=0;
$y=0;
$q="select data from Map;";
$data=$mysqli->query($q);
#echo $data."\n";
//printf("data = %d\n",$data);
$row=$data->fetch_array();
echo $row['data'];
$row=$data->fetch_array();
echo $row['data']."\n";

$ip="127.0.0.1";
$port=5000;
$i=0;

set_time_limit(0);

$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP) or die("socket_create fail\n");
$result = socket_bind($socket,$ip,$port) or die("socket_bind fail\n");
$result = socket_listen($socket,5) or die("socket_listen fail\n");

$client = socket_accept($socket) or die("accept error\n");

while(1){
$dd=0;

//$input = socket_read($client,1024) or die("read input error\n");
//$input = trim($input);
//$rst=array($input);

//for($i=0;$i<72;$i++){
//	$x = $i/9;
//	$y = $i%9;
//	$input = socket_read($client,1) or die("read input error\n");
//	$q = "update Map set data='".$input."' where x=".$x." && y=".$y.";";
//	$data = $mysqli->query($q);	

//	$dd = socket_recv($client,$input,1,MSG_WAITALL);
//	echo $input." ";
//	if($i%9==0)
//		echo "\n";
//}
echo "RECV START\n\n";

for($x=0;$x<8;$x++){
	for($y=0;$y<9;$y++){
		$input = socket_read($client,1) or die("read input error\n");
		$q = "update Map set data='".$input."' where x=".$x." && y=".$y.";";
		$data = $mysqli->query($q);
	}
	echo "\n";
}
echo "RECV COMPLETE\n\n";

//echo "Message from Client >> ".(int)$rst[0]."\n";

//echo "Second >> ".(int)$rst[22]."\n";
//$k=0;

for($i=0;$i<9;$i++){
	for($j=0;$j<8;$j++){
//		$input = socket_read($client,1) or die("read input error\n");
		$q="select data from Map where x=".$j." && y=".$i.";";
		$data = $mysqli->query($q);
		$row = $data->fetch_array();
		echo $row['data']." ";		
	}
	echo "\n";
}
echo "\n\n";

//$output = strrev($input)."\n";
//socket_write($client,$output,strlen($output)) or die("write error\n");

}
socket_close($client);
socket_close($socket);

?>
