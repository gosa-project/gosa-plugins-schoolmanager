#!/usr/bin/env php
<?php

/**
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
 *
 * ----------------------------------------------------------------------------------
 * Background Worker Script for Asynchronous LDAP Account Import
 *
 * This CLI script processes LDAP account imports in the background, allowing
 * the web interface to remain responsive during large import operations.
 *
 * Usage:
 *   php -f class_cli_import_worker.inc <job_file_path>
 * ----------------------------------------------------------------------------------
 */

// Hardcoded retention policy (hours/days)
$COMPLETED_HOURS = 24;  // Keep completed/failed jobs for 1 day
$STALE_HOURS     = 168; // Keep stale            jobs for 7 days
$ORPHANED_DAYS   = 30;  // Keep orphaned         jobs for 30 days

$JOB_DIR = '/var/spool/gosa/schoolmgr-import';

// Ensure we're running from CLI
if (php_sapi_name() !== 'cli') {
    die("ERROR: This script must be run from the command line\n");
}

// Check for cleanup mode BEFORE any GOsa² bootstrap
if ($argc === 2 && $argv[1] === '--cleanup') {
    perform_cleanup();
    exit(0);
}

// Check arguments
if ($argc < 2) {
    fwrite(STDERR, "Usage: {$argv[0]} <job_file_path>\n");
    fwrite(STDERR, "   OR: {$argv[0]} --cleanup\n");
    exit(1);
}

$job_file_path = $argv[1];

// Extract job_id from file path (e.g., /var/spool/gosa/schoolmgr-import/123_abc.json -> 123_abc)
$job_id = basename($job_file_path, '.json');

// Set up job-specific log file path
$job_log_file = dirname($job_file_path) . '/' . $job_id . '.log';

/**
 * Custom logging function for worker process
 *
 * Logs to both error_log (syslog) and job-specific log file
 *
 * @param string $message Log message
 * @param string $level   Log level (INFO, ERROR, WARNING, DEBUG)
 *
 * @return none
 */
function worker_log($message, $level = 'INFO')
{
    global $job_log_file, $job_id;

    $timestamp = date('Y-m-d H:i:s');
    $pid = getmypid();
    $formatted_message = sprintf(
        "[%s] [%s] [PID:%d] [Job:%s] %s",
        $timestamp,
        $level,
        $pid,
        $job_id,
        $message
    );

    // Log to error_log (syslog)
    error_log("[ImportWorker] " . $message);

    // Append to job-specific log file
    $log_entry = $formatted_message . "\n";
    if (file_put_contents($job_log_file, $log_entry, FILE_APPEND | LOCK_EX) === false) {
        // If we can't write to log file, at least log the error
        error_log("[ImportWorker] WARNING: Failed to write to log file: {$job_log_file}");
    }
}

// Log start of worker
worker_log(sprintf("Starting worker for job: %s (PID: %d)", $job_id, getmypid()), 'INFO');

try {
    // ============================================================================
    // STEP 1: Bootstrap GOsa² Environment
    // ============================================================================

    // Find GOsa² installation directory
    $gosa_base = '/usr/share/gosa';
    if (!is_dir($gosa_base)) {
        throw new Exception("GOsa² base directory not found: {$gosa_base}");
    }

    // Bootstrap GOsa² core
    include_once "{$gosa_base}/include/php_setup.inc";
    include_once "{$gosa_base}/include/functions.inc";

    // Load schoolmanager plugin classes
    $plugin_dir = dirname(__FILE__);
    include_once "{$plugin_dir}/class_importjob.inc";

    worker_log("GOsa² environment bootstrapped");

    // ============================================================================
    // STEP 2: Load Job State and Restore Session
    // ============================================================================

    // Load job by job_id
    $job = new importjob($job_id);

    if (!$job->exists()) {
        throw new Exception("Job file not found for job_id: {$job_id}");
    }

    $job_data = $job->readJob();
    if (!$job_data) {
        throw new Exception("Failed to read job data for job_id: {$job_id}");
    }

    worker_log("Job state loaded successfully");

    // ============================================================================
    // STEP 2a: Restore GOsa² Session and Configuration
    // ============================================================================

    $state = $job_data['state'];
    $session_id = $state['session_id'] ?? null;

    if (!$session_id) {
        throw new Exception("No session_id found in job state - cannot restore session");
    }

    worker_log("Attempting to restore session: {$session_id}");

    // Close current session (if any) and restore the original one
    if (session_status() === PHP_SESSION_ACTIVE) {
        session_write_close();
    }

    // Restore the session from the web process
    session_id($session_id);
    session_start();

    worker_log("Session restored successfully");

    // Restore HTTP_GOSAKEY from job state (required for credential decryption)
    $gosakey = $state['gosakey'] ?? null;
    if ($gosakey !== null) {
        $_SERVER['HTTP_GOSAKEY'] = $gosakey;
        worker_log("HTTP_GOSAKEY restored for credential decryption");
    } else {
        worker_log("WARNING: No HTTP_GOSAKEY found in job state", 'WARNING');
    }

    // Now we can access the GOsa² session data
    if (!isset($_SESSION['config']) || !is_object($_SESSION['config'])) {
        throw new Exception("GOsa² config not found in restored session");
    }

    // Get the actual GOsa² config object from session
    $config = $_SESSION['config'];
    worker_log("GOsa² config loaded from session");

    // Get UI object for user information
    $ui = $_SESSION['ui'] ?? null;
    if ($ui) {
        $username = $ui->username ?? 'unknown';
        $dn = $ui->dn ?? 'unknown';
        worker_log("Restored user context: {$username} (DN: {$dn})");
    }

    // IMPORTANT: Close session immediately after reading
    // This releases the session lock so the web interface can continue
    // responding to requests (progress checks, other actions, etc.)
    session_write_close();
    worker_log("Session closed - released lock for web interface");

    // Recreate importjob
    $job = new importjob($job_id);

    worker_log("GOsa² environment fully initialized with session context");

    // ============================================================================
    // STEP 3: Register Worker (Update PID and Status)
    // ============================================================================

    $job_data['worker_pid'] = getmypid();
    $job_data['status'] = 'running';
    $job_data['started_at'] = time();

    // Initialize progress if not set
    if (!isset($job_data['progress'])) {
        $job_data['progress'] = array(
            'current' => 0,
            'total' => 0,
            'percent' => 0,
            'message' => 'Starting import...',
            'timestamp' => time(),
        );
    }

    // Write updated job data (worker now owns this file)
    if (!$job->saveState($job_data)) {
        throw new Exception("Failed to update job state with worker PID");
    }

    worker_log(sprintf("Worker registered (PID: %d, Status: running)", getmypid()));

    // ============================================================================
    // STEP 4: Extract Job State Variables
    // ============================================================================

    $state = $job_data['state'];
    $csvinfo = $state['csvinfo'];
    $import_account_type = $state['import_account_type'];

    // Validate required data
    if (!isset($csvinfo['data_preldap']) || !is_array($csvinfo['data_preldap'])) {
        throw new Exception("Invalid job state: data_preldap not found or not an array");
    }

    $total_accounts = count($csvinfo['data_preldap']);
    worker_log(sprintf("Processing %d accounts (type: %s)", $total_accounts, $import_account_type));

    // Update progress with total count
    $job_data['progress']['total'] = $total_accounts;
    $job_data['progress']['current'] = 0;
    $job_data['progress']['message'] = sprintf('Processing %d accounts...', $total_accounts);
    $job->saveState($job_data);

    // ============================================================================
    // STEP 5: Initialize Statistics
    // ============================================================================

    $stats = array(
        'total' => $total_accounts,
        'created' => 0,
        'updated' => 0,
        'skipped' => 0,
        'failed' => 0,
    );

    $start_time = microtime(true);

    // ============================================================================
    // STEP 5a: Establish LDAP Connection
    // ============================================================================

    worker_log("Establishing LDAP connection");

    // Get LDAP link from config (this uses credentials from the restored session)
    try {
        $ldap = $config->get_ldap_link();
    } catch (Exception $e) {
        throw new Exception("Failed to establish LDAP connection: " . $e->getMessage());
    }

    // Set LDAP base DN
    $ldap->cd($config->current['BASE']);

    $base_dn = $config->current['BASE'] ?? 'unknown';
    worker_log("LDAP connection established (Base DN: {$base_dn})");

    // ============================================================================
    // STEP 6: Load Import Class with Restored Context
    // ============================================================================

    worker_log("Loading import class: import{$import_account_type}");

    // Dynamically load the appropriate import class
    $import_class = 'import' . $import_account_type;

    if (!class_exists($import_class)) {
        // Try to load the class file
        $class_file = "{$plugin_dir}/class_{$import_class}.inc";
        if (file_exists($class_file)) {
            include_once $class_file;
        } else {
            throw new Exception("Import class file not found: {$class_file}");
        }
    }

    // Instantiate the import class with the restored config
    $import_instance = new $import_class($config);

    // Restore the csvinfo state
    $import_instance->csvinfo = $csvinfo;
    $import_instance->_ldap = $ldap;
    $import_instance->ui = $ui;

    worker_log("Import instance created and configured");

    // ============================================================================
    // STEP 7: Process Accounts (Main Loop)
    // ============================================================================

    worker_log("Starting account processing loop");

    foreach ($csvinfo['data_preldap'] as $idx => $user_data) {
        // Import the user's main account
        if (
            isset($user_data['main_account'])
            and !empty($user_data['main_account'])
        ) {
            try {
                $account_data = $user_data['main_account'];
                $uid = $account_data['uid'][0] ?? 'unknown';

                worker_log(
                    sprintf(
                        "Processing 'main_account' %d/%d: %s",
                        $idx + 1,
                        $total_accounts,
                        $import_instance->serializeAccount($account_data),
                    )
                );

                // ================================================================
                // LDAP OPERATION - Import main account
                // ================================================================

                // Throws exception if fails
                $result = $import_instance->importLDAPUserObject($account_data);

                // Update stats and mark account status
                $main_acct = &$csvinfo['data_preldap'][$idx]['main_account'];
                if ($result === 'created') {
                    $stats['created']++;
                    $main_acct['_status'][0] = 'created';
                    $main_acct['_imported'][0] = true;
                    $main_acct['_error'][0] = '';
                } elseif ($result === 'updated') {
                    $stats['updated']++;
                    $main_acct['_status'][0] = 'updated';
                    $main_acct['_imported'][0] = true;
                    $main_acct['_error'][0] = '';
                } elseif ($result === 'skipped') {
                    $stats['skipped']++;
                    $main_acct['_status'][0] = 'skipped';
                    $main_acct['_imported'][0] = true;
                    $main_acct['_error'][0] = '';
                } else {
                    throw new Exception("Unexpected import result: {$result}");
                }
            } catch (Exception $e) {
                // Log error but CONTINUE processing (per-account error)
                $stats['failed']++;
                $error_msg = sprintf(
                    "Main account %d/%d: %s: %s",
                    $idx + 1,
                    $total_accounts,
                    $import_instance->serializeAccount($account_data),
                    $e->getMessage()
                );

                worker_log("ERROR: {$error_msg}", 'ERROR');

                // Mark account with error flag
                $main_acct = &$csvinfo['data_preldap'][$idx]['main_account'];
                $main_acct['_status'][0] = 'failed';
                $main_acct['_imported'][0] = false;
                $main_acct['_error'][0] = $e->getMessage();
            }
        }

        // Import the user's aux accounts (e.g., parent accounts)
        if (
            isset($user_data['aux_accounts'])
            and !empty($user_data['aux_accounts'])
        ) {
            foreach ($user_data['aux_accounts'] as $idx_aux => $aux_account) {
                try {
                    $uid = $aux_account['uid'][0] ?? 'unknown';

                    worker_log(
                        sprintf(
                            "Processing 'aux_account' #%d for row %d/%d: %s",
                            $idx_aux,
                            $idx + 1,
                            $total_accounts,
                            $import_instance->serializeAccount($aux_account),
                        )
                    );

                    // ========================================================
                    // LDAP OPERATION - Import aux account
                    // ========================================================
                    $result = $import_instance->importLDAPUserObject(
                        $aux_account
                    );

                    // Update stats and mark account status
                    $aux_acct = &$csvinfo['data_preldap'][$idx]
                                         ['aux_accounts'][$idx_aux];
                    if ($result === 'created') {
                        $stats['created']++;
                        $aux_acct['_status'][0] = 'created';
                        $aux_acct['_imported'][0] = true;
                        $aux_acct['_error'][0] = '';
                    } elseif ($result === 'updated') {
                        $stats['updated']++;
                        $aux_acct['_status'][0] = 'updated';
                        $aux_acct['_imported'][0] = true;
                        $aux_acct['_error'][0] = '';
                    } elseif ($result === 'skipped') {
                        $stats['skipped']++;
                        $aux_acct['_status'][0] = 'skipped';
                        $aux_acct['_imported'][0] = true;
                        $aux_acct['_error'][0] = '';
                    } else {
                        throw new Exception(
                            "Unexpected import result: {$result}"
                        );
                    }
                } catch (Exception $e) {
                    // Log error but CONTINUE processing (per-account error)
                    $stats['failed']++;
                    $error_msg = sprintf(
                        "Aux account #%d for row %d/%d: %s: %s",
                        $idx_aux,
                        $idx + 1,
                        $total_accounts,
                        $import_instance->serializeAccount($aux_account),
                        $e->getMessage()
                    );

                    worker_log("ERROR: {$error_msg}", 'ERROR');

                    // Mark account with error flag
                    $aux_acct = &$csvinfo['data_preldap'][$idx]
                                         ['aux_accounts'][$idx_aux];
                    $aux_acct['_status'][0] = 'failed';
                    $aux_acct['_imported'][0] = false;
                    $aux_acct['_error'][0] = $e->getMessage();
                }
            }
        }

        $percent = round((($idx + 1) / $total_accounts) * 100);

        $job_data['progress'] = array(
            'current' => $idx + 1,
            'total' => $total_accounts,
            'percent' => $percent,
            'message' => sprintf(
                'Processed %d/%d accounts (%d%%)',
                $idx + 1,
                $total_accounts,
                $percent
            ),
            'timestamp' => time(),
        );

        // Update csvinfo in job state (for streaming results to UI)
        $job_data['state']['csvinfo'] = $csvinfo;

        // Write updated state to disk
        $job->saveState($job_data);

        worker_log(
            sprintf(
                "Progress: %d/%d (%d%%)",
                $idx + 1,
                $total_accounts,
                $percent
            )
        );
    }

    // ============================================================================
    // STEP 7: Finalize Results
    // ============================================================================

    $duration = microtime(true) - $start_time;

    worker_log(
        sprintf(
            "Import completed - Created: %d, Updated: %d, Skipped: %d," .
            " Failed: %d (Duration: %.2fs)",
            $stats['created'],
            $stats['updated'],
            $stats['skipped'],
            $stats['failed'],
            $duration
        )
    );

    // Prepare final results
    $job_data['results'] = array(
        'stats' => $stats,
        'duration' => round($duration, 2),
    );

    // Set final status
    $job_data['status'] = 'completed';
    $job_data['completed_at'] = time();

    // Final progress update
    $job_data['progress'] = array(
        'current' => $total_accounts,
        'total' => $total_accounts,
        'percent' => 100,
        'message' => sprintf(
            'Import completed: %d created, %d updated, %d skipped, %d failed',
            $stats['created'],
            $stats['updated'],
            $stats['skipped'],
            $stats['failed']
        ),
        'timestamp' => time(),
    );

    // Write final state
    $job->saveState($job_data);

    worker_log("Job completed successfully");

    // ============================================================================
    // STEP 8: Trigger Cleanup of Old Job Files
    // ============================================================================

    worker_log("Triggering cleanup of old job files");
    perform_cleanup();

    // ============================================================================
    // STEP 9: Exit Successfully
    // ============================================================================
    // Note: Session was already closed earlier after reading config/ui
    // to release the lock and allow web interface to remain responsive

    exit(0);
} catch (Exception $e) {
    // ============================================================================
    // STEP 8: Error Handling (Fatal Errors)
    // ============================================================================

    $error_msg = sprintf(
        "FATAL ERROR in worker (PID %d): %s\n%s\n",
        getmypid(),
        $e->getMessage(),
        $e->getTraceAsString()
    );

    worker_log($error_msg, 'FATAL');
    fwrite(STDERR, $error_msg);

    // Try to update job status to 'failed'
    try {
        if (isset($job) && $job->exists()) {
            $job_data = $job->readJob();
            if ($job_data) {
                $job_data['status'] = 'failed';
                $job_data['error'] = $e->getMessage();
                $job_data['failed_at'] = time();

                $job_data['progress']['message'] = 'Import failed: ' .
                                                   $e->getMessage();

                $job->saveState($job_data);
            }
        }
    } catch (Exception $cleanup_error) {
        $cleanup_msg = "Failed to update job status during error handling: " .
                       $cleanup_error->getMessage();
        worker_log($cleanup_msg, 'ERROR');
    }

    exit(1);
}

/**
 * Cleanup old import job files
 *
 * RETENTION POLICY (based on file mtime):
 *   - Completed/failed: 24 hours
 *   - Stale (running):  7 days (catch zombie processes)
 *   - Orphaned/invalid: 30 days
 *
 * EXECUTION:
 *   - Automatic: After each import completion
 *   - Scheduled: Weekly via systemd timer (Debian)
 *
 * MANUAL USAGE:
 *   php class_cli_import_worker.inc --cleanup
 *   journalctl -u gosa-plugin-schoolmanager-cleanup-import-jobs.service -n 50
 *
 * FILES:
 *   /var/spool/gosa/schoolmgr-import/import_*_*.{json,log}
 *
 * @return none
 */
function perform_cleanup()
{
    global $JOB_DIR, $COMPLETED_HOURS, $STALE_HOURS, $ORPHANED_DAYS;

    error_log(
        sprintf(
            "[ImportCleanup] Starting cleanup: dir=%s, completed=%dh, stale=%dh, orphaned=%dd",
            $JOB_DIR,
            $COMPLETED_HOURS,
            $STALE_HOURS,
            $ORPHANED_DAYS
        )
    );

    // Validate directory exists
    if (!is_dir($JOB_DIR)) {
        error_log("[ImportCleanup] Job directory does not exist: {$JOB_DIR}");
        exit(0); // Not an error - maybe no imports yet
    }

    // Find all job files matching pattern: import_<timestamp>_<random>.json
    $pattern = $JOB_DIR . '/import_*_*.json';
    $files = glob($pattern);

    if ($files === false || count($files) === 0) {
        error_log("[ImportCleanup] No job files found in {$JOB_DIR}");
        exit(0);
    }

    error_log(sprintf("[ImportCleanup] Found %d job files", count($files)));

    // Calculate time thresholds
    $now = time();
    $COMPLETED_THRESHOLD = $now - ($COMPLETED_HOURS * 3600);
    $STALE_THRESHOLD     = $now - ($STALE_HOURS     * 3600);
    $ORPHANED_THRESHOLD  = $now - ($ORPHANED_DAYS   * 86400);

    $deleted_count = 0;
    $skipped_count = 0;
    $errors = 0;

    foreach ($files as $file) {
        // Validate file still exists (race condition with concurrent cleanup)
        if (!file_exists($file)) {
            continue;
        }

        $job_id = basename($file, '.json');
        $file_mtime = filemtime($file);
        $file_age_hours = ($now - $file_mtime) / 3600;

        // Parse JSON to determine job status
        $json_content = @file_get_contents($file);
        if ($json_content === false) {
            error_log("[ImportCleanup] WARNING: Cannot read file: {$file}");
            $errors++;
            continue;
        }

        $data = @json_decode($json_content, true);
        if ($data === null) {
            error_log("[ImportCleanup] WARNING: Invalid JSON in {$file}, treating as orphaned");
            $status = 'unknown';
        } else {
            $status = $data['status'] ?? 'unknown';
        }

        // Apply retention policy
        $should_delete = false;
        $delete_reason = '';

        if (in_array($status, ['completed', 'failed'], true)) {
            if ($file_mtime <= $COMPLETED_THRESHOLD) {
                $should_delete = true;
                $delete_reason = sprintf(
                    "status=%s, age=%.1fh > %dh",
                    $status,
                    $file_age_hours,
                    $COMPLETED_HOURS
                );
            }
        } elseif (in_array($status, ['running', 'idle'], true)) {
            if ($file_mtime <= $STALE_THRESHOLD) {
                $should_delete = true;
                $delete_reason = sprintf(
                    "STALE status=%s, age=%.1fh > %dh",
                    $status,
                    $file_age_hours,
                    $STALE_HOURS
                );
            }
        } else {
            // Unknown/invalid status - use orphaned timeout
            if ($file_mtime <= $ORPHANED_THRESHOLD) {
                $should_delete = true;
                $delete_reason = sprintf(
                    "orphaned status=%s, age=%.1fd > %dd",
                    $status,
                    $file_age_hours / 24,
                    $ORPHANED_DAYS
                );
            }
        }

        if (!$should_delete) {
            error_log(
                sprintf(
                    "[ImportCleanup] Skipped %s: status=%s, age=%.1fh",
                    $file,
                    $status,
                    $file_age_hours
                )
            );
            $skipped_count++;
            continue;
        }

        // Attempt deletion of JSON file
        if (@unlink($file)) {
            $deleted_count++;
            error_log("[ImportCleanup] Deleted {$file}: {$delete_reason}");

            // Also delete corresponding log file
            $log_file = $JOB_DIR . '/' . $job_id . '.log';
            if (file_exists($log_file)) {
                if (@unlink($log_file)) {
                    error_log("[ImportCleanup] Deleted {$log_file}");
                } else {
                    error_log("[ImportCleanup] WARNING: Failed to delete log file: {$log_file}");
                    $errors++;
                }
            }
        } else {
            error_log("[ImportCleanup] WARNING: Failed to delete {$file}");
            $errors++;
        }
    }

    error_log(
        sprintf(
            "[ImportCleanup] Cleanup completed: scanned=%d, deleted=%d, skipped=%d, errors=%d",
            count($files),
            $deleted_count,
            $skipped_count,
            $errors
        )
    );
}

// vim:tabstop=4:expandtab:shiftwidth=4:filetype=php:syntax:ruler:
