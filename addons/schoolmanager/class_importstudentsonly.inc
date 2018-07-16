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
class importstudentsonly extends importaccounts {

    /* Definitions */
    var $plHeadline= "Import Students (Only)";
    var $plDescription= "GOsa2 School Manager Module: Import Students (Only)";
    var $access="";

    var $default_template_main = "student";
    /* constructor */
    function __construct(&$config,$dn=NULL) {

        $this->initTime=microtime (TRUE);

        /* Include config object */
        $this->config=&$config;
        $this->ui=get_userinfo ();
        stats::log ('plugin',$class=get_class ($this),$category=array ($this->acl_category),$action='open',$amount=1,$duration= (microtime (TRUE)-$this->initTime));

        parent::__construct($config,$dn=NULL);

    }

    function getAttributes()
    {
        $attrs= array();

        $attrs[]= "no";
        $attrs[]= "uid";
        $attrs[]= "userPassword";
        $attrs[]= "employeeNumber";
        $attrs[]= "sn";
        $attrs[]= "givenName";
        $attrs[]= "dateOfBirth";
        $attrs[]= "gender";
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
         *        When done -> move method to parent class (importaccounts).
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

            if ( $this->CSVRowSanityChecksOk ($row_sorted, $idx) )
            {

                /* main account */
                $student=array ();
                if(isset($row_sorted['uid']) && $row_sorted['uid'] && (($row_sorted['uid'] != "%auto%") && ($row_sorted['uid'] != "%uid%") && ($row_sorted['uid'] != "%{uid}")) )
                {
                    $student['uid']=array(strtolower(iconv('UTF-8', 'US-ASCII//TRANSLIT', trim($row_sorted['uid']))));
                }
                else {
                    $student['uid']=array('{%uid}');
                }
                if(isset($row_sorted['userPassword']) && $row_sorted['userPassword'] && (($row_sorted['userPassword'] != "%auto%") && ($row_sorted['userPassword'] != "%password%")) )
                {
                    $student['userPassword']=array($row_sorted['userPassword']);
                }
                else {
                    $student['userPassword']=array('{%userPassword}');
                }
                $student['sn']=array (trim($row_sorted['sn']));
                $student['givenName']=array (trim($row_sorted['givenName']));

                $_dateOfBirth= trim($row_sorted['dateOfBirth']);
                /* FIXME 1: On a German localized GOsa², this is not required..., but how about other locales???
                 * FIXME 2: If date is formatted like "dd.mm.yy" GOsa² will not use yy as the birthyear but print error
                 */
                if (strpos($_dateOfBirth,'.')) {
                    list($day, $month, $year)= explode(".", $_dateOfBirth, 3);
                    $_dateOfBirth = $year."-".$month."-".$day;
                    //$_dateOfBirth = $day.".".$month.".20".$year;
                }
                $student['dateOfBirth']=array ($_dateOfBirth);
                $student['gender']=array ($this->normalizeGenderString(trim($row_sorted['gender'])));
                if(isset($row_sorted['employeeNumber']) and $row_sorted['employeeNumber'])
                {
                    $student['employeeNumber']= array($row_sorted['employeeNumber']);
                }
                if(isset($row_sorted['mail']) and $row_sorted['mail'])
                {
                    /* FIXME: Check for valid e-Mail format here */
                    $student['mail']= array($row_sorted['mail']);
                }
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
                $student['_actions']=array('none');
                $student['_group_actions']= array('none');

                /* groups for this account */

                /* class group */
                $class_group= array ();
                if (isset($row_sorted['class']) && $row_sorted['class'])
                {
                    /* parse value for attribute "class", requires later work to make it more generic */
                    $tmpclassparts=explode (".", strtolower ($row_sorted['class']));
                    $tmpclassparts[0]=trim ($tmpclassparts[0]);
                    if(isset($tmpclassparts[1]))
                    {
                        $tmpclassparts[1]=trim ($tmpclassparts[1]);
                        $tmpclassparts[1]=substr ($tmpclassparts[1],1);
                    }
                    $row_sorted['class']=$tmpclassparts[0];
                    if (isset($tmpclassparts[1]))
                    {
                        $row_sorted['class'] = $row_sorted['class'].$tmpclassparts[1];
                    }

                    $class_group['cn']= array($this->validGroupName('class_'.$row_sorted['class']));
                    $class_group['objectClass']= array(
                        'posixGroup',
                    );
                    $class_group['description']= array(sprintf(_('SchoolManager class group %s'), $class_group['cn'][0]));
                    $class_group['mail']= array($class_group['cn'][0].'@'.$this->csvinfo['domain_groups']);
                    $class_group['_status']= array('unchecked');
                    $class_group['_actions']= array('none');
                }

                /* course groups */
                $course_groups= array ();
                foreach ($row_sorted as $key=>$value) {
                    if (substr ($key,0,6)==="course") {
                        $row_sorted[$key]=$this->validGroupName('course_'.$value);
                        if ($row_sorted[$key])
                        {
                            $_cn= $row_sorted[$key];
                            $course_groups[$_cn]= array(
                                'objectClass' => array(
                                    'posixGroup',
                                ),
                                'cn' => array($_cn),
                                'description' => array(sprintf(_('SchoolManager course group %s'), $row_sorted[$key])),
                                'mail' => array($row_sorted[$key].'@'.$this->csvinfo['domain_groups']),
                                '_status' => array('unchecked'),
                                '_actions' => array('none')
                            );
                        }
                    }
                }
            }

            $data_preldap[]= array(
                'main_account' => $student,
                'groups' => $groups,
            );

        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
