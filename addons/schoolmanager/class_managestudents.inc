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

class managestudents extends plugin
{
  /* Definitions */
  var $plHeadline= "Manage Students";
  var $plDescription= "This does something";
  var $access= "";

  /* Array with csv informations*/
  var $csvinfo=array();

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();
  var $view_logged = FALSE;

  function managestudents (&$config, $dn= NULL)
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

      $new = array();
      $new[0] = "no";
      $new[1] = "id";
      $new[2] = "sn";
      $new[3] = "gn";
      $new[4] = "birthday";
      $new[5] = "gender";
      $new[6] = "class";
      $new[7] = "mothersn";
      $new[8] = "mothergn";
      $new[9] = "mothermail";
      $new[10] = "fathersn";
      $new[11] = "fathergn";
      $new[12] = "fathermail";
      $new[13] = "subjectclass";

      $this->csvinfo['attr'] = $new;

      $this->csvinfo['arrtemplates'][]="None";

    }

    $tmp = array();
    for($i = 0; $i < 20; $i++) {
      $j = $i;
      if($i >= count($new)) {
        $j =  count($new) - 1;
      }
      $tmp[$i] = $j;
    }
    $this->csvinfo['arr_selected']= $tmp;



    $smarty->assign("templates",$this->csvinfo['arrtemplates']);
    $smarty->assign("attrs",$this->csvinfo['attr']);

    /* Check permissions for import */
    $acl = $this->ui->get_permissions($this->config->current['BASE'],"all/all");
    if(!preg_match("/w/",$acl)){
      if(isset($_POST['userfile']) || isset($_POST['sorted']) || isset($_POST['fileup'])){
      	msg_dialog::display(_("Permission error"), _("You've no permission to import CSV files!"), ERROR_DIALOG);
      }
      return ($smarty->fetch (get_template_path('content_managestudents.tpl', TRUE)));
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

          $smarty->assign("fileup",TRUE);
          $str = "";


          /* Reading content */
          while(!feof($handle)) {
            $str .= fread($handle,1024);
          }

          $lines = preg_split("/\n/",$str);
          $anz  = 0;
          $rest = 0;
          $data = array();

          /* check colum count */
          if(is_array($lines))
            foreach($lines as $line) {
              /* continue if theres a comment */
              if(substr(trim($line),0,1)=="#"){
                continue;
              }

              $line= str_replace ("\t","",$line);
              $cells  = explode(",",$line )  ;

              if(count($cells)> $anz ){
                $anz = count($cells);
              }
            }

          /* At least one entry */
          if($anz >1) {

            /* Generate array with outpu info  */
            if(is_array($lines))
              foreach($lines as $line) {
                $rest = 0;
                $cnt  = 0;

                /* dont use comments or empty lines */
                if((substr(trim($line),0,1)=="#")||(empty($line))){
                  continue;
                }

                /* replace \t to , */
                $line= str_replace ("\t"  ,"" ,$line);

                /* get all elements  */
                $cells  = explode(",",$line )  ;

                /* attach all elements to data array */
                if(is_array($cells))
                  foreach($cells as $cell) {
                    if(!empty($cell)) {
                      $cnt++; 
                      $data[]=trim($cell);
                    }
                  }

                /* cell count less than anz, attach some empty fields */
                if(($cnt != $anz)&&(!empty($cnt))) {
                  $rest = $anz - $cnt;
                  for($i = 0 ; $i < $rest ; $i ++){
                    $data[] = " ";
                  }
                }
              }    

            unset($this->csvinfo['sorteddata']);
            unset($this->csvinfo['colsstr']);
            unset($this->csvinfo['sorteddata']);

            $this->csvinfo['cols']        = $anz;
            $this->csvinfo['data']        = array();
            $this->csvinfo['data']        = $data;
            $this->csvinfo['count']       = count($this->csvinfo['data']);

            if($this->csvinfo['count']> (6* $this->csvinfo['cols'])) {
              /* only show 6 rows in ouptuttablei */
              $datouttemp =  array_chunk($this->csvinfo['data'],(6* $this->csvinfo['cols']));
              $this->csvinfo['dataout']=$datouttemp[0];
              $this->csvinfo['shownrowscount'] = 6;
            } else {
              $this->csvinfo['shownrowscount'] = (count($this->csvinfo['data']))/$this->csvinfo['cols'];
              $this->csvinfo['dataout']= $this->csvinfo['data'];  
            }

            $this->csvinfo['tplid']       = $_POST['template'];
            $this->csvinfo['templatestr'] = $this->csvinfo['arrtemplates' ][$this->csvinfo['tplid']];
            $this->csvinfo['count']       = count($this->csvinfo['data']);
            $this->csvinfo['rows']        = (count($this->csvinfo['data'])/$anz);

            $i =  0; 
            $tmp = $tmp2= array();
            if(is_array($this->csvinfo['dataout']))
              foreach($this->csvinfo['dataout'] as $dat){
                $tmp[]= $dat;
                $i++;
                if($i >=$this->csvinfo['cols']){
                  $i = 0;
                  $tmp2[] = $tmp;
                  $tmp = array();
                }

              }

            /* Set Templateid  */
            $smarty->assign("tplid",$this->csvinfo['tplid']);

            /* Set Template  */
            $smarty->assign("tpl",$this->csvinfo['templatestr']);

            /* Temp var 1 */
            $smarty->assign("ia",1); 

            /* Temp var 2 */
            $smarty->assign("i",0); 

            /* Num rows    */
            $smarty->assign("rows",$this->csvinfo['shownrowscount']);


            for($i  = 0 ; $i < $anz; $i ++)
              $this->csvinfo['arr_selected'][]="0";

            $smarty->assign("selectedattrs",$this->csvinfo['arr_selected']);

            /* Entrie count5 */
            $smarty->assign("anz",$this->csvinfo['cols']);                                                    

            /* Array with data */
            $smarty->assign("data",$tmp2);   

            @fclose($handle);
          } else {
            $smarty->assign("tpl",$this->csvinfo['attr'][$_POST['template']]);
            $smarty->assign("LDIFError",TRUE);
            $smarty->assign("fileup",FALSE);
        msg_dialog::display(_("Error"), _("Cannot find CSV data in the selected file!"), ERROR_DIALOG);
          }


        }
      }
    }                                              

    /* Show main page */
    return ($smarty->fetch (get_template_path('content_managestudents.tpl', TRUE)));
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>

