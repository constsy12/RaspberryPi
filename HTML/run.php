#!/usr/bin/php -q
<?php
set_time_limit(0);
$i=1;
$page=$_SERVER['PHP_SELF'];
$sec="1";
//while(true){
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
			<h2> TEST PAGE </h2>
		</header>

		<section>
			<section class="test_L">
			<h1> LEFT </h1>
			<h2><?php echo $i++ ?></h2>
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
<?php
//sleep(1);
//$i++;
//}
?>

