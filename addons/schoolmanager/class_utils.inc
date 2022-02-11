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
    var bool $DEBUG_MODE = true;


    function __construct(&$config)
    {
        $this->config = & $config;
    }


    // str_contains is not available for PHP 7.
    // So we write our own function wrapper.
    function strcontains(string $haystack, string $needle): bool {
        return strpos($haystack, $needle) !== FALSE;
    }


    function debug_to_console($data) {
        // Don't print anything to js console, if DEBUG_MODE isn't 'true'.
        if ($this->DEBUG_MODE !== true) return;

        if (is_array($data))
            $output = implode('', $data);

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
        $debug_mode_then = $this->DEBUG_MODE;
        $this->DEBUG_MODE = FALSE;
        $conf_value = strtolower($this->getConfigStrValue($key));
        $this->DEBUG_MODE = $debug_mode_then;

        $ret = FALSE;

        // Only return TRUE if value is literally "true"…
        if ($conf_value === "true") {
            $ret = TRUE;
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
            $ret_array[0] = FALSE; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        }

        $handle = NULL;
        $filename = gosa_file_name($_FILES['userfile']['tmp_name']);

        // TODO: FIXME: Check if file does actually end with '.csv'…

        if (!isset($_FILES['userfile']['name'])) {
            $ret_array[0] = FALSE; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        } elseif (!$_FILES['userfile']['size'] > 0) {
            $ret_array[0] = FALSE; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file is empty"));
            return $ret_array;
        } elseif (!file_exists($filename)) { // Is there a tmp file which we can use?
            $ret_array[0] = FALSE; // Not successful.
            $ret_array[1] = sprintf(_("Cannot read uploaded file: %s"), _("file not found"));
            return $ret_array;
        } elseif (!$handle = @fopen($filename, "r")) {
            $ret_array[0] = FALSE; // Not successful.
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
                $ret_array[0] = FALSE; // Not successful.
                $ret_array[1] = sprintf(
                    _("Cannot read uploaded file: %s"),
                    _("file encoding not supported")
                );
                return $ret_array;
            }
        }

        $ret_array[0] = TRUE; // Successful!
        $ret_array[1] = $raw_csv_data;
        return $ret_array;
    }


    function compareObjects($object_a, $object_b, $attrs = array("sn", "givenName"), $prefix = "", $ci = FALSE): ?array
    {
        $unequal = array();
        foreach ($attrs as $key => $attr) {

            $val_a = $val_b = NULL;
            if (isset($object_a[$attr][0])) {
                $val_a = ($ci === TRUE) ? strtolower($object_a[$attr][0]) : $object_a[$attr][0];
            }
            if (isset($object_b[$attr][0])) {
                $val_b = ($ci === TRUE) ? strtolower($object_b[$attr][0]) : $object_b[$attr][0];
            }

            if ($val_a !== $val_b) {
                $unequal[] = $prefix . $attr;
            }
        }

        if (empty($unequal)) {
            // If 'null' is returned, objects regarding the given attributes are identical
            return null;
        }
        else {
            // Otherwise return those attribute names that did not give a match
            return $unequal;
        }
    }


    function parseCSV(string $raw_csv_data,
                      string $delimiter_str,
                      bool $skip_first_line): array
    {
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
        if (!is_array($lines)) return array();

        // Check column count, if it stays zero,
        // we probably don't have a *comma* separated CSV file.
        foreach ($lines as $line) {
            $line = trim($line);

            // Ignore empty lines and comments.
            if (!is_string($line) || !$line) continue;
            if (substr($line, 0, 1) == "#") continue;

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
                }
                elseif ((!$concatenating) && (substr($cell, 0, 1) == "'")) {
                    $concat_cell = substr($cell, 1);
                    $concatenating = true;
                    $concat_quote = '"';
                }
                elseif (($concatenating) && (substr($cell, -1) == $concat_quote)) {
                    $concat_cell = $concat_cell . $delimiter_str . substr($cell, 0, -1);
                    $cells[] = $concat_cell;
                    $concatenating = false;
                    $concat_cell = "";
                    $concat_quote = "";
                }
                elseif ($concatenating) {
                    $concat_cell = $concat_cell . $delimiter_str . $cell;
                }
                else {
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
        if ($num_max_columns < 1) return array();

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
?>
