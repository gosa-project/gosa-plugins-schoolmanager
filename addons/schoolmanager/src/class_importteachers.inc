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


class importteachers extends importaccounts
{
    /* Definitions */
    var $plHeadline = "Import Teachers";
    var $plDescription = "GOsa2 School Manager Module: Import Teachers";
    var $access = "";

    var $default_template_main = "teacher";

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
        $this->feature_accounts_in_class_ou = false;

        // Indicates if the import type has the feature 'aliases_in_schooldomain' enabled.
        $this->feature_aliases_in_schooldomain = true;
    }

    function getAttributes()
    {
        $attrs = array();

        $attrs[] = "no";
        $attrs[] = "uid";
        $attrs[] = "userPassword";
        $attrs[] = "sn";
        $attrs[] = "givenName";
        $attrs[] = "dateOfBirth";
        $attrs[] = "gender";
        $attrs[] = "mail";
        $attrs[] = "subjects";
        $attrs[] = "class";
        $attrs[] = "course";

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
        for ($i = 0; $i < 10; $i++) {
             $selection[] = $i;
        }
        for ($i = 10; $i < $size; $i++) {
             $selection[] = 10;
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
                $teacher = array ();
                if (isset($row_sorted['uid']) && $row_sorted['uid'] && (($row_sorted['uid'] != "%auto%") && ($row_sorted['uid'] != "%uid%")  && ($row_sorted['uid'] != "%{uid}") )) {
                    $teacher['uid'] = array(strtolower(iconv('UTF-8', 'US-ASCII//TRANSLIT', trim($row_sorted['uid']))));
                } else {
                    $teacher['uid'] = array('{%uid}');
                }
                if (isset($row_sorted['userPassword']) && $row_sorted['userPassword'] && (($row_sorted['userPassword'] != "%auto%") && ($row_sorted['userPassword'] != "%password%"))) {
                    $teacher['userPassword'] = array($row_sorted['userPassword']);
                } else {
                    $teacher['userPassword'] = array('{%userPassword}');
                }
                if (isset($row_sorted['subjects'])) {
                    $_subjects = trim($row_sorted['subjects']);
                    $_subjects = str_replace(",", "_", $_subjects);
                    $_subjects = str_replace(";", "_", $_subjects);
                    $teacher['departmentNumber'] = array($_subjects);
                }
                $teacher['sn']        = array (trim($row_sorted['sn']));
                $teacher['givenName'] = array (trim($row_sorted['givenName']));
                $teacher['gender']    = array ($this->utils->normalizeGenderString(trim($row_sorted['gender'])));
                $_dateOfBirth = trim($row_sorted['dateOfBirth']);

                if (isset($row_sorted['mail']) and trim($row_sorted['mail']) != "") {
                    /* FIXME: Check for valid e-Mail format here */
                    $teacher['mail'] = array(trim($row_sorted['mail']));
                } else {
                    /* FIXME: this needs to become much more configurable */
                    $teacher['mail'] = array($teacher['uid'][0] . "@" . $this->csvinfo['domain_school']);
                }
                $teacher['alias'] = array();

                /* FIXME: On a German localized GOsa², this is not required..., but how about other locales??? */
                if (strpos($_dateOfBirth, '.')) {
                    list($day, $month, $year) = explode(".", $_dateOfBirth, 3);
                    $_dateOfBirth = $year . "-" . $month . "-" . $day;
                }

                $teacher['dateOfBirth'] = array ($_dateOfBirth);
                $teacher['_template'] = array($this->csvinfo['template_main']);
                $teacher['_status'] = array('unchecked');
                $teacher['_actions'] = array('none');
                $teacher['_group_actions'] = array('none');

                 /* groups for this account */

                /* class group(s) */
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
                        $row_sorted['class'] += $row_sorted['class'] . $tmpclassparts[1];
                    }

                    $class_groups = array();
                    $_classes = str_replace(";", "_", trim($row_sorted['class']));
                    foreach (explode("_", $_classes) as $class) {
                        $_class = $this->validGroupName('class_' . $class);
                        if ($_class) {
                            $_cn = $_class;
                            $class_groups[$_cn] = array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array($_cn),
                                'description' => array(sprintf(_('SchoolManager class group %s'), $_class)),
                                'mail' => array($_class . '@' . $this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_actions' => array('none')
                            );
                        }
                    }
                }

                $subject_groups = array();
                if (isset($row_sorted['subjects']) && $row_sorted['subjects']) {
                    /* the $_subjects variable has already been declared further above */
                    foreach (explode("_", $_subjects) as $subject) {
                        $_subject = $this->validGroupName('subject_' . $subject);
                        if ($_subject) {
                            $_cn = $_subject;
                            $subject_groups[$_cn] = array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array($_cn),
                                'description' => array(sprintf(_('SchoolManager subject group %s'), $_subject)),
                                'mail' => array($_subject . '@' . $this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_actions' => array('none')
                            );
                        }
                    }
                }

                /* course groups */
                $course_groups = array ();
                $_extra_classes = array();
                foreach ($row_sorted as $key => $value) {
                    if ((substr($key, 0, 6) === "course") && trim($value)) {
                        $row_sorted[$key] = $this->validGroupName('course_' . $value);
                        if ($row_sorted[$key]) {
                            $_cn = $row_sorted[$key];
                            $course_groups[$_cn] = array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array($_cn),
                                'description' => array(sprintf(_('SchoolManager course group %s'), $row_sorted[$key])),
                                'mail' => array(trim($row_sorted[$key]) . '@' . $this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_actions' => array('none')
                            );

                            /* make teachers class members for courses they teach in a class */
                            if ($this->csvinfo['add_course_members_to_class_group']) {
                                $_class_arr = explode('-', substr($row_sorted[$key], 7));
                                array_pop($_class_arr);
                                $_class = implode('-', $_class_arr);

                                if (strpos($_class, "-") === false) {
                                    $_class = 'class_' . $_class;
                                    $_extra_classes[$_class] = array(
                                        'objectClass' => array('posixGroup'),
                                        'cn' => array($_class),
                                        '_status' => array('unchecked'),
                                        '_actions' => array('none'),
                                        '_key' => array('optional_groups'),
                                    );
                                } else {
                                    /* this code now becomes really German (Schleswig-Holstein)... */
                                    $grades = array('1','2','3','4','5','6','7','8','9','10','11','12','13','e','q1','q2');
                                    $subs = array('a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z');

                                    /* we should be able to split the $_class string into two classes (sort of...) */
                                    $from_sub = explode('-', $_class)[0];
                                    $from_grade = substr($from_sub, 0, strlen($from_sub) - 1);
                                    $from_sub = str_replace($from_grade, '', $from_sub);
                                    $to_sub = explode('-', $_class)[1];
                                    if (strlen($to_sub) === 1) {
                                        $to_grade = $from_grade;
                                    } else {
                                        $to_grade = substr($to_sub, 0, strlen($to_sub) - 1);
                                    }
                                    $to_sub = str_replace($to_grade, '', $to_sub);

                                    $from_grade_pos = array_search($from_grade, $grades);
                                    $from_sub_pos = array_search($from_sub, $subs);
                                    $to_grade_pos = array_search($to_grade, $grades);
                                    $to_sub_pos = array_search($to_sub, $subs);

                                    foreach (array_slice($grades, $from_grade_pos, ($to_grade_pos - $from_grade_pos + 1)) as $grade) {
                                        foreach (array_slice($subs, $from_sub_pos, ($to_sub_pos - $from_sub_pos + 1)) as $sub) {
                                            $_class = 'class_' . $grade . $sub;
                                            $_extra_classes[$_class] = array(
                                                'objectClass' => array('posixGroup'),
                                                'cn' => array($_class),
                                                '_status' => array('unchecked'),
                                                '_actions' => array('none'),
                                                '_key' => array('optional_groups'),
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                /* FIXME: missing group (todo) -> one group per at-school-taught subject... */

                $groups = array ();
                if (!empty($class_groups)) {
                    $groups = array_merge($groups, $class_groups);
                }
                if (!empty($course_groups)) {
                    $groups = array_merge($groups, $course_groups);
                }
                if (!empty($subject_groups)) {
                    $groups = array_merge($groups, $subject_groups);
                }

                $data_preldap[] = array(
                    'main_account' => $teacher,
                    'groups' => $groups,
                    'optional_groups' => $_extra_classes,
                );
            }
        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
