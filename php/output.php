<?php
	if (isset($_GET['mode']) && !empty($_GET['mode'])) {
		$bash_display = "";

		if ($_GET['mode'] == "stream") {
			$bash_display = '/usr/bin/sudo /var/www/html/pidisplay/c/LedMatrix -o stream';
		}
		else if ($_GET['mode'] == "text" && isset($_GET['message']) && !empty($_GET['message']) && strlen($_GET['message'] <= 100)) {
			$bash_display = '/usr/bin/sudo /var/www/html/pidisplay/c/LedMatrix -o text "' . escapeshellcmd($_GET['message'])  . '" & 2>&1';
		}
		else if ($_GET['mode'] == "scheduled") {
			shell_exec("/usr/bin/sudo /var/www/html/pidisplay/scripts/start.sh &");
			exit;
		}
		else {
			exit;
		}

		$bash_interrupt_prev_display = '/usr/bin/sudo /var/www/html/pidisplay/scripts/endPs.sh';
		shell_exec($bash_interrupt_prev_display);
		echo shell_exec($bash_display);
	}
?>
