<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2018-2025 Daniel Teichmann
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


class import_phases extends import
{
    /** Template URI (.tpl file) */
    public const TEMPLATE_URI = "importaccounts/content_importaccounts_base.tpl";

    /*
     * Prepare PHASE 01: form presets
     */
    function prepare_phase1($smarty)
    {
        // Get default delimiter from config
        $delim_char_default = $this->utils->getConfigStrValue("csv_column_delimiter");

        // Define standard delimiters (actual characters)
        $delims_chars = array(
            0 => ',',
            1 => ';',
            2 => "\t",  // Actual tab character
            3 => ' '
        );

        // Define delimiter display texts
        $delims_texts = array(
            0 => '","'  . " - " . _("comma separated values"),
            1 => '";"'  . " - " . _("semicolon separated values"),
            2 => '"\t"' . " - " . _("tabstop separated values"),
            3 => '" "'  . " - " . _("blank separated values")
        );

        // Find which delimiter index matches the default
        $delims_default_key = -1;
        for ($i = 0; $i < 4; $i++) {
            if ($delims_chars[$i] === $delim_char_default) {
                $delims_default_key = $i;
                break;
            }
        }

        // If default delimiter is not standard, add it as custom (index 4)
        if ($delims_default_key === -1) {
            $delims_chars[4] = $delim_char_default;
            $delims_texts[4] = '"' . $delim_char_default . '" - ' . _("values separated by custom character");
            $delims_default_key = 4;
        }

        $smarty->assign('delims_default_key', $delims_default_key);
        $smarty->assign('delims_chars', $delims_chars);
        $smarty->assign('delims_texts', $delims_texts);

        // Get setting's default.
        $value = $this->utils->getConfigBoolValue("ignore_first_row_of_csv_file");
        $smarty->assign('preset_csv_with_column_headers', $value);

        // User import attributes
        $this->csvinfo['attributes_all']   = $this->getAttributes();
        $this->csvinfo['attributes_multi'] = $this->getMultiAttributes();
        $smarty->assign("attributes_all", $this->csvinfo['attributes_all']);
        $smarty->assign("attributes_multi", $this->csvinfo['attributes_multi']);
    }

    /*
     * Parse PHASE 01:
     *  - Delimiter
     *  - First CSV row is column headers
     */
    function parse_phase1($smarty)
    {
        $_delim_id                = $_POST['delimiter_id'];
        $_csv_with_column_headers = $_POST['csv_with_column_headers'];

        // Parse user settings…
        switch ($_delim_id) {
            case 0:
                // Comma separated values
                $delimiter_char = ",";
                break;
            case 1:
                // Semicolon separated values
                $delimiter_char = ";";
                break;
            case 2:
                // Tabstop separated values
                $delimiter_char = "\t";
                break;
            case 3:
                // Blank separated values
                $delimiter_char = " ";
                break;
            default:
                // Default CSV delimiter.
                $delimiter_char = $this->utils->getConfigStrValue("csv_column_delimiter");
                break;
        }

        $this->csvinfo['csv_delimiter'] = $delimiter_char;

        if (isset($_csv_with_column_headers)) {
            $this->csvinfo['skip_first_line'] = true;
        } else {
            $this->csvinfo['skip_first_line'] = false;
        }

        if (!isset($_POST['column_head_count']) or !is_numeric($_POST['column_head_count'])) {
            $this->failure_in_this_phase = true;
            $this->failure_messages[] = _("There is form data missing.");
            return;
        }

        $num_columns = intval($_POST['column_head_count']);
        if ($num_columns <= 0) {
            $this->failure_in_this_phase = true;
            $this->failure_messages[] = _("There is invalid form data.");
            return;
        }

        $new_attrs_selected = array();
        for ($i = 0; $i < $num_columns; $i++) {
            if (!isset($_POST["column_head_$i"])) {
                $this->failure_in_this_phase = true;
                $this->failure_messages[] = _("There is form data missing.");
                return;
            }

            $val = $_POST["column_head_$i"];

            // Client data sanitation
            if ($val != "---" && !array_key_exists($val, $this->csvinfo['attributes_all'])) {
                $this->failure_in_this_phase = true;
                $this->failure_messages[] = sprintf(_("The selected attribute '%s' is invalid."), bold($val));
                return;
            }

            $new_attrs_selected[] = $val;
        }

        /*
         * Maps attributes selected by the user in the web form
         * to the actual CSV columns.
         *
         * Example:
         * 0 => 'uid',       // 0'th CSV column has uid attribute
         * 1 => 'sn',        // 1'st CSV column has sn attribute
         * 2 => 'givenName', // 2'nd CSV column has givenName attribute
         * ...
         */
        $this->csvinfo['attributes_selected'] = $new_attrs_selected;
    }

    /*
     * PHASE 01:
     *   - Check uploaded file
     *   - Parse CSV file
     *   - Prepare '$this->csvinfo' with templates
     */
    function execute_phase1($smarty, $quick_import = false)
    {
        $this->parse_phase1($smarty);
        if ($this->failure_in_this_phase === true) {
            foreach ($this->failure_messages as $msg) {
                msg_dialog::display(
                    _("Error"),
                    _("Error while processing selected CSV column headers.") . "<br>" .
                    $msg . "<br>" .
                    _("Sorry, please try again."),
                    ERROR_DIALOG
                );
            }

            $this->prepare_phase1($smarty);
            return;
        }

        $ret_array = $this->utils->checkUploadedFile();

        // If successful.
        if ($ret_array[0]) {
            $raw_csv_data = $ret_array[1];
        } else {
            $this->failure_in_this_phase = true;
            $smarty->assign("LDIFError", true);

            msg_dialog::display(_("Error"), $ret_array[1], ERROR_DIALOG);
            $this->prepare_phase1($smarty);
            return;
        }

        // Parse CSV file using utils class.
        $parser_ret = $this->utils->parseCSV(
            $raw_csv_data,
            $this->csvinfo['csv_delimiter'],
            $this->csvinfo['skip_first_line']
        );

        unset($ret_array);    // Keep memory footprint low.
        unset($raw_csv_data); // Keep memory footprint low.

        if (empty($parser_ret)) {
            $smarty->assign("LDIFError", true);
            $this->failure_in_this_phase = true;

            msg_dialog::display(_("Error"), _("Cannot find CSV data in the selected file!"), ERROR_DIALOG);

            $this->prepare_phase1($smarty);
            return;
        }

        // Copy each element from $parser_ret into $this->csvinfo…
        $this->csvinfo = array_merge($this->csvinfo, $parser_ret);

        // We don't need it anymore. Keep memory footprint low.
        unset($parser_ret);

        // Transform $this->csvinfo['data'] ---> $this->csvinfo['data_sorted'] using $this->csvinfo['attributes_selected']'
        // Sort the CSV data table according to how it got re-ordered by the plugin-user
        $this->csvinfo['data_sorted'] = array();
        $attributes_selected = $this->csvinfo['attributes_selected'];
        $attributes_multi    = $this->csvinfo['attributes_multi'];

        // At this point we can be sure that
        // number of columns matches the number of selected attributes

        foreach ($this->csvinfo['data'] as $data_row) {
            $attrs_counter = array();
            $data_row_sorted = array();

            for ($i = 0; $i < count($data_row); $i++) {
                $data_in_cell = $data_row[$i];
                $value_of_selection = $attributes_selected[$i];

                // Skip every '---'
                if ($value_of_selection == "---") {
                    continue;
                }

                if (in_array($value_of_selection, $attributes_multi)) {
                    if (isset($attrs_counter[$value_of_selection])) {
                        $attrs_counter[$value_of_selection] = $attrs_counter[$value_of_selection];
                    } else {
                        $attrs_counter[$value_of_selection] = 0;
                    }
                    $data_row_sorted[$value_of_selection . $attrs_counter[$value_of_selection]] = $data_in_cell;
                    $attrs_counter[$value_of_selection]++;
                } elseif (empty($attrs_counter[$value_of_selection])) {
                    $data_row_sorted[$value_of_selection] = $data_in_cell;
                    $attrs_counter[$value_of_selection] = 1;
                } else {
                    $this->failure_in_this_phase = true;
                    $smarty->assign("LDIFError", true);
                    msg_dialog::display(
                        _("Error"),
                        sprintf(
                            _("The attribute '%s' is only allowed to be selected once!"),
                            bold($value_of_selection)
                        ),
                        ERROR_DIALOG
                    );

                    // Prepare for reloading this phase's web page again…
                    $this->prepare_phase1($smarty);
                    return;
                }
            }

            $this->csvinfo['data_sorted'][] = $data_row_sorted;
        }

        // We don't need it anymore. Keep memory footprint low.
        unset($this->csvinfo['data']);
        unset($data_row);
        unset($data_row_sorted);

        if ($this->failure_in_this_phase === true) {
            // Prepare for reloading this phase's web page again…
            $this->prepare_phase1($smarty);
            return;
        }

        $this->prepare_phase2($smarty);
    }


    /*
     * Prepare PHASE 02:
     *   - Populate template selectors
     *   - Populate OU tree selector
     *   - Set form presets
     */
    function prepare_phase2($smarty)
    {
        /* --- Populate the Template Selectors for PHASE 2 --- */
        $this->csvinfo["template_main"] = 0;
        $this->csvinfo["template_aux"]  = 0;
        $this->csvinfo['templates']     = array();

        // Add the "None" template first
        $this->csvinfo['templates']['formfields']   = array();
        $this->csvinfo['templates']['formfields'][] = "None";
        $this->csvinfo['templates']['DNs']   = array();
        $this->csvinfo['templates']['DNs'][] = "";

        // Set the default mail domain for users, groups and for the school here (they might differ)…
        $this->csvinfo['domain_school'] = $this->utils->getConfigStrValue("domain_school");
        $this->csvinfo['domain_users']  = $this->utils->getConfigStrValue("domain_users");
        $this->csvinfo['domain_groups'] = $this->utils->getConfigStrValue("domain_groups");

        // TODO: FIXME: Check the domains above for validity!

        // Search for GOsa² user templates in LDAP-tree
        $_ldapsearch = $this->_ldap->search("(objectClass=gosaUserTemplate)", array("*"));

        // Only store the *index* of gosaUserTemplate
        $i = 0;

        // Add found gosaUserTemplate objects
        while ($result = $this->_ldap->fetch($_ldapsearch)) {
            $tmp = $result['sn'][0] . " - " .
            $this->config->idepartments[preg_replace("/^[^,]+," . preg_quote(get_people_ou(), '/') . "/i", "", $result['dn'])];

            $this->csvinfo['templates']['formfields'][] = $tmp;
            $this->csvinfo['templates']['DNs'][] = $result['dn'];

            $i = $i + 1;

            if (
                ($this->default_template_main != "") and (preg_match(
                    "/" . $this->default_template_main . "/i",
                    $result['sn'][0]
                ))
            ) {
                $this->csvinfo["template_main"] = $i;

                // User mail domain

                // Obtain mail domain from pre-selected main user template, if that is available
                if (isset($result['mail']) and $this->utils->strcontains($result['mail'][0], '@')) {
                    $this->csvinfo["domain_users"] = trim(preg_split('/@/', $result['mail'][0], 2)[1]);
                }

                // Group mail domain

                // Obtain mail domain from group template object of pre-selected main user template, if available
                if (isset($result['uid'])) {
                    $_ldapsearch = "(&(objectClass=gosaMailAccount)(objectClass=posixGroup)(" .
                                        "gidNumber=" . $result['gidNumber'][0] .
                                    "))";
                    $_ldap_group_search = $this->_ldap->search($_ldapsearch);
                    while ($_template_group = $this->_ldap->fetch($_ldap_group_search)) {
                        if (
                            isset($_template_group['mail']) and
                            $this->utils->strcontains($_template_group['mail'][0], '@')
                        ) {
                            // Split mail of template and extract domain…
                            $domain_for_group = preg_split('/@/', $_template_group['mail'][0], 2)[1];
                            $this->csvinfo["domain_groups"] = trim($domain_for_group);
                        }

                        /*
                         * There should only be one template group with given 'gidNumber'
                         * if otherwise, there is something wrong (and let's ignore it here)
                         */
                        break;
                    }
                }

                // Otherwise use $this->csvinfo["domain_users"] as detected above
                elseif (isset($this->csvinfo["domain_users"])) {
                    $this->csvinfo["domain_groups"] = $this->csvinfo["domain_users"];
                }
            }
            if (
                $this->default_template_aux != "" and
                (preg_match("/" . $this->default_template_aux . "/i", $result['sn'][0]))
            ) {
                $this->csvinfo["template_aux"] = $i;
            }
        }

        $smarty->assign("templates", $this->csvinfo['templates']['formfields']);

        /*
         * Search OUs with objectClass gosaDepartment to find available OUs
         * for group placement and populate the OU tree selector
         */
        $this->csvinfo['ou_tree'] = array();
        $_ldapsearch = $this->_ldap->search("(objectClass=gosaDepartment)", array("ou", "description"));

        // Create arrays for search results
        $this->csvinfo['ou_tree']['formfields'] = array();
        $this->csvinfo['ou_tree']['OUs']        = array();

        // Add found gosaDepartment objects
        $i = 0;
        $default_ou_for_groups = 0;
        while ($result = $this->_ldap->fetch($_ldapsearch)) {
            $this->csvinfo['ou_tree']['OUs'][]        = $result['ou'][0];
            $this->csvinfo['ou_tree']['formfields'][] = $result['ou'][0] . " - " . $result['description'][0];
            $this->csvinfo['ou_tree']['DNs'][]        = $result['dn'];

            if (strcasecmp($result['ou'][0], $this->default_groups_ou) == 0) {
                $default_ou_for_groups = $i;
            }
            $i = $i + 1;
        }

        $smarty->assign("ous_available", $this->csvinfo['ou_tree']['formfields']);

        // Set import configuration defaults for next phase.
        $this->csvinfo['ou_groups']                         = $default_ou_for_groups;
        $this->csvinfo['accounts_in_class_ou'] = $this->feature_accounts_in_class_ou ? $this->utils->getConfigBoolValue("accounts_in_class_ou") : false;
        $this->csvinfo['add_course_members_to_class_group'] = $this->utils->getConfigBoolValue("add_course_members_to_class_group");
        $this->csvinfo['aliases_in_schooldomain'] = $this->feature_aliases_in_schooldomain ? $this->utils->getConfigBoolValue("aliases_in_schooldomain") : false;
        $this->csvinfo['try_mail_as_uid']                   = $this->utils->getConfigBoolValue("try_mail_as_uid");
        $this->csvinfo['sel_ldap_match_attr_studentid'] = $this->utils->getConfigBoolValue("ldap_match_attr_studentid");
        $this->csvinfo['sel_ldap_match_attr_givenname'] = $this->utils->getConfigBoolValue("ldap_match_attr_givenname");
        $this->csvinfo['sel_ldap_match_attr_snname']    = $this->utils->getConfigBoolValue("ldap_match_attr_snname");
        $this->csvinfo['sel_ldap_match_attr_birthday']  = $this->utils->getConfigBoolValue("ldap_match_attr_birthday");
        $this->csvinfo['sel_ldap_match_attr_gender']    = $this->utils->getConfigBoolValue("ldap_match_attr_gender");

        // Provide pre-set values for account template forms
        $smarty->assign("preset_template_" . $this->import_account_type, $this->csvinfo['template_main']);
        $smarty->assign("preset_template_" . $this->import_account_type . "_aux", $this->csvinfo['template_aux']);
        $smarty->assign("preset_ou_groups", $this->csvinfo['ou_groups']);
        $smarty->assign("preset_domain_school", $this->csvinfo['domain_school']);
        $smarty->assign("preset_domain_users", $this->csvinfo['domain_users']);
        $smarty->assign("preset_domain_groups", $this->csvinfo['domain_groups']);
        $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
        $smarty->assign("preset_aliases_in_schooldomain", $this->csvinfo['aliases_in_schooldomain']);
        $smarty->assign("preset_try_mail_as_uid", $this->csvinfo['try_mail_as_uid']);
        $smarty->assign("preset_add_course_members_to_class_group", $this->csvinfo['add_course_members_to_class_group']);
        $smarty->assign("preset_sel_ldap_match_attr_studentid", $this->csvinfo['sel_ldap_match_attr_studentid']);
        $smarty->assign("preset_sel_ldap_match_attr_givenname", $this->csvinfo['sel_ldap_match_attr_givenname']);
        $smarty->assign("preset_sel_ldap_match_attr_snname", $this->csvinfo['sel_ldap_match_attr_snname']);
        $smarty->assign("preset_sel_ldap_match_attr_birthday", $this->csvinfo['sel_ldap_match_attr_birthday']);
        $smarty->assign("preset_sel_ldap_match_attr_gender", $this->csvinfo['sel_ldap_match_attr_gender']);
        $smarty->assign("file_uploaded", true);
    }

    /*
     * Parse PHASE 02:
     */
    function parse_phase2($smarty)
    {
        /* Load the properties we need in this phase */
        $this->csvinfo['template_main'] = $_POST["template_" . $this->import_account_type];
        $this->csvinfo['template_aux']  = $_POST["template_" . $this->import_account_type . "_aux"] ?? null;
        $this->csvinfo['ou_groups']     = $_POST["ou_groups"];
        $this->csvinfo['domain_users']  = $_POST["domain_users"];
        $this->csvinfo['domain_groups'] = $_POST["domain_groups"];

        $this->csvinfo['accounts_in_class_ou']    = $this->feature_accounts_in_class_ou    ? isset($_POST["accounts_in_class_ou"])    : false;
        $this->csvinfo['aliases_in_schooldomain'] = $this->feature_aliases_in_schooldomain ? isset($_POST["aliases_in_schooldomain"]) : false;
        $this->csvinfo['try_mail_as_uid']                   = isset($_POST["try_mail_as_uid"]);
        $this->csvinfo['add_course_members_to_class_group'] = isset($_POST["add_course_members_to_class_group"]);
        $this->csvinfo['sel_ldap_match_attr_studentid']     = isset($_POST["sel_ldap_match_attr_studentid"]);
        $this->csvinfo['sel_ldap_match_attr_givenname']     = isset($_POST["sel_ldap_match_attr_givenname"]);
        $this->csvinfo['sel_ldap_match_attr_snname']        = isset($_POST["sel_ldap_match_attr_snname"]);
        $this->csvinfo['sel_ldap_match_attr_birthday']      = isset($_POST["sel_ldap_match_attr_birthday"]);
        $this->csvinfo['sel_ldap_match_attr_gender']        = isset($_POST["sel_ldap_match_attr_gender"]);
    }

    /*
     * PHASE 02:
     *   - Set import options which the user configured.
     *   - Checkboxes, OU's for new groups & users, etc…
     */
    function execute_phase2($smarty)
    {
        $this->parse_phase2($smarty);
        if ($this->failure_in_this_phase === true) {
            $this->prepare_phase2($smarty);
            return;
        }

        $_template      = $this->csvinfo['template_main'];
        $_template_aux  = $this->csvinfo['template_aux'];
        $_ou_groups     = $this->csvinfo['ou_groups'];
        $_domain_users  = $this->csvinfo["domain_users"];
        $_domain_groups = $this->csvinfo["domain_groups"];

        // Configure import options
        if (isset($_template) and isset($_ou_groups)) {
            $this->csvinfo['template_main'] = $_template;

            // Obtaining the OU name can be done like this:
            // $this->csvinfo['ou_tree']['OUs'][$this->csvinfo['ou_groups']]
            $this->csvinfo['ou_groups'] = $_ou_groups;

            if (isset($_domain_users)) {
                if (empty($_domain_users)) {
                    $this->csvinfo['domain_users'] = "";
                    $smarty->assign("LDIFError", true);
                    msg_dialog::display(_("Error"), _("Domain for new users is empty!"), ERROR_DIALOG);
                    $this->failure_in_this_phase = true;
                } elseif (substr($_domain_users, 0, 1) == "@") {
                    // $_domain_users begins with '@'.
                    // Get the part after '@', trim and save in $this->csvinfo['domain_users'].
                    $this->csvinfo['domain_users'] = trim(substr($_domain_users, 1, strlen($_domain_users)));
                } else {
                    $this->csvinfo['domain_users'] = trim($_domain_users);
                }
            } else {
                /*
                 * TODO: Get default config property here…
                 */
                $this->csvinfo['domain_users'] = "intern";
            }

            if (empty($_domain_groups)) {
                $this->csvinfo['domain_groups'] = "";
                $smarty->assign("LDIFError", true);
                msg_dialog::display(_("Error"), _("Domain for new groups is empty!"), ERROR_DIALOG);
                $this->failure_in_this_phase = true;
            } elseif (substr($_domain_groups, 0, 1) == "@") {
                // $_domain_groups begins with '@'.
                // Get the part after '@', trim and save in $this->csvinfo['domain_groups'].
                $this->csvinfo['domain_groups'] = trim(substr($_domain_groups, 1, strlen($_domain_groups)));
            } else {
                $this->csvinfo['domain_groups'] = trim($_domain_groups);
            }

            if (isset($_template_aux)) {
                $this->csvinfo['template_aux'] = $_template_aux;
            }
        } else {
            /* Something is not right! */
            $this->failure_in_this_phase = true;
            msg_dialog::display(
                _("Error"),
                _("Something went wrong! Something from the form went missing! Please try again."),
                ERROR_DIALOG
            );
        }

        // Transform 'data_sorted' ---> 'data_preldap' array
        // by using by sub-class overloaded prepareLdapImport()
        $this->csvinfo['data_preldap'] = $this->prepareLdapImport($this->csvinfo['data_sorted']);

        // We don't need it anymore. Keep memory footprint low.
        unset($this->csvinfo['data_sorted']);

        if ($this->failure_in_this_phase === true) {
            // Prepare for reloading this phase's web page again…
            $this->prepare_phase2($smarty);
            return;
        }

        $this->prepare_phase3($smarty);
    }


    /*
     * Prepare PHASE 03:
     */
    function prepare_phase3($smarty)
    {
        $this->accountStatusCheck();
        $smarty->assign("data", $this->csvinfo['data_preldap']);
        $smarty->assign("import_configured", true);
    }

    /*
     * Parse PHASE 03:
     */
    function parse_phase3($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 03:
     */
    function execute_phase3($smarty)
    {
        $this->parse_phase3($smarty);
        if ($this->failure_in_this_phase === true) {
            $this->prepare_phase3($smarty);
            return;
        }

        // Check if job immediately became idle (shouldn't happen normally)
        $idle_status = array(
            'is_idle' => true,
            'reason'  => '',    // If $job = null, use default idle_status.
            'error'   => ''
        );

        try {
            // Create and spawn new worker
            list($job, $job_data) = $this->prepareImportJob();

            // Spawn background worker process
            $worker_script = __DIR__ . "/class_cli_import_worker.inc";
            $pid = $job->spawnWorker($worker_script, $this->utils);

            $this->utils->debug_to_console(
                sprintf(
                    _("✅ Spawned worker process '%s' (PID: %d)"),
                    $worker_script,
                    $pid
                )
            );

            // Store job ID in session for monitoring
            $this->csvinfo['async_job_id'] = $job->getJobId();

            // Check if job is idle (completed or died)
            $timeout_seconds = $this->utils->getConfigIntValue('async_import_timeout', 10);
            $idle_status = $job->isIdle($timeout_seconds);
        } catch (Exception $e) {
            $this->utils->debug_to_console(
                sprintf(_("❌ Async import failed: %s"), $e->getMessage())
            );

            // Set idle_status to indicate exception/failure
            $idle_status = array(
                'is_idle' => true,
                'reason'  => 'exception',
                'error'   => sprintf(_("Failed to start async import: %s"), $e->getMessage())
            );
        }

        if ($idle_status['is_idle']) {
            $error_message = null;
            $failure_message = null;

            switch ($idle_status['reason']) {
                case 'exception':
                    // Exception occurred during job creation/spawning
                    $error_message = $idle_status['error'];
                    $failure_message = $idle_status['error'];
                    break;
                case 'failed':
                    // Worker failed immediately
                    $error_message = sprintf(
                        _("Import worker failed to start: %s"),
                        $idle_status['error']
                    );
                    $failure_message = $idle_status['error'];
                    break;
                case 'timeout':
                    // Worker timed out (suspicious if just spawned)
                    $error_message = sprintf(
                        _("Import worker did not start properly: %s"),
                        $idle_status['error']
                    );
                    $failure_message = $idle_status['error'];
                    break;
                case 'no_job_file':
                    // Job file disappeared - serious error
                    $error_message = _("Job file not found after spawning worker");
                    $failure_message = _("Job file not found");
                    break;
                case 'completed':
                    // Worker finished very quickly (unlikely but OK)
                    break;
                default:
                    // Generic error for unknown status
                    $error_message = sprintf(
                        _("Unexpected import worker status: %s"),
                        $idle_status['reason']
                    );
                    $failure_message = $idle_status['reason'];
                    break;
            }

            // Display error message if an error occurred
            if ($error_message !== null) {
                $this->failure_messages[] = $failure_message;
                $this->failure_in_this_phase = true;
            }
        }

        // Get refresh time for page auto-reload (seconds)
        $refresh_time = $this->utils->getConfigIntValue('async_import_refresh_time', 5);

        // Assign job information to template
        $smarty->assign("data", $this->csvinfo['data_preldap']);
        $smarty->assign("job_id", $this->csvinfo['async_job_id']);
        $smarty->assign("import_job_refresh_time", $refresh_time);

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("accounts_reviewed", true);
        } else {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            // Prepare for reloading this phase's web page again…
            $this->prepare_phase3($smarty);
            return;
        }

        $this->prepare_phase4($smarty);
    }


    /*
     * Prepare PHASE 04:
     */
    function prepare_phase4($smarty)
    {
        // TODO: prepare here…
    }

    /*
     * Parse PHASE 04:
     */
    function parse_phase4($smarty)
    {
        if (isset($_POST['ajax_status_check'])) {
            $this->csvinfo['ajax_status_check'] = true;
        } else {
            $this->csvinfo['ajax_status_check'] = false;
        }
    }

    /*
     * PHASE 04:
     */
    function execute_phase4($smarty)
    {
        $this->parse_phase4($smarty);
        if ($this->failure_in_this_phase === true) {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            $this->prepare_phase4($smarty);
            return;
        }

        // AJAX request detection - return JSON status and exit
        if ($this->csvinfo['ajax_status_check'] === true) {
            $status = $this->getCurrentJobStatusJSON();

            header('Content-Type: application/json; charset=utf-8');
            echo json_encode($status);
            exit;
        }

        if ($this->failure_in_this_phase === true) {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            $this->prepare_phase4($smarty);
            return;
        }

        $this->prepare_phase5($smarty);
    }


    /*
     * Prepare PHASE 05:
     */
    function prepare_phase5($smarty)
    {
        // Get refresh time for AJAX polling (seconds)
        $refresh_time = $this->utils->getConfigIntValue('async_import_refresh_time', 5);

        // Assign job information to template for initial page load
        // JavaScript StatusMonitor will handle all status checking via AJAX
        $smarty->assign("job_id", $this->csvinfo['async_job_id'] ?? '');
        $smarty->assign("import_job_refresh_time", $refresh_time);

        $smarty->assign("accounts_imported", true);

        foreach ($this->csvinfo['data_preldap'] as $idx => $user_data) {
            if ((isset($user_data['main_account']) and (!empty($user_data['main_account'])))) {
                $_uid = $user_data['main_account']['uid'][0];

                $_all_groups = array();
                if (isset($user_data['groups'])) {
                    $_all_groups = array_merge($_all_groups, $user_data['groups']);
                }
                if (isset($user_data['aux_accounts_groups'])) {
                    $_all_groups = array_merge($_all_groups, $user_data['aux_accounts_groups']);
                }

                foreach ($_all_groups as $group_key => $group_data) {
                    if (isset($group_data['_key'][0])) {
                        $_group_type_key = $group_data['_key'][0];
                    } else {
                        $_group_type_key = 'groups';
                    }

                    // Find+Replace {%uid} in group CN with the user's UID string
                    $_group_name = $group_data['cn'][0];
                    if (preg_match('/\{%uid\}/', $_group_name)) {
                        $this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key]['cn'][0] = str_replace('{%uid}', $_uid, $_group_name);
                    }
                    if (preg_match('/\{%uid\}/', $group_key)) {
                        $this->csvinfo['data_preldap'][$idx][$_group_type_key][str_replace('{%uid}', $_uid, $group_key)] = $this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key];
                        unset($this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key]);
                    }
                }
            }
        }

        $this->groupStatusCheck();

        $smarty->assign("data_groups", $this->csvinfo['data_preldap_posixgroups']);
        $smarty->assign("data_ogroups", $this->csvinfo['data_preldap_ogroups']);
    }

    /*
     * Parse PHASE 05:
     */
    function parse_phase5($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 05:
     */
    function execute_phase5($smarty)
    {
        $this->parse_phase5($smarty);
        if ($this->failure_in_this_phase === true) {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            $this->prepare_phase5($smarty);
            return;
        }

        // Import posixgroups objects into LDAP-tree.
        foreach ($this->csvinfo['data_preldap_posixgroups'] as $group_name => $group_data) {
            // Only perform on this group if _actions is not "none" or "ignore"
            if (
                $this->utils->strcontains($group_data['_actions'][0], 'none')   === false or
                $this->utils->strcontains($group_data['_actions'][0], 'ignore') === false
            ) {
                $this->importLDAPGroupObject($group_name, $group_data);
            }
        }

        // Import ogroups objects into LDAP-tree.
        foreach ($this->csvinfo['data_preldap_ogroups'] as $group_name => $group_data) {
            // Only perform on this group if _actions is not "none" or "ignore"
            if (
                $this->utils->strcontains($group_data['_actions'][0], 'none')   === false or
                $this->utils->strcontains($group_data['_actions'][0], 'ignore') === false
            ) {
                $this->importLDAPGroupObject($group_name, $group_data);
            }
        }

        $this->groupStatusCheck();

        $smarty->assign("data_groups", $this->csvinfo['data_preldap_posixgroups']);
        $smarty->assign("data_ogroups", $this->csvinfo['data_preldap_ogroups']);

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("groups_reviewed", true);
        } else {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            // Prepare for reloading this phase's web page again…
            $this->prepare_phase5($smarty);
            return;
        }

        $this->prepare_phase6($smarty);
    }


    /*
     * Prepare PHASE 06:
     */
    function prepare_phase6($smarty)
    {
        $this->groupStatusCheck();
        $smarty->assign("data_groups", $this->csvinfo['data_preldap_posixgroups']);
        $smarty->assign("data_ogroups", $this->csvinfo['data_preldap_ogroups']);
    }

    /*
     * Parse PHASE 06:
     */
    function parse_phase6($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 06:
     */
    function execute_phase6($smarty)
    {
        $this->parse_phase6($smarty);
        if ($this->failure_in_this_phase === true) {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            $this->prepare_phase6($smarty);
            return;
        }

        $this->accountStatusCheck();
        $this->groupStatusCheck();
        $this->prepareGroupMemberships();
        $smarty->assign("data", $this->csvinfo['data_preldap']);

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("groups_imported", true);
        } else {
            $this->prepare_phase6($smarty);
            return;
        }

        $this->prepare_phase7($smarty);
    }


    /*
     * Prepare PHASE 07:
     */
    function prepare_phase7($smarty)
    {
        $this->prepareGroupMemberships();
    }

    /*
     * Parse PHASE 07:
     */
    function parse_phase7($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 07:
     */
    function execute_phase7($smarty)
    {
        $this->parse_phase7($smarty);
        if ($this->failure_in_this_phase === true) {
            $this->prepare_phase7($smarty);
            return;
        }

        $this->addLDAPUserObjectsToGroups($this->posixgroup_members);
        $this->addLDAPUserObjectsToOGroups($this->groupofnames_members);
        $this->moveLDAPPrimGroupObjects();

        /*
         * FIXME: present a message dialog box that informs on the statistics of
         *        the LDAP group membership updates.
         */

        $this->accountStatusCheck();
        $this->groupStatusCheck();
        $smarty->assign("data", $this->csvinfo['data_preldap']);

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("accounts_groupmembers_reviewed", true);
        } else {
            $this->prepare_phase7($smarty);
            return;
        }

        $this->prepare_phase8($smarty);
    }


    /*
     * Prepare PHASE 08:
     */
    function prepare_phase8($smarty)
    {
        // TODO: prepare here…
    }

    /*
     * Parse PHASE 08:
     */
    function parse_phase8($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 08:
     */
    function execute_phase8($smarty)
    {
        $this->parse_phase8($smarty);
        if ($this->failure_in_this_phase === true) {
            $this->prepare_phase8($smarty);
            return;
        }

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("accounts_groupmembers_updated", true);
        } else {
            // Prepare for reloading this phase's web page again…
            $this->prepare_phase8($smarty);
            return;
        }

        $this->prepare_phase9($smarty);
    }


    /*
     * Prepare PHASE 09:
     */
    function prepare_phase9($smarty)
    {
        // TODO: prepare here…
    }

    /*
     * Parse PHASE 09:
     */
    function parse_phase9($smarty)
    {
        // TODO: parse here…
    }

    /*
     * PHASE 09:
     */
    function execute_phase9($smarty)
    {
        $this->parse_phase9($smarty);
        if ($this->failure_in_this_phase === true) {
            $this->prepare_phase9($smarty);
            return;
        }

        if ($this->failure_in_this_phase === false) {
            /*
             * FIXME: If automatic password generation is used, present a download link
             *        for a generated credentials sheet of paper.
             */

            /* Render the final SUCCESS page */
            $smarty->assign("cleanup_completed", true);
        } else {
            /* FIXME: no clue what to do on failures here… We will see. */
            $this->prepare_phase9($smarty);
            return;
        }

        // $this->prepare_phaseX($smarty);
    }


    function execute()
    {
        /* Call parent execute */
        plugin::execute();

        $this->import_account_type = substr(get_class($this), 6, strlen(get_class($this)));

        // Log current view
        if (!$this->view_logged) {
            $this->view_logged = true;
            new log("view", "all/" . get_class($this), $this->dn);
        }

        /* initiate smarty */
        $smarty = get_smarty();

        /* Import (students and parents), (students only) or (teachers only) */
        $smarty->assign("import_account_type", $this->import_account_type);


        /* PHASE 01 done, set $file_uploaded to TRUE */
        $smarty->assign("file_uploaded", 0);

        /* PHASE 02 done, set $import_configured to TRUE  */
        $smarty->assign("import_configured", 0);

        /* PHASE 03 done, set $accounts_reviewed to TRUE  */
        $smarty->assign("accounts_reviewed", 0);

        /* PHASE 04 done, set $accounts_imported to TRUE  */
        $smarty->assign("accounts_imported", 0);

        /* PHASE 05 done, set $groups_reviewed to TRUE  */
        $smarty->assign("groups_reviewed", 0);

        /* PHASE 06 done, set $groups_imported to TRUE  */
        $smarty->assign("groups_imported", 0);

        /* PHASE 07 done, set $accounts_groupmembers_reviewed to TRUE */
        $smarty->assign("accounts_groupmembers_reviewed", 0);

        /* PHASE 08 done, set $accounts_groupmembers_updated to TRUE */
        $smarty->assign("accounts_groupmembers_updated", 0);

        /* PHASE 09 done, set $cleanup_completed to TRUE */
        $smarty->assign("cleanup_completed", 0);

        /* Get the LDAP link, to generate the Export */
        $this->_ldap = $this->config->get_ldap_link();

        /* initialize CSV Info array */
        if (!is_array($this->csvinfo)) {
            $this->csvinfo = array();
        }

        /* Check permissions for import */
        $acl = $this->ui->get_permissions($this->config->current['BASE'], "all/all");
        if (!preg_match("/w/", $acl)) {
            if (
                isset($_POST['file_uploaded']) or
                isset($_POST['import_configured']) or
                isset($_POST['accounts_reviewed']) or
                isset($_POST['accounts_imported']) or
                isset($_POST['groups_reviewed']) or
                isset($_POST['groups_imported']) or
                isset($_POST['accounts_groupmembers_reviewed']) or
                isset($_POST['accounts_groupmembers_updated']) or
                isset($_POST['cleanup_completed'])
            ) {
                msg_dialog::display(_("Permission error"), _("You've no permission to import user data!"), ERROR_DIALOG);
            }
            return ($smarty->fetch(get_template_path(self::TEMPLATE_URI, true)));
        }

        // Give the base template the directory path where all phases are located.
        $_phases_path = $smarty->getTemplateDir()[0];
        $_phases_path .= get_template_path("importaccounts/", true);
        $smarty->assign("phases_path", $_phases_path);

        /*
         * PHASES
         */

        // Reset the failure status from last template view…
        $this->failure_in_this_phase = false;

        // Execute next phase, if user didn't abort.
        if (!isset($_POST['cancel_import'])) {
            if (isset($_POST['phase_01']) and isset($_POST['file_uploaded'])) {
                $this->execute_phase1($smarty);
            } elseif (isset($_POST['phase_02']) and isset($_POST['import_configured'])) {
                $smarty->assign("file_uploaded", true);
                $this->execute_phase2($smarty);
            } elseif (isset($_POST['phase_03']) and isset($_POST['accounts_reviewed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $this->execute_phase3($smarty);
            } elseif (isset($_POST['phase_04']) and isset($_POST['accounts_imported'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $this->execute_phase4($smarty);
            } elseif (isset($_POST['phase_04']) and isset($_POST["ajax_status_check"])) {
                $this->execute_phase4($smarty);
                // Will never return here, as execute_phase4() exits on AJAX requests.
            } elseif (isset($_POST['phase_05']) and isset($_POST['groups_reviewed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $this->execute_phase5($smarty);
            } elseif (isset($_POST['phase_06']) and isset($_POST['groups_imported'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $this->execute_phase6($smarty);
            } elseif (isset($_POST['phase_07']) and isset($_POST['accounts_groupmembers_reviewed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $this->execute_phase7($smarty);
            } elseif (isset($_POST['phase_08']) and isset($_POST['accounts_groupmembers_updated'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $smarty->assign("accounts_groupmembers_reviewed", true);
                $this->execute_phase8($smarty);
            } elseif (isset($_POST['phase_9']) and isset($_POST['cleanup_completed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $smarty->assign("accounts_groupmembers_reviewed", true);
                $smarty->assign("accounts_groupmembers_updated", true);
                $this->execute_phase9($smarty);
            } else {
                // Initial phase, so Phase 0…
                // Delete old references…
                $this->csvinfo = array();

                // Prepare smarty variables and such for main page (Phase 1)
                $this->prepare_phase1($smarty);
            }
        } else {
            // User aborted import, return to phase 1.
            msg_dialog::display(_("Import cancelled"), _("The import process has been cancelled by the user."), INFO_DIALOG);
        }

        // Show main page…
        return ($smarty->fetch(get_template_path(self::TEMPLATE_URI, true)));
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
