<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2025 Daniel Teichmann
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

/**
 * Import Job Manager for Asynchronous Import Processing
 *
 * This class manages background import jobs with file-based state management.
 * It handles job creation, state serialization, progress tracking, and security
 * through token-based authentication and filesystem permissions.
 *
 * Security Design:
 * - Job state files protected by filesystem permissions (mode 0600)
 * - Job directory protected by mode 0700
 * - LDAP credentials NEVER written to disk
 * - Simple timestamp-based job IDs with random suffix
 */
class importjob
{
    /**
     * Hardcoded directory for import job files
     * Job/Log files: import_<timestamp>_<random>.{json,log}
     */
    const JOB_DIR = '/var/spool/gosa/schoolmgr-import';

    // Unique job identifier (timestamp + random suffix)
    var $job_id;

    // Track if job file has been written at least once
    private $_file_written = false;

    /**
     * Constructor - Create new job or load existing job
     *
     * @param string|null $job_id Optional job ID to load existing job.
     *                            If null, generates timestamp-based job ID.
     */
    public function __construct($job_id = null)
    {
        // Generate new job ID or use provided one
        if ($job_id === null) {
            // Simple timestamp-based job ID with random suffix
            $this->job_id = 'import_' . time() . '_' . sprintf('%04x', mt_rand(0, 0xFFFF));
        } else {
            // Validate provided job ID (security: prevent directory traversal)
            if (preg_match('/[^a-zA-Z0-9_-]/', $job_id)) {
                throw new Exception("Invalid job ID format: " . $job_id);
            }
            $this->job_id = $job_id;

            // If loading existing job, check if file exists
            if ($this->exists()) {
                $this->_file_written = true;
            }
        }

        // Ensure job directory exists with proper permissions
        $this->_ensureJobDirectoryExists();
    }

    /**
     * Get the job ID
     *
     * @return string The job ID
     */
    public function getJobId()
    {
        return $this->job_id;
    }

    /**
     * Ensure job directory exists with proper permissions
     *
     * Security: Directory accessible only by owner (www-data)
     *
     * @throws Exception If directory cannot be created or permissions cannot be set
     */
    private function _ensureJobDirectoryExists()
    {
        // Create main job directory if it doesn't exist
        if (!is_dir(self::JOB_DIR)) {
            if (!mkdir(self::JOB_DIR, 0700, true)) {
                throw new Exception("Failed to create job directory: " . self::JOB_DIR);
            }
        }

        // Ensure proper permissions on main directory (mode 0700 - owner only)
        if (!chmod(self::JOB_DIR, 0700)) {
            throw new Exception("Failed to set permissions on job directory: " . self::JOB_DIR);
        }
    }

    /**
     * Get the full path to the job file for this job
     * Single JSON file contains ALL job data (state, progress, results, errors)
     *
     * @return string Absolute path to job file
     */
    private function _getJobFilePath()
    {
        return self::JOB_DIR . '/' . $this->job_id . '.json';
    }

    /**
     * Check if job file exists
     *
     * @return bool True if job file exists
     */
    public function exists()
    {
        return file_exists($this->_getJobFilePath());
    }

    /**
     * Read the entire job structure from disk
     * Returns complete job data including state, progress, results, errors
     *
     * @param int $retry_attempt Current retry attempt (internal use)
     *
     * @return array|null Job data array or null on error
     */
    public function readJob($retry_attempt = 0)
    {
        $max_retries = 2;  // 2 retries = 3 tries total
        $retry_delay_ms = 50;

        $job_file = $this->_getJobFilePath();
        if (!file_exists($job_file)) {
            new log(
                "error",
                "schoolmanager/importjob",
                "",
                array(),
                "Job file not found: " . $job_file
            );
            return null;
        }

        // Try to open file with shared lock
        $fp = fopen($job_file, 'r');
        if (!$fp) {
            // Failed to open - retry or fail
            if ($retry_attempt < $max_retries) {
                usleep($retry_delay_ms * 1000);
                return $this->readJob($retry_attempt + 1);
            }
            new log(
                "error",
                "schoolmanager/importjob",
                "",
                array(),
                "Failed to open job file after " .
                ($max_retries + 1) . " attempts: " . $job_file
            );
            return null;
        }

        // Acquire shared lock (multiple readers OK, blocks writers)
        if (!flock($fp, LOCK_SH)) {
            // Failed to acquire lock - retry or fail
            fclose($fp);
            if ($retry_attempt < $max_retries) {
                usleep($retry_delay_ms * 1000);
                return $this->readJob($retry_attempt + 1);
            }
            new log(
                "error",
                "schoolmanager/importjob",
                "",
                array(),
                "Failed to acquire lock after " .
                ($max_retries + 1) . " attempts: " . $job_file
            );
            return null;
        }

        // Read file contents
        $json = stream_get_contents($fp);
        flock($fp, LOCK_UN);
        fclose($fp);

        if ($json === false) {
            // Failed to read - retry or fail
            if ($retry_attempt < $max_retries) {
                usleep($retry_delay_ms * 1000);
                return $this->readJob($retry_attempt + 1);
            }
            new log(
                "error",
                "schoolmanager/importjob",
                "",
                array(),
                "Failed to read job file after " .
                ($max_retries + 1) . " attempts: " . $job_file
            );
            return null;
        }

        // Decode JSON
        $job_data = json_decode($json, true);

        if ($job_data === null && json_last_error() !== JSON_ERROR_NONE) {
            // JSON decode failed - retry or fail
            if ($retry_attempt < $max_retries) {
                new log(
                    "warning",
                    "schoolmanager/importjob",
                    "",
                    array(),
                    sprintf(
                        "JSON decode attempt %d/%d failed: %s - %s",
                        $retry_attempt + 1,
                        $max_retries + 1,
                        json_last_error_msg(),
                        $job_file
                    )
                );
                usleep($retry_delay_ms * 1000);
                return $this->readJob($retry_attempt + 1);
            }
            new log(
                "error",
                "schoolmanager/importjob",
                "",
                array(),
                "Failed to decode job file JSON after " .
                ($max_retries + 1) . " attempts: " . $job_file
            );
            return null;
        }

        return $job_data;
    }

    /**
     * Write the entire job structure to disk
     * Updates the single JSON file with all job data
     *
     * @param array $job_data Complete job data structure
     *
     * @return bool True on success
     */
    private function _writeJob($job_data)
    {
        $job_file = $this->_getJobFilePath();

        // Always update last_modified timestamp
        $job_data['last_modified'] = time();

        $json = json_encode($job_data, JSON_PRETTY_PRINT);

        // LOCK_EX: Exclusive lock prevents readers during write
        $success = file_put_contents($job_file, $json, LOCK_EX);

        if ($success !== false) {
            chmod($job_file, 0600);  // Only owner can read/write
            $this->_file_written = true;  // Mark file as written
            return true;
        }

        return false;
    }

    /**
     * Save job state (public wrapper for external callers)
     *
     * Allows external code to write initial job state before spawning worker.
     * After worker is spawned, only the worker should update the job file.
     *
     * IMPORTANT: Returns false if job is not idle (actively running) AND
     * caller is NOT the CLI worker. This prevents race conditions where
     * the web process tries to write while worker is actively updating.
     *
     * @param array $job_data Complete job data structure
     *
     * @return bool True on success, false if job is not idle or write failed
     */
    public function saveState($job_data)
    {
        // If we're running from CLI (worker process), always allow write
        $is_cli = (php_sapi_name() === 'cli');

        // Check if job is idle (not actively running)
        // This prevents race conditions with the worker process
        // BUT: CLI worker is always allowed to write (it's the one doing the work)
        // ALSO: Skip idle check if file hasn't been written yet (first write)
        if (!$is_cli && $this->_file_written) {
            $idle_status = $this->isIdle();
            if (!$idle_status['is_idle']) {
                // Job is actively running and caller is NOT the worker - reject
                return false;
            }
        }

        return $this->_writeJob($job_data);
    }

    /**
     * Check if the job is idle (not actively running)
     *
     * A job is considered idle when:
     * 1. Job completed successfully, OR
     * 2. Job failed, OR
     * 3. Worker is not active (no recent updates AND process not running)
     *
     * @param int $timeout_seconds Inactivity timeout in seconds (default 10)
     *
     * @return array Array with ['is_idle' => bool, 'reason' => string, 'error' => string|null]
     *               - is_idle: True if job is idle/complete, false if still active
     *               - reason: One of: 'completed', 'failed', 'timeout', 'no_job_file', 'running'
     *               - error: Error message if status is 'failed', null otherwise
     */
    public function isIdle($timeout_seconds = 10)
    {
        $job_data = $this->readJob();
        if (!$job_data) {
            return [
                'is_idle' => true,
                'reason' => 'no_job_file',
                'error' => _("Job file does not exist")
            ];
        }

        // Check if job completed successfully
        if (isset($job_data['status']) && $job_data['status'] === 'completed') {
            return [
                'is_idle' => true,
                'reason' => 'completed',
                'error' => null
            ];
        }

        // Check if job failed
        if (isset($job_data['status']) && $job_data['status'] === 'failed') {
            $error_message = isset($job_data['error']) ? $job_data['error'] : _("Unknown error occurred");
            return [
                'is_idle' => true,
                'reason' => 'failed',
                'error' => $error_message
            ];
        }

        // Check if worker is still active using two methods:

        // Method 1: Timestamp - has file been updated recently?
        if (isset($job_data['last_modified'])) {
            $seconds_since_update = time() - $job_data['last_modified'];
            if ($seconds_since_update < $timeout_seconds) {
                return [
                    'is_idle' => false,
                    'reason' => 'running',
                    'error' => null
                ];  // Recently updated = still active
            }
        }

        // Method 2: PID - is process still running?
        if (isset($job_data['worker_pid'])) {
            if ($this->_isProcessRunning($job_data['worker_pid'])) {
                return [
                    'is_idle' => false,
                    'reason' => 'running',
                    'error' => null
                ];  // Process exists = still active
            }
        }

        // No activity detected = idle due to timeout
        return [
            'is_idle' => true,
            'reason' => 'timeout',
            'error' => sprintf(
                _("Worker process timed out (no activity for %d seconds)"),
                $timeout_seconds
            )
        ];
    }

    /**
     * Check if a process with given PID is currently running
     *
     * Uses POSIX signals to check process existence without killing it
     * Signal 0 = null signal, just checks if we can send signals to PID
     *
     * @param int $pid Process ID to check
     *
     * @return bool True if process is running
     */
    private function _isProcessRunning($pid)
    {
        if (!$pid || $pid <= 0) {
            return false;
        }

        // Use posix_kill with signal 0 to check if process exists
        // Signal 0 doesn't actually send a signal, just checks permission
        if (function_exists('posix_kill')) {
            return posix_kill($pid, 0);
        }

        // Fallback: Check /proc filesystem (Linux)
        return file_exists("/proc/" . $pid);
    }

    /**
     * Check if job has completed successfully
     *
     * @return bool True if job status is 'completed'
     */
    public function isComplete()
    {
        $job_data = $this->readJob();
        if (!$job_data) {
            return false;
        }

        return isset($job_data['status']) && $job_data['status'] === 'completed';
    }

    /**
     * Check if job has failed
     *
     * @return bool True if job status is 'failed'
     */
    public function hasFailed()
    {
        $job_data = $this->readJob();
        if (!$job_data) {
            return false;
        }

        return isset($job_data['status']) && $job_data['status'] === 'failed';
    }

    /**
     * Get job status
     *
     * @return string|null Job status ('pending', 'running', 'completed', 'failed') or null
     */
    public function getStatus()
    {
        $job_data = $this->readJob();
        if (!$job_data) {
            return null;
        }

        return isset($job_data['status']) ? $job_data['status'] : null;
    }

    /**
     * Spawn background worker process to execute import job
     *
     * Creates a detached background process that will process the import job.
     * Does NOT update job file after spawning - the worker script is responsible
     * for updating its own PID, status, and progress.
     *
     * @param string $worker_script Path to worker script
     * @param utils  $utils         Utils instance for process spawning
     *
     * @return int Worker process PID
     *
     * @throws Exception If worker process fails to spawn or job file doesn't exist
     */
    public function spawnWorker($worker_script, $utils)
    {
        // Verify job file exists before spawning
        if (!$this->exists()) {
            throw new Exception("Cannot spawn worker: job file does not exist");
        }

        // Verify worker script exists and is readable
        if (!file_exists($worker_script)) {
            throw new Exception("Worker script not found: " . $worker_script);
        }

        if (!is_readable($worker_script)) {
            throw new Exception("Worker script not readable: " . $worker_script);
        }

        // Note: spawnAsyncProcess defaults to PHP binary when command is null
        // and automatically adds '-f' flag for .php scripts
        // spawnAsyncProcess throws Exception on failure
        try {
            $job_file_path = $this->_getJobFilePath();
            $pid = $utils->spawnAsyncProcess(
                null,
                array($worker_script, $job_file_path),
                11
            );
        } catch (Exception $e) {
            throw new Exception("Failed to spawn worker process: " . $e->getMessage());
        }

        return $pid;
    }

    /**
     * Delete the job file
     *
     * @return bool True if file deleted successfully
     */
    public function cleanup()
    {
        $job_file = $this->_getJobFilePath();
        if (file_exists($job_file)) {
            return unlink($job_file);
        }
        return true;
    }
}

// vim:tabstop=4:expandtab:shiftwidth=4:filetype=php:syntax:ruler:
