<?php
	$logFrames = fopen("/var/www/html/pidisplay/c/log_frames1.txt", "r");
	if ($logFrames) {
		while(!feof($logFrames)) {
			$dataLine = fgets($logFrames);

			if (true && true) {
				$fileName = "/var/www/html/pidisplay/c/input_frame.txt";
				$inputFrame = fopen($fileName, "w");
				if ($inputFrame) {
					if (flock($inputFrame, LOCK_EX)) {
						fwrite($inputFrame, $dataLine);
						flock($inputFrame, LOCK_UN);
					}
				}

				fclose($inputFrame);
			}

			usleep(100000);
		}
	}

	fclose($logFrames);
?>