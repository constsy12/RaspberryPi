<!--#!/usr/bin/php -q-->
<?php
#!/usr/bin/php -q

/* DATABASE SETUP */
$host='localhost';
$user='root';
$pw='1234';
$db='HUB';
$mysqli=new mysqli($host,$user,$pw,$db);

if(!$mysqli){
	echo "MYSQL FAIL";
}

set_time_limit(0);


$page=$_SERVER['PHP_SELF'];
$sec="1";
?>
<!DOCTYPE html>

<html>
<head>
<style>
	body{background-color: powderblue; font-family: "Raleway",sans-serif;
	     text-align: center;}
	div.container{width: 100%; background-color: powderblue;
		      text-align: center;}
	header, footer{margin: 20px; color: white; text-align: center;
		       height: 100px; width: 900px; }
	section{color: black; background-color: powderblue;
		height: 400px; width:900px;}
	section.test_L{margin: 10px 5px 10px 10px; color: black; 
		       background-color: white;
		       text-align: center; max-width: 400px; height: 400px; 
		       float: left;  
		     }
	section.test_R{margin: 10px 10px 10px 5px; color: black; 
		       background-color: white;
		       text-align: center; max-width: 400px; height: 400px;
		       float: left;}
</style>
	<meta http-equiv="refresh" content="<?php echo $sec?>;URL='<?php echo $page?>'">

</head>

<body align="center">
	<div class="container" align="center">
		<header align="center">
			<h2> TEST PAGE </h2>
		</header>

		<section align="center">
			<section class="test_L">
			<h1 style="padding-bottom: 15px;"> MAP </h1>
			<table align="center" style="width:300px; height:300px; ">
				
				<?php
			//	$q="select data from Map;";
			//	$result=$mysqli->query($q);
				#$row=$result->fetch_assoc();
				
				
				for($y=0;$y<9;$y++){
					?><tr style="height:20px;"><?php
					for($x=0;$x<8;$x++){
						$q = "select data from Map where x=".$x." && y=".$y.";";
                                                $result = $mysqli->query($q);
                                                $row=$result->fetch_array();
						
						switch($row['data']){
						case 'E':
	?><th style="color:white; width:15px; height:20px;"><?php
							echo $row['data'];
							break;
						case 'S':
	?><th style="background-color:Tomato; width:15px; height:20px;"><?php
							echo $row['data'];
							break;
						case 'R':	
	?><th style="background-color:Yellow; width:15px; height:20px;"><?php
							echo $row['data'];
							break;
						case 'D':
	?><th style="background-color:Green; width:15px; height:20px;"><?php
							echo $row['data'];
							break;
						}

						?></th><?php
					}
					?></tr><?php
				}
				?>

			</table>
			</section>

			<section class="test_R">
			<h1> LOCATION STATE </h1>
			</section>
		</section>

		<footer align="center">
			<h4> FOOTER </h4>	
		</footer>
	</div>
</body>
</html>	

