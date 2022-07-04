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


class archiveaccounts extends plugin
{
    /** Template URI (.tpl file) */
    public const TEMPLATE_URI = "../templates/archiveaccounts/content_archiveaccounts_base.tpl";

    /* GOsa² specifics */
    public $plHeadline = "Archive Accounts";
    public $plDescription = "GOsa2 School Manager Module: Archive Accounts";
    public $access = "";

    /**
     * @var Array String array with csv informations.
     *            Contains all user CSV data too.
     */
    private $csvinfo = [];

    /** @var string Archive type. Should be either 'students' or 'teachers' */
    private $archive_type = "";
    private $view_logged  = false;

    /** @var Array Raw unsorted account array structures. */
    private $accounts_raw = [];

    /** @var Array Sorted account array structures. */
    private $accounts_sorted = [];

    /** @var Array Array of prepped Account objects. */
    private $accounts_prepped = [];


    public function __construct(&$config, $dn = null)
    {
        $this->initTime = microtime(true);

        /* Include config object */
        $this->config = &$config;

        /* Initialize utils class */
        $this->utils = new schoolmgr_utils($config);

        $this->ui = get_userinfo();
        stats::log(
            'plugin',
            $class = get_class($this),
            $category = [$this-> acl_category],
            $action = 'open',
            $amount = 1,
            $duration = (microtime(true) - $this->initTime),
        );
    }


    private function getAttributes()
    {
        $attrs = [];

        $attrs[] = "no";
        $attrs[] = "uid";
        $attrs[] = "userPassword";

        /* Only Students have a Student ID */
        if ($this->archive_type === "students") {
            $attrs[] = "employeeNumber";
        }

        $attrs[] = "sn";
        $attrs[] = "givenName";
        $attrs[] = "dateOfBirth";
        $attrs[] = "gender";

        return $attrs;
    }


    public function execute()
    {
        /* Call parent execute */
        plugin::execute();

        /* Log view */
        if (!$this->view_logged) {
            $this->view_logged = true;
            new log("view", "all/" . get_class($this), $this->dn);
        }

        /* Initiate smarty */
        $smarty = get_smarty();

        /* Get the LDAP link, to generate the Export */
        $this->_ldap = $this->config->get_ldap_link();

        /* Initialize CSV Info array */
        if (!is_array($this->csvinfo)) {
            $this->csvinfo = [];
        }

        /* Setup default states for phases */
        /* If PHASE is done, set variable to TRUE */
        $smarty->assign("chosen_archivetype", false); // PHASE 0
        $smarty->assign("file_uploaded", false);      // PHASE 1
        $smarty->assign("archive_configured", false); // PHASE 2
        $smarty->assign("data_sorted", false);        // PHASE 3
        $smarty->assign("accounts_reviewed", false);  // PHASE 4
        $smarty->assign("accounts_archived", false);  // PHASE 5
        $smarty->assign("primgroups_moved", false);   // PHASE 6

        /* Reset our failure status from last template view… */
        $this->failure_in_this_phase = false;

        /* Archive types */
        $smarty->assign(
            'available_archive_types',
            [0 => _("Students"), 1 => _("Teachers")]
        );
        $smarty->assign('preset_archive_type_id', 0);

        $this->csvinfo['attrs'] = $this->getAttributes();
        $this->csvinfo['attrs'][] = "---";

        // Give the base template the directory path where all phases are located.
        $_phases_path = $smarty->getTemplateDir()[0];
        $_phases_path .= get_template_path("../templates/archiveaccounts/", true);
        $smarty->assign("phases_path", $_phases_path);

        /*
        * PHASES
        */
        $result = -1;
        if (isset($_POST['phase_00']) && isset($_POST['chosen_archivetype']) && (!isset($_POST['cancel_archiving']))) {
            /*
             * PHASE 00
             * Choose the archive type. (students, teachers..)
             */
            $result = $this->execute_phase_0($smarty);
        } elseif (isset($_POST['phase_01']) && isset($_POST['file_uploaded']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 01
            * Upload CSV File.
            */
            $result = $this->execute_phase_1($smarty);
        } elseif (isset($_POST['phase_02']) && isset($_POST['archive_configured']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 02
            * Configure uploaded CSV File.
            */
            $result = $this->execute_phase_2($smarty);
        } elseif (isset($_POST['phase_03']) && isset($_POST['data_sorted']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 03
            * Configure CSV to LDAP Attributes.
            */
            $result = $this->execute_phase_3($smarty);
        } elseif (isset($_POST['phase_04']) && isset($_POST['accounts_reviewed']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 04
            * Review and confirm the User objects that will be archived.
            */
            $result = $this->execute_phase_4($smarty);
        } elseif (isset($_POST['phase_05']) && isset($_POST['accounts_archived']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 05
            * Confirm the archived User objects.
            */
            $result = $this->execute_phase_5($smarty);
        } elseif (isset($_POST['phase_06']) && isset($_POST['primgroups_moved']) && (!isset($_POST['cancel_archiving']))) {
            /*
            * PHASE 06
            * Moving the primgroups
            */
            $result = $this->execute_phase_6($smarty);
        }

        /*
        * $result is the return value of the phase function.
        * So if somethings fails, we can pass the error code.
        * If successfull then $result will be TRUE if not then its an integer higher than 0.
        * It can be usefull for later applications.
        * The value '1' means that an error msg was shown
        */
        if ($result < 0) {
            /* No Phase was executed */
        } elseif ($result === 1) {
            /* Error message has already been shown */
        } elseif ($result !== true || $this->failure_in_this_phase === true) {
            msg_dialog::display(
                _("Error"),
                _("An unknown error occurred! Error code: $result"),
                ERROR_DIALOG
            );
        }
        $this->failure_in_this_phase = false;

        /* Show main page */
        return $smarty->fetch(get_template_path(self::TEMPLATE_URI, true));
    }


    private function execute_phase_0($smarty)
    {
        $returnval = true;

        if (isset($_POST['archivetype_id'])) {
            $archive_type = $_POST['archivetype_id'];

            switch ($archive_type) {
                case 0:
                    $archive_type = "students";
                    break;
                case 1:
                    $archive_type = "teachers";
                    break;
                default:
                       $this->failure_in_this_phase = true;
                    msg_dialog::display(
                        _("Error"),
                        _(
                            "Something went wrong. " .
                            "The archive type is not valid. Please try again."
                        ),
                        ERROR_DIALOG
                    );

                    return 1; // See parent execute() func. for error code meaning.
            }
        } else {
            $this->failure_in_this_phase = true;
            msg_dialog::display(
                _("Error"),
                _("Archive type did get lost! Please try again!"),
                ERROR_DIALOG
            );

            return 1; // See parent execute() func. for error code meaning.
        }

        $smarty->assign("chosen_archivetype", true);
        $this->archive_type = $archive_type;
        $smarty->assign("archive_type", $this->archive_type);

        // Get setting's default.
        $delim_char = $this->utils->getConfigStrValue("csv_column_delimiter");

        switch ($delim_char) {
            case ",":
                $delimiter_index = 0;
                break;
            case ";":
                $delimiter_index = 1;
                break;
            case "\\t": // Not the actual tab char but literally '\t'
                $delimiter_index = 2;
                break;
            case " ":
                $delimiter_index = 3;
                break;
            default:
                // Custom character…
                $delimiter_index = 4;
                break;
        }

        $smarty->assign('preset_delimiter_id', $delimiter_index);
        $available_delims = [
            0 => '","'  . " - " . _("comma separated values"),
            1 => '";"'  . " - " . _("semicolon separated values"),
            2 => '"\t"' . " - " . _("tabstop separated values"),
            3 => '" "'  . " - " . _("blank separated values"),
        ];

        // Add custom character radio button only if it should appear.
        if ($delimiter_index === 4) {
            $available_delims[4] = '"' . $delim_char . '" - ' . _("values seperated by custom character");
        }

        $smarty->assign('available_delimiters', $available_delims);

        // Get setting's default.
        $value = $this->utils->getConfigBoolValue("ignore_first_row_of_csv_file");
        $smarty->assign('preset_csv_with_column_headers', $value);

        return 1; // See parent execute() func. for error code meaning.
    }


    private function execute_phase_1($smarty)
    {
        $smarty->assign("chosen_archivetype", true); // PHASE 0

        $returnval = true;

        $smarty->assign("archive_type", $this->archive_type);

        // Only set default options if phase_1 was successful
        if ($this->archive_type === "students") {
            $smarty->assign("preset_sel_ldap_match_attr_studentid", false);
        }
        $smarty->assign("preset_sel_ldap_match_attr_name", true);
        $smarty->assign("preset_sel_ldap_match_attr_snname", true);
        $smarty->assign("preset_sel_ldap_match_attr_birthday", false);
        $smarty->assign("preset_sel_ldap_match_attr_gender", true);

        $ret_array = $this->utils->checkUploadedFile();

        // If successful.
        if ($ret_array[0]) {
            $raw_csv_data = $ret_array[1];
            $smarty->assign("file_uploaded", true);
        } else {
            $this->failure_in_this_phase = true;
            $smarty->assign("LDIFError", true);

            $smarty->assign('available_delimiters', [
                0 => '","  - ' . _("comma separated values"),
                1 => '";"  - ' . _("semicolon separated values"),
                2 => '"\t" - ' . _("tabstop separated values"),
                3 => '" "  - ' . _("blank separated values")
            ]);
            $smarty->assign('preset_delimiter_id', 0);

            msg_dialog::display(_("Error"), $ret_array[1], ERROR_DIALOG);
            return 1; /* Error msg was already shown -> error code 1 */
        }

        // Parse user settings…
        switch ($_POST['delimiter_id']) {
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

        if (isset($_POST['csv_with_column_headers'])) {
            $this->csvinfo['skip_first_line'] = true;
        } else {
            $this->csvinfo['skip_first_line'] = false;
        }

        // Parse CSV file using utils class.
        $parser_ret = $this->utils->parseCSV(
            $raw_csv_data,
            $delimiter_char,
            $this->csvinfo['skip_first_line']
        );

        if (empty($parser_ret)) {
            $smarty->assign("LDIFError", true);
            $smarty->assign("file_uploaded", false);
            $this->failure_in_this_phase = true;

            msg_dialog::display(_("Error"), _("Cannot find CSV data in the selected file!"), ERROR_DIALOG);

            return 1; /* Error msg was already shown -> error code 1 */
        } else {
            // Copy each element from $parser_ret into $this->csvinfo…
            $this->csvinfo = array_merge($this->csvinfo, $parser_ret);

            // We don't need it anymore. Keep memory footprint low.
            unset($parser_ret);
        }

        // Search OUs with objectClass gosaDepartment
        $this->csvinfo['ou_tree'] = [];
        $this->csvinfo['ou_tree']['ldapsearch'] = $this->_ldap->search(
            "(objectClass=gosaDepartment)",
            ["ou", "description"]
        );

        // Create arrays for search results
        $this->csvinfo['ou_tree']['formfields'] = [];
        $this->csvinfo['ou_tree']['OUs']        = [];

        $i = 0;
        $default_ou_users = 0;
        $default_ou_matching_users = 0;
        // Scan LDAP tree and set default OU's while we're at it
        while ($result = $this->_ldap->fetch($this->csvinfo['ou_tree']['ldapsearch'])) {
            $this->csvinfo['ou_tree']['OUs'][]        = $result['ou'][0];
            $this->csvinfo['ou_tree']['formfields'][] = $result['ou'][0] . " - " . $result['description'][0];
            $this->csvinfo['ou_tree']['DNs'][]        = $result['dn'];

            // Use schoolmanager OU if it exists.
            if (strcasecmp($result['ou'][0], "schoolmanager") == 0) {
                $default_ou_users = $i;
            } elseif (strcasecmp($result['ou'][0], "students") == 0) {
                $default_ou_matching_users = $i;
            }

            $i++;
        }

        $this->csvinfo['ou_users']          = $default_ou_users;
        $this->csvinfo['ou_matching_users'] = $default_ou_matching_users;
        $smarty->assign("preset_ou_users", $this->csvinfo['ou_users']);
        $smarty->assign("preset_ou_matching_users", $this->csvinfo['ou_matching_users']);
        $smarty->assign("ous_available", $this->csvinfo['ou_tree']['formfields']);

        return $returnval; // See parent execute() func. for error code meaning.
    }


    private function execute_phase_2($smarty)
    {
        $smarty->assign("file_uploaded", true);      // PHASE 1
        $smarty->assign("chosen_archivetype", true); // PHASE 0

        $returnval = true;

        $this->csvinfo['sel_ldap_match_attr_studentid'] = isset($_POST["sel_ldap_match_attr_studentid"]);
        $this->csvinfo['sel_ldap_match_attr_name']      = isset($_POST["sel_ldap_match_attr_name"]);
        $this->csvinfo['sel_ldap_match_attr_snname']    = isset($_POST["sel_ldap_match_attr_snname"]);
        $this->csvinfo['sel_ldap_match_attr_birthday']  = isset($_POST["sel_ldap_match_attr_birthday"]);
        $this->csvinfo['sel_ldap_match_attr_gender']    = isset($_POST["sel_ldap_match_attr_gender"]);

        if (isset($_POST['ou_users'])) {
            $this->csvinfo['ou_users']          = $_POST['ou_users'];
        }
        if (isset($_POST['ou_matching_users'])) {
            $this->csvinfo['ou_matching_users'] = $_POST['ou_matching_users'];
        }

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("archive_configured", true); // PHASE 2

            // Initialize $this->csvinfo['attrs_selected'], only do this here and once
            $this->csvinfo['attrs_selected'] = $this->getAttrsPreSelection($this->csvinfo['num_cols']);

            // Student import attributes
            $smarty->assign("attrs", $this->csvinfo['attrs']);

            // Per row selected student import attributes
            $smarty->assign("attrs_selected", $this->csvinfo['attrs_selected']);

            // Number of CSV columns -> number of rows in 90°-counter-clockwise-rotated table
            $smarty->assign("num_rows", $this->csvinfo['num_cols']);

            // CSV data
            $smarty->assign("data_size", $this->csvinfo['num_rows']);
            $smarty->assign("data", array_slice($this->accounts_raw, 0, 5));
        }

        return $returnval; // See parent execute() func. for error code meaning.
    }


    private function execute_phase_3($smarty)
    {
        $smarty->assign("archive_configured", true); // PHASE 2
        $smarty->assign("file_uploaded", true);      // PHASE 1
        $smarty->assign("chosen_archivetype", true); // PHASE 0

        $returnval = true;

        /* Sanity checks on LDAP attributes assignments */

        // Read attributes assignments from $_POST
        $new_attrs_selected = [];
        for ($i = 0; $i < count($this->csvinfo['attrs_selected']); $i++) {
            if (isset($_POST["column_head_$i"])) {
                $new_attrs_selected[] = $_POST["column_head_$i"];
            } else {
                $new_attrs_selected[] = $i;
            }
        }
        $this->csvinfo['attrs_selected'] = $new_attrs_selected;

        /* Sort the CSV date table according to how it got re-ordered by the webUI admin user */
        $this->accounts_sorted = [];
        $multi_attrs = $this->getMultiAttributes();
        foreach ($this->accounts_raw as $data_row) {
            $attrs_counter   = [];
            $data_row_sorted = [];

            for ($i = 0; $i < count($data_row); $i++) {
                $data_in_cell = $data_row[$i];
                $selection_in_cell = $this->csvinfo['attrs_selected'][$i];
                $value_of_selection = $this->csvinfo['attrs'][$selection_in_cell];
                if ($value_of_selection == "---") {
                } elseif (in_array($value_of_selection, $multi_attrs)) {
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
                        sprintf(_("The attribute %s is only allowed to select once!"), bold($value_of_selection)),
                        ERROR_DIALOG
                    );
                    $returnval = 1; // See parent execute() func. for error code meaning.
                }
            }

            $this->accounts_sorted[] = $data_row_sorted;
        }

        // Transform data_sorted_accounts to data_prepped_accounts array
        $this->accounts_prepped = $this->prepareLdapImport($this->accounts_sorted);

        // $this->failure_in_this_phase may have been set above or in $this->prepareLdapImport…
        if ($this->failure_in_this_phase !== false) {
            /* Prepare smarty for reloading this phase's web page again */
            $smarty->assign("attrs", $this->csvinfo['attrs']);
            $smarty->assign("attrs_selected", $this->csvinfo['attrs_selected']);
            $smarty->assign("num_rows", $this->csvinfo['num_cols']);
            $smarty->assign("data_size", $this->csvinfo['num_rows']);
            $smarty->assign("data", array_slice($this->accounts_raw, 0, 5));
            msg_dialog::display("Error", "Something went terribly wrong. You shouldn't see this message.", INFO_DIALOG);
        }

        // Free some memory…
        $this->accounts_raw    = [];
        $this->accounts_sorted = [];

        $retval = $this->accountStatusCheck();
        if ($this->failure_in_this_phase === true) {
            if ($retval !== 1) {
                msg_dialog::display(
                    _("Error"),
                    sprintf(_("accountStatusCheck() failed and returned an unknown Error: %s"), $retval),
                    ERROR_DIALOG
                );
            }
            return $returnval = 1;  // See parent execute() func. for error code meaning.
        }

        // Assign data for webpage
        $smarty->assign("data_sorted", true);
        $smarty->assign("data", $this->accounts_prepped);

        return $returnval;
    }


    private function execute_phase_4($smarty)
    {
        $smarty->assign("data_sorted", true);        // PHASE 3
        $smarty->assign("archive_configured", true); // PHASE 2
        $smarty->assign("file_uploaded", true);      // PHASE 1
        $smarty->assign("chosen_archivetype", true); // PHASE 0

        $returnval = true;

        foreach ($this->accounts_prepped as $idx => $user_data) {
            $return = false;
            if (isset($user_data['main_account']) && !empty($user_data['main_account'])) {
                $return = $this->archiveLDAPUserObject($user_data['main_account']);
            }

            if ($return === true) {
                // *Lock* the User account.
                $this->utils->lockUser($user_data['main_account']['_dn'][0]);

                /* Setting _status of user to 'archived,acc-locked' */
                $this->accounts_prepped[$idx]['main_account']['_status'][0] = "archived,acc-locked";
                $this->accounts_prepped[$idx]['main_account']['_actions'][0] = "none";

                $expected_primgroup_dn = "cn=" . $this->accounts_prepped[$idx]['main_account']['uid'][0] . "," .
                get_groups_ou() . dn2base($this->accounts_prepped[$idx]['main_account']['_dn_ldap'][0]);

                $this->_ldap->cat($expected_primgroup_dn, ['dn']);

                $primgroup = $this->_ldap->fetch();
                if (!$primgroup) {
                        $this->_ldap->search(
                            "(&(objectClass=posixGroup)(cn=" . $this->accounts_prepped[$idx]['main_account']['uid'][0] . "))",
                            ["dn"]
                        );

                          $primgroup = $this->_ldap->fetch();
                    if ($primgroup) {
                            $this->accounts_prepped[$idx]['main_account']['_group_actions'] = ['move-primgroup'];
                    }
                } else {
                    $this->accounts_prepped[$idx]['main_account']['_group_actions'] = ['move-primgroup'];
                }

                $this->accounts_prepped[$idx]['main_account']['_ogroup_actions'] = ['none'];

                $new_primgroup_base = $user_data['main_account']['_base'][0];
                $this->utils->checkOUExists($new_primgroup_base, ["description" => _("Archived accounts")]);
                // TODO: Check return code.

                $new_primgroup_dn = "cn=archived-" . date("Y-m-d") . "-" .
                                $user_data['main_account']['uid'][0] . ',' .
                                get_groups_ou() . $new_primgroup_base;

                $this->accounts_prepped[$idx]['main_account']['_primgroup_dn'][0] = $new_primgroup_dn;
                $this->accounts_prepped[$idx]['main_account']['_primgroup_dn_ldap'][0] = var_export($primgroup['dn'], true);
            } elseif ($return !== 1) {
                // return val 1 -> error msg. was already shown.
                $failed_accs[] = sprintf(
                    _("'%s' '%s' with error code: '%s'"),
                    $user_data['main_account']['givenName'][0],
                    $user_data['main_account']['sn'][0],
                    $return
                );
            }
        }

        if (count($failed_accs) >= 1) {
            msg_dialog::display(
                _("Error"),
                sprintf(
                    _("The following users could not be archived: %s"),
                    var_export($failed_accs, true)
                ),
                ERROR_DIALOG
            );
        }

        $smarty->assign("data", $this->accounts_prepped);
        $smarty->assign("accounts_reviewed", true); // PHASE 4

        return $returnval;
    }


    private function execute_phase_5($smarty)
    {
        $smarty->assign("accounts_reviewed", true);    // PHASE 4
        $smarty->assign("data_sorted", true); // PHASE 3
        $smarty->assign("archive_configured", true);   // PHASE 2
        $smarty->assign("file_uploaded", true);        // PHASE 1
        $smarty->assign("chosen_archivetype", true);   // PHASE 0

        $returnval = true;

        foreach ($this->accounts_prepped as $idx => $user_data) {
            // Skip invalid accounts.
            if (!isset($user_data['main_account']) || (empty($user_data['main_account']))) {
                continue;
            }

            if ($this->utils->strcontains($user_data['main_account']['_group_actions'][0], 'move-primgroup')) {
                /* Old UID == Groupname (Hopefully) */
                $group_name = $user_data['main_account']['uid'][0];
                $new_group_name = ('archived-' . date("Y-m-d") . '-' . $group_name);
                $group_data = [
                    'objectClass' => [
                        'posixGroup'
                    ],
                    'cn' => [
                        $new_group_name
                    ],
                    '_dn' => [
                        'cn=' . $new_group_name . ',' . get_groups_ou() . $user_data['main_account']['_base_ldap'][0]
                    ],
                    '_actions' => [
                        'move'
                    ]
                ];

                $returnval = $this->archiveLDAPGroupObject($group_name, $group_data);
            }
        }

        $smarty->assign("data", $this->accounts_prepped);
        $smarty->assign("accounts_archived", true); // PHASE 5 (this phase)

        return $returnval;
    }


    private function execute_phase_6($smarty)
    {
        $smarty->assign("accounts_archived", true);    // PHASE 5
        $smarty->assign("accounts_reviewed", true);    // PHASE 4
        $smarty->assign("data_sorted", true); // PHASE 3
        $smarty->assign("archive_configured", true);   // PHASE 2
        $smarty->assign("file_uploaded", true);        // PHASE 1
        $smarty->assign("chosen_archivetype", true);   // PHASE 0

        $returnval = true;

        $smarty->assign("data", $this->accounts_prepped);

        $smarty->assign("primgroups_moved", true); // PHASE 6 (this phase)

        return $returnval;
    }


    private function archiveLDAPUserObject($user_data)
    {
        // only perform on this user dataset if _actions does contain a valid action…
        if (
            ($this->utils->strcontains($user_data ['_actions'] [0], 'none'))
            or ($this->utils->strcontains($user_data ['_actions'] [0], 'ignore'))
        ) {
            /* Errorcode 1403 */
            return 1403;
        }

        // Retrieve user object from LDAP
        $usertab = new usertabs(
            $this->config,
            $this->config->data['TABS']['USERTABS'],
            $user_data['_dn_ldap'][0]
        );
        $_base = dn2base($user_data['_dn_ldap'][0]);
        $usertab->by_object['user']->base = $_base;

        $this->failure_messages = [];
        if (count($usertab->check())) {
            $this->failure_messages[] = $usertab->check();

            /*
             * FIXME: collect failure statistics here!!!
             */

            $this->failure_in_this_phase = true;

            /* Errorcode 3004 */
            return 3004;
        } else {
            /* Moving and renaming user */
            $attr = "uid";
            $old_user_dn = $user_data['_dn_ldap'][0];
            /* Extracting new UID from new DN */
            $new_user_dn = $user_data['_dn'][0];
            $new_user_uid = explode(",", $new_user_dn, 2)[0];
            $new_user_uid = substr($new_user_uid, strpos($new_user_uid, "=") + 1);

            if (isset($usertab->$attr)) {
                $usertab->$attr = $new_user_uid;
            }

            foreach ($usertab->by_object as $pname => $plugin) {
                if (isset($usertab->by_object[$pname]->$attr)) {
                    $usertab->by_object[$pname]->$attr = $new_user_uid;
                }
            }

            /* Saving changes made to the user */
            $usertab->save();

            /* Present a status update in GOsa²'s logging system (/var/log/syslog mostly) */
            new log(
                "modify",
                "users/user",
                $old_user_dn,
                [],
                "Existing user '" . $usertab->givenName . "' '" . $usertab->sn .
                "' (" . $user_data['uid'][0] . ") archived via SchoolManager add-on."
            );

            // Move account to new dn.
            $usertab->by_object['user']->rename($usertab->dn, $new_user_dn);
        }

        return true;
    }


    private function archiveLDAPGroupObject($group_name, $group_data)
    {
        $returnval = "TRUE";

        // Only perform on this user dataset if _actions does contain a valid action…
        if (
            $this->utils->strcontains($group_data['_actions'][0], 'none')
            or $this->utils->strcontains($group_data['_actions'][0], 'ignore')
        ) {
            /* Errorcode 1403 */
            return 1403;
        }

        if (in_array('posixGroup', $group_data['objectClass'])) {
            $_group_obclass    = "posixGroup";
            $_group_tabs       = "GROUPTABS";
            $_group_tabs_class = "grouptabs";
            $_group_RDN        = get_groups_ou();
            $_group_obj        = "group";
        } else {
            $_group_obclass    = "gosaGroupOfNames";
            $_group_tabs       = "OGROUPTABS";
            $_group_tabs_class = "ogrouptabs";
            $_group_RDN        = get_ou("group", "ogroupRDN");
            $_group_obj        = "ogroup";
        }

        if (
            $this->utils->strcontains($group_data['_actions'][0], 'create')
            or $this->utils->strcontains($group_data['_actions'][0], 'move')
        ) {
            /* Instantiate a new group object via GOsa²'s API */
            $grouptab = new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs], 'new');

            if (isset($group_data['_dn'][0])) {
                $grouptab->by_object[$_group_obj]->base = dn2base($group_data['_dn'][0]);
            }
        } else {
            /* Retrieve group object from LDAP via GOsa²'s API */
            $grouptab = new $_group_tabs_class(
                $this->config,
                $this->config->data['TABS'][$_group_tabs],
                $group_data['_dn'][0]
            );
        }

        if ($this->utils->strcontains($group_data['_actions'][0], 'move')) {
            /* Do an ldapsearch for the source object and prepare a copy+paste action */

            $_ldapsearch = $this->_ldap->search(
                "(&(objectClass=" . $_group_obclass . ")(cn=" . $group_name . "))",
                ["*"]
            );

            /*
            * There should only be _ONE_ object of this CN in the given LDAP (sub)tree scope.
            * Thus, only fetching the first object.
            */
            $source = $this->_ldap->fetch($_ldapsearch);

            if (!empty($source)) {
                foreach ($grouptab->by_object as $pname => $plugin) {
                    $grouptab->by_object[$pname]->prepareForCopyPaste($source);
                }

                $remove_later_grouptab = new $_group_tabs_class(
                    $this->config,
                    $this->config->data['TABS'][$_group_tabs],
                    $source['dn']
                );
            }
        }

        /* Collect group properties from $group_data */
        $_group_properties = ["cn" => $group_data['cn'][0]];

        if (
            isset($group_data['description'][0])
            and ($this->utils->strcontains($group_data['_actions'][0], 'update-description')
            or $this->utils->strcontains($group_data['_actions'][0], 'create'))
        ) {
            $_group_properties["description"] = $group_data['description'][0];
        }

        if (
            isset($group_data['mail'][0])
            and ($this->utils->strcontains($group_data['_actions'][0], 'update-mail')
            or $this->utils->strcontains($group_data['_actions'][0], 'create'))
        ) {
            $_group_properties["mail"] = $group_data['mail'][0];
        }

        /*
        * FIXME: Make mail address creation (from CN) configurable somewhere…
        */

        /* Populate sub-object attributes for $grouptab with same values */
        foreach ($_group_properties as $attr => $val) {
            if (isset($grouptab->$attr)) {
                $grouptab->$attr = $val;
            }

            foreach ($grouptab->by_object as $pname => $plugin) {
                if (isset($grouptab->by_object[$pname]->$attr)) {
                    $grouptab->by_object[$pname]->$attr = $val;
                }
            }
        }

        // If we do import mail properties, make sure the mailgroup object is activated
        if (isset($_group_properties['mail']) && isset($grouptab->by_object['mailgroup'])) {
            $grouptab->by_object['mailgroup']->is_account = true;
        }

        /* Run GOsa²'s groups/group | ogroups/ogroup checks */

        if (count($grouptab->check())) {
            msg_dialog::displayChecks($grouptab->check());

            /*
            * FIXME: collect failure statistics here!!!
            */

            $this->failure_in_this_phase = true;
            return 808;
        } else {
            if (isset($remove_later_grouptab)) {
                $remove_later_grouptab->delete();
            }

            /* Save group object to LDAP */
            $grouptab->save();
        }

        /* Present a status update in GOsa²'s logging system (/var/log/syslog mostly) */
        if ($this->utils->strcontains($group_data['_actions'][0], 'create')) {
            new log(
                "create",
                "groups/group",
                $grouptab->dn,
                [],
                "New group created via SchoolManager add-on."
            );
        } elseif ($this->utils->strcontains($group_data['_actions'][0], 'none')) {
            new log(
                "modify",
                "groups/group",
                $grouptab->dn,
                [],
                "Existing group modified via SchoolManager add-on."
            );
        }

        return $returnval;
    }


    private function getAttrsPreSelection($size)
    {
        $attrs_count = count($this->getAttributes());
        $selection = [];
        for ($i = 0; $i < $attrs_count; $i++) {
            $selection[] = $i;
        }

        for ($i = $attrs_count; $i < $size; $i++) {
            $selection[] = $attrs_count;
        }

        return $selection;
    }


    private function getMultiAttributes()
    {
        $multiAttrs = [];
        return $multiAttrs;
    }


    private function prepareLdapImport(array $sorted_accounts)
    {
        $accounts_to_archive = [];

        foreach ($sorted_accounts as $idx => $row_sorted) {
            if ($this->CSVRowSanityChecksOk($row_sorted, $idx)) {
                /* Main account */

                if (
                    isset($row_sorted['uid']) &&
                    $row_sorted['uid'] &&
                    (($row_sorted['uid'] != "%auto%") &&
                     ($row_sorted['uid'] != "%uid%") &&
                     ($row_sorted['uid'] != "%{uid}"))
                ) {
                    $person['uid'] = [strtolower(iconv(
                        'UTF-8',
                        'US-ASCII//TRANSLIT',
                        trim($row_sorted['uid'])
                    ))];
                } else {
                    $person['uid'] = ['{%uid}'];
                }

                $person['sn']        = [trim($row_sorted['sn'])];
                $person['givenName'] = [trim($row_sorted['givenName'])];

                $_dateOfBirth = trim($row_sorted['dateOfBirth']);
                /*
                 * FIXME 1: On a German localized GOsa², this is not required…, but how about other locales???
                 * FIXME 2: If date is formatted like "dd.mm.yy" GOsa² will not use yy as the birthyear but print error
                 */
                if ($this->utils->strcontains($_dateOfBirth, '.')) {
                    list($day, $month, $year) = explode(".", $_dateOfBirth, 3);
                    $_dateOfBirth = $year . "-" . $month . "-" . $day;
                    // $_dateOfBirth = $day.".".$month.".20".$year;
                }
                $person['dateOfBirth'] = [$_dateOfBirth];

                $person['gender'] = [
                    $this->utils->normalizeGenderString(trim($row_sorted['gender']))
                ];

                if (
                    isset($row_sorted['employeeNumber']) &&
                    $row_sorted['employeeNumber']
                ) {
                    $person['employeeNumber'] = [
                        $row_sorted['employeeNumber']
                    ];
                }

                /*
                 * Keeping the memory footprint low:
                 *
                 * $person['_template']
                 * is only an int value.
                 *
                 * Retrieve the actual template DN string later like this
                 * $template_dn = $this->csvinfo['templates']['DNs'][$student['_template']];
                 */
                // $person ['_template'] = array (
                //     $this->csvinfo ['template_main']
                // );
                $person['_status'] = [
                 'unchecked'
                ];
                $person['_actions'] = [
                 'none'
                ];
                $person['_group_actions'] = [
                 'none'
                ];
            }

            $accounts_to_archive[] = ['main_account' => $person];
        }

        return $accounts_to_archive;
    }


    private function CSVRowSanityChecksOk($csv_row, $idx)
    {
        $ok = false;
        if (
            (empty($csv_row['sn'])) ||
            (empty($csv_row['givenName'])) ||
            (!isset($csv_row['sn'])) ||
            (!isset($csv_row['givenName']))
        ) {
            /* Output Error about missing the least set of attributes */
            msg_dialog::display(
                _("Error"),
                sprintf(
                    _("Need at least %s and %s to create users (Check line %d in CSV file)!"),
                    bold("sn"),
                    bold("givenName"),
                    $idx + 1
                ),
                ERROR_DIALOG
            );
        } /*
           *
           * FIXME: Add plenty of more CSV data sanity checks here!!!!
           *
           */
        else {
            $ok = true;
        }

        if (!$ok) {
            $this->failure_in_this_phase = true;
        }

        return $ok;
    }


    private function accountStatusCheck()
    {
        // If we don't have the $this->accounts_prepped dataset, we are doomed to fail…
        if (
            !is_array($this->accounts_prepped) ||
            empty($this->accounts_prepped)
        ) {
            $this->failure_in_this_phase = true;
            msg_dialog::display(
                _("Error"),
                _("csvinfo['data_prepped_accounts'] is empty or not an array! Can't complete accountStatusCheck(). Please try again."),
                ERROR_DIALOG
            );

            return 1; // We showed an error msg so return '1'
        }

        $new_base_ou = "ou=archived-" . date("Y-m-d");
        $new_base_ou .= "," . $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_users']];

        // Check if the new OU exists, if not create it.
        // TODO: Check return code of checkOUExists
        $this->utils->checkOUExists(
            $new_base_ou,
            ["description" => sprintf(_("Archived accounts - %s"), date('Y-m-d'))]
        );

        $search_attr = [
            "uid",
            "sn",
            "givenName",
            "gender",
            "dateOfBirth",
            "mail",
            "departmentNumber",
            "employeeNumber"
        ];

        if (isset($this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_matching_users']])) {
            $this->_ldap->cd($this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_matching_users']]);
            $ldapsearch = $this->_ldap->search("(objectClass=gosaAccount)", $search_attr);
        } else {
            $this->_ldap->cd($this->config->current['BASE']);
            $ldapsearch = $this->_ldap->search("(objectClass=gosaAccount)", $search_attr);
        }

        // Free some memory…
        unset($search_attr);

        // This will probably scale very very badly… Improvement needed. Suggestions?
        while ($gosa_account = $this->_ldap->fetch($ldapsearch)) {
            // Skip newstudent template.
            if (strtolower($gosa_account['uid'][0]) === "newstudent") {
                continue;
            }

            foreach ($this->accounts_prepped as $key => $row) {
                if (isset($row['main_account'])) {
                    $test_attrs = [];
                    if ($this->csvinfo['sel_ldap_match_attr_studentid']) {
                        $test_attrs[] = "employeeNumber";
                    }
                    if ($this->csvinfo['sel_ldap_match_attr_snname']) {
                        $test_attrs[] = "sn";
                    }
                    if ($this->csvinfo['sel_ldap_match_attr_name']) {
                           $test_attrs[] = "givenName";
                    }
                    if ($this->csvinfo['sel_ldap_match_attr_gender']) {
                           $test_attrs[] = "gender";
                    }
                    if ($this->csvinfo['sel_ldap_match_attr_birthday']) {
                        $test_attrs[] = "dateOfBirth";
                    }

                    // Detect status of account
                    if ($this->utils->compareObjects($row['main_account'], $gosa_account, $test_attrs, "", true) === null) {
                        if (
                            (!isset($row['main_account']['uid'][0])) ||
                            ($row['main_account']['uid'][0] === "{%uid}") ||
                            ($row['main_account']['uid'][0] === "%uid")
                        ) {
                            $this->accounts_prepped[$key]['main_account']['uid'] = [
                                $gosa_account['uid'][0]
                            ];

                            $row['main_account']['uid'] = $this->accounts_prepped[$key]['main_account']['uid'];
                        }

                        // $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
                        // $expected_account_dn = "uid=" . $row['main_account']['uid'][0] . ',' . get_people_ou() . $expected_base;
                        // $expected_account_uid = $row['main_account']['uid'][0];

                        // Explode into array, pick first then get the part after uid=
                        $gosa_account_dn = $gosa_account['dn'];
                        $gosa_account_uid = explode(",", $gosa_account_dn, 2)[0];
                        $gosa_account_uid = substr($gosa_account_uid, strpos($gosa_account_uid, "=") + 1);

                        $gosa_account_base = explode(",", $gosa_account_dn, 3);
                        $gosa_account_base = $gosa_account_base[2];

                        $new_uid = "uid=" . $row['main_account']['uid'][0] . "_archived-" . date("Y-m-d");
                        $new_dn = $new_uid . ',' . get_people_ou() . $new_base_ou;

                        $this->accounts_prepped[$key]['main_account']['_status'][0] = 'exists';

                        /* Set actions */
                        $_actions = [];
                        if ($this->accounts_prepped[$key]['main_account']['_status'][0] === 'exists') {
                            $_actions[] = "archive";
                        }

                        if (empty($_actions)) {
                            $_actions[] = 'none';
                        }
                           /* ----- */

                           // actions array -> comma seperated string
                           $this->accounts_prepped[$key]['main_account']['_actions'][0] = implode(",", $_actions);

                           $this->accounts_prepped[$key]['main_account']['_dn']      = [$new_dn];
                           $this->accounts_prepped[$key]['main_account']['_dn_ldap'] = [$gosa_account_dn];
                    }
                }
            }
        }

        // Mark unhandled accounts accordingly.
        foreach ($this->accounts_prepped as $key => $account) {
            if (isset($row['main_account'])) {
                if ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] == 'unchecked') {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'not-found';
                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = 'ignore,check-manually';
                }
            }
        }
    }


    private function genAccountBase($user_data, $groups_data = [])
    {
        if ($user_data['_template'][0] != 0) {
            $template_dn = $this->csvinfo['templates']['DNs'][$user_data['_template'][0]];
            $template_base = preg_replace("/^[^,]+," . preg_quote(get_people_ou(), '/i') . "/", '', $template_dn);
        } else {
            $template_base = $this->config->current['BASE'];
        }

        /* Put accounts into a sub-OU of the template's base DN if requested */
        if ($this->csvinfo['accounts_in_class_ou'] === true) {
            $_class_group = "";
            foreach ($groups_data as $group_key => $group_data) {
                if ($this->utils->strcontains($group_data['cn'][0], 'class_')) {
                    $_class_group = 'ou=' . str_replace('class_', '', $group_data['cn'][0]) . ',';
                    /*
                     * we take the first class group we find…
                     * presuming that students only have _one_ class group assigned(!)
                     */
                    break;
                }
            }
            $template_base = $_class_group . $template_base;
        }
        return $template_base;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
