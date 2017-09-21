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
$sec="2";
?>
<!DOCTYPE html>

<html>
<head>
<style>
	body{background-color: powderblue; font-family: "Raleway",sans-serif;
	     text-align: center; }
	div.container{width: 900px; background-color: powderblue;
		      text-align: center; margin: 0 auto;}
	header, footer{margin: 0 auto; color: white; text-align: center;
		       height: 100px; width: 900px; padding-top:30px;}
	section{color: black; background-color: powderblue;
		height: 400px; width:900px;}
	section.test_L{margin: 10px 10px 10px 10px; color: black; 
		       background-color: white;
		       text-align: center; width: 430px; height: 430px; 
		       float: left;  
		     }
	section.test_R{margin: 10px 10px 10px 10px; color: black; 
		       background-color: white;
		       text-align: center; width: 430px; height: 430px;
		       float: left;
		     }
	div.box{ background-color: powderblue; margin: 0 auto;
		 text-align: center; margin-bottom: 35px; 
		 border-radius: 15px; color: white; width:80px;}
</style>
	<meta http-equiv="refresh" content="<?php echo $sec?>;URL='<?php echo $page?>'">

</head>

<body align="center">
	<div class="container" align="center">
		<header align="center">
			<h1> A.L.S </h1>
		</header>

		<section align="center">
			<section class="test_L">
<!--	<h1 style="padding-bottom:15px; color:white;"> MAP </h1>  -->
			<div class="box" align="center" >
				<h1>MAP</h1>
			</div>
			<table align="center" style="width:320px; height:320px;
				      padding: 15px; border: 5px solid powderblue; ">
				
				<?php
				
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
			<div class="box" align="center" style="width: 250px;" >
				<h1> LOCATION STATE </h1>
			</section>
		</section>

		<footer align="center">
			<br><br>
			<h3> DAILY-C </h3>
			<h4> Jin Taekyun . Kang Seoyun . Lee Jaeseok </h4>	
		</footer>
	</div>
</body>
</html>	

