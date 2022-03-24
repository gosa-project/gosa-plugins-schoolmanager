<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2018-2022 Daniel Teichmann
 * Copyright (C) 2015-2022 Mike Gabriel
 * Copyright (C) 2015 Marius Rasch
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


class schoolmgr_utils
{
    // Enables/Disables debug prints to js console.
    var $ENABLE_DEBUG_JS_CONSOLE = true;


    function __construct(&$config)
    {
        $this->ui = get_userinfo();
        $this->config = & $config;
    }


    // str_contains is not available for PHP 7 (but in PHP 8).
    // So we write our own function wrapper.
    function strstartswith(string $haystack, string $needle): bool
    {
        $version = explode('.', PHP_VERSION);
        if ($version[0] > 7) {
            return str_starts_with($haystack, $needle);
        } else {
            $length = strlen($needle);
            return (substr($haystack, 0, $length) === $needle);
        }
    }


    // str_contains is not available for PHP 7 (but in PHP 8).
    // So we write our own function wrapper.
    function strcontains($haystack, $needle): bool
    {
        // Fail silently rather than crash.
        if (!is_string($haystack) or !is_string($needle)) {
            $this->debug_to_console(
                _("❌ schoolmgr_utils->strcontains() was called with broken data: ") .
                (is_string($haystack) ? "" : _("\$haystack isn't a string! ")) .
                (is_string($needle)   ? "" : _("\$needle isn't a string! "))
            );

            return false;
        }

        $version = explode('.', PHP_VERSION);
        if ($version[0] > 7) {
            return str_contains($haystack, $needle);
        } else {
            $length = strlen($needle);
            return strpos($haystack, $needle) !== false;
        }
    }


    function debug_to_console($data)
    {
        // Don't print anything to js console, if ENABLE_DEBUG_JS_CONSOLE isn't 'true'.
        if ($this->ENABLE_DEBUG_JS_CONSOLE !== true) {
            return;
        }

        if (is_array($data)) {
            $output = implode('', $data);
        }

        echo "<script>console.log('" . addslashes($data) . "');</script>";
    }


    function getConfigStrValue(string $key): string
    {
        // Define our main class
        $class = "schoolmgr";

        // Get config value from LDAP-Tree, gosa config file (gosa.conf) or
        // defaults from class_schoolmgr.inc
        $ret = $this->config->get_cfg_value($class, $key);

        // Debug property to js console (if debugging is enabled!).
        $debug_str = "Got string property '" . str_pad($key . "':", 58) . "'" . $ret . "'";
        $this->debug_to_console($debug_str);

        return $ret;
    }


    function getConfigBoolValue(string $key): bool
    {
        // Get config value, but pause debugging output.
        $ENABLE_DEBUG_JS_CONSOLE_then = $this->ENABLE_DEBUG_JS_CONSOLE;
        $this->ENABLE_DEBUG_JS_CONSOLE = false;
        $conf_value = strtolower($this->getConfigStrValue($key));
        $this->ENABLE_DEBUG_JS_CONSOLE = $ENABLE_DEBUG_JS_CONSOLE_then;

        $ret = false;

        // Only return TRUE if value is literally "true"…
        if ($conf_value === "true") {
            $ret = true;
        }

        // Debug property to js console (if debugging is enabled!).
        $ret_str = $ret ? "✅" : "❌";
        $debug_str = "Got bool property '" . str_pad($key . "':", 60) . "'" . $ret_str . "'";
        $this->debug_to_console($debug_str);

        return $ret;
    }


    function checkUploadedFile(): array
    {
        // ret_array[0] = (successful): bool
        // ret_array[1] = (error msg / raw csv data): string
        $ret_array = array();

        // Make sure there is a file uploaded…
        if (empty($_FILES['userfile']['name'])) {
            $ret_array[0] = false; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        }

        $handle = null;
        $filename = gosa_file_name($_FILES['userfile']['tmp_name']);

        // TODO: FIXME: Check if file does actually end with '.csv'…

        if (!isset($_FILES['userfile']['name'])) {
            $ret_array[0] = false; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        } elseif (!$_FILES['userfile']['size'] > 0) {
            $ret_array[0] = false; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file is empty"));
            return $ret_array;
        } elseif (!file_exists($filename)) { // Is there a tmp file which we can use?
            $ret_array[0] = false; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        } elseif (!$handle = @fopen($filename, "r")) {
            $ret_array[0] = false; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not readable"));
            return $ret_array;
        }

        $raw_csv_data = "";

        // Reading content of file in 1024 chunks
        while (!feof($handle)) {
            $raw_csv_data .= fread($handle, 1024);
        }
        @fclose($handle);

        // Most of the time the files are encoded in 'UTF-8' or 'ISO-8859-1'.
        if (!mb_check_encoding($raw_csv_data, "UTF-8")) {
            if (mb_check_encoding($raw_csv_data, "iso-8859-1")) {
                $raw_csv_data = utf8_encode($raw_csv_data);
            } else {
                $ret_array[0] = false; // Not successful.
                $ret_array[1] = sprintf(
                    _("Cannot read uploaded file: %s"),
                    _("file encoding not supported")
                );
                return $ret_array;
            }
        }

        $ret_array[0] = true; // Successful!
        $ret_array[1] = $raw_csv_data;
        return $ret_array;
    }


    /**
     * @param array $default_params If $default_params is non-null and $base
     *              (or children of $base) do not exist, create OU with these params.
     * @return int error code (0 = success)
     */
    function checkOUExists(string $base = null, array $default_params = null): int
    {
        if (is_null($base)) {
            $this->debug_to_console(
                _("❌ schoolmgr_utils->checkOUExists() was called with a null-string.")
            );

            return 1; // Errorcode 1
        }

        // Get LDAP link from config.
        $_ldap = $this->config->get_ldap_link();

        $_ldap->cat($base, array('dn'));
        $_response = $_ldap->fetch();
        if (!$_response) {
            if (is_null($default_params)) {
                $this->debug_to_console(
                    sprintf(_("❗ The OU '%s' doesn't exist but \$default_params isn't set. Not creating OU."), $base)
                );
            }

            $this->debug_to_console(
                sprintf(_("❗ The OU '%s' doesn't exist! Creating it…"), $base)
            );

            // change UI "dir" in LDAP to the place where we want to create sub-OUs…
            $_ui = get_userinfo();
            $old_ui_dn = $_ui->dn;

            $old_current_main_base = session::global_is_set("CurrentMainBase") ?
                session::global_get("CurrentMainBase") : $_ui->dn;

            $_pre_ou = explode(",", $base, 2);
            $_pre_ou = $_pre_ou[1];

            $_ldap->cat($_pre_ou);
            if (!$_ldap->fetch()) {
                $this->debug_to_console(
                    sprintf(_("❗ The OU '%s' doesn't exist! Creating it…"), $base)
                );

                if ($this->checkOUExists($_pre_ou, $default_params) !== 0) {
                    // No need to show an error message here again.
                    return 2; // Errorcode 2
                }
            }

            change_ui_dn($old_ui_dn, $_pre_ou);
            session::global_set("CurrentMainBase", $_pre_ou);

            // Create _base DN as a GOsa² department
            $_ou = preg_replace("/^ou=([^,]+),.*/", "$1", $base);

            $deptab = new deptabs($this->config, $this->config->data['TABS']['DEPTABS'], 'new', "deptabs");
            $deptab->by_object['department']->ou = $_ou;
            $deptab->by_object['department']->description = $default_params['description'];
            $deptab->save();

            // Reload departments
            $this->config->get_departments();
            $this->config->make_idepartments();

            // Change UI dir back to where we came from
            change_ui_dn(dn2base($_pre_ou . "," . $this->config->current['BASE']), $old_ui_dn);
            session::global_set("CurrentMainBase", $old_current_main_base);
        } else {
            $this->debug_to_console(
                sprintf(_("❗ The OU '%s' already exists! Using it…"), $base)
            );
        }

        return 0; // Success
    }


    /**
     * Locks the given user.
     *
     * @param string $account_dn User account DN to lock.
     */
    function lockUser(string $account_dn): void
    {
        $this->setLockStatusUsers(array($account_dn), "lock");
    }


    /**
     * Unlocks the given user.
     *
     * @param string $account_dn User account DN to unlock.
     */
    function unlockUser(string $account_dn): void
    {
        $this->setLockStatusUsers(array($account_dn), "unlock");
    }


    /**
     * Toggles lock state of the given user.
     *
     * @param string $account_dn User account DN to toggle.
     */
    function toggleLockOfUser(string $account_dn): void
    {
        $this->setLockStatusUsers(array($account_dn), "toggle");
    }


    /**
     * Locks/unlocks/toggles the given user(s).
     *
     * @param array  $entry Array of user DN's
     * @param string $type  'toggle', 'lock' or 'unlock' user accounts.
     */
    function setLockStatusUsers(array $entry, string $type = "lock"): void
    {
        // Filter out entries we are not allowed to modify
        $disallowed = array();
        $allowed    = array();

        // Get LDAP link from config.
        $_ldap = $this->config->get_ldap_link();

        foreach ($entry as $dn) {
            if (!preg_match("/w/", $this->ui->get_permissions($dn, "users/password"))) {
                $disallowed[] = $dn;
            } else {
                $allowed[] = $dn;
            }
        }

        if (count($disallowed)) {
            msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
        }

        // Try to lock/unlock/toggle the rest of the entries.
        foreach ($allowed as $dn) {
            $_ldap->cat($dn, array('userPassword'));

            if ($_ldap->count() == 1) {
                // We can't lock users with empty passwords.
                $val = $_ldap->fetch();
                if (!isset($val['userPassword'])) {
                    continue;
                }

                // Detect the password method and try to lock/unlock/toggle.
                $pwd = $val['userPassword'][0];
                $method = passwordMethod::get_method($pwd, $val['dn']);
                $success = true;

                if ($method instanceof passwordMethod) {
                    if ($type == "toggle") {
                        if ($method->is_locked($this->config, $val['dn'])) {
                            $this->debug_to_console(sprintf(_("🔓 Unlocking account: '%s'"), $dn));
                            $success = $method->unlock_account($this->config, $val['dn']);
                        } else {
                            $this->debug_to_console(sprintf(_("🔒 Locking account: '%s'"), $dn));
                            $success = $method->lock_account($this->config, $val['dn']);
                        }
                    } elseif (
                        $type == "lock" &&
                        !$method->is_locked($this->config, $val['dn'])
                    ) {
                        $this->debug_to_console(sprintf(_("🔒 Locking account: '%s'"), $dn));
                        $success = $method->lock_account($this->config, $val['dn']);
                    } elseif (
                        $type == "unlock" &&
                        $method->is_locked($this->config, $val['dn'])
                    ) {
                        $this->debug_to_console(sprintf(_("🔓 Unlocking account: '%s'"), $dn));
                        $success = $method->unlock_account($this->config, $val['dn']);
                    }

                    // Check if everything went fine.
                    if (!$success) {
                        $hn = $method->get_hash_name();
                        // Get zeroth entry, if array.
                        if (is_array($hn)) {
                            $hn = $hn[0];
                        }
                        msg_dialog::display(
                            _("Account locking"),
                            sprintf(_("Password method '%s' does not support locking. " .
                                      "Account (%s) has not been modified!"), $hn, $dn),
                            WARNING_DIALOG
                        );
                    }
                } else {
                    // Can't lock unknown methods.
                }
            } elseif ($_ldap->count() > 1) {
                msg_dialog::display(
                    _("Account locking"),
                    sprintf(_("Found multiple accounts in LDAP-tree for DN: '%s'"), $dn),
                    ERROR_DIALOG
                );
            } elseif ($_ldap->count() <= 0) {
                msg_dialog::display(
                    _("Account locking"),
                    sprintf(_("Found no account in LDAP-tree for DN: '%s'"), $dn),
                    ERROR_DIALOG
                );
            }
        }
    }


    /**
     * getLockStatusUsers() wrapper for a single user.
     *
     * @return string Which can be:
     *                - "LOCKED"
     *                - "UNLOCKED"
     *                - "UNACCESSIBLE"
     */
    function getLockStatusUser(string $account_dn): string
    {
        return $this->getLockStatusUsers(array($account_dn))[$account_dn];
    }


    /**
     * Gets lock status of the given user(s).
     *
     * @param array $account_dns Array of user DN's
     * @return array with '$dn => $state' pairs.
     *              $state can be:
     *                - "LOCKED"
     *                - "UNLOCKED"
     *                - "UNACCESSIBLE"
     */
    function getLockStatusUsers(array $account_dns): array
    {
        $return_array = array();

        // Filter out entries we are not allowed to modify
        $allowed      = array();
        $disallowed   = array();

        // String constants
        $_LOCKED       = "LOCKED";
        $_UNLOCKED     = "UNLOCKED";
        $_UNACCESSIBLE = "UNACCESSIBLE";

        // Get LDAP link from config.
        $_ldap = $this->config->get_ldap_link();

        foreach ($account_dns as $dn) {
            if (!preg_match("/w/", $this->ui->get_permissions($dn, "users/password"))) {
                $disallowed[] = $dn;
                $return_array[$dn] = $_UNACCESSIBLE;
            } else {
                $allowed[] = $dn;
            }
        }

        if (count($disallowed)) {
            msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
        }

        // Try to lock/unlock/toggle the rest of the entries.
        foreach ($allowed as $dn) {
            $_ldap->cat($dn, array('userPassword'));

            if ($_ldap->count() == 1) {
                // We can't lock users with empty passwords.
                $val = $_ldap->fetch();
                if (!isset($val['userPassword'])) {
                    continue;
                }

                // Detect the password method and try to lock/unlock/toggle.
                $pwd = $val['userPassword'][0];
                $method = passwordMethod::get_method($pwd, $val['dn']);

                if ($method instanceof passwordMethod) {
                    if ($method->is_locked($this->config, $val['dn'])) {
                        $return_array[$dn] = $_LOCKED;
                        $this->debug_to_console(sprintf(_("🔒 Account is locked: '%s'"), $dn));
                    } else {
                        $return_array[$dn] = $_UNLOCKED;
                        $this->debug_to_console(sprintf(_("🔓 Account is unlocked: '%s'."), $dn));
                    }
                } else {
                    // Can't check unknown methods.
                    $return_array[$dn] = $_UNACCESSIBLE;
                }
            } elseif ($_ldap->count() > 1) {
                msg_dialog::display(
                    _("Account locking"),
                    sprintf(_("Found multiple accounts in LDAP-tree for DN: '%s'"), $dn),
                    ERROR_DIALOG
                );

                $return_array[$dn] = $_UNACCESSIBLE;
            } elseif ($_ldap->count() <= 0) {
                msg_dialog::display(
                    _("Account locking"),
                    sprintf(_("Found no account in LDAP-tree for DN: '%s'"), $dn),
                    ERROR_DIALOG
                );

                $return_array[$dn] = $_UNACCESSIBLE;
            }
        }

        return $return_array;
    }


    function compareObjects($object_a, $object_b, $attrs = array("sn", "givenName"), $prefix = "", $ci = false): ?array
    {
        $unequal = array();
        foreach ($attrs as $key => $attr) {
            $val_a = $val_b = null;
            if (isset($object_a[$attr][0])) {
                $val_a = ($ci === true) ? strtolower($object_a[$attr][0]) : $object_a[$attr][0];
            }
            if (isset($object_b[$attr][0])) {
                $val_b = ($ci === true) ? strtolower($object_b[$attr][0]) : $object_b[$attr][0];
            }

            if ($val_a !== $val_b) {
                $unequal[] = $prefix . $attr;
            }
        }

        if (empty($unequal)) {
            // If 'null' is returned, objects regarding the given attributes are identical
            return null;
        } else {
            // Otherwise return those attribute names that did not give a match
            return $unequal;
        }
    }


    /**
     * @param  string $gender Gender of this user. In following format:
     *                        First letter (translated) of Female, Male, Diverse
     *                        or whole word (translated) Female, Male, Diverse.
     *                        * 'f', 'm' or 'd'
     *                        * 'female', 'male', 'diverse'
     *                        * _('f'), _('m'), _('d')
     *                        * _('female'), _('male'), _('diverse')
     *
     * @return string Normalized gender string (F => Female, M => Male, D => Diverse)
     *                can be **empty** if unsuccessful.
     */
    function normalizeGenderString(string $gender): string
    {
        // Gender should be case-insenstive.
        $gender = strtolower($gender);

        $gender_mapping = array(
            "F" => array("f", "female", _("female")),
            "M" => array("m", "male", _("male")),
            "D" => array("d", "diverse", _("diverse")),
        );

        foreach ($gender_mapping as $key => $map) {
            if (in_array_strict($gender, $map)) {
                return $key;
            }
        }

        return "";
    }


    function parseCSV(
        string $raw_csv_data,
        string $delimiter_str,
        bool $skip_first_line
    ): array {
        // Delete all '\r' occurences.
        $raw_csv_data = str_replace('\r', '', $raw_csv_data);

        $lines = preg_split("/\n/", $raw_csv_data);
        $num_max_columns = 0;
        $rest = 0;
        $data = array();

        if ($skip_first_line) {
            // Pops first element…
            array_shift($lines);
        }

        // $lines should be an array at this point…
        if (!is_array($lines)) {
            return array();
        }

        // Check column count, if it stays zero,
        // we probably don't have a *comma* separated CSV file.
        foreach ($lines as $line) {
            $line = trim($line);

            // Ignore empty lines and comments.
            if (!is_string($line) || !$line) {
                continue;
            }
            if (substr($line, 0, 1) == "#") {
                continue;
            }

            $cells = array();
            $raw_cells = explode($delimiter_str, $line);

            $concatenating = false;
            $concat_cell = "";
            $concat_quote = "";

            foreach ($raw_cells as $cell) {
                if ((!$concatenating) && (substr($cell, 0, 1) == '"')) {
                    $concat_cell = substr($cell, 1);
                    $concatenating = true;
                    $concat_quote = '"';
                } elseif ((!$concatenating) && (substr($cell, 0, 1) == "'")) {
                    $concat_cell = substr($cell, 1);
                    $concatenating = true;
                    $concat_quote = '"';
                } elseif (($concatenating) && (substr($cell, -1) == $concat_quote)) {
                    $concat_cell = $concat_cell . $delimiter_str . substr($cell, 0, -1);
                    $cells[] = $concat_cell;
                    $concatenating = false;
                    $concat_cell = "";
                    $concat_quote = "";
                } elseif ($concatenating) {
                    $concat_cell = $concat_cell . $delimiter_str . $cell;
                } else {
                    $cells[] = $cell;
                }
            }

            // Update $num_max_columns if this line had more cells.
            if (count($cells) > $num_max_columns) {
                $num_max_columns = count($cells);
            }

            $data[] = $cells;
        }

        // We surely want more than 0 columns…
        if ($num_max_columns < 1) {
            return array();
        }

        // Generate array with output info
        foreach ($data as $row) {
            // Fill up this row with empty columns,
            // if it has less columns then $num_columns
            if (is_array($row)) {
                if (count($row) <= $num_max_columns) {
                    // Calculate how many fields we wanna append.
                    $rest = $num_max_columns - count($row);

                    // Append $rest amount of empty strings.
                    for ($i = 0; $i < $rest; $i++) {
                        $row[] = "";
                    }
                }
            }
        }

        $csvinfo['data']     = $data;
        $csvinfo['num_cols'] = $num_max_columns;
        $csvinfo['num_rows'] = count($csvinfo['data']);
        return $csvinfo;
    }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
