<?php
	if (isset($_GET['message']) && !empty($_GET['message']) && strlen($_GET['message'] <= 100)) {
		//echo $_GET["message"] . "&nbsp;&nbsp;" . urldecode($_GET["message"]) . "<br>";
		//$redirectOutput = "2>&1";
		$bash = '/usr/bin/sudo /var/www/html/LedMatrix "' . escapeshellcmd($_GET['message'])  . '" & 2>&1';
		//echo $bash . "<br>";
		shell_exec("/usr/bin/sudo /var/www/html/endPs.sh &");
		echo shell_exec($bash);
	}
?>
