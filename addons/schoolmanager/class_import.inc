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
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
