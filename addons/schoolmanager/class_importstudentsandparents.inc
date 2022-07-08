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


class importstudentsandparents extends importaccounts
{
    /* Definitions */
    var $plHeadline = "Import Students and Parents";
    var $plDescription = "GOsa2 School Manager Module: Import Students and Parents";
    var $access = "";

    var $default_template_main = "student";
    var $default_template_aux = "parent";

    /* constructor */
    function __construct(&$config, $dn = null)
    {

        $this->initTime = microtime(true);

        /* Include config object */
        $this->config = &$config;
        $this->ui = get_userinfo();
        stats::log('plugin', $class = get_class($this), $category = array ($this->acl_category), $action = 'open', $amount = 1, $duration = (microtime(true) - $this->initTime));

        /* Initialize utils class */
        $this->utils = new schoolmgr_utils($config);

        parent::__construct($config, $dn = null);

        // Overrides parent class default.
        // Indicates if the import type has the feature 'accounts_in_class_ou' enabled.
        $this->feature_accounts_in_class_ou = true;
    }

    function getAttributes()
    {
        $attrs = array();

        $attrs[] = "no";
        $attrs[] = "uid";
        $attrs[] = "userPassword";
        $attrs[] = "employeeNumber";
        $attrs[] = "sn";
        $attrs[] = "givenName";
        $attrs[] = "dateOfBirth";
        $attrs[] = "gender";
        $attrs[] = "class";
        $attrs[] = "parent1_sn";
        $attrs[] = "parent1_givenName";
        $attrs[] = "parent1_mail";
        $attrs[] = "parent2_sn";
        $attrs[] = "parent2_givenName";
        $attrs[] = "parent2_mail";
        $attrs[] = "course";
        /* FIXME: allow "mail" as possible attribute...
         *        If "mail" is given, it should override the default value from the GOsa user
         *        template (if provided there).
         */

        return $attrs;
    }

    function getMultiAttributes()
    {
        $multiAttrs = array();
        $multiAttrs[] = "course";
        return $multiAttrs;
    }

    function getAttrsPreSelection($size)
    {

        /* FIXME: Make this more generic by analyzing the return value of $this->getAttributes()
         *        When done -> move method to parent class (importaccounts).
         */

        $selection = array();
        for ($i = 0; $i < 15; $i++) {
             $selection[] = $i;
        }
        for ($i = 15; $i < $size; $i++) {
             $selection[] = 15;
        }
        return $selection;
    }


    function prepareLdapImport($csv_data_sorted)
    {
        /* Get element */
        $data_preldap = array();

        foreach ($csv_data_sorted as $idx => $row_sorted) {
            if ($this->CSVRowSanityChecksOk($row_sorted, $idx)) {
                /* main account */
                $student = array ();
                if (isset($row_sorted['uid']) && $row_sorted['uid'] && (($row_sorted['uid'] != "%auto%") && ($row_sorted['uid'] != "%uid%") && ($row_sorted['uid'] != "%{uid}"))) {
                    $student['uid'] = array(strtolower(iconv('UTF-8', 'US-ASCII//TRANSLIT', trim($row_sorted['uid']))));
                } else {
                    $student['uid'] = array('{%uid}');
                }
                if (isset($row_sorted['userPassword']) && $row_sorted['userPassword'] && (($row_sorted['userPassword'] != "%auto%") && ($row_sorted['userPassword'] != "%password%"))) {
                    $student['userPassword'] = array($row_sorted['userPassword']);
                } else {
                    $student['userPassword'] = array('{%userPassword}');
                }
                $student['sn'] = array (trim($row_sorted['sn']));
                $student['givenName'] = array (trim($row_sorted['givenName']));

                if (isset($row_sorted['dateOfBirth']) and $row_sorted['dateOfBirth']) {
                    $_dateOfBirth = trim($row_sorted['dateOfBirth']);
                    /* FIXME 1: On a German localized GOsa², this is not required..., but how about other locales???
                     * FIXME 2: If date is formatted like "dd.mm.yy" GOsa² will not use yy as the birthyear but print error
                     */
                    if (strpos($_dateOfBirth, '.')) {
                        list($day, $month, $year) = explode(".", $_dateOfBirth, 3);
                        $_dateOfBirth = $year . "-" . $month . "-" . $day;
                        //$_dateOfBirth = $day.".".$month.".20".$year;
                    }
                    $student['dateOfBirth'] = array ($_dateOfBirth);
                }
                if (isset($row_sorted['gender']) and $row_sorted['gender']) {
                    $student['gender'] = array ($this->utils->normalizeGenderString(trim($row_sorted['gender'])));
                }
                if (isset($row_sorted['employeeNumber']) and $row_sorted['employeeNumber']) {
                    $student['employeeNumber'] = array($row_sorted['employeeNumber']);
                }
                if (isset($row_sorted['mail']) and $row_sorted['mail']) {
                    /* FIXME: Check for valid e-Mail format here */
                    $student['mail'] = array($row_sorted['mail']);
                }
                /* Keeping the memory footprint low:
                 *
                 *     $student['_template']
                 * is only an int value.
                 *
                 * Retrieve the actual template DN string later like this
                 * $template_dn = $this->csvinfo['templates']['DNs'][$student['_template']];
                 */
                $student['_template'] = array($this->csvinfo['template_main']);
                $student['_status'] = array('unchecked');
                $student['_actions'] = array('none');
                $student['_group_actions'] = array('none');

                /* auxiliary accounts */
                $parent1 = array ();
                if (isset($row_sorted['parent1_sn']) and ($row_sorted['parent1_sn'] !== "")) {
                    $parent1['sn'] = array (trim($row_sorted['parent1_sn']));
                }
                if (isset($row_sorted['parent1_givenName']) and ($row_sorted['parent1_givenName'] !== "")) {
                    $parent1['givenName'] = array (trim($row_sorted['parent1_givenName']));
                }

                if (isset($row_sorted['parent1_mail']) and $row_sorted['parent1_mail']) {
                    /* FIXME: Check for valid e-Mail format here */
                    $parent1['mail'] = array (trim($row_sorted['parent1_mail']));
                }

                $parent1['userPassword'] = array("{%userPassword}");
                /* see above for how to retrieve the template's DN */
                $parent1['_template'] = array($this->csvinfo['template_aux']);
                $parent1['_status'] = array('unchecked');
                $parent1['_actions'] = array('none');
                $parent1['_group_actions'] = array('none');

                $parent2 = array ();
                if (isset($row_sorted['parent2_sn']) and ($row_sorted['parent2_sn'] !== "")) {
                    $parent2['sn'] = array (trim($row_sorted['parent2_sn']));
                }
                if (isset($row_sorted['parent2_givenName']) and ($row_sorted['parent2_givenName'] !== "")) {
                    $parent2['givenName'] = array (trim($row_sorted['parent2_givenName']));
                }

                if (isset($row_sorted['parent2_mail']) and $row_sorted['parent2_mail']) {
                    /* FIXME: Check for valid e-Mail format here */
                    $parent2['mail'] = array (trim($row_sorted['parent2_mail']));
                }

                $parent2['userPassword'] = array("{%userPassword}");
                /* see above for how to retrieve the template's DN */
                $parent2['_template'] = array(trim($this->csvinfo['template_aux']));
                $parent2['_status'] = array('unchecked');
                $parent2['_actions'] = array('none');
                $parent2['_group_actions'] = array('none');

                // If both parents email addresses are the same, handle parents as ONE gosa-account.
                if (
                    (isset($row_sorted['parent1_mail']) and $row_sorted['parent1_mail']) and
                    (isset($row_sorted['parent2_mail']) and $row_sorted['parent2_mail']) and
                    (isset($row_sorted['parent1_sn']) and ($row_sorted['parent1_sn'] !== "")) and
                    (isset($row_sorted['parent2_sn']) and ($row_sorted['parent2_sn'] !== "")) and
                    (isset($row_sorted['parent1_givenName']) and ($row_sorted['parent1_givenName'] !== "")) and
                    (isset($row_sorted['parent2_givenName']) and ($row_sorted['parent2_givenName'] !== ""))
                ) {
                    if (strtolower(trim($row_sorted['parent1_mail'])) === strtolower(trim($row_sorted['parent2_mail']))) {
                        // Do not import second parent account.
                        $parent2['_status']      = array('same-mail-as-parent1');

                        // givenName: Jon & Jane
                        // second name: Doe & Smith  OR (if both have same the name):
                        // second name: Doe
                        if (strtolower(trim($row_sorted['parent1_sn'])) === strtolower(trim($row_sorted['parent2_sn']))) {
                            $parent_surname = trim($row_sorted['parent1_sn']);
                        } else {
                            $parent_surname = trim($row_sorted['parent1_sn']) . " & " . trim($row_sorted['parent2_sn']);
                        }

                        // Always indicate that this account represents two real people (if both have the same name, it would be something like Jon & Jon)
                        $parent_givenName = trim($row_sorted['parent1_givenName']) . " & " . trim($row_sorted['parent2_givenName']);

                        // *Parent1* represents two real people now. (since both have the same email address.)
                        $parent1['sn'] = array($parent_surname);
                        $parent1['givenName'] = array($parent_givenName);
                    }
                }

                /*
                 * Groups for this account:
                 */

                /* class group */
                $class_group = array ();
                if (isset($row_sorted['class']) && $row_sorted['class']) {
                    /* parse value for attribute "class", requires later work to make it more generic */
                    $tmpclassparts = explode(".", strtolower($row_sorted['class']));
                    $tmpclassparts[0] = trim($tmpclassparts[0]);
                    if (isset($tmpclassparts[1])) {
                        $tmpclassparts[1] = trim($tmpclassparts[1]);
                        $tmpclassparts[1] = substr($tmpclassparts[1], 1);
                    }
                    $row_sorted['class'] = $tmpclassparts[0];
                    if (isset($tmpclassparts[1])) {
                        $row_sorted['class'] = $row_sorted['class'] . $tmpclassparts[1];
                    }

                    $class_group['cn'] = array($this->validGroupName('class_' . $row_sorted['class']));
                    $class_group['objectClass'] = array(
                        'posixGroup',
                    );
                    $class_group['description'] = array(sprintf(_('SchoolManager class group %s'), $class_group['cn'][0]));
                    $class_group['mail'] = array($class_group['cn'][0] . '@' . $this->csvinfo['domain_groups']);
                    $class_group['_status'] = array('unchecked');
                    $class_group['_actions'] = array('none');
                }

                /* course groups */
                $course_groups = array ();
                foreach ($row_sorted as $key => $value) {
                    if (substr($key, 0, 6) === "course") {
                        $row_sorted[$key] = $this->validGroupName('course_' . $value);
                        if ($row_sorted[$key]) {
                            $_cn = $row_sorted[$key];
                            $course_groups[$_cn] = array(
                                'objectClass' => array(
                                    'posixGroup',
                                ),
                                'cn' => array($_cn),
                                'description' => array(sprintf(_('SchoolManager course group %s'), $row_sorted[$key])),
                                'mail' => array($row_sorted[$key] . '@' . $this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_actions' => array('none')
                            );
                        }
                    }
                }
                $parent_group = array ();
                $parent_group['cn'] = array('parents_{%uid}');
                $parent_group['description'] = array(sprintf(_('parent group of user %s %s'), $student['givenName'][0], $student['sn'][0]));
                $parent_group['objectClass'] = array(
                    'gosaGroupOfNames'
                );

                $parent_group['member'] = array();
                if (isset($parent1['mail'])) {
                    $parent_group['member'][] = 'uid=' . strtolower($parent1['mail'][0]) . ',' . get_people_ou() . dn2base($this->csvinfo['templates']['DNs'][$this->csvinfo['template_aux']]);
                }
                if (isset($parent2['mail'])) {
                    $parent_group['member'][] = 'uid=' . strtolower($parent2['mail'][0]) . ',' . get_people_ou() . dn2base($this->csvinfo['templates']['DNs'][$this->csvinfo['template_aux']]);
                }
                $parent_group['_status'] = array('unchecked');
                $parent_group['_actions'] = array('none');
                $parent_group['_key'] = array('aux_accounts_groups');

                $groups = array();
                if (!empty($class_group)) {
                    $groups = array_merge($groups, array($class_group['cn'][0] => $class_group));
                }
                $groups = array_merge($groups, $course_groups);

                $aux_accounts_groups = array();
                if (!empty($parent_group)) {
                    $aux_accounts_groups[$parent_group['cn'][0]] = $parent_group;
                }
            }

            $_aux_accounts = array();
            if (isset($parent1['sn'][0]) and isset($parent1['givenName'][0])) {
                $_aux_accounts[] = $parent1;
            }
            if (isset($parent2['sn'][0]) and isset($parent2['givenName'][0])) {
                $_aux_accounts[] = $parent2;
            }

            $data_preldap[] = array(
                'main_account' => $student,
                'aux_accounts' => $_aux_accounts,
                'groups' => $groups,
                'aux_accounts_groups' => $aux_accounts_groups,
            );
        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
