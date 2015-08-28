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
    var $plDescription="GOsa² School Manager Module: Manage Teachers";
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

        $attrs[0]= "no";
        $attrs[1]= "uid";
        $attrs[2]= "sn";
        $attrs[3]= "givenName";
        $attrs[4]= "dateOfBirth";
        $attrs[5]= "gender";
        $attrs[6]= "subjects";
        for ($i=7;$i<20;$i++) {
            $attrs[$i]="course". ($i-6);
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

                /* course values shall be all lower case */
                foreach ($row_sorted as $key=>$value) {
                    if (substr ($key,0,6)==="course") {
                        $row_sorted[$key]=strtolower ($value);
                    }
                }

                /* main account */
                $teacher=array ();
                $teacher['sn']=array ($row_sorted['sn']);
                $teacher['givenName']=array ($row_sorted['givenName']);
                $teacher['birthday']=array ($row_sorted['dateOfBirth']);
                $teacher['gender']=array ($row_sorted['gender']);
                $teacher['_template']=$this->csvinfo['template_main'];
            }
            $data_preldap[]= array(
                'main_account' => $teacher
            );
        }
        return $data_preldap;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
