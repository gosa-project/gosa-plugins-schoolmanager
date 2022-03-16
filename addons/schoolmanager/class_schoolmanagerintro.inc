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


class schoolmanagerintro extends plugin
{
    /* Definitions */
    var $plHeadline    = "Introduction";
    var $plDescription = "GOsa2 School Manager Module: Introduction";
    var $access        = "";

    /* Attribute list for save action */
    var $attributes    = [];
    var $objectclasses = [];
    var $view_logged   = FALSE;

    /* SchoolManager specific properties */
    var $ldapinfo = [];


    function __construct(&$config, $dn= NULL)
    {
        $this->initTime = microtime(TRUE);

        /* Include config object */
        $this->config= &$config;

        // Initialize utils class
        $this->utils = new schoolmgr_utils($config);

        // Initialize smflushgroupmembers class
        $this->smflushgroupmembers = new smflushgroupmembers($config, $this->utils);

        $this->ui = get_userinfo();
        stats::log(
            'plugin',
            $class = get_class($this),
            $category = array($this-> acl_category),
            $action = 'open',
            $amount = 1,
            $duration = (microtime(TRUE) - $this->initTime),
        );
    }


    function execute()
    {
        /* Call parent execute */
        plugin::execute();

        /* Log view */
        if(!$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","all/".get_class($this),$this->dn);
        }

        /* Initialize smarty */
        $smarty= get_smarty();

        /* Initialize LDAP */
        $this->_ldap = $this->config->get_ldap_link ();
        if(!is_array($this->ldapinfo)) {
            $this->ldapinfo = [];
        }

        // Scan LDAP for ou tree when "Introduction" page is loaded and
        // remember that information…
        if(!isset($this->ldapinfo['ou_tree'])) {
            $this->ldapinfo['ou_tree'] = [];

            $this->_ldap->cd($this->config->current['BASE']);

            /* Read OU tree from LDAP */
            $ldapsearch = $this->_ldap->search(
                "(objectClass=gosaDepartment)",
                array("ou","description")
            );

            /* Create arrays for search results */
            $this->ldapinfo['ou_tree']['formfields'] = [];
            $this->ldapinfo['ou_tree']['OUs']        = [];

            /* Collect all gosaDepartment objects in ldapinfo['ou_tree'] */
            $i = 0;
            $default_ou_groups = 0;
            while($result = $this->_ldap->fetch($ldapsearch)) {
                $this->ldapinfo['ou_tree']['OUs'][]        = $result['ou'][0];
                $this->ldapinfo['ou_tree']['formfields'][] = $result['ou'][0]." - ".$result['description'][0];
                $this->ldapinfo['ou_tree']['DNs'][]        = $result['dn'];
                if (strcasecmp($result['ou'][0], "schoolmanager") == 0) {
                    $default_ou_groups = $i;
                    break;
                }
                $i = $i + 1;
            }

            $this->ldapinfo['ou_groups']= $default_ou_groups;
        }

        if (isset($_POST["ou_groups"])) {
            $this->ldapinfo['ou_groups'] = $_POST["ou_groups"];
        }

        // Showing this template next…
        $new_template = NULL;

        if (isset($_POST["empty_schoolmgr_groups_now_phase1"]) ||
            isset($_POST["empty_schoolmgr_groups_now_phase2"]) ||
            isset($_POST["empty_schoolmgr_groups_now_phase3"]))
        {
            $new_template = $this->smflushgroupmembers->execute($smarty, $this->_ldap);
        } else {
            // Provide pre-set values for account template forms
            $smarty->assign("ous_available",$this->ldapinfo['ou_tree']['formfields']);
            $smarty->assign("preset_ou_groups", $this->ldapinfo['ou_groups']);

            // Show introduction page…
            $new_template = $smarty->fetch(
                get_template_path('content_schoolmanagerintro.tpl', TRUE)
            );
        }

        return $new_template;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
