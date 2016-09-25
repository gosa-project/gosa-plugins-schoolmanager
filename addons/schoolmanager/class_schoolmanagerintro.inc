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

function startswith($haystack, $needle)
{
    $length = strlen($needle);
    return (substr($haystack, 0, $length) === $needle);
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

    /* SchoolManager specific properties */
    var $ldapinfo= array();

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
        if(!is_array($this->ldapinfo)) {
            $this->ldapinfo = array();
        }

        /* Scan LDAP for ou tree when "Introduction" page is loaded and remember that information... */
        if(!isset($this->ldapinfo['ou_tree'])) {
            $this->ldapinfo['ou_tree'] = array();

            $this->_ldap->cd($this->config->current['BASE']);

            /* read OU tree from LDAP */
            $ldapsearch = $this->_ldap->search("(objectClass=gosaDepartment)",array("ou","description"));

            /* create arrays for search results */
            $this->ldapinfo['ou_tree']['formfields'] = array();
            $this->ldapinfo['ou_tree']['OUs'] = array();

            /* Collect all gosaDepartment objects in ldapinfo['ou_tree'] */
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

            $this->ldapinfo['ou_groups']= $default_ou_groups;
        }

        if (isset($_POST["ou_groups"])) {
            $this->ldapinfo['ou_groups'] = $_POST["ou_groups"];
        }

        if (isset($_POST["empty_schoolmanager_groups_now"])) {

            $this->ldapinfo['cleanup_stats'] = array();

            /* User requested group member flushing on our "Introduction" page... */

            $this->ldapinfo['groups'] = array();

            /* Search LDAP tree for SchoolManager maintained POSIX groups */
            $this->_ldap->cd($this->ldapinfo['ou_tree']['DNs'][$this->ldapinfo['ou_groups']]);
            /* read groups from SchoolManager OU */
            $ldapsearch = $this->_ldap->search("(&(objectClass=posixGroup)(|(cn=class_*)(cn=course_*)(cn=subject_*)))",array("objectClass","cn","description","memberUid"));
            while($result = $this->_ldap->fetch($ldapsearch)) {
                if(in_array('posixGroup', $result['objectClass'])) {
                    $this->ldapinfo['groups'][] = $result;
                }
            }

            /* Remove some information from the search result (ease displaying stuff in the .tpl file) */
            $_reduced_groups = array();
            foreach($this->ldapinfo['groups'] as $group) {
                $_reduced_groups[] = reduce_group($group);
            }

            $smarty->assign("data_groups", $_reduced_groups );
            $smarty->assign("data_ogroups", array() );

            /* Show main page */
            return ($smarty->fetch (get_template_path('content_smflushgroupmembers.tpl', TRUE)));

        }
        else if (isset($_POST["really_empty_schoolmanager_groups_now"])) {

            $this->ldapinfo['cleanup_stats']['classes'] = 0;
            $this->ldapinfo['cleanup_stats']['courses'] = 0;
            $this->ldapinfo['cleanup_stats']['subjects'] = 0;
            $this->ldapinfo['cleanup_stats']['unmanaged_groups'] = 0;
            $this->ldapinfo['cleanup_stats']['classes_empty'] = 0;
            $this->ldapinfo['cleanup_stats']['courses_empty'] = 0;
            $this->ldapinfo['cleanup_stats']['subjects_empty'] = 0;
            $this->ldapinfo['cleanup_stats']['unmanaged_groups_empty'] = 0;

            /* User confirmed flushing members from SchoolManager maintained POSIX groups
             * Doing this now (classes, courses, subjects) ...
             */

            /* Drop all group members (from POSIX goups) */
            foreach ($this->ldapinfo['groups'] as $group) {

                $group_name = $group['cn'][0];
                if(isset($group['memberUid']) && count($group['memberUid']) > 0)
                {
                    $group_dn = $group['dn'];
                    $grouptab= new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'],$group_dn, "groups" );

                    $group_members=$grouptab->by_object['group']->memberUid;
                    foreach($group_members as $group_member)
                    {
                        $grouptab->by_object['group']->removeUser($group_member);
                    }

                    /* Run GOsa²'s groups/group checks */
                    if(count($grouptab->check()))
                    {
                        msg_dialog::displayChecks($grouptab->check());

                        /*
                         * FIXME: collect failure statistics here!!!
                         */

                    } else
                    {
                        if(startswith($group_name, 'class_'))
                        {
                            $this->ldapinfo['cleanup_stats']['classes']++;
                        } else if (startswith($group_name, 'subject_'))
                        {
                            $this->ldapinfo['cleanup_stats']['subjects']++;
                        } else if (startswith($group_name, 'course_'))
                        {
                            $this->ldapinfo['cleanup_stats']['courses']++;
                        } else {
                            $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                        }

                        /* Save group object to LDAP */
                        $grouptab->save();
                    }
                } else {

                    if(startswith($group_name, 'class_'))
                    {
                        $this->ldapinfo['cleanup_stats']['classes_empty']++;
                    } else if (startswith($group_name, 'subject_'))
                    {
                        $this->ldapinfo['cleanup_stats']['subjects_empty']++;
                    } else if (startswith($group_name, 'course_'))
                    {
                        $this->ldapinfo['cleanup_stats']['courses_empty']++;
                    } else {
                        $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                    }
                }
            }

            /* Prepare for next step...
             * Search LDAP tree for SchoolManager maintained object groups (gosaGroupOfNames)
             */
            $this->ldapinfo['ogroups'] = array();
            $this->_ldap->cd($this->ldapinfo['ou_tree']['DNs'][$this->ldapinfo['ou_groups']]);

            $ldapsearch = $this->_ldap->search("(&(objectClass=gosaGroupOfNames)(cn=parents_*))",array("objectClass","cn","description","member"));
            while($result = $this->_ldap->fetch($ldapsearch)) {
                if(in_array('gosaGroupOfNames', $result['objectClass'])) {
                    $this->ldapinfo['ogroups'][] = $result;
                }
            }

            $_reduced_ogroups = array();
            foreach($this->ldapinfo['ogroups'] as $ogroup) {
                $_reduced_ogroups[] = reduce_group($ogroup);
            }

            $smarty->assign("data_groups", array() );
            $smarty->assign("data_ogroups", $_reduced_ogroups );

            /* Show main page */
            return ($smarty->fetch (get_template_path('content_smflushgroupmembers.tpl', TRUE)));

        }
        else if (isset($_POST["really_empty_schoolmanager_ogroups_now"])) {

            $this->ldapinfo['cleanup_stats']['parents'] = 0;
            $this->ldapinfo['cleanup_stats']['unmanaged_ogroups'] = 0;
            $this->ldapinfo['cleanup_stats']['parents_empty'] = 0;
            $this->ldapinfo['cleanup_stats']['unmanaged_ogroups_empty'] = 0;

            /* User confirmed flushing members from SchoolManager maintained object groups
             * Doing this now (parents) ...
             */

            /* Drop all group members (object goups) */
            foreach ($this->ldapinfo['ogroups'] as $ogroup) {

                $ogroup_name = $ogroup['cn'][0];
                if(isset($ogroup['member']) && count($ogroup['member']) > 0)
                {
                    $ogroup_dn = $ogroup['dn'];
                    $ogrouptab= new ogrouptabs($this->config, $this->config->data['TABS']['OGROUPTABS'], $ogroup_dn, "ogroups" );

                    $ogrouptab->by_object['ogroup']->member= array();

                    /* Run GOsa²'s ogroups/ogroup checks */
                    if(count($ogrouptab->check()))
                    {
                        msg_dialog::displayChecks($ogrouptab->check());

                        /*
                         * FIXME: collect failure statistics here!!!
                        */

                    } else
                    {
                        if(startswith($ogroup_name, 'parents_'))
                        {
                            $this->ldapinfo['cleanup_stats']['parents']++;
                        } else {
                            $this->ldapinfo['cleanup_stats']['unmanaged']++;
                        }

                        /* Save group object to LDAP */
                        $ogrouptab->save();
                    }
                } else {

                    if(startswith($ogroup_name, 'parents_'))
                    {
                        $this->ldapinfo['cleanup_stats']['parents_empty']++;
                    } else {
                        $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                    }

                }
            }

            $smarty->assign("cleanup_stats", $this->ldapinfo['cleanup_stats']);
            $smarty->assign("data_groups", array() );
            $smarty->assign("data_ogroups", array() );

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
