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


class smflushgroupmembers
{
    /** Template URI (.tpl file) */
    const TEMPLATE_URI = "../templates/smflushgroupmembers/content_smflushgroupmembers.tpl";

    function __construct(&$config, schoolmgr_utils &$utils)
    {
        $this->config = $config;
        $this->utils  = $utils;
        $this->ldapinfo = [];
    }

    private function populate_groups()
    {
        $this->ldapinfo['groups']        = [];

        // Search LDAP tree for SchoolManager maintained POSIX groups
        $this->_ldap->cd($this->ldapinfo['ou_tree']['DNs'][$this->ldapinfo['ou_groups']]);

        // Read groups from SchoolManager OU
        $ldapsearch = $this->_ldap->search(
            "(&(objectClass=posixGroup)(|(cn=class_*)(cn=course_*)(cn=subject_*)))",
            array("objectClass", "cn", "description", "memberUid")
        );

        while ($result = $this->_ldap->fetch($ldapsearch)) {
            if (in_array('posixGroup', $result['objectClass'])) {
                $this->ldapinfo['groups'][] = $result;
            }
        }
    }

    private function populate_ogroups()
    {
        $this->ldapinfo['ogroups'] = [];

        // Search LDAP tree for SchoolManager maintained groupOfNames objects
        $this->_ldap->cd($this->ldapinfo['ou_tree']['DNs'][$this->ldapinfo['ou_groups']]);

        $ldapsearch = $this->_ldap->search(
            "(&(objectClass=gosaGroupOfNames)(cn=parents_*))",
            array("objectClass", "cn", "description", "member")
        );

        while ($result = $this->_ldap->fetch($ldapsearch)) {
            if (in_array('gosaGroupOfNames', $result['objectClass'])) {
                $this->ldapinfo['ogroups'][] = $result;
            }
        }
    }

    private function reduce_group($group_obj)
    {
        $_reduced_group = $group_obj;

        foreach ($group_obj as $key => $attr) {
            if (!is_array($attr)) {
                unset($_reduced_group[$key]);
            } else {
                foreach ($attr as $idx => $values) {
                    if ($idx === "count") {
                        unset($_reduced_group[$key][$idx]);
                    }
                }
            }
        }

        return $_reduced_group;
    }


    public function execute(&$smarty, &$_ldap, &$ldapinfo): string
    {
        $this->smarty = $smarty;
        $this->_ldap = $_ldap;
        $this->ldapinfo = $ldapinfo;

        $new_template = null;

        if (isset($_POST["empty_schoolmgr_groups_now_phase1"])) {
            $this->populate_groups();
            $new_template = $this->execute_empty_schoolmgr_groups_now_phase1($smarty);
        } elseif (isset($_POST["empty_schoolmgr_groups_now_phase2"])) {
            $smarty->assign("empty_schoolmgr_groups_now_phase1_done", true);

            $this->populate_groups();
            $this->populate_ogroups();
            $new_template = $this->execute_empty_schoolmgr_groups_now_phase2($smarty);
        } elseif (isset($_POST["empty_schoolmgr_groups_now_phase3"])) {
            $smarty->assign("empty_schoolmgr_groups_now_phase1_done", true);
            $smarty->assign("empty_schoolmgr_groups_now_phase2_done", true);

            $this->populate_ogroups();
            $new_template = $this->execute_empty_schoolmgr_groups_now_phase3($smarty);
        }

        return $new_template;
    }


    function execute_empty_schoolmgr_groups_now_phase1($smarty): string
    {
        // User requested group member flushing on our "Introduction" page…

        $this->ldapinfo['cleanup_stats'] = [];

        // Remove some information from the search result
        // (ease displaying stuff in the .tpl file)
        $_reduced_groups = [];
        foreach ($this->ldapinfo['groups'] as $group) {
            $_reduced_groups[] = $this->reduce_group($group);
        }

        $smarty->assign("data_groups", $_reduced_groups);
        $smarty->assign("data_ogroups", []);

        // Return main page…
        return $smarty->fetch(
            get_template_path(self::TEMPLATE_URI, true)
        );
    }


    function execute_empty_schoolmgr_groups_now_phase2($smarty): string
    {
        $this->ldapinfo['cleanup_stats']['classes']                = 0;
        $this->ldapinfo['cleanup_stats']['courses']                = 0;
        $this->ldapinfo['cleanup_stats']['subjects']               = 0;
        $this->ldapinfo['cleanup_stats']['unmanaged_groups']       = 0;
        $this->ldapinfo['cleanup_stats']['classes_empty']          = 0;
        $this->ldapinfo['cleanup_stats']['courses_empty']          = 0;
        $this->ldapinfo['cleanup_stats']['subjects_empty']         = 0;
        $this->ldapinfo['cleanup_stats']['unmanaged_groups_empty'] = 0;

        /*
         * User confirmed flushing members from SchoolManager
         * maintained POSIX groups.
         * Doing this now (classes, courses, subjects)…
         */

        // Drop all group members (from POSIX goups)
        foreach ($this->ldapinfo['groups'] as $group) {
            $group_name = $group['cn'][0];

            if (isset($group['memberUid']) && count($group['memberUid']) > 0) {
                $group_dn = $group['dn'];
                $grouptab = new grouptabs(
                    $this->config,
                    $this->config->data['TABS']['GROUPTABS'],
                    $group_dn,
                    "groups"
                );

                // Remove all members.
                $group_members = $grouptab->by_object['group']->memberUid;
                foreach ($group_members as $group_member) {
                    $grouptab->by_object['group']->removeUser($group_member);
                }

                // Run GOsa²'s groups/group checks
                if (count($grouptab->check())) {
                    msg_dialog::displayChecks($grouptab->check());

                    /*
                     * FIXME: collect failure statistics here!!!
                     */
                } else {
                    if ($this->utils->strstartswith($group_name, 'class_')) {
                        $this->ldapinfo['cleanup_stats']['classes']++;
                    } elseif ($this->utils->strstartswith($group_name, 'subject_')) {
                        $this->ldapinfo['cleanup_stats']['subjects']++;
                    } elseif ($this->utils->strstartswith($group_name, 'course_')) {
                        $this->ldapinfo['cleanup_stats']['courses']++;
                    } else {
                        $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                    }

                    // Save group object to LDAP…
                    $grouptab->save();
                }
            } else {
                if ($this->utils->strstartswith($group_name, 'class_')) {
                    $this->ldapinfo['cleanup_stats']['classes_empty']++;
                } elseif ($this->utils->strstartswith($group_name, 'subject_')) {
                    $this->ldapinfo['cleanup_stats']['subjects_empty']++;
                } elseif ($this->utils->strstartswith($group_name, 'course_')) {
                    $this->ldapinfo['cleanup_stats']['courses_empty']++;
                } else {
                    $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                }
            }
        }

        // Remove some information from the search result
        // (ease displaying stuff in the .tpl file)
        $_reduced_ogroups = [];
        foreach ($this->ldapinfo['ogroups'] as $ogroup) {
            $_reduced_ogroups[] = $this->reduce_group($ogroup);
        }

        $smarty->assign("data_groups", []);
        $smarty->assign("data_ogroups", $_reduced_ogroups);

        // Return main page…
        return $smarty->fetch(
            get_template_path(self::TEMPLATE_URI, true)
        );
    }


    function execute_empty_schoolmgr_groups_now_phase3($smarty): string
    {
        $this->ldapinfo['cleanup_stats']['parents']                 = 0;
        $this->ldapinfo['cleanup_stats']['unmanaged_ogroups']       = 0;
        $this->ldapinfo['cleanup_stats']['parents_empty']           = 0;
        $this->ldapinfo['cleanup_stats']['unmanaged_ogroups_empty'] = 0;

        /*
         * User confirmed flushing members from SchoolManager maintained object groups
         * Doing this now (parents)…
         */

        /* Drop all group members (object goups) */
        foreach ($this->ldapinfo['ogroups'] as $ogroup) {
            $ogroup_name = $ogroup['cn'][0];
            if (isset($ogroup['member']) && count($ogroup['member']) > 0) {
                $ogroup_dn = $ogroup['dn'];
                $ogrouptab = new ogrouptabs($this->config, $this->config->data['TABS']['OGROUPTABS'], $ogroup_dn, "ogroups");

                $ogrouptab->by_object['ogroup']->member = [];

                /* Run GOsa²'s ogroups/ogroup checks */
                if (count($ogrouptab->check())) {
                    msg_dialog::displayChecks($ogrouptab->check());

                    /*
                     * FIXME: collect failure statistics here!!!
                    */
                } else {
                    if ($this->utils->strstartswith($ogroup_name, 'parents_')) {
                        $this->ldapinfo['cleanup_stats']['parents']++;
                    } else {
                        $this->ldapinfo['cleanup_stats']['unmanaged']++;
                    }

                    // Save group object to LDAP…
                    $ogrouptab->save();
                }
            } else {
                if ($this->utils->strstartswith($ogroup_name, 'parents_')) {
                    $this->ldapinfo['cleanup_stats']['parents_empty']++;
                } else {
                    $this->ldapinfo['cleanup_stats']['unmanaged_empty']++;
                }
            }
        }

        $smarty->assign("cleanup_stats", $this->ldapinfo['cleanup_stats']);
        $smarty->assign("data_groups", []);
        $smarty->assign("data_ogroups", []);

        // Return main page…
        return $smarty->fetch(
            get_template_path(self::TEMPLATE_URI, true)
        );
    }
}
