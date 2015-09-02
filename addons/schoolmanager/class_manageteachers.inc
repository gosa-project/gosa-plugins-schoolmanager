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
class manageteachers extends manageaccounts {

    /* Definitions */
    var $plHeadline="Manage Teachers";
    var $plDescription="GOsa2 School Manager Module: Manage Teachers";
    var $access="";

    /* constructor */
    function manageteachers (&$config,$dn=NULL) {

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

        $attrs[]= "no";
        $attrs[]= "uid";
        $attrs[]= "userPassword";
        $attrs[]= "sn";
        $attrs[]= "givenName";
        $attrs[]= "dateOfBirth";
        $attrs[]= "gender";
        $attrs[]= "subjects";
        $attrs[]= "class";
        $attrs[]= "course";
        /* FIXME: allow "mail" as possible attribute...
         *        If "mail" is given, it should override the default value from the GOsa user
         *        template (if provided there).
         */

        return $attrs;
    }

    function getMultiAttributes()
    {
        $multiAttrs=array();
        $multiAttrs[]="course";
        return $multiAttrs;
    }

    function getAttrsPreSelection($size)
    {
        /* FIXME: Make this more generic by analyzing the return value of $this->getAttributes()
         *        When done -> move method to parent class (manageaccounts).
         */

        $selection = array();
        for($i=0; $i<9; $i++)
        {
             $selection[] = $i;
        }
        for($i=9; $i<$size; $i++)
        {
             $selection[] = 9;
        }
        return $selection;
    }


    function prepareLdapImport($csv_data_sorted)
    {
        /* Get elemenet */
        $data_preldap= array();

        foreach ($csv_data_sorted as $idx => $row_sorted)
        {
            if ( $this->CSVRowSanityChecksOk ($row_sorted) )
            {

                /* main account */
                $teacher=array ();
                if(isset($row_sorted['uid']) && $row_sorted['uid'] && ($row_sorted['uid'] != "%auto%"))
                {
                    $teacher['uid']=array(strtolower(iconv('UTF-8', 'US-ASCII//TRANSLIT', trim($row_sorted['uid']))));
                }
                else {
                    $teacher['uid']="{%uid}";
                }
                if(isset($row_sorted['userPassword']) && $row_sorted['userPassword'] && ($row_sorted['userPassword'] != "%auto%"))
                {
                    $teacher['userPassword']=array($row_sorted['userPassword']);
                }
                if (isset($row_sorted['subjects']))
                {
                    $_subjects= str_replace(";","_", trim($row_sorted['subjects']));
                    $teacher['departmentNumber']=array($_subjects);
                }
                $teacher['sn']=array (trim($row_sorted['sn']));
                $teacher['givenName']=array (trim($row_sorted['givenName']));
                $teacher['gender']=array ($this->normalizeGenderString(trim($row_sorted['gender'])));
                $_dateOfBirth= trim($row_sorted['dateOfBirth']);

                /* FIXME: On a German localized GOsa², this is not required..., but how about other locales??? */
                //if (strpos($_dateOfBirth,'.')) {
                //    list($day, $month, $year)= explode(".", $_dateOfBirth, 3);
                //    $_dateOfBirth = $year."-".$month."-".$day;
                //}

                $teacher['dateOfBirth']=array ($_dateOfBirth);
                $teacher['_template']=array($this->csvinfo['template_main']);
                $teacher['_status']=array('unchecked');
                $teacher['_action']=array('none');

                 /* groups for this account */

                /* class group(s) */
                if (isset($row_sorted['class']) && $row_sorted['class'])
                {
                    /* parse value for attribute "class", requires later work to make it more generic */
                    $tmpclassparts=explode (".", strtolower ($row_sorted['class']));
                    $tmpclassparts[0]=trim ($tmpclassparts[0]);
                    $tmpclassparts[1]=trim ($tmpclassparts[1]);
                    $tmpclassparts[1]=substr ($tmpclassparts[1],1);
                    $row_sorted['class']=$tmpclassparts[0];
                    if ($tmpclassparts[1])
                    {
                        $row_sorted['class']+= $row_sorted['class'].$tmpclassparts[1];
                    }

                    $class_groups= array();
                    $_classes= str_replace(";","_", trim($row_sorted['class']));
                    foreach(explode("_", $_classes) as $class)
                    {
                        $_class= str_replace(':', '-', str_replace(' ', '_',strtolower (trim($class))));
                        if ($_class)
                        {
                            $class_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array('class_'.$_class),
                                'description' => array('SchoolManager class group class_'.$_subject),
                                'mail' => array('class_'.$_class.'@'.$this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_action' => array('none')
                            );
                        }
                    }
                }

                $subject_groups= array();
                if (isset($row_sorted['subjects']) && $row_sorted['subjects'])
                {
                    /* the $_subjects variable has already been declared further above */
                    foreach(explode("_", $_subjects) as $subject)
                    {
                        $_subject= str_replace(':', '-', str_replace(' ', '_',strtolower (trim($subject))));
                        if ($_subject)
                        {
                            $subject_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array('subject_'.$_subject),
                                'description' => array('SchoolManager subject group subject_'.$_subject),
                                'mail' => array('subject_'.$_subject.'@'.$this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_action' => array('none')
                            );
                        }
                    }
                }

                /* course groups */
                $course_groups= array ();
                foreach ($row_sorted as $key=>$value) {
                    if ((substr ($key,0,6)==="course") && trim($value))
                    {
                        $row_sorted[$key]=str_replace(':', '-', str_replace(' ', '_',strtolower (trim($value))));
                        if ($row_sorted[$key])
                        {
                            $course_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array('course_'.$row_sorted[$key]),
                                'description' => array('SchoolManager course group course_'.$row_sorted[$key]),
                                'mail' => array('course_'.trim($row_sorted[$key]).'@'.$this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_action' => array('none')
                            );
                        }
                    }
                }

                /* FIXME: missing group (todo) -> one group per at-school-taught subject... */

                $groups= array ();
                if (!empty($class_groups))   { $groups= array_merge($groups, $class_groups); }
                if (!empty($course_groups))  { $groups= array_merge($groups, $course_groups); }
                if (!empty($subject_groups)) { $groups= array_merge($groups, $subject_groups); }

                $data_preldap[]= array(
                    'main_account' => $teacher,
                    'groups' => $groups
                );
            }
        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
