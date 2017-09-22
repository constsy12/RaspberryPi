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
$sec="3";




?>
<!DOCTYPE html>

<html>
<head>
<style>
	body{ background-color: powderblue; font-family: "Raleway",sans-serif;
	      text-align: center; 
	    }
	div.container{ width: 900px; background-color: powderblue;
		       text-align: center; margin: 0 auto;
		     }
	header, footer{ margin: 0 auto; color: white; text-align: center;
		        height: 100px; width: 900px; padding-top:30px;
		      }
	section{ color: black; background-color: powderblue;
		 height: 400px; width:900px;
	       }
	section.test_L{ margin: 10px 10px 10px 10px; color: black; 
		        background-color: white;
		        text-align: center; width: 430px; height: 430px; 
		        float: left;  
		      }
	section.test_R{ margin: 10px 10px 10px 10px; color: black; 
		        background-color: white;
		        text-align: center; width: 430px; height: 430px;
		        float: left;
		      }
	div.box{ background-color: powderblue; margin: 0 auto;
		 text-align: center; margin-bottom: 30px; margin-top: 20px; 
		 border-radius: 15px; color: white; width:100px;
	       }
</style>
	<meta http-equiv="refresh" content="<?php echo $sec?>;URL='<?php echo $page?>'">

</head>

<body align="center">
	<div class="container" align="center">
		<header align="center">
			<h1> A . L . S </h1>
		</header>

		<section align="center">
			<section class="test_L">
			<div class="box" align="center" style="height: 30px">
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
						case 'M':
	?><th style="color:white; width:15px; height:20px;"><?php
						//	echo $row['data'];
							break;
						case 'A':
						case 'B':
						case 'C':
	?><th style="background-color:Tomato; width:15px; height:20px;"><?php
						//	echo $row['data'];
							break;
						case 'R':	
	?><th style="background-color:Yellow; width:15px; height:20px;"><?php
						//	echo $row['data'];
							break;
						case 'W':
	?><th style="background-color:Green; width:15px; height:20px;"><?php
						//	echo $row['data'];
							break;
						case 'F':
        ?><th style="background-color:Green; width:15px; height:20px;"><?php
                                                //	echo $row['data'];
                                                        break;
						case 'E':
        ?><th style="background-color:Tomato; width:15px; height:20px;"><?php
                                                //	echo $row['data'];
                                                        break;												
						}
						echo $row['data'];
						?></th><?php
					}
					?></tr><?php
				}
				?>

			</table>
			</section>

			<section class="test_R" align="center">
			<div class="box" align="center" style="width: 250px;
							height: 30px;" >
				<h1>STORED QUANTITY</h1>
			</div>
			<table style="height: 300px; width: 300px;
				      font-size: 30px; padding: 20px;
				      color: powderblue;"
			       align="center">
			  <tr>
				<td style="background-color: powderblue;
					   border-radius: 15px; 
					   border: 5px solid white;
					   width: 100px; color: white;">
				A</td>
				<td><?php
					$q = "select count from Store where StoreNo=1;";
					
					$rst = $mysqli->query($q);
					$row = $rst->fetch_array(); 
					echo $row['count'];
				?></td>
			  </tr>
			  <tr>
				<td style="background-color: powderblue;
					   border-radius: 15px;
					   border: 5px solid white;
					   width: 100px; color: white;">
				B</td>
				<td><?php 
					$q = "select count from Store where StoreNo=2;";
					$rst = $mysqli->query($q);
                                        $row = $rst->fetch_array();
                                        echo $row['count'];

				?></td>
			  </tr>
			  <tr>
				<td style="background-color: powderblue;
					   border-radius: 15px;
					   border: 5px solid white;
					   width: 100px; color: white;">
				C</td>
				<td><?php 
					$q = "select count from Store where StoreNo=3;";
					$rst = $mysqli->query($q);
                                        $row = $rst->fetch_array();
                                        echo $row['count'];
				?></td>
			  </tr>
			</table>
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

