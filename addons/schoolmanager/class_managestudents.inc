<?php
/*
  This code is an addon for GOsa (https://gosa.gonicus.de)
  Copyright (C) 2015 Mike Gabriel
  Copyright (C) 2015 Marius Rasch

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
class managestudents extends manageaccounts {

    /* Definitions */
    var $plHeadline="Manage Students";
    var $plDescription="GOsa² School Manager Module: Manage Students";
    var $access="";

    /* constructor */
    function managestudents (&$config,$dn=NULL) {

        $this->initTime=microtime (TRUE);

        /* Include config object */
        $this->config=&$config;
        $this->ui=get_userinfo ();
        stats::log ('plugin',$class=get_class ($this),$category=array ($this->acl_category),$action='open',$amount=1,$duration= (microtime (TRUE)-$this->initTime));

        parent::manageaccounts($config,$dn=NULL);
    }

    function getAttributes()
    {
        $attrs= array();

        $attrs[0]= "no";
        $attrs[1]= "employeeNumber";
        $attrs[2]= "sn";
        $attrs[3]= "givenName";
        $attrs[4]= "dateOfBirth";
        $attrs[5]= "gender";
        $attrs[6]= "class";
        $attrs[7]= "parent1_sn";
        $attrs[8]= "parent1_givenName";
        $attrs[9]= "parent1_mail";
        $attrs[10] ="parent2_sn";
        $attrs[11]= "parent2_givenName";
        $attrs[12]= "parent2_mail";
        for ($i=13;$i<33;$i++) {
            $attrs[$i]="course". ($i-12);
        }

        return $attrs;
    }

    function prepareLdapImport($csv_data_sorted)
    {
        /* Get elemenet */
        $data_preldap= array();

        foreach ($csv_data_sorted as $row_sorted)
        {
            /* If not all needed vars set .... */
            if ( (empty ($row_sorted['sn'])) || (empty ($row_sorted['givenName']))|| (!isset ($row_sorted['sn']))|| (!isset ($row_sorted['givenName']))) {

                /* Output Error */
                msg_dialog::display (_ ("Error"),sprintf (_ ("Need at least %s and %s to create users!"),bold ("sn"),bold ("gn")),ERROR_DIALOG);
            }
            else {

                /* parse value for attribute "class", requires later work to make it more generic */
                $tmpclassparts=explode (".", strtolower ($row_sorted['class']));
                $tmpclassparts[0]=trim ($tmpclassparts[0]);
                $tmpclassparts[1]=trim ($tmpclassparts[1]);
                $tmpclassparts[1]=substr ($tmpclassparts[1],1);
                $row_sorted['class']=$tmpclassparts[0].".".$tmpclassparts[1];

                /* main account */
                $student=array ();
                $student['sn']=array ($row_sorted['sn']);
                $student['givenName']=array ($row_sorted['givenName']);
                $student['birthday']=array ($row_sorted['dateOfBirth']);
                $student['gender']=array ($row_sorted['gender']);
                /* Keeping the memory footprint low:
                 *
                 *     $student['_template']
                 * is only an int value.
                 *
                 * Retrieve the actual template DN string later like this
                 * $template_dn = $this->csvinfo['templates']['DNs'][$student['_template']];
                 */
                $student['_template']=$this->csvinfo['template_main'];

                /* auxiliary accounts */
                $parent1=array ();
                $parent1['sn']=array ($row_sorted['parent1_sn']);
                $parent1['givenName']=array ($row_sorted['parent1_givenName']);
                $parent1['mail']=array ($row_sorted['parent1_mail']);
                /* see above for how to retrieve the template's DN */
                $parent1['_template']=$this->csvinfo['template_aux'];

                $parent2=array ();
                $parent2['sn']=array ($row_sorted['parent2_sn']);
                $parent2['givenName']=array ($row_sorted['parent2_givenName']);
                $parent2['mail']=array ($row_sorted['parent2_mail']);
                /* see above for how to retrieve the template's DN */
                $parent2['_template']=$this->csvinfo['template_aux'];

                /* groups for this account */

                /* class group */
                $class_group= array ();
                $class_group['cn']= array('class_'.$row_sorted['class']);
                $class_group['objectClass']= array(
                    'top',
                    'posixGroup',
                );

                /* course groups */
                $course_groups= array ();
                foreach ($row_sorted as $key=>$value) {
                    if (substr ($key,0,6)==="course") {
                        $row_sorted[$key]=str_replace(':', '.', strtolower ($value));
                        if ($row_sorted[$key])
                        {
                            $course_groups[]= array(
                                'objectClass' => array(
                                    'top',
                                    'posixGroup',
                                ),
                                'cn' => array('course_'.$row_sorted[$key])
                            );
                        }
                    }
                }
                $parent_group= array ();
                $parent_group['cn']= array('parents_{%uid}');
                $parent_group['objectClass'] = array(
                    'top',
                    'groupOfNames'
                );
                /* FIXME: can we generate the DN here already??? */
                $parent_group['member']= array(
                    '{%dn[parent1]}',
                    '{%dn[parent2]}'
                );
                $groups= array ($class_group, $parent_group) + $course_groups;

            }
            $data_preldap[]= array(
                'main_account' => $student,
                'aux_accounts' => array(
                    0 => $parent1,
                    1 => $parent2
                ),
                'groups' => $groups
            );
        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
