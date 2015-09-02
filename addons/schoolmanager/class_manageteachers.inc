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
        for($i=0; $i<7; $i++)
        {
             $selection[] = $i;
        }
        for($i=7; $i<$size; $i++)
        {
             $selection[] = 7;
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

                /* main account */
                $teacher=array ();
                if(isset($row_sorted['uid']))
                {
                    $teacher['uid']=array(strtolower(iconv('UTF-8', 'US-ASCII//TRANSLIT', trim($row_sorted['uid']))));
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
                if (strpos($_dateOfBirth,'.')) {
                    list($day, $month, $year)= explode(".", $_dateOfBirth, 3);
                    $_dateOfBirth = $year."-".$month."-".$day;
                }
                $teacher['dateOfBirth']=array ($_dateOfBirth);
                $teacher['_template']=array($this->csvinfo['template_main']);
                $teacher['_status']=array('unchecked');
                $teacher['_action']=array('none');

                 /* groups for this account */

                /* class group */
                $class_group= array ();
                if (isset($row_sorted['class']) && $row_sorted['class'])
                {
                    /* parse value for attribute "class", requires later work to make it more generic */
                    $tmpclassparts=explode (".", strtolower ($row_sorted['class']));
                    $tmpclassparts[0]=trim ($tmpclassparts[0]);
                    $tmpclassparts[1]=trim ($tmpclassparts[1]);
                    $tmpclassparts[1]=substr ($tmpclassparts[1],1);
                    $row_sorted['class']=$tmpclassparts[0]."-".$tmpclassparts[1];

                    $class_group['cn']= array('class_'.trim($row_sorted['class']));
                    $class_group['objectClass']= array(
                        'posixGroup',
                    );
                    $class_group['description']= array("SchoolManager class group ".$class_group['cn'][0]);
                    /* FIXME: do not hard-code email address domain here */
                    $class_group['mail']= array($class_group['cn'][0].'@intern');
                    $class_group['mail']= array($class_group['cn'][0].'@'.$this->csvinfo['domain_groups']);
                    $class_group['_status']=array('unchecked');
                    $class_group['_action']=array('none');
                }

                $subject_groups= array();
                if (isset($row_sorted['subjects']) && $row_sorted['subjects'])
                {
                    /* the $_subjects variable has already been declared further above */
                    foreach(explode("_", $_subjects) as $subject)
                    {
                        $_subject= trim(strtolower($subject));
                        if ($_subject)
                        {
                            $subject_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array('subject_'.$_subject),
                                'description' => array('SchoolManager subject group subject_'.$_subject),
                                'mail' => array('subject_'.$_subject.'@intern'),
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
                        $row_sorted[$key]=str_replace(':', '-', strtolower ($value));
                        if ($row_sorted[$key])
                        {
                            $course_groups[]= array(
                                'objectClass' => array(
                                    'posixGroup'
                                ),
                                'cn' => array('course_'.trim($row_sorted[$key])),
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
                if (!empty($class_group))    { $groups= array_merge($groups, array($class_group)); }
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
