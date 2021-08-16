<?php
	if (isset($_POST['data']) && !empty($_POST['data'])) {
		$fileName = "/var/www/html/pidisplay/c/input_frame.txt";
		$inputFrame = fopen($fileName, 'w');
		if ($inputFrame) {
			if (flock($inputFrame, LOCK_EX)) {
				fwrite($inputFrame, $_POST['data']);
				flock($inputFrame, LOCK_UN);
			}
		}

		fclose($inputFrame);
	}
?>