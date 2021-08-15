<?php
    if (isset($_POST['data']) && !empty($_POST['data'])) {
        // $send_frame = '/usr/bin/sudo /var/www/html/pidisplay/c/updateframe ' . escapeshellcmd($_POST['data']);
        // echo shell_exec($send_frame);

        // $logFileName = "/var/www/html/pidisplay/c/log_frames_server.txt";
        // $logFrames = fopen($logFileName, 'a');
        $fileName = "/var/www/html/pidisplay/c/input_frame.txt";
        $inputFrame = fopen($fileName, 'w');
        if ($inputFrame) {
            if (flock($inputFrame, LOCK_EX)) {
                fwrite($inputFrame, $_POST['data']);
                // fwrite($logFrames, $_POST['data'] . "\n");
                flock($inputFrame, LOCK_UN);
            }
        }

        fclose($inputFrame);
        // fclose($logFrames);
    }
?>