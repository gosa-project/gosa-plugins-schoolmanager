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


class importaccounts extends import
{
    /** Template URI (.tpl file) */
    public const TEMPLATE_URI = "importaccounts/content_importaccounts_base.tpl";

    /*
     * PHASE 01:
     *   - Check uploaded file
     *   - Parse CSV file
     *   - Prepare '$this->csvinfo' with templates
     *   - Set form presets
     */
    function execute_phase1($smarty, $quick_import = false)
    {
        // This is the first phase, so no need to worry about quick-import.
        $_delim_id                = $_POST['delimiter_id'];
        $_csv_with_column_headers = $_POST['csv_with_column_headers'];

        $ret_array = $this->utils->checkUploadedFile();

        // If successful.
        if ($ret_array[0]) {
            $raw_csv_data = $ret_array[1];
            $smarty->assign("file_uploaded", true);
        } else {
            $this->failure_in_this_phase = true;
            $smarty->assign("LDIFError", true);

            msg_dialog::display(_("Error"), $ret_array[1], ERROR_DIALOG);
            return;
        }


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

        if (isset($_csv_with_column_headers)) {
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

            return;
        } else {
            // Copy each element from $parser_ret into $this->csvinfo…
            $this->csvinfo = array_merge($this->csvinfo, $parser_ret);

            // We don't need it anymore. Keep memory footprint low.
            unset($parser_ret);
        }

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
         * Search OUs with objectClass gosaDepartment
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

            if (strcasecmp($result['ou'][0], "schoolmanager") == 0) {
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
    }


    /*
     * PHASE 02:
     *   - Set import options which the user configured.
     *     - Checkboxes, OU's for new groups & users, etc…
     */
    function execute_phase2($smarty, $quick_import = false)
    {
        /* Load the properties we need in this phase */
        if ($quick_import === false) {
            $_template      = $_POST["template_" . $this->import_account_type];
            $_template_aux  = $_POST["template_" . $this->import_account_type . "_aux"] ?? null;
            $_ou_groups     = $_POST["ou_groups"];
            $_domain_users  = $_POST["domain_users"];
            $_domain_groups = $_POST["domain_groups"];
            $this->csvinfo['accounts_in_class_ou'] = $this->feature_accounts_in_class_ou ? isset($_POST["accounts_in_class_ou"]) : false;
            $this->csvinfo['aliases_in_schooldomain'] = $this->feature_aliases_in_schooldomain ? isset($_POST["aliases_in_schooldomain"]) : false;
            $this->csvinfo['try_mail_as_uid']                   = isset($_POST["try_mail_as_uid"]);
            $this->csvinfo['add_course_members_to_class_group'] = isset($_POST["add_course_members_to_class_group"]);
            $this->csvinfo['sel_ldap_match_attr_studentid']     = isset($_POST["sel_ldap_match_attr_studentid"]);
            $this->csvinfo['sel_ldap_match_attr_givenname']     = isset($_POST["sel_ldap_match_attr_givenname"]);
            $this->csvinfo['sel_ldap_match_attr_snname']        = isset($_POST["sel_ldap_match_attr_snname"]);
            $this->csvinfo['sel_ldap_match_attr_birthday']      = isset($_POST["sel_ldap_match_attr_birthday"]);
            $this->csvinfo['sel_ldap_match_attr_gender']        = isset($_POST["sel_ldap_match_attr_gender"]);
        } else {
            // A few items were already computed in phase 1!
            // So, we just use the presets which phase 1 set.
            $_template                          = $this->csvinfo['template_main'];
            $_template_aux                      = $this->csvinfo['template_aux'];
            $_ou_groups                         = $this->csvinfo['ou_groups'];
            $_domain_users                      = $this->csvinfo["domain_users"];
            $_domain_groups                     = $this->csvinfo["domain_groups"];
        }

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

            if ($this->failure_in_this_phase === false) {
                $smarty->assign("import_configured", true);

                $this->csvinfo['attrs'] = $this->getAttributes();
                $this->csvinfo['attrs'][] = "---";

                // Initialize $this->csvinfo['attrs_selected'], only do this here and once
                $this->csvinfo['attrs_selected'] = $this->getAttrsPreSelection($this->csvinfo['num_cols']);

                // User import attributes
                $smarty->assign("attrs", $this->csvinfo['attrs']);

                // Per row selected user import attributes
                $smarty->assign("attrs_selected", $this->csvinfo['attrs_selected']);

                // Number of CSV columns --==-> Number of rows in 90°-counter-clockwise-rotated table
                $smarty->assign("num_rows", $this->csvinfo['num_cols']);

                // CSV data
                $smarty->assign("data_size", $this->csvinfo['num_rows']);
                $smarty->assign("data", array_slice($this->csvinfo['data'], 0, 5));
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

        if ($this->failure_in_this_phase === true) {
            // Prepare for reloading this phase's web page again…
            $smarty->assign("templates", $this->csvinfo['templates']['formfields']);
            $smarty->assign("ous_available", $this->csvinfo['ou_tree']['formfields']);
            $smarty->assign("preset_template_" . $this->import_account_type, $this->csvinfo['template_main']);
            $smarty->assign("preset_template_" . $this->import_account_type . "_aux", $this->csvinfo['template_aux']);
            $smarty->assign("preset_ou_groups", $this->csvinfo['ou_groups']);
            $smarty->assign("preset_domain_groups", $this->csvinfo['domain_groups']);
            $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
            $smarty->assign("preset_aliases_in_schooldomain", $this->csvinfo['aliases_in_schooldomain']);
            $smarty->assign("preset_try_mail_as_uid", $this->csvinfo['try_mail_as_uid']);
            $smarty->assign("preset_sel_ldap_match_attr_studentid", $this->csvinfo['sel_ldap_match_attr_studentid']);
            $smarty->assign("preset_sel_ldap_match_attr_name", $this->csvinfo['sel_ldap_match_attr_name']);
            $smarty->assign("preset_sel_ldap_match_attr_snname", $this->csvinfo['sel_ldap_match_attr_snname']);
            $smarty->assign("preset_sel_ldap_match_attr_birthday", $this->csvinfo['sel_ldap_match_attr_birthday']);
            $smarty->assign("preset_sel_ldap_match_attr_gender", $this->csvinfo['sel_ldap_match_attr_gender']);
            $smarty->assign("preset_add_course_members_to_class_group", $this->csvinfo['add_course_members_to_class_group']);
        }
    }


    /*
     * PHASE 03:
     *   - Sort data using user-selected attributes.
     */
    function execute_phase3($smarty, $quick_import = false)
    {
        /* Sanity checks on LDAP attributes assignments */

        if ($quick_import === true) {
            // Just use already set $this->csvinfo['attrs_selected'].
        } else {
            // Read attributes assignments from $_POST
            $new_attrs_selected = array();
            for ($i = 0; $i < count($this->csvinfo['attrs_selected']); $i++) {
                if (isset($_POST["column_head_$i"])) {
                    $new_attrs_selected[] = $_POST["column_head_$i"];
                } else {
                    $new_attrs_selected[] = $i;
                }
            }
            $this->csvinfo['attrs_selected'] = $new_attrs_selected;
        }

        // Transform $this->csvinfo['data'] ---> $this->csvinfo['data_sorted'] using $this->csvinfo['attrs_selected']'
        // Sort the CSV data table according to how it got re-ordered by the plugin-user
        $this->csvinfo['data_sorted'] = array();
        $multi_attrs = $this->getMultiAttributes();
        foreach ($this->csvinfo['data'] as $data_row) {
            $attrs_counter = array();
            $data_row_sorted = array();

            for ($i = 0; $i < count($data_row); $i++) {
                $data_in_cell = $data_row[$i];
                $selection_in_cell = $this->csvinfo['attrs_selected'][$i];
                $value_of_selection = $this->csvinfo['attrs'][$selection_in_cell];

                // Skip every '---'
                if ($value_of_selection == "---") {
                    continue;
                }

                if (in_array($value_of_selection, $multi_attrs)) {
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
                }
            }

            $this->csvinfo['data_sorted'][] = $data_row_sorted;
        }

        // Transform 'data_sorted' ---> 'data_preldap' array
        // by using by sub-class overloaded prepareLdapImport()
        $this->csvinfo['data_preldap'] = $this->prepareLdapImport($this->csvinfo['data_sorted']);

        // $this->failure_in_this_phase may have been set above or in $this->prepareLdapImport()…
        if ($this->failure_in_this_phase === false) {
            // Free old & now unused memory
            unset($this->csvinfo['data_sorted']);

            $this->accountStatusCheck();

            $smarty->assign("data_sorted", true);
            $smarty->assign("data", $this->csvinfo['data_preldap']);
        } else {
            // Prepare for reloading this phase's web page again…
            $smarty->assign("attrs", $this->csvinfo['attrs']);
            $smarty->assign("attrs_selected", $this->csvinfo['attrs_selected']);
            $smarty->assign("num_rows", $this->csvinfo['num_cols']);
            $smarty->assign("data", array_slice($this->csvinfo['data'], 0, 5));
        }
    }


    /*
     * PHASE 04:
     */
    function execute_phase4($smarty)
    {
        /*
         * Add users to / update users in LDAP
         */
        foreach ($this->csvinfo['data_preldap'] as $idx => $user_data) {
            // Import the user's main account
            if (isset($user_data['main_account']) and !empty($user_data['main_account'])) {
                $this->importLDAPUserObject($user_data['main_account']);
            }

            // Import the user's aux account (e.g., parent accounts)
            if (isset($user_data['aux_accounts']) and !empty($user_data['aux_accounts'])) {
                foreach ($user_data['aux_accounts'] as $idx_aux => $aux_account) {
                    // Import aux account $aux_account_data
                    $this->importLDAPUserObject($aux_account);
                }
            }
        }

        if ($this->failure_in_this_phase === false) {
            /*
             * FIXME: Present a message dialog box that informs on the statistics of
             *        the LDAP group import. (How many groups got created? How many
             *        groups have been updated? How many groups were not empty and
             *        required dropping of all members? Etc…)
             */

            $smarty->assign("accounts_reviewed", true);

            $this->accountStatusCheck();
            $smarty->assign("data", $this->csvinfo['data_preldap']);
        } else {
            // Show failure messages
            foreach ($this->failure_messages as $msg) {
                msg_dialog::displayChecks(array($msg));
            }

            // Prepare for reloading this phase's web page again…
            $this->accountStatusCheck();
            $smarty->assign("data", $this->csvinfo['data_preldap']);
        }
    }


    /*
     * PHASE 05:
     */
    function execute_phase5($smarty)
    {
        if ($this->failure_in_this_phase !== false) {
            // Prepare for reloading this phase's web page again…
            $this->accountStatusCheck();
            $smarty->assign("data", $this->csvinfo['data_preldap']);

            return;
        }

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
     * PHASE 06:
     */
    function execute_phase6($smarty)
    {
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
        }
    }


    /*
     * PHASE 07:
     */
    function execute_phase7($smarty)
    {
        $this->accountStatusCheck();
        $this->groupStatusCheck();
        $this->prepareGroupMemberships();
        $smarty->assign("data", $this->csvinfo['data_preldap']);

        if ($this->failure_in_this_phase === false) {
            $smarty->assign("groups_imported", true);
        } else {
            // Prepare for reloading this phase's web page again…
            $this->groupStatusCheck();
            $smarty->assign("data_groups", $this->csvinfo['data_preldap_posixgroups']);
            $smarty->assign("data_ogroups", $this->csvinfo['data_preldap_ogroups']);
        }
    }


    /*
     * PHASE 08:
     */
    function execute_phase8($smarty)
    {
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
            // Prepare for reloading this phase's web page again…
            $this->prepareGroupMemberships();
        }
    }


    /*
     * PHASE 09:
     */
    function execute_phase9($smarty)
    {
        if ($this->failure_in_this_phase === false) {
            $smarty->assign("accounts_groupmembers_updated", true);
        } else {
            // Prepare for reloading this phase's web page again…
        }
    }


    /*
     * PHASE 10:
     */
    function execute_phase10($smarty)
    {
        if ($this->failure_in_this_phase === false) {
            /*
             * FIXME: If automatic password generation is used, present a download link
             *        for a generated credentials sheet of paper.
             */

            /* Render the final SUCCESS page */
            $smarty->assign("cleanup_completed", true);
        } else {
            /* FIXME: no clue what to do on failures here… We will see. */
        }
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
        $smarty->assign("file_uploaded_do_quickimport", 0);

        /* PHASE 02 done, set $import_configured to TRUE  */
        $smarty->assign("import_configured", 0);

        /* PHASE 03 done, set $data_sorted to TRUE  */
        $smarty->assign("data_sorted", 0);

        /* PHASE 04 done, set $accounts_reviewed to TRUE  */
        $smarty->assign("accounts_reviewed", 0);

        /* PHASE 05 done, set $accounts_imported to TRUE  */
        $smarty->assign("accounts_imported", 0);

        /* PHASE 06 done, set $groups_reviewed to TRUE  */
        $smarty->assign("groups_reviewed", 0);

        /* PHASE 07 done, set $groups_imported to TRUE  */
        $smarty->assign("groups_imported", 0);

        /* PHASE 08 done, set $accounts_groupmembers_reviewed to TRUE */
        $smarty->assign("accounts_groupmembers_reviewed", 0);

        /* PHASE 09 done, set $accounts_groupmembers_updated to TRUE */
        $smarty->assign("accounts_groupmembers_updated", 0);

        /* PHASE 10 done, set $cleanup_completed to TRUE */
        $smarty->assign("cleanup_completed", 0);

        /* Get the LDAP link, to generate the Export */
        $this->_ldap = $this->config->get_ldap_link();

        /* initialize CSV Info array */
        if (!is_array($this->csvinfo)) {
            $this->csvinfo = array();
        }

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
        $available_delims = array(
            0 => '","'  . " - " . _("comma separated values"),
            1 => '";"'  . " - " . _("semicolon separated values"),
            2 => '"\t"' . " - " . _("tabstop separated values"),
            3 => '" "'  . " - " . _("blank separated values"),
        );

        // Add custom character radio button only if it should appear.
        if ($delimiter_index === 4) {
            $available_delims[4] = '"' . $delim_char . '" - ' . _("values seperated by custom character");
        }

        $smarty->assign('available_delimiters', $available_delims);

        // Get setting's default.
        $value = $this->utils->getConfigBoolValue("ignore_first_row_of_csv_file");
        $smarty->assign('preset_csv_with_column_headers', $value);

        /* Check permissions for import */
        $acl = $this->ui->get_permissions($this->config->current['BASE'], "all/all");
        if (!preg_match("/w/", $acl)) {
            if (
                isset($_POST['file_uploaded']) or
                isset($_POST['file_uploaded_do_quickimport']) or
                isset($_POST['import_configured']) or
                isset($_POST['data_sorted']) or
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
            if (isset($_POST['phase_01']) and isset($_POST['file_uploaded_do_quickimport'])) {
                // Delete old references…
                $this->csvinfo = array();

                $this->utils->debug_to_console("Doing a Quick-Import…");

                $functions = [
                    "Phase 1"  => "execute_phase1",
                    "Phase 2"  => "execute_phase2",
                    "Phase 3"  => "execute_phase3",
                    // "Phase 4"  => "execute_phase4",
                    // "Phase 5"  => "execute_phase5",
                    // "Phase 6"  => "execute_phase6",
                    // "Phase 7"  => "execute_phase7",
                    // "Phase 8"  => "execute_phase8",
                    // "Phase 9"  => "execute_phase9",
                    // "Phase 10" => "execute_phase10",
                ];

                // Call each function defined above and check if there was an error.
                foreach ($functions as $key => $value) {
                    $this->utils->debug_to_console("Executing " . $key . "…");
                    call_user_func(array($this, $value), $smarty, true);

                    if ($this->failure_in_this_phase === true) {
                        // Phase failed.
                        $this->utils->debug_to_console("Failed executing " . $key . "…");
                        break;
                    }
                }
            } elseif (isset($_POST['phase_01']) and isset($_POST['file_uploaded'])) {
                // Delete old references…
                $this->csvinfo = array();

                $this->utils->debug_to_console("Executing normal Phase1…");
                $this->execute_phase1($smarty);
            } elseif (isset($_POST['phase_02']) and isset($_POST['import_configured'])) {
                $this->utils->debug_to_console("Executing normal Phase2…");
                $smarty->assign("file_uploaded", true);
                $this->execute_phase2($smarty);
            } elseif (isset($_POST['phase_03']) and isset($_POST['data_sorted'])) {
                $this->utils->debug_to_console("Executing normal Phase3…");
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $this->execute_phase3($smarty);
            } elseif (isset($_POST['phase_04']) and isset($_POST['accounts_reviewed'])) {
                $this->utils->debug_to_console("Executing normal Phase4…");
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $this->execute_phase4($smarty);
            } elseif (isset($_POST['phase_05']) and isset($_POST['accounts_imported'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $this->execute_phase5($smarty);
            } elseif (isset($_POST['phase_06']) and isset($_POST['groups_reviewed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $this->execute_phase6($smarty);
            } elseif (isset($_POST['phase_07']) and isset($_POST['groups_imported'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $this->execute_phase7($smarty);
            } elseif (isset($_POST['phase_08']) and isset($_POST['accounts_groupmembers_reviewed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $this->execute_phase8($smarty);
            } elseif (isset($_POST['phase_09']) and isset($_POST['accounts_groupmembers_updated'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $smarty->assign("accounts_groupmembers_reviewed", true);
                $this->execute_phase9($smarty);
            } elseif (isset($_POST['phase_10']) and isset($_POST['cleanup_completed'])) {
                $smarty->assign("file_uploaded", true);
                $smarty->assign("import_configured", true);
                $smarty->assign("data_sorted", true);
                $smarty->assign("accounts_reviewed", true);
                $smarty->assign("accounts_imported", true);
                $smarty->assign("groups_reviewed", true);
                $smarty->assign("groups_imported", true);
                $smarty->assign("accounts_groupmembers_reviewed", true);
                $smarty->assign("accounts_groupmembers_updated", true);
                $this->execute_phase10($smarty);
            }
        }

        // Show main page…
        return ($smarty->fetch(get_template_path(self::TEMPLATE_URI, true)));
    }


    function CSVRowSanityChecksOk($csv_row, $idx)
    {
        $ret_ok = false;

        // Check for most basic columns.
        if (
            empty($csv_row['sn']) or  empty($csv_row['givenName']) or
            !isset($csv_row['sn']) or !isset($csv_row['givenName'])
        ) {
            // Output error about missing the least set of attributes…
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
        } else {
            $ret_ok = true;
        }

        /*
         *
         * FIXME: Add plenty of more CSV data sanity checks here!!!!
         *
         */

        if (!$ret_ok) {
            $this->failure_in_this_phase = true;
        }

        return $ret_ok;
    }


    function moveLDAPPrimGroupObjects()
    {
        foreach ($this->csvinfo['data_preldap'] as $idx => $row) {
            if ($this->utils->strcontains($row['main_account']['_actions'][0], 'ignore')) {
                // We shouldn't need this, but just in case…
                continue;
            }

            $user_data = $row['main_account'];

            if (
                $this->utils->strcontains($user_data['_group_actions'][0], 'create-primgroup') === false and
                $this->utils->strcontains($user_data['_group_actions'][0], 'move-primgroup')   === false
            ) {
                // Neither 'create-primgroup' nor 'move-primgroup' given,
                // so nothing to do for this user account…
                continue;
            }

            // Prepare data for group_data object
            $group_data_dn   = "cn=" . $row['main_account']['uid'][0] . "," .
                                get_groups_ou() . dn2base($row['main_account']['_dn'][0]);
            $group_data_name = $user_data['uid'][0];

            // Create a group_data object that we can pass on to importLDAPGroupObject…
            $group_data = array(
                'objectClass' => array('posixGroup'),
                'cn'          => array($group_data_name),
                '_dn'         => array($group_data_dn),
                '_actions'    => array(), // To be filled below.
            );

            // Set the action for group_data, derived from the account's _group_actions.
            if ($this->utils->strcontains($user_data['_group_actions'][0], 'create-primgroup')) {
                $group_data['_actions'][] = 'create';
                /*
                 * FIXME: We need to update the user accounts gidNumber attribute description here…
                 */
            } elseif ($this->utils->strcontains($user_data['_group_actions'][0], 'move-primgroup')) {
                $group_data['_actions'][] = 'move';
            } else {
                // Arriving here means that we have a serious bug, cause
                // a few lines above we checked if '_group_actions' has an
                // valid action.
                continue;
            }

            // Create or move account's primary group.
            $this->importLDAPGroupObject($group_data_name, $group_data);
        }
    }


    function importLDAPGroupObject($group_name, $group_data)
    {
        // Only perform on this user dataset if _actions does not contain "none"…
        if (
            ($this->utils->strcontains($group_data['_actions'][0], 'none')) or
            ($this->utils->strcontains($group_data['_actions'][0], 'ignore'))
        ) {
            return;
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
            $this->utils->strcontains($group_data['_actions'][0], 'create') or
            $this->utils->strcontains($group_data['_actions'][0], 'move')
        ) {
            /* Instantiate a new group object via GOsa²'s API */

            $grouptab = new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs], 'new');
            if (isset($group_data['_dn'][0])) {
                $grouptab->by_object[$_group_obj]->base = dn2base($group_data['_dn'][0]);
            } else {
                $grouptab->by_object[$_group_obj]->base = $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];
            }
        } else {
            /* Retrieve group object from LDAP via GOsa²'s API */

            $_dn  = 'cn=' . $group_name . ',' . $_group_RDN;
            $_dn .= $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];

            $grouptab = new $_group_tabs_class(
                $this->config,
                $this->config->data['TABS'][$_group_tabs],
                $_dn
            );
        }

        if ($this->utils->strcontains($group_data['_actions'][0], 'move')) {
            /* Do an ldapsearch for the source object and prepare a copy+paste action */

            $_ldapsearch = $this->_ldap->search("(&(objectClass=" . $_group_obclass . ")(cn=" . $group_name . "))", array("*"));

            /*
             * There should only be *ONE* object of this CN in the given LDAP (sub)tree scope.
             * Thus, only fetching the first object.
             */
            $source = $this->_ldap->fetch($_ldapsearch);

            if (!empty($source)) {
                foreach ($grouptab->by_object as $pname => $plugin) {
                    $grouptab->by_object[$pname]->prepareForCopyPaste($source);
                }
                $remove_later_grouptab = new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs], $source['dn']);
            }
        }

        /* Collect group properties from $group_data */
        $_group_properties = array(
            "cn" => $group_data['cn'][0],
        );

        // Update 'description' if appropriate.
        if (
            isset($group_data['description'][0]) and
            ($this->utils->strcontains($group_data['_actions'][0], 'update-description') or
            $this->utils->strcontains($group_data['_actions'][0], 'create'))
        ) {
            $_group_properties["description"] = $group_data['description'][0];
        }

        // Update 'mail' if appropriate.
        if (
            isset($group_data['mail'][0]) and
            ($this->utils->strcontains($group_data['_actions'][0], 'update-mail') or
            $this->utils->strcontains($group_data['_actions'][0], 'create'))
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
        if (isset($_group_properties['mail']) and isset($grouptab->by_object['mailgroup'])) {
            $grouptab->by_object['mailgroup']->is_account = true;
        }

        /* Run GOsa²'s groups/group | ogroups/ogroup checks */
        $grouptab_check = $grouptab->check();
        if (count($grouptab_check)) {
            $this->failure_messages += $grouptab_check;

            /*
             * FIXME: collect failure statistics here!!!
             */

            $this->failure_in_this_phase = true;

            return;
        } else {
            if (isset($remove_later_grouptab)) {
                $remove_later_grouptab->delete();
            }
            /* Save group object to LDAP */
            $grouptab->save();
        }

        // Present a status update in GOsa²'s logging system (/var/log/syslog mostly)
        if ($this->utils->strcontains($group_data['_actions'][0], 'create')) {
            new log(
                "create",
                "groups/group",
                $grouptab->dn,
                array(),
                "New group created via SchoolManager add-on."
            );
        } elseif ($this->utils->strcontains($group_data['_actions'][0], 'none')) {
            new log(
                "modify",
                "groups/group",
                $grouptab->dn,
                array(),
                "Existing group modified via SchoolManager add-on."
            );
        } else {
            // Nothing…
        }
    }


    function importLDAPUserObject($user_data)
    {
        // Only perform on this user dataset if '_actions' does NOT contain "none" or "ignore"
        if (
            $this->utils->strcontains($user_data['_actions'][0], 'none')  or
            $this->utils->strcontains($user_data['_actions'][0], 'ignore')
        ) {
            return;
        }

        // Only perform on this user dataset, if a valid action was found:
        if (
            $this->utils->strcontains($user_data['_actions'][0], 'update-') and
            $this->utils->strcontains($user_data['_actions'][0], 'add-uid-at-domain-to-aliases') and
            $this->utils->strcontains($user_data['_actions'][0], 'drop-uid-at-domain-from-aliases') and
            $this->utils->strcontains($user_data['_actions'][0], 'create') and
            $this->utils->strcontains($user_data['_actions'][0], 'move') and
            $this->utils->strcontains($user_data['_actions'][0], 'rename-uid')
        ) {
            return;
        }

        // These should contain the right data
        $_user_properties_base   = array();
        $_user_properties_extras = array();

        if ($this->utils->strcontains($user_data['_actions'][0], 'rename-uid')) {
            if (
                isset($user_data['uid'][0]) and
                $user_data['uid'][0] !== "{%uid}" and
                $user_data['uid'][0] !== ""
            ) {
                $_user_properties_base["uid"] = $user_data['uid'][0];
            }
        }

        if ($this->utils->strcontains($user_data['_actions'][0], 'create')) {
            // Instantiate a new user object
            $usertab = new usertabs($this->config, $this->config->data['TABS']['USERTABS'], 'new');

            // Fill most basic properties.
            $_user_properties_base["sn"]        = $user_data['sn'][0];
            $_user_properties_base["givenName"] = $user_data['givenName'][0];

            // The below attributes must exist and be set with meaningful data once we arrive here!
            if (isset($user_data['uid'][0]) and $user_data['uid'][0] !== "{%uid}") {
                $_user_properties_base["uid"] = $user_data['uid'][0];
            } elseif (isset($user_data['sn'][0]) and isset($user_data['givenName'][0])) {
                if (
                    isset($user_data['uid'][0]) and
                    ($user_data['uid'][0] === "{%uid}" or
                    $user_data['uid'][0] === "")
                ) {
                    // Get "idGenerator" property from "core" class.
                    $_genStr = $this->config->get_cfg_value("core", "idGenerator");

                    if ($_genStr != "") {
                        // Use _attributes to generate a unique ID.
                        $_attributes = array(
                            'sn'        => $user_data['sn'][0],
                            'givenName' => $user_data['givenName'][0]
                        );
                        $_genUids = gen_uids($_genStr, $_attributes);

                        // If successfull, use the first UID we generated above.
                        if (is_countable($_genUids) and (count($_genUids) >= 1)) {
                            $_user_properties_base['uid'] = $_genUids[0];
                        } else {
                            msg_dialog::display(
                                _("ERROR"),
                                _("Failure during UID auto-generation for " . $user_data['givenName'][0] .
                                    " " . $user_data['sn'][0] . "! No list of possible UIDs returned."),
                                ERROR_DIALOG
                            );
                        }
                    } else {
                        msg_dialog::display(
                            _("ERROR"),
                            _("We can only auto-generate UID strings if " .
                            "the idGenerator is configured!"),
                            ERROR_DIALOG
                        );
                    }
                } elseif (
                    ($this->csvinfo['try_mail_as_uid'] === true) and
                    (isset($user_data['mail'][0]) and $user_data['mail'][0])
                ) {
                    $_user_properties_base['uid'] = strtolower($user_data['mail'][0]);
                } else {
                    $error_msg = _("No exact UIDs given, no UID column in the CSV sheet, " .
                    "no permission to use the MAIL attribute as UID…");
                    new log(_("ERROR"), $error_msg, $this->dn);
                    msg_dialog::display(_("ERROR"), $error_msg, ERROR_DIALOG);
                }
            } else {
                $error_msg = _("If UID is not provided and sn+givenName is not " .
                "provided either or incomplete, we cannot go on…");
                new log(
                    _("ERROR"),
                    $error_msg,
                    $this->dn
                );
                msg_dialog::display(_("ERROR"), $error_msg, ERROR_DIALOG);
            }

            // Update $usertab with $_user_properties_base
            foreach ($_user_properties_base as $attr => $val) {
                if (isset($usertab->$attr)) {
                    $usertab->$attr = $val;
                }

                foreach ($usertab->by_object as $pname => $plugin) {
                    if (isset($usertab->by_object[$pname]->$attr)) {
                        $usertab->by_object[$pname]->$attr = $val;
                    }
                }
            }
        } else {
            // Just retrieve user object from LDAP, instead of creating a new one.
            $usertab = new usertabs(
                $this->config,
                $this->config->data['TABS']['USERTABS'],
                $user_data['_dn_ldap'][0]
            );
        }

        /*
         * On existing account, we allow the below attributes to be updated
         * and if we create an account each attribute will be 'updated' too.
         */
        $properties_which_are_allowed_to_be_updated = array(
            "employeeNumber",
            "departmentNumber",
            "mail",
            "sn",
            "givenName",
            "dateOfBirth",
            "gender",
        );

        // Iterate over array above and update property if appropriate.
        foreach ($properties_which_are_allowed_to_be_updated as $update_property) {
            if (
                isset($user_data[$update_property][0]) and
                ($this->utils->strcontains($user_data['_actions'][0], 'create') or
                $this->utils->strcontains($user_data['_actions'][0], ('update-' . $update_property)))
            ) {
                // Special cases for 'mail' property:
                if ($update_property === "mail") {
                    /*
                     * FIXME: We need to dearly improve this bit and make this more configurable
                     *        I.e. allow for more patterns, such as {%uid}@{%domain_teachers}, etc.
                     */
                    if ($user_data['mail'][0] == "{%uid}@{%domain_users}") {
                        $_user_properties_extras["mail"] = $user_data['uid'][0] . "@" . $this->csvinfo['domain_users'];
                        continue;
                    } elseif ($user_data['mail'][0] == "{%uid}@{%domain_school}") {
                        $_user_properties_extras["mail"] = $user_data['uid'][0] . "@" . $this->csvinfo['domain_school'];
                        continue;
                    }
                }

                $_user_properties_extras[$update_property] = $user_data[$update_property][0];
            }
        }

        // We'll need the 'mail' extra property further down...
        if (!isset($_user_properties_extras["mail"])) {
            if (
                isset($user_data['mail']) and
                is_countable($user_data['mail']) and
                count($user_data['mail']) > 0
            ) {
                /* the mail attribute of userTab is a string(!!!), not an array */
                $_user_properties_extras["mail"] = $user_data['mail'][0];
            }
        }

        // Use existing 'alias' field (if any) and modify that if needed.
        if (
            isset($user_data['alias']) and
            is_countable($user_data['alias']) and
            count($user_data['alias']) > 0
        ) {
            $_user_properties_extras["alias"] = $user_data['alias'];
        }

        // Prepare $uid_at_school_domain with <UID>@<domain>
        if (isset($user_data['uid'][0])) {
            if ($user_data['uid'][0] === "{%uid}") {
                $_uid = $_user_properties_base['uid'];
            } else {
                $_uid = $user_data['uid'][0];
            }
            $uid_at_school_domain = $_uid . "@" . $this->csvinfo['domain_school'];
        }

        // Drop $uid_at_school_domain from alias field, if this is supposed to happen
        if (
            isset($_user_properties_extras["alias"]) and
            is_array($_user_properties_extras["alias"]) and
            isset($user_data['_actions'][0]) and
            $this->utils->strcontains($user_data['_actions'][0], 'drop-uid-at-domain-from-aliases')
        ) {
            // Drop $uid_at_school_domain from $_user_properties_extras["alias"].
            $_user_properties_extras["alias"] = array_values(
                array_diff($_user_properties_extras["alias"], array($uid_at_school_domain))
            );
        }

        // Add $uid_at_school_domain to alias field, if this is supposed to happen.
        if ($this->csvinfo['aliases_in_schooldomain']) {
            if (
                $this->utils->strcontains($user_data['_actions'][0], 'add-uid-at-domain-to-aliases') or
                (
                    $this->utils->strcontains($user_data['_actions'][0], 'create') and
                    !$this->utils->strcontains($user_data['_actions'][0], 'skip-aliases')
                )
            ) {
                // Include $uid_at_school_domain in $_user_properties_extras["alias"]
                // if it isn't already.
                if (
                    (
                        isset($_user_properties_extras["alias"]) and
                        is_array($_user_properties_extras["alias"]) and
                        !in_array($uid_at_school_domain, $_user_properties_extras["alias"])
                    ) and (
                        isset($_user_properties_extras["mail"]) and
                        ($_user_properties_extras["mail"] === $uid_at_school_domain)
                    )
                ) {
                    if (!isset($_user_properties_extras["alias"])) {
                        $_user_properties_extras["alias"] = array();
                    }
                    $_user_properties_extras["alias"][] = $uid_at_school_domain;
                }
            }
        }

        if ($user_data['_template'][0] != 0) {
            $template_dn = $this->csvinfo['templates']['DNs'][$user_data['_template'][0]];
        }

        /* test if base DN exists, this is only relevant for actions "create" and "move" but does not hurt for updates, either */
        $this->_ldap->cat($user_data['_base'][0]);
        if (!$this->_ldap->fetch()) {
            /* change UI "dir" in LDAP to the place where we want to create sub-OUs… */
            $_ui = get_userinfo();
            $old_ui_dn = $_ui->dn;
            $old_current_main_base = session::global_is_set("CurrentMainBase") ? session::global_get("CurrentMainBase") : $_ui->dn;
            if ($user_data['_template'][0] != 0) {
                change_ui_dn($old_ui_dn, dn2base($template_dn));
                session::global_set("CurrentMainBase", dn2base($template_dn));
            } else {
                change_ui_dn($old_ui_dn, $this->config->current['BASE']);
                session::global_set("CurrentMainBase", $this->config->current['BASE']);
            }

            /* create _base DN as a GOsa² department */
            $_ou = preg_replace("/^ou=([^,]+),.*/", "$1", $user_data['_base'][0]);
            $deptab = new deptabs($this->config, $this->config->data['TABS']['DEPTABS'], 'new', "deptabs");
            $deptab->by_object['department']->ou = $_ou;
            $deptab->by_object['department']->description = sprintf(_("Class: %s"), $_ou);
            $deptab->save();

            /* Reload departments */
            $this->config->get_departments();
            $this->config->make_idepartments();

            /* change UI dir back to where we came from */
            change_ui_dn(dn2base($template_dn), $old_ui_dn);
            session::global_set("CurrentMainBase", $old_current_main_base);
        }

        if (
            $this->utils->strcontains($user_data['_actions'][0], 'create') and
            $user_data['_template'][0] != 0
        ) {
            /* adapt new user object from a GOsa user object template */
            $usertab->adapt_from_template($template_dn, array("uid", "cn", "givenName", "sn"));
            $usertab->by_object['user']->base = $user_data['_base'][0];
        }

        if (isset($_user_properties_extras["mail"]) and isset($usertab->by_object['mailAccount'])) {
            /*
             * FIXME: check if a mailAccount plugin is available at all!!!
             */
            $usertab->by_object['mailAccount']->is_account = true;
        }

        /* Populate sub-object attributes for $usertab with same values */
        foreach ($_user_properties_extras as $attr => $val) {
            $usertab->$attr = $val;
            foreach ($usertab->by_object as $pname => $plugin) {
                if (isset($usertab->by_object[$pname]->$attr)) {
                    $usertab->by_object[$pname]->$attr = $val;
                }
            }
        }

        /* Insert into the LDAP tree */
        $usertab_check = $usertab->check();
        if (count($usertab_check)) {
            $this->failure_messages += $usertab_check;

            /*
             * FIXME: Collect failure statistics here!
             */

            $this->failure_in_this_phase = true;

            return;
        } else {
            $usertab->save();

            // Present a status update in GOsa²'s logging system (/var/log/syslog mostly)
            if ($this->utils->strcontains($user_data['_actions'][0], 'create')) {
                new log("create", "users/user", $usertab->dn, array(), "New user (" . $usertab->sn . ", " . $usertab->givenName . ") created via SchoolManager add-on.");
            } elseif ($this->utils->strcontains($user_data['_actions'][0], 'none')) {
                new log("modify", "users/user", $usertab->dn, array(), "Existing user  (" . $usertab->sn . ", " . $usertab->givenName . ") modified via SchoolManager add-on.");
            }

            if ($this->utils->strcontains($user_data['_actions'][0], 'set-password')) {
                if (!change_password($usertab->dn, $user_data['userPassword'][0], false, '', '', $message)) {
                    msg_dialog::displayChecks(array($message));
                }
            }

            if ($this->utils->strcontains($user_data['_actions'][0], 'move')) {
                $usertab->by_object['user']->rename($usertab->dn, $user_data['_dn'][0]);
            }
            if ($this->utils->strcontains($user_data['_actions'][0], 'rename-uid')) {
                $usertab->by_object['user']->rename($usertab->dn, $user_data['_dn'][0]);
            }

            /*
             * FIXME: Collect success statistics here!
             */
        }
    }


    function addLDAPUserObjectsToGroups($group_members_data)
    {
        foreach ($group_members_data as $group_name => $new_members) {
            if (empty($new_members)) {
                continue;
            } else {
                $grouptab = new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'], 'cn=' . $group_name . ',' . get_groups_ou() . $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']], "groups");

                $current_members = $grouptab->by_object['group']->memberUid;
                foreach ($new_members as $new_member) {
                    if (!in_array($new_member, $current_members)) {
                        $grouptab->by_object['group']->addUser($new_member);
                    }
                }

                /* Run GOsa²'s groups/group checks */
                if (count($grouptab->check())) {
                    msg_dialog::displayChecks($grouptab->check());

                    /*
                     * FIXME: collect failure statistics here!!!
                     */

                    $this->failure_in_this_phase = true;
                } else {
                    /* Save group object to LDAP */
                    $grouptab->save();
                }
            }
        }
    }


    function addLDAPUserObjectsToOGroups($ogroup_members_data)
    {
        foreach ($ogroup_members_data as $ogroup_name => $new_members) {
            if (empty($new_members)) {
                continue;
            } else {
                $ogrouptab = new ogrouptabs($this->config, $this->config->data['TABS']['OGROUPTABS'], 'cn=' . $ogroup_name . ',' . get_ou('group', 'ogroupRDN') . $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']], "ogroups");

                $current_members = $ogrouptab->by_object['ogroup']->member;
                foreach ($new_members as $new_member) {
                    if (!in_array($new_member, $current_members)) {
                        $ogrouptab->by_object['ogroup']->member[$new_member] = $new_member;
                    }
                }

                // Update the object type in gosaGroupObjects at the end of adding new members
                $obj_types = preg_replace('/[\[\]]/', '', $ogrouptab->by_object['ogroup']->gosaGroupObjects);
                if ($this->utils->strcontains($obj_types, 'U') === false) {
                    $obj_types .= 'U';
                    $ogrouptab->by_object['ogroup']->gosaGroupObjects = '[' . $obj_types . ']';
                }

                // Run GOsa²'s ogroups/ogroup checks
                if (count($ogrouptab->check())) {
                    msg_dialog::displayChecks($ogrouptab->check());

                    /*
                     * FIXME: collect failure statistics here!!!
                     */

                    $this->failure_in_this_phase = true;
                } else {
                    /* Save group object to LDAP */
                    $ogrouptab->save();
                }
            }
        }
    }


    function genAccountBase($user_data, $groups_data = array())
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
                     * We take the first class group we find…
                     * Presuming that students only have _one_ class group assigned(!)
                     */
                    break;
                }
            }
            $template_base = $_class_group . $template_base;
        }
        return $template_base;
    }


    function prepareGroupMemberships()
    {
        /*
         * Collect group memberships from user accounts and store the information
         * in separate arrays (one for posixGroups, one for gosaGroupOfNames)
         */

        $this->posixgroup_members   = array();
        $this->groupofnames_members = array();

        foreach ($this->csvinfo['data_preldap'] as $idx => $user_data) {
            if ((isset($user_data['main_account']) and (!empty($user_data['main_account'])))) {
                $_groups = $user_data['groups'];

                // Process memberships in optional groups, only for groups that exist
                if (isset($user_data['optional_groups'])) {
                    foreach ($user_data['optional_groups'] as $opt_group) {
                        if (
                            isset($opt_group['cn'][0]) and
                            ($this->utils->strcontains($user_data['optional_groups'][$opt_group['cn'][0]]['_status'][0], 'exists'))
                        ) {
                            $_groups[$opt_group['cn'][0]] = $opt_group;
                        }
                    }
                }

                foreach ($_groups as $group_key => $group_data) {
                    if (
                        in_array('posixGroup', $group_data['objectClass']) and
                        ($this->utils->strcontains($user_data['main_account']['_group_actions'][0], 'update-memberships')) and
                        ($user_data['main_account']['_group_actions'][$group_key] === "add")
                    ) {
                        $_uid = $user_data['main_account']['uid'][0];
                        if (!in_array($group_key, array_keys($this->posixgroup_members))) {
                            $this->posixgroup_members[$group_key] = array();
                        }
                        $this->posixgroup_members[$group_key][] = $_uid;
                    }

                    if (
                        in_array('gosaGroupOfNames', $group_data['objectClass']) and
                        ($this->utils->strcontains($user_data['main_account']['_ogroup_actions'][0], 'update-memberships')) and
                        ($user_data['main_account']['_ogroup_actions'][$group_key] === "add")
                    ) {
                        if (isset($user_data['main_account']['_dn_ldap'])) {
                            $this->groupofnames_members[$group_key][] = $user_data['main_account']['_dn_ldap'];
                        }
                    }
                }
            }

            if (
                (isset($user_data['aux_accounts']))        and (!empty($user_data['aux_accounts'])) and
                (isset($user_data['aux_accounts_groups'])) and (!empty($user_data['aux_accounts_groups']))
            ) {
                foreach ($user_data['aux_accounts_groups'] as $group_key => $group_data) {
                    foreach ($user_data['aux_accounts'] as $aux_account) {
                        if (
                            in_array('posixGroup', $group_data['objectClass']) and
                            ($this->utils->strcontains($aux_account['_group_actions'][0], 'update-memberships')) and
                            ($aux_account['_group_actions'][$group_key] === "add")
                        ) {
                            $_uid = $aux_account['uid'][0];
                            if (!in_array($group_key, array_keys($this->posixgroup_members))) {
                                $this->posixgroup_members[$group_key] = array();
                            }
                            $this->posixgroup_members[$group_key][] = $_uid;
                        }
                        if (
                            in_array('gosaGroupOfNames', $group_data['objectClass']) and
                            isset($aux_account['_ogroup_actions'][0]) and
                            isset($aux_account['_ogroup_actions'][$group_key]) and
                            ($this->utils->strcontains($aux_account['_ogroup_actions'][0], 'update-memberships')) and
                            ($aux_account['_ogroup_actions'][$group_key] === "add")
                        ) {
                            if (!in_array($group_key, array_keys($this->groupofnames_members))) {
                                $this->groupofnames_members[$group_key] = array();
                            }
                            if (isset($aux_account['_dn_ldap'])) {
                                /*
                                 * FIXME: DN memberships are case insensitive, we need to convert
                                 *        $groupofnames_members[$group_key] items into lower case and then
                                 *        compare with strtolower($aux_account['_dn_ldap'][0]).
                                 *        Only if there is no case-insensitive match, we may add
                                 *        another member DN to this group.
                                 */

                                $this->groupofnames_members[$group_key][] = $aux_account['_dn_ldap'][0];
                            }
                        }
                    }
                }
            }
        }
    }


    function validGroupName(string $group_name): string
    {
        /*
         * FIXME: 1 This can be better done with regular expressions!!!
         * FIXME: 2 The transliteration part is locale-dependent.
         *          We tried iconv and it did not work…
         */

        $_grp = strtolower(trim($group_name));
        $_grp = str_replace('ä', 'ae', $_grp);
        $_grp = str_replace('ö', 'oe', $_grp);
        $_grp = str_replace('ü', 'ue', $_grp);
        $_grp = str_replace('ß', 'ss', $_grp);
        $_grp = str_replace('.', '', $_grp);
        $_grp = str_replace(':', '-', $_grp);
        $_grp = str_replace('(', '_', $_grp);
        $_grp = str_replace(')', '', $_grp);
        $_grp = str_replace(' ', '_', $_grp);

        if (preg_match('/^[a-z][-a-z0-9_]*[a-z0-9]$/', $_grp)) {
            return $_grp;
        } else {
            return "";
        }
    }


    function randomAlphaNumPassword(int $length): string
    {
        $chars = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ0123456789";
        $chars_max = strlen($chars) - 1;
        $pass = "";
        for ($i = 0; $i < $length; $i++) {
            $pass .= $chars[random_int(0, $chars_max)];
        }
        return $pass;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
