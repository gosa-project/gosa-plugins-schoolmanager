<?php

/*
  This code is an addon for GOsa (https://gosa.gonicus.de)
  Copyright (C) 2015 Mike Gabriel

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

function reduce_group($group_obj) {

    $_reduced_group = $group_obj;

    foreach($group_obj as $key => $attr) {
        if(!is_array($attr)) {
            unset($_reduced_group[$key]);
        }
        else {
            foreach($attr as $idx => $values) {
                if($idx === "count") {
                    unset($_reduced_group[$key][$idx]);
                }
            }
        }
    }
    return $_reduced_group;
}

class schoolmanagerintro extends plugin
{
    /* Definitions */
    var $plHeadline= "Introduction";
    var $plDescription= "GOsa2 School Manager Module: Introduction";
    var $access= "";

    /* attribute list for save action */
    var $attributes= array();
    var $objectclasses= array();
    var $view_logged = FALSE;

    function __construct(&$config, $dn= NULL)
    {
        $this->initTime = microtime(TRUE);

        /* Include config object */
        $this->config= &$config;

        $this->ui = get_userinfo();
        stats::log('plugin', $class = get_class($this), $category = array($this->acl_category),  $action = 'open',
                   $amount = 1, $duration = (microtime(TRUE) - $this->initTime));
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

        /* initialize smarty */
        $smarty= get_smarty();

        /* initialize LDAP */
        $this->_ldap = $this->config->get_ldap_link ();
        $this->ldapinfo = array();
        $this->ldapinfo['ou_tree'] = array();

        $this->_ldap->cd($this->config->current['BASE']);

        /* read OU tree from LDAP */
        $ldapsearch = $this->_ldap->search("(objectClass=gosaDepartment)",array("ou","description"));

        /* create arrays for search results */
        $this->ldapinfo['ou_tree']['formfields'] = array();
        $this->ldapinfo['ou_tree']['OUs'] = array();

        /* add found gosaDepartment objects */
        $i = 0;
        $default_ou_groups = 0;
        while($result = $this->_ldap->fetch($ldapsearch)) {
            $this->ldapinfo['ou_tree']['OUs'][] = $result['ou'][0];
            $this->ldapinfo['ou_tree']['formfields'][] = $result['ou'][0]." - ".$result['description'][0];
            $this->ldapinfo['ou_tree']['DNs'][] = $result['dn'];
            if (strcasecmp($result['ou'][0], "schoolmanager") == 0) {
                $default_ou_groups = $i;
            }
            $i = $i + 1;
        }

        if (isset($_POST["ou_groups"])) {
            $this->ldapinfo['ou_groups'] = $_POST["ou_groups"];
        } else {
            /* import configuration defaults, only set them if PHASE 1 has been successful */
            $this->ldapinfo['ou_groups']= $default_ou_groups;
        }

        if (isset($_POST["empty_schoolmanager_groups_now"])) {

            $this->ldapinfo['groups'] = array();
            $this->ldapinfo['ogroups'] = array();

            $this->_ldap->cd($this->ldapinfo['ou_tree']['DNs'][$this->ldapinfo['ou_groups']]);
            /* read groups from SchoolManager OU */
            $ldapsearch = $this->_ldap->search("(|(objectClass=posixGroup)(objectClass=gosaGroupOfNames))",array("objectClass","cn","description","memberUid","member"));
            while($result = $this->_ldap->fetch($ldapsearch)) {
                if(in_array('posixGroup', $result['objectClass'])) {
                    $this->ldapinfo['groups'][] = $result;
                }
                else if(in_array('gosaGroupOfNames', $result['objectClass'])) {
                    $this->ldapinfo['ogroups'][] = $result;
                }
            }

            foreach($this->ldapinfo['groups'] as $cn => $group) {
                $_reduced_groups[$cn] = reduce_group($group);
            }
            foreach($this->ldapinfo['ogroups'] as $cn => $ogroup) {
                $_reduced_ogroups[$cn] = reduce_group($ogroup);
            }

            $smarty->assign("data_groups", $_reduced_groups );
            $smarty->assign("data_ogroups", $_reduced_ogroups );

            /* Show main page */
            return ($smarty->fetch (get_template_path('content_smflushgroupmembers.tpl', TRUE)));

        }
        else {

            /* provide pre-set values for account template forms */
            $smarty->assign("ous_available",$this->ldapinfo['ou_tree']['formfields']);
            $smarty->assign("preset_ou_groups", $this->ldapinfo['ou_groups']);

            /* Show introduction page */
            return ($smarty->fetch (get_template_path('content_schoolmanagerintro.tpl', TRUE)));

        }
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
