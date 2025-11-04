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

// Make Account objects available.
require_once "SchoolManager/Objects/Account.inc";
require_once "SchoolManager/Objects/Guardian.inc";
require_once "SchoolManager/Objects/Student.inc";
require_once "SchoolManager/Objects/Teacher.inc";

class schoolmgr extends plugin
{
    /* Definitions */
    var $plHeadline = "School Manager";
    var $plDescription = "Administration Utility for Schools";
    var $plIcon = "plugins/schoolmanager/images/schoolmgr.png";
    var $matIcon = "school";

    /* attribute list for save action */
    var $attributes = array();
    var $objectclasses = array();
    var $schoolmgr;

    function __construct(&$config)
    {
        $this->initTime = microtime(true);

        stats::log(
            'plugin',
            $class    = get_class($this),
            $category = array($this->acl_category),
            $action   = 'open',
            $amount   = 1,
            $duration = (microtime(true) - $this->initTime)
        );


        // Include config object
        $this->schoolmgr = new schoolmanagertab($config, $config->data['TABS']['SCHOOLMANAGERTAB'], "");
    }


    function execute()
    {
        /* Call parent execute */
        plugin::execute();

        /* Show main page */
        return ($this->schoolmgr->execute());
    }


    /* Return plugin informations for ACL handling */
    static function plInfo()
    {
        // Get properties from below.
        $properties = schoolmgr::getProperties();

        return (array(
                "plShortName"    => _("School Manager"),
                "plDescription"  => _("Administration Utility for Schools"),
                "plSelfModify"   => false,
                "plDepends"      => array(),
                "plPriority"     => 1,
                "plSection"      => array("addon"),
                "plCategory"     => array("schoolmgr" => array("description" => _("School Manager"))),
                "plProperties"   => $properties,
                "plProvidedAcls" => array(),
            )
        );
    }

    static function getProperties()
    {
        $matching_attr_arr = array(
            array(
                "name"        => "ldap_match_attr_studentid",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "false",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should this attribute be used to identify users " .
                                   "from the CSV file as existing users in the LDAP-tree?"),
            ),
            array(
                "name"        => "ldap_match_attr_givenname",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should this attribute be used to identify users " .
                                   "from the CSV file as existing users in the LDAP-tree?"),
            ),
            array(
                "name"        => "ldap_match_attr_snname",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should this attribute be used to identify users " .
                                   "from the CSV file as existing users in the LDAP-tree?"),
            ),
            array(
                "name"        => "ldap_match_attr_birthday",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "false",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should this attribute be used to identify users " .
                                   "from the CSV file as existing users in the LDAP-tree?"),
            ),
            array(
                "name"        => "ldap_match_attr_gender",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should this attribute be used to identify users " .
                                   "from the CSV file as existing users in the LDAP-tree?"),
            ),
        );

        $ret_arr = array(
            // Gets loaded as defaults for phase 1…
            array(
                "name"        => "csv_column_delimiter",
                "type"        => "string",
                "check"       => "gosaProperty::isString",
                "group"       => "schoolmgr",
                "default"     => ";",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The default delimiter for CSV file columns."),
            ),
            array(
                "name"        => "ignore_first_row_of_csv_file",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The first row of the CSV file contains column heads."),
            ),
            // Gets loaded as defaults for phase 2…
            array(
                "name"        => "aliases_in_schooldomain",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should aliases be created for teachers using the school domain? " .
                                   "For example: <uid>@<domain_school>"),
            ),
            array(
                "name"        => "add_course_members_to_class_group",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Should teachers be included as class members for courses they teach in a class?"),
            ),
            array(
                "name"        => "accounts_in_class_ou",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Create a sub-OU for each class and place student " .
                                   "accounts belonging to the same class into their " .
                                   "corresponding class OU?"),
            ),
            array(
                "name"        => "domain_groups",
                "type"        => "string",
                "check"       => "gosaProperty::isString",
                "group"       => "schoolmgr",
                "default"     => "intern", // Without leading '@'
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The groups domain. Example: 'lists.john-doe-school.edu'"),
            ),
            array(
                "name"        => "domain_users",
                "type"        => "string",
                "check"       => "gosaProperty::isString",
                "group"       => "schoolmgr",
                "default"     => "intern", // Without leading '@'
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The users (pupil) domain. Example: 'pupils.john-doe-school.edu'"),
            ),
            array(
                "name"        => "domain_school",
                "type"        => "string",
                "check"       => "gosaProperty::isString",
                "group"       => "schoolmgr",
                "default"     => "intern", // Without leading '@'
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The school domain. Example: 'john-doe-school.edu'"),
            ),
            array(
                "name"        => "try_mail_as_uid",
                "type"        => "bool",
                "check"       => "gosaProperty::isBool",
                "group"       => "schoolmgr",
                "default"     => "true",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("If CSV data does not contain a user ID (uid) " .
                                   "column, use the mail address as user ID instead?"),
            ),
            array(
                "name"        => "ou_for_new_groups",
                "type"        => "dn",
                "check"       => "gosaProperty::isDn",
                "group"       => "schoolmgr",
                "default"     => "ou=SchoolManager,dc=skole,dc=skolelinux,dc=no",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("The OU for new groups."),
            ),
            array(
                "name"        => "template_for_new_students-importstudentsonly",
                "type"        => "dn",
                "check"       => "gosaProperty::isDn",
                "group"       => "schoolmgr",
                "default"     => "uid=newstudent,ou=people,ou=Students,dc=skole,dc=skolelinux,dc=no",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Import students only: Select a template for student accounts."),
            ),
            array(
                "name"        => "template_for_new_students-importstudentsandparents",
                "type"        => "dn",
                "check"       => "gosaProperty::isDn",
                "group"       => "schoolmgr",
                "migrate"     => "",
                "default"     => "uid=newstudent,ou=people,ou=Students,dc=skole,dc=skolelinux,dc=no",
                "mandatory"   => true,
                "description" => _("Import students and parents: Select a template for student accounts."),
            ),
            array(
                "name"        => "template_for_new_parents-importstudentandparents",
                "type"        => "dn",
                "check"       => "gosaProperty::isDn",
                "group"       => "schoolmgr",
                "default"     => "uid=newparent,ou=people,ou=Parents,dc=skole,dc=skolelinux,dc=no",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Import students and parents: Select a template for parents accounts."),
            ),
            array(
                "name"        => "template_for_new_teachers-importteachers",
                "type"        => "dn",
                "check"       => "gosaProperty::isDn",
                "group"       => "schoolmgr",
                "default"     => "uid=newteacher,ou=people,ou=Teacher,dc=skole,dc=skolelinux,dc=no",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Import teachers: Select a template for teacher accounts."),
            ),
            // Async import configuration
            array(
                "name"        => "async_import_timeout",
                "type"        => "integer",
                "check"       => "gosaProperty::isInteger",
                "group"       => "schoolmgr",
                "default"     => "10",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Timeout in seconds for checking if async job is idle."),
            ),
            array(
                "name"        => "async_import_refresh_time",
                "type"        => "integer",
                "check"       => "gosaProperty::isInteger",
                "group"       => "schoolmgr",
                "default"     => "3",
                "migrate"     => "",
                "mandatory"   => true,
                "description" => _("Page refresh interval in seconds while async job is running."),
            ),
        );

        return array_merge($ret_arr, $matching_attr_arr);
    }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
