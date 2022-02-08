<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2015 Mike Gabriel
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


class schoolmgr extends plugin
{
    /* Definitions */
    var $plHeadline= "School Manager";
    var $plDescription= "Administration Utility for Schools";
    var $plIcon = "plugins/schoolmanager/images/schoolmgr.png";

    /* attribute list for save action */
    var $attributes= array();
    var $objectclasses= array();
    var $schoolmgr;

    function __construct(&$config)
    {
        $this->initTime = microtime(TRUE);

        stats::log('plugin', $class = get_class($this), $category = array($this->acl_category),  $action = 'open',
                $amount = 1, $duration = (microtime(TRUE) - $this->initTime));


        /* Include config object */
        $this->schoolmgr= new schoolmanagertab($config, $config->data['TABS']['SCHOOLMANAGERTAB'], "");

    }

    function execute()
    {
        /* Call parent execute */
        plugin::execute();

        /* Show main page */
        return ($this->schoolmgr->execute());
    }

    /* Return plugin informations for ACL handling */
    static function plInfo() {
        return (
            array(
                "plShortName"    => _("School Manager"),
                "plDescription"  => _("Administration Utility for Schools"),
                "plSelfModify"   => FALSE,
                "plDepends"      => array(),
                "plPriority"     => 1,
                "plSection"      => array("addon"),
                "plCategory"     => array("schoolmgr" => array("description" => _("School Manager"))),
                "plProperties"   => array(
                    array(
                        "name" => "csv_column_delimiter",
                        "type" => "string",
                        "default" => ";",
                        "description" => _("The default delimiter for CSV columns."),
                        "check" => "gosaProperty::isString",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "accounts_in_class_ou",
                        "type" => "bool",
                        "default" => "true",
                        "description" => _("Create a sub-OU for each class and place student " .
                                           "accounts belonging to the same class into their " .
                                           "corresponding class OU?"),
                        "check" => "gosaProperty::isBool",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "try_mail_as_uid",
                        "type" => "bool",
                        "default" => "true",
                        "description" => _("If CSV data does not contain a user ID (uid) " .
                                           "column, use the mail address as user ID instead?"),
                        "check" => "gosaProperty::isBool",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "ignore_first_row_of_csv_file",
                        "type" => "bool",
                        "default" => "true",
                        "description" => _("The first row of the CSV file contains column heads."),
                        "check" => "gosaProperty::isBool",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "ou_for_new_groups",
                        "type" => "dn",
                        "default" => "ou=SchoolManager,dc=skole,dc=skolelinux,dc=no",
                        "description" => _("The OU for new groups."),
                        "check" => "gosaProperty::isDn",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "domain_for_new_groups",
                        "type" => "string",
                        "default" => "@intern",
                        "description" => _("In what mail domain should these new groups be?"),
                        "check" => "gosaProperty::isString",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "template_for_new_students-importstudentsonly",
                        "type" => "dn",
                        "default" => "uid=newstudent,ou=people,ou=Students,dc=skole,dc=skolelinux,dc=no",
                        "description" => _("Select a template for student accounts."),
                        "check" => "gosaProperty::isDn",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "template_for_new_students-importstudentsandparents",
                        "type" => "dn",
                        "default" => "uid=newstudent,ou=people,ou=Students,dc=skole,dc=skolelinux,dc=no",
                        "description" => _("Select a template for student accounts."),
                        "check" => "gosaProperty::isDn",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "template_for_new_parents-importstudentandparents",
                        "type" => "dn",
                        "default" => "uid=newparent,ou=people,ou=Parents,dc=skole,dc=skolelinux,dc=no",
                        "description" => _("Select a template for parents accounts."),
                        "check" => "gosaProperty::isDn",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                    array(
                        "name" => "template_for_new_teachers-importteachers",
                        "type" => "dn",
                        "default" => "uid=newteacher,ou=people,ou=Teacher,dc=skole,dc=skolelinux,dc=no",
                        "description" => _("Select a template for teacher accounts."),
                        "check" => "gosaProperty::isDn",
                        "migrate" => "",
                        "group" => "schoolmgr",
                        "mandatory" => TRUE
                    ),
                ),
                "plProvidedAcls" => array(),
            )
        );
    }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
