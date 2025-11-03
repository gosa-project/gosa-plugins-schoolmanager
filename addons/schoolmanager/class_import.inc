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


class import extends plugin
{
    // GOsa² definitions
    var $plHeadline = "Manage Accounts";
    var $plDescription = "GOsa2 School Manager base class for managing accounts";
    var $access = "";

    // Array with csv informations
    var $csvinfo = array();

    // Attribute list for save action
    var $attributes = array();
    var $objectclasses = array();
    var $view_logged = false;

    // Indicates if a phase failed for whatever reason
    var $failure_in_this_phase = false;

    // Class name without 'import' at the beginning
    var $import_account_type = "";

    // To be overriden by sub-classes.
    var $default_template_main = "";
    var $default_template_aux = "";

    // To be overriden by sub-classes.
    // Indicates if the import type has the feature 'accounts_in_class_ou' enabled.
    var $feature_accounts_in_class_ou = false;

    // Indicates if the import type has the feature 'aliases_in_schooldomain' enabled.
    var $feature_aliases_in_schooldomain = false;

    // Collection container for failure messages
    var $failure_messages = array();

    // import class constructor
    function __construct(&$config, $dn = null)
    {

        $this->initTime = microtime(true);

        /* Include config object */
        $this->config = &$config;
        $this->ui = get_userinfo();

        /* Initialize utils class */
        $this->utils = new schoolmgr_utils($config);

        stats::log(
            'plugin',
            $class    = get_class($this),
            $category = array($this->acl_category),
            $action   = 'open',
            $amount   = 1,
            $duration = (microtime(true) - $this->initTime)
        );
    }


    // To be overriden by sub-classes.
    function getAttributes()
    {
        /* CSV columns required for import */
        $attrs = array();

        $attrs[0] = "uid";
        $attrs[1] = "sn";
        $attrs[2] = "givenName";

        return $attrs;
    }


    /*
     * The next few functions should be overriden by it's sub-classes
     * like class_importstudents, class_archiveaccounts, etc…
     */
    function prepareLdapImport($csv_data_sorted)
    {
        return array();
    }
    function getMultiAttributes()
    {
        return array();
    }
    function getAttrsPreSelection($size)
    {
        return array();
    }


    function accountStatusCheck()
    {
        /* If we don't have the $this->csvinfo['data_preldap'] dataset, we are doomed to fail… */
        if (!is_array($this->csvinfo['data_preldap']) or (empty($this->csvinfo['data_preldap']))) {
            $this->failure_in_this_phase = true;

            /* FIXME: Show a proper error message here... */
            echo "FIXME: BOOM";

            return;
        }

        $this->_ldap->cd($this->config->current['BASE']);

        // Build list of LDAP attributes to test for matching existing accounts
        // The user previously selected these attributes via the form.
        $test_attrs = array();
        $match_attrs = array(
            'sel_ldap_match_attr_studentid' => 'employeeNumber',
            'sel_ldap_match_attr_snname'    => 'sn',
            'sel_ldap_match_attr_givenname' => 'givenName',
            'sel_ldap_match_attr_gender'    => 'gender',
            'sel_ldap_match_attr_birthday'  => 'dateOfBirth',
        );
        foreach ($match_attrs as $config_key => $ldap_attr) {
            if ($this->csvinfo[$config_key]) {
                $test_attrs[] = $ldap_attr;
            }
        }

        /* this will probably scale very very badly… Improvement needed. Suggestions? */
        foreach ($this->csvinfo['data_preldap'] as $key => $row) {
            if (!isset($row['main_account'])) {
                continue;
            }

            $_ldapfilter = "(objectClass=gosaAccount)";
            if (count($test_attrs) > 0) {
                $_ldapfilter = "(&" . $_ldapfilter;
                foreach ($test_attrs as $attr) {
                    if (isset($row['main_account'][$attr][0])) {
                        $_ldapfilter = $_ldapfilter . "(" . $attr . "=" . $row['main_account'][$attr][0] . ")";
                    }
                }
                $_ldapfilter = $_ldapfilter . ")";
            }

            $_arr_props = array("uid", "sn", "givenName", "gender",
                                "dateOfBirth", "mail", "alias",
                                "departmentNumber", "employeeNumber");
            $ldapsearch = $this->_ldap->search($_ldapfilter, $_arr_props);

            $gosa_main_accounts = array();
            while ($ldap_obj = $this->_ldap->fetch($ldapsearch)) {
                $gosa_main_accounts[] = $ldap_obj;
            }

            if ((is_countable($gosa_main_accounts)) and (count($gosa_main_accounts) == 0)) {
                /* let's try "fuzzy" search (only look for givenName and sn) */
                $_ldapfilter = "(&(objectClass=gosaAccount)(sn=" . $row['main_account']['sn'][0] . ")(givenName=" . $row['main_account']['givenName'][0] . "))";
                $ldapsearch = $this->_ldap->search($_ldapfilter, array("uid", "sn", "givenName", "gender", "dateOfBirth", "mail", "alias", "departmentNumber", "employeeNumber"));

                $gosa_main_accounts = array();
                while ($ldap_obj = $this->_ldap->fetch($ldapsearch)) {
                    $gosa_main_accounts[] = $ldap_obj;
                }

                if ((is_countable($gosa_main_accounts)) and (count($gosa_main_accounts) == 1)) {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'similar-object-found';
                } elseif ((is_countable($gosa_main_accounts)) and (count($gosa_main_accounts) > 1)) {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'similar-objects-found';
                }
                $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = 'ignore,check-manually';
            } elseif ((is_countable($gosa_main_accounts)) and (count($gosa_main_accounts) == 1)) {
                $gosa_account = $gosa_main_accounts[0];

                // Detect status of account.
                if ($this->utils->compareObjects($row['main_account'], $gosa_account, $test_attrs, "", true) === null) {
                    if ((!isset($row['main_account']['uid'][0])) or ($row['main_account']['uid'][0] === "{%uid}")) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['uid'] = array($gosa_account['uid'][0]);
                        $row['main_account']['uid'] = $this->csvinfo['data_preldap'][$key]['main_account']['uid'];
                    }
                    $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
                    $expected_account_dn = "uid=" . $row['main_account']['uid'][0] . ',' . get_people_ou() . $expected_base;
                    $expected_account_uid = $row['main_account']['uid'][0];
                    $gosa_account_dn = $gosa_account['dn'];

                    /* Explode into array, pick first then get the part after uid= */
                    $gosa_account_uid = explode(",", $gosa_account_dn, 2);
                    $gosa_account_uid = $gosa_account_uid[0];
                    $gosa_account_uid = substr($gosa_account_uid, strpos($gosa_account_uid, "=") + 1);

                    /* Test if base is equal */
                    if (strtolower($gosa_account_dn) == strtolower($expected_account_dn)) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'exists';
                    }
                    /* Test if UID needs to be renamed otherwise it exists somewhere else */ elseif (strtolower($gosa_account_uid) == strtolower($expected_account_uid)) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'exists-elsewhere';
                    } else {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'needs-renaming';
                    }

                    /* detect actions */
                    $_actions = array();
                    if ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] === 'exists-elsewhere') {
                        $_actions[] = "move";
                    } elseif ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] === 'needs-renaming') {
                        $_actions[] = "rename-uid";
                    }

                    /*
                     * Only update attributes that are already set in the main account dataset
                     * People should be able to change everything. For example because of a typo or a new name.
                     */
                    $_updatable_attrs = array();
                    foreach (array("mail", "employeeNumber", "departmentNumber", "sn", "givenName", "gender", "dateOfBirth") as $_attr) {
                        if (!in_array($_attr, $row['main_account'])) {
                            continue;
                        }

                        $_value = $row['main_account'][$_attr];

                        if (!isset($_value) || empty($_value) || empty($_value[0])) {
                            continue;
                        }

                        $_updatable_attrs[] = $_attr;
                    }

                    $_check_attrs = $this->utils->compareObjects($row['main_account'], $gosa_account, $_updatable_attrs, "update-");
                    if ($_check_attrs === null) {
                        // Nothing to do
                    } else {
                        $_actions = array_merge($_actions, $_check_attrs);
                    }

                    /*
                        * Obtain the alias field from the LDAP object if present.
                        * This is required for showing the aliases in the user property overview page.
                        */
                    if (isset($gosa_account['alias'])) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['alias'] = $gosa_account['alias'];
                    }

                    /* Check if the alias LDAP attribute needs to be modified. */

                    $_alias = $row['main_account']['uid'][0] . "@" . $this->csvinfo['domain_school'];
                    if ($this->csvinfo['aliases_in_schooldomain']) {
                        if (isset($gosa_account['alias']) and (in_array($_alias, $gosa_account['alias']))) {
                            /* nothing to do */
                        } elseif (isset($gosa_account['mail']) and (in_array($_alias, $gosa_account['mail']))) {
                            /* nothing to do either, <uid>@<domain_school> is primary mail address */
                        } else {
                            $_actions[] = 'add-uid-at-domain-to-aliases';
                        }
                    } else {
                        if (isset($gosa_account['alias']) and (in_array($_alias, $gosa_account['alias']))) {
                            $_actions[] = 'drop-uid-at-domain-from-aliases';
                        }
                    }

                    /* Don't allow primary mail address ending up in 'alias' attribute description */
                    if (
                        isset($gosa_account['mail'][0]) and isset($gosa_account['alias']) and
                        isset($row['main_account']['mail'][0]) and
                        (in_array($row['main_account']['mail'][0], $gosa_account['alias']))
                    ) {
                        $_actions[] = 'drop-uid-at-domain-from-aliases';
                    }

                    if (empty($_actions)) {
                        $_actions[] = 'none';
                    }

                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = implode(",", $_actions);

                    $this->csvinfo['data_preldap'][$key]['main_account']['_dn_ldap'] = array($gosa_account_dn);
                    $this->csvinfo['data_preldap'][$key]['main_account']['_dn'] = array($expected_account_dn);
                    $this->csvinfo['data_preldap'][$key]['main_account']['_base'] = array($expected_base);
                } elseif ($this->utils->compareObjects($row['main_account'], $gosa_account, array("uid"), "", true) === null) {
                    // The LDAP tree already has an account with this UID, this requires manual introspection

                    $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
                    $expected_account_dn = "uid=" . $row['main_account']['uid'][0] . ',' . get_people_ou() . $expected_base;
                    $gosa_account_dn = $gosa_account['dn'];

                    if (strtolower($gosa_account_dn) == strtolower($expected_account_dn)) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'uid-used-in-same-ou';
                    } else {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'uid-used-in-other-ou';
                    }
                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = 'ignore,check-manually';
                }
            } elseif ((is_countable($gosa_main_accounts)) and (count($gosa_main_accounts) > 1)) {
                msg_dialog::display(_("LDAP Tree Error"), _("The LDAP search query with filter '" . $_ldapfilter . "' returned more than one account object. There should only be one such object in the LDAP tree."), ERROR_DIALOG);
            } elseif ((!is_countable($gosa_main_accounts))) {
                msg_dialog::display(_("LDAP Error"), _("The LDAP search query with filter '.$_ldapfilter.' returned with an unknown error."), ERROR_DIALOG);
            }

            if (!isset($row['aux_accounts'])) {
                continue;
            }

            foreach ($row['aux_accounts'] as $idx_aux => $aux_account) {
                /*
                    * Don't perform on this aux-user dataset if:
                    *   * _actions has been set to something containing 'ignore' which can mean:
                    *     - parent1 and parent2 have the same email AND
                    *     - aux-user is parent2
                    */
                if ($this->utils->strcontains($aux_account['_status'][0], 'same-mail-as-parent1')) {
                    $aux_account['_actions'] = array('ignore,combine-with-parent1');
                    continue;
                }


                // $gosa_aux_accounts still holds old values..
                unset($gosa_aux_accounts);

                if (isset($aux_account['mail'][0])) {
                    $_ldapfilter = "(&(objectClass=gosaAccount)(uid=" . $aux_account['mail'][0] . "))";
                    $ldapsearch = $this->_ldap->search($_ldapfilter, array("sn", "givenName", "mail"));


                    $gosa_aux_accounts = array();
                    while ($ldap_obj = $this->_ldap->fetch($ldapsearch)) {
                        $gosa_aux_accounts[] = $ldap_obj;
                    }
                } else {
                    // E-Mail is not set. Skip.
                    continue;
                }


                if ((is_countable($gosa_aux_accounts)) and (count($gosa_aux_accounts) == 0)) {
                    // No account was found using this email address. So just create the account.
                } elseif ((is_countable($gosa_aux_accounts)) and (count($gosa_aux_accounts) == 1)) {
                    // We only found one account, assume it's at 0th place.
                    $gosa_account = $gosa_aux_accounts[0];

                    $expected_base = $this->genAccountBase($aux_account);
                    $gosa_account_dn = $gosa_account['dn'];

                    /* Detect status */

                    /*
                        * FIXME: Support automatic moving of AUX accounts!!!
                        */

                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0] = 'exists';

                    $_actions = array();

                    /*
                        * FIXME: Hard-code a "none" action here in case we found a matching account in LDAP.
                        *        Later, we need to make actions more configurable…
                        */

                    // Only update attributes that are already set in the main account dataset
                    $_updatable_attrs = array();
                    foreach (array("sn", "givenName") as $_attr) {
                        if (isset($aux_account[$_attr])) {
                            $_updatable_attrs[] = $_attr;
                        }
                    }

                    $_check_attrs = $this->utils->compareObjects($aux_account, $gosa_account, $_updatable_attrs, "update-");
                    if ($_check_attrs === null) {
                        // Nothing to do
                    } else {
                        $_actions = array_merge($_actions, $_check_attrs);
                    }

                    if (empty($_actions)) {
                        $_actions[] = 'none';
                    }

                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0] = implode(",", $_actions);

                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_dn_ldap'] = array($gosa_account_dn);

                    /*
                        * FIXME: We need a check that DN generation for aux accounts will actually work, probably when
                        *        constructing $this->csvinfo['data_preldap'].
                        */
                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_base'] = array($expected_base);
                } elseif ((is_countable($gosa_aux_accounts)) and (count($gosa_aux_accounts) > 1)) {
                    msg_dialog::display(_("LDAP Tree Error"), _("The LDAP search query with filter '" . $_ldapfilter . "' returned more than one account object. There should only be one such object in the LDAP tree."), ERROR_DIALOG);
                } elseif ((!is_countable($gosa_aux_accounts))) {
                    msg_dialog::display(_("LDAP Error"), _("The LDAP search query with filter '.$_ldapfilter.' returned with an unknown error."), ERROR_DIALOG);
                }
            }
        }

        // Post-processing, probably appendable to above code block…
        foreach ($this->csvinfo['data_preldap'] as $key => $row) {
            if (isset($row['main_account'])) {
                if ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] == 'unchecked') {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]  = 'not-found';
                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = 'create,set-password';

                    // Generate passwords for main accounts (if not provided)
                    if (
                        (!isset($row['main_account']['userPassword'][0])) or
                        ($row['main_account']['userPassword'][0] === "")  or
                        ($row['main_account']['userPassword'][0] === '{%userPassword}')
                    ) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] .= ",password-generated";
                        $this->csvinfo['data_preldap'][$key]['main_account']['userPassword'][0] = $this->randomAlphaNumPassword(12);
                    }

                    // Set the main account's base for object creation
                    $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
                    if (isset($row['main_account']['uid'][0])) {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn'] = array("uid=" . $row['main_account']['uid'][0] . "," . get_people_ou() . $expected_base);
                    } else {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn'] = array("new");
                    }
                    $this->csvinfo['data_preldap'][$key]['main_account']['_base'] = array($expected_base);
                }

                if (isset($row['aux_accounts'])) {
                    foreach ($row['aux_accounts'] as $idx_aux => $aux_account) {
                        if ($this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0] == 'unchecked') {
                            if (isset($aux_account['sn']) and isset($aux_account['mail']) and isset($aux_account['givenName'])) {
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0] = 'not-found';
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0] = 'create,set-password,skip-aliases';
                                /* set the main account's base for object creation */
                                $expected_base = $this->genAccountBase($aux_account);
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_base'] = array($expected_base);
                            } else {
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0] = 'data-incomplete';
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0] = 'ignore';
                            }

                            // generate passwords for aux accounts (if not provided)
                            if (
                                (!isset($aux_account['userPassword'][0])) or
                                ($aux_account['userPassword'][0] === "") or
                                ($aux_account['userPassword'][0] === '{%userPassword}')
                            ) {
                                if ($this->utils->strcontains($this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0], 'create')) {
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0] .= ",password-generated";
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['userPassword'][0] = $this->randomAlphaNumPassword(12);
                                } else {
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['userPassword'] = array("<keep>");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
