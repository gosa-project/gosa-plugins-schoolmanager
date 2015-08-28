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
        $attrs[7]= "mother_sn";
        $attrs[8]= "mother_givenName";
        $attrs[9]= "mother_mail";
        $attrs[10] ="father_sn";
        $attrs[11]= "father_givenName";
        $attrs[12]= "father_mail";
        for ($i=13;$i<33;$i++) {
            $attrs[$i]="subjectclass". ($i-12);
        }

        return $attrs;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
