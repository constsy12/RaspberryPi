<!--#!/usr/bin/php -q-->
<?php
#!/usr/bin/php -q
$host='localhost';
$user='root';
$pw='1234';
$db='HUB';
$mysqli=new mysqli($host,$user,$pw,$db);

if($mysqli){
//	echo "MYSQL SUCCESS";
}else{
	echo "MYSQL FAIL";
}

set_time_limit(0);
$i=1;
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
		       height: 100px;}
	section{color: black; background-color: powderblue;
		height: 400px; width: 100%;}
	section.test_L{margin: 10px 5px 10px 10px; color: black; 
		       background-color: white;
		       text-align: center; width: 48%; height: 300px; 
		       float: left;
		     }
	section.test_R{margin: 10px 10px 10px 5px; color: black; 
		       background-color: white;
		       text-align: center; width: 48%; height: 300px;
		       float: right;}
</style>
	<meta http-equiv="refresh" content="<?php echo $sec?>;URL='<?php echo $page?>'">

</head>

<body>
	<div class="container">
		<header>
			<h2 style="virtical-align:center;"> TEST PAGE </h2>
		</header>

		<section>
			<section class="test_L">
			<h1> LEFT </h1>
			<table style="width: 300px">
				
				<?php
				$q="select data from Map;";
				$result=$mysqli->query($q);
				#$row=$result->fetch_assoc();
				
				
				for($y=0;$y<9;$y++){
					?><tr><?php
					for($x=0;$x<8;$x++){
						
						?>
						<th><?php
							$row=$result->fetch_array(); 
							echo " ".$row['data']." ";
					
							  
						?></th>
						<?php
					}
					?></tr><?php
				
				}
				?>

			</table>
			</section>

			<section class="test_R">
			<h1> RIGHT </h1>
			</section>
		</section>

		<footer>
			<h4> FOOTER </h4>	
		</footer>
	</div>
</body>
</html>	

