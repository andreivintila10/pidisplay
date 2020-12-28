<?php
	if (isset($_GET['message']) && !empty($_GET['message']) && strlen($_GET['message'] <= 100)) {
		$bash_interrupt_prev_display = '/usr/bin/sudo /var/www/html/pidisplay/scripts/endPs.sh &';
		$bash_display = '/usr/bin/sudo /var/www/html/pidisplay/c/LedMatrix "' . escapeshellcmd($_GET['message'])  . '" & 2>&1';
		shell_exec($bash_interrupt_prev_display);
		echo shell_exec($bash_display);
	}
?>
