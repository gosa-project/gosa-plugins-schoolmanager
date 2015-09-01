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

class managecourses extends plugin
{
    /* Definitions */
    var $plHeadline= _("Manage Courses");
    var $plDescription= _("GOsa2 School Manager Module: Manage Courses");
    var $access= "";

    /* Array with csv informations*/
    var $csvinfo=array();

    /* attribute list for save action */
    var $attributes= array();
    var $objectclasses= array();
    var $view_logged = FALSE;

    function managecourses (&$config, $dn= NULL)
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

        /* initiate smarty */
        $smarty= get_smarty();

        /* [...] */

        return ($smarty->fetch (get_template_path('content_managecourses.tpl', TRUE)));
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
