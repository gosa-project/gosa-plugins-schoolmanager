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
    var $plHeadline= "Manage Students";
    var $plDescription= "GOsa2 School Manager Module: Manage Students";
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
        $attrs[10]= "parent2_sn";
        $attrs[11]= "parent2_givenName";
        $attrs[12]= "parent2_mail";
        $attrs[13]= "course";

        return $attrs;
    }

    function getMultiAttributes()
    {
        $multiAttrs=array();
        $multiAttrs[0]="course";
        return $multiAttrs;
    }

    function getAttrsPreSelection($size)
    {
        $selection = array();
        for($i=0; $i<13; $i++)
        {
             $selection[] = $i;
        }
        for($i=13; $i<$size; $i++)
        {
             $selection[] = 13;
        }
        return $selection;
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
                $row_sorted['class']=$tmpclassparts[0]."-".$tmpclassparts[1];

                /* main account */
                $student=array ();
                $student['sn']=array (trim($row_sorted['sn']));
                $student['givenName']=array (trim($row_sorted['givenName']));

                $_dateOfBirth= trim($row_sorted['dateOfBirth']);
                if (strpos($_dateOfBirth,'.')) {
                    list($day, $month, $year)= explode(".", $_dateOfBirth, 3);
                    $_dateOfBirth = $year."-".$month."-".$day;
                }
                $student['dateOfBirth']=array ($_dateOfBirth);
                $student['gender']=array ($this->normalizeGenderString(trim($row_sorted['gender'])));
                /* Keeping the memory footprint low:
                 *
                 *     $student['_template']
                 * is only an int value.
                 *
                 * Retrieve the actual template DN string later like this
                 * $template_dn = $this->csvinfo['templates']['DNs'][$student['_template']];
                 */
                $student['_template']=array($this->csvinfo['template_main']);
                $student['_status']=array('unchecked');
                $student['_action']=array('none');

                /* auxiliary accounts */
                $parent1=array ();
                $parent1['sn']=array (trim($row_sorted['parent1_sn']));
                $parent1['givenName']=array (trim($row_sorted['parent1_givenName']));
                $parent1['mail']=array (trim($row_sorted['parent1_mail']));
                /* see above for how to retrieve the template's DN */
                $parent1['_template']=array($this->csvinfo['template_aux']);
                $parent1['_status']=array('unchecked');
                $parent1['_action']=array('none');

                $parent2=array ();
                $parent2['sn']=array (trim($row_sorted['parent2_sn']));
                $parent2['givenName']=array (trim($row_sorted['parent2_givenName']));
                $parent2['mail']=array (trim($row_sorted['parent2_mail']));
                /* see above for how to retrieve the template's DN */
                $parent2['_template']=array(trim($this->csvinfo['template_aux']));
                $parent2['_status']=array('unchecked');
                $parent2['_action']=array('none');

                /* groups for this account */

                /* class group */
                $class_group= array ();
                $class_group['cn']= array('class_'.trim($row_sorted['class']));
                $class_group['objectClass']= array(
                    'posixGroup',
                );
                $class_group['description']= array("SchoolManager class group ".$class_group['cn'][0]);
                $class_group['mail']= array($class_group['cn'][0].'@intern');
                $class_group['_status']= array('unchecked');
                $class_group['_action']= array('none');

                /* course groups */
                $course_groups= array ();
                foreach ($row_sorted as $key=>$value) {
                    if (substr ($key,0,6)==="course") {
                        $row_sorted[$key]=strtolower ($value);
                        if ($row_sorted[$key])
                        {
                            $course_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup',
                                ),
                                'cn' => array('course_'.trim($row_sorted[$key])),
                                'description' => array('SchoolManager course group course_'.$row_sorted[$key]),
                                /* FIXME: do not hard-code email address domain here */
                                'mail' => array('course_'.$row_sorted[$key].'@intern'),
                                '_status' => array('unchecked'),
                                '_action' => array('none')
                            );
                        }
                    }
                }
                $parent_group= array ();
                $parent_group['cn']= array('parents_{%uid}');
                $parent_group['objectClass'] = array(
                    'groupOfNames'
                );
                /* FIXME: can we generate the DN here already??? */
                $parent_group['member']= array(
                    '{%dn[parent1]}',
                    '{%dn[parent2]}'
                );
                $parent_group['_status']=array('none');
                $parent_group['_action']=array('skip');
                $groups= array($class_group, $parent_group) + $course_groups;
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
