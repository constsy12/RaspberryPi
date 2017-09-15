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
echo $row['data'];

$ip="127.0.0.1";
$port=5000;
$i=0;

set_time_limit(0);

$socket = socket_create(AF_INET,SOCK_STREAM,0) or die("socket_create fail\n");
$result = socket_bind($socket,$ip,$port) or die("socket_bind fail\n");
$result = socket_listen($socket,5) or die("socket_listen fail\n");

$client = socket_accept($socket) or die("accept error\n");

while($i<5){
$input = socket_read($client,1024) or die("read input error\n");
$input = trim($input);
echo "Message from Client >> ".$input."\n";

$output = strrev($input)."\n";
socket_write($client,$output,strlen($output)) or die("write error\n");
$i++;
}
socket_close($client);
socket_close($socket);

?>
