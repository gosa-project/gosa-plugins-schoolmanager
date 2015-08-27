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

class manageparents extends plugin
{
  /* Definitions */
  var $plHeadline= "Manage Parents";
  var $plDescription= "This does something";
  var $access= "";

  /* Array with csv informations*/
  var $csvinfo=array();

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();
  var $view_logged = FALSE;

  function manageparents (&$config, $dn= NULL)
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
    $smarty->assign("selectedattrs",array(0=>""));
    $smarty->assign("data",array(0=>""));
    $smarty->assign("head",array(0=>""));
    $smarty->assign("sorted",0);
    $smarty->assign("fileup",0);

    /* Get the LDAP link, to generate the Export */
    $ldap = $this->config->get_ldap_link();

    $arrtemplates = array();
    $tempvar =0;

    /* Array to fill in Formfields */
    $arrtemplates[$tempvar] = "None";
    if(!is_array($this->csvinfo)){
      $this->csvinfo=array();
    }

    /* Set Usertemplate information and get all Attribute from userclass */
    unset ($this->csvinfo['arrtemplates']);
    unset ($this->csvinfo['arrtempls']);

    /* Generate Template Array, Attribute Array */
    if(!isset($this->csvinfo['arrtempls'])){

    }

    $arr_temp  = array_flip($this->csvinfo['attr']);
    $this->csvinfo['arr_selected']= array($arr_temp['uid'],$arr_temp['sn'],$arr_temp['givenName'],$arr_temp['userPassword']);

    $smarty->assign("templates",$this->csvinfo['arrtemplates']);
    $smarty->assign("attrs",$this->csvinfo['attr']);

    /* Check permissions for import */
    $acl = $this->ui->get_permissions($this->config->current['BASE'],"all/all");
    if(!preg_match("/w/",$acl)){
      if(isset($_POST['userfile']) || isset($_POST['sorted']) || isset($_POST['fileup'])){
      	msg_dialog::display(_("Permission error"), _("You've no permission to import CSV files!"), ERROR_DIALOG);
      }
      return ($smarty->fetch (get_template_path('content_manageparents.tpl', TRUE)));
    }

    /* If the given dat from the csv File are sorted by the attributes */
    if(isset($_POST['sorted'])) {


      /* If theres a File uploaded */
    } else {
      /* Check if theres a file uploaded */
      if(!empty($_FILES['userfile']['name'])){

        $handle = NULL;

        $filename = gosa_file_name($_FILES['userfile']['tmp_name']);

        if((!isset($_FILES['userfile']['name']))||(!isset($_POST['fileup'])))
        {
            msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
            $smarty->assign("LDIFError",TRUE);
        }
        elseif(!$_FILES['userfile']['size'] > 0 )
        {
            msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file is empty")), ERROR_DIALOG);
            $smarty->assign("LDIFError",TRUE);
        }
        /* Is there a tmp file, which we can use ? */
        elseif(!file_exists($filename))
        {
            msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
            $smarty->assign("LDIFError",TRUE);
        }
        elseif(!$handle = @fopen($filename,"r"))
        {
            msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not readable")), ERROR_DIALOG);
            $smarty->assign("LDIFError",TRUE);
        }
        else
        {


        }
      }
    }

    /* Show main page */
    return ($smarty->fetch (get_template_path('content_manageparents.tpl', TRUE)));
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
