<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2025 Daniel Teichmann <daniel.teichmann@das-netzwerkteam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


/*
 * Timeout protection utility for GOsa² SchoolManager imports
 * Catches fatal errors and timeouts, logs to syslog via GOsa's log system
 */
class timeoutProtection
{
    private $start_time;
    private $script_name;
    private $instance_ref;

    /**
     * Constructor - Initializes timeout protection
     *
     * @param string $script_name Name of the script/class for logging
     * @param int $time_limit Time limit in seconds
     * @param mixed $instance Reference to the calling instance (optional)
     */
    public function __construct($script_name, $time_limit, &$instance = null)
    {
        $this->start_time = microtime(true);
        $this->script_name = $script_name;
        $this->instance_ref = $instance;

        // Register shutdown function to catch fatal errors and timeouts
        register_shutdown_function(array($this, 'handleShutdown'));

        // Set execution time limit
        @set_time_limit($time_limit);
    }

    /**
     * Reset the timer to current time
     * Call this at the start of each phase execution
     */
    public function resetTimer()
    {
        $this->start_time = microtime(true);
    }

    /**
     * Get elapsed time since last timer reset
     *
     * @return float Elapsed time in seconds (rounded to 2 decimals)
     */
    public function getElapsedTime()
    {
        return round(microtime(true) - $this->start_time, 2);
    }

    /**
     * Shutdown handler - Catches fatal errors and timeouts
     * This function is automatically called when the script terminates
     */
    public function handleShutdown()
    {
        $error = error_get_last();

        if ($error === null) {
            // Normal completion, no error
            return;
        }

        // Check if it's a fatal error
        $fatal_errors = [E_ERROR, E_PARSE, E_CORE_ERROR, E_COMPILE_ERROR, E_USER_ERROR];
        if (!in_array($error['type'], $fatal_errors)) {
            // Not a fatal error, ignore
            return;
        }

        $elapsed = $this->getElapsedTime();

        // Determine if it's a timeout
        $is_timeout = (
            stripos($error['message'], 'Maximum execution time') !== false ||
            stripos($error['message'], 'exceeded') !== false
        );

        if ($is_timeout) {
            $log_msg = "TIMEOUT in {$this->script_name} after {$elapsed}s: {$error['message']}";
            $error_type = "Timeout";
        } else {
            $log_msg = "FATAL ERROR in {$this->script_name} after {$elapsed}s: {$error['message']}";
            $error_type = "Fatal Error";
        }

        new log(
            "error",
            "addons/schoolmanager",
            "",
            array(),
            $log_msg . " at " . $error['file'] . ":" . $error['line']
        );

        // Store error in session for user display on next page load
        session::set('importaccounts_timeout_error', array(
            'type' => $error_type,
            'message' => $error['message'],
            'elapsed' => $elapsed,
            'file' => $error['file'],
            'line' => $error['line']
        ));
    }
}
