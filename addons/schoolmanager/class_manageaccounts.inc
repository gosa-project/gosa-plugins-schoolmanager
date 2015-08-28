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
class manageaccounts extends plugin{

    /* Definitions */
    var $plHeadline="Manage Accounts";
    var $plDescription="GOsa² School Manager super-class for managing accounts";
    var $access="";

    /* Array with csv informations */
    var $csvinfo=array ();

    /* attribute list for save action */
    var $attributes=array ();
    var $objectclasses=array ();
    var $view_logged=FALSE;

    var $import_account_type="";

    /* constructor */
    function manageaccounts (&$config,$dn=NULL) {

        $this->initTime=microtime (TRUE);

        /* Include config object */
        $this->config=&$config;
        $this->ui=get_userinfo ();
        stats::log ('plugin',$class=get_class ($this),$category=array ($this->acl_category),$action='open',$amount=1,$duration= (microtime (TRUE)-$this->initTime));
    }

    function getAttributes()
    {
        /* CSV columns required for import */
        $attrs= array ();

        $attrs[0]= "uid";
        $attrs[1]= "sn";
        $attrs[2]= "givenName";

        return $attrs;
    }

    function execute () {

        /* Call parent execute */
        plugin::execute ();

        $import_account_type = substr(get_class($this), 6, strlen(get_class($this)));

        /* Log view */
        if (!$this->view_logged) {
            $this->view_logged=TRUE;
            new log ("view","all/".get_class ($this),$this->dn);
        }

        /* initiate smarty */
        $smarty=get_smarty ();

        /* Import students or teachers */
        $smarty->assign ("import_account_type",$import_account_type);

        /* PHASE 1 done, set $file_uploaded to TRUE */
        $smarty->assign ("file_uploaded",0);

        /* PHASE 2 done, set $templates_selected to TRUE  */
        $smarty->assign ("templates_selected",0);

        /* PHASE 3 done, set $data_sorted to TRUE  */
        $smarty->assign ("data_sorted",0);

        /* PHASE 4 done, set $summary_accepted to TRUE  */
        $smarty->assign ("summary_accepted",0);

        /* Get the LDAP link, to generate the Export */
        $ldap=$this->config->get_ldap_link ();

        /* initialize CSV Info array */
        if (!is_array ($this->csvinfo)) {
            $this->csvinfo=array ();
        }

        $this->csvinfo['attrs']=$this->getAttributes();
        /* Check permissions for import */
        $acl=$this->ui->get_permissions ($this->config->current['BASE'],"all/all");
        if (!preg_match ("/w/",$acl)) {
            if (isset ($_POST['userfile'])||isset ($_POST['sorted'])||isset ($_POST['fileup'])) {
                msg_dialog::display (_ ("Permission error"),_ ("You've no permission to import CSV files!"),ERROR_DIALOG);
            }
            return ($smarty->fetch (get_template_path ('content_manageaccounts.tpl',TRUE)));
        }

        /*
         * PHASES
         *
         * 0: File Upload
         * 1: Selecting User Templates
         * 2: Data Sorting
         * 3: Importing to LDAP
         */
        /*
         * PHASE 1: CSV file has been selected for upload. We now upload the file and parse its data.
         *          For PHASE 2 we search LDAP for available gosaUserTemplate objects and provide them
         *          in $this->csvinfo['templates'].
         */
        if (isset ($_POST['phase1'])) {

            /* Check if theres a file uploaded */
            if (!empty ($_FILES['userfile']['name'])) {
                $handle=NULL;
                $filename=gosa_file_name ($_FILES['userfile']['tmp_name']);
                if ( (!isset ($_FILES['userfile']['name']))|| (!isset ($_POST['fileup']))) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not found")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                elseif (!$_FILES['userfile']['size']>0) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file is empty")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }

                /* Is there a tmp file, which we can use ? */
                elseif (!file_exists ($filename)) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not found")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                elseif (!$handle=@fopen ($filename,"r")) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot read uploaded file: %s"),_ ("file not readable")),ERROR_DIALOG);
                    $smarty->assign ("LDIFError",TRUE);
                }
                else {
                    $smarty->assign ("file_uploaded",TRUE);
                    $raw_csv_data="";

                    /* Reading content */
                    while (!feof ($handle)) {
                        $raw_csv_data.=fread ($handle,1024);
                    }

                    @fclose ($handle);
                    
                    $raw_csv_data = utf8_encode($raw_csv_data);

                    $parser_ok = manageaccounts::parseCSV($raw_csv_data);

                    if ($parser_ok != FALSE)
                    {

                        /* Populate the Template Selectors for PHASE 2 */

                        /* Search all Templates    */
                        $this->csvinfo['templates'] = array();
                        $this->csvinfo['templates']['ldapsearch'] = $ldap->search("(objectClass=gosaUserTemplate)",array("*"));

                        /* add the "None" template first */
                        $this->csvinfo['templates']['formfields'] = array();
                        $this->csvinfo['templates']['formfields'][] = "None";
                        $this->csvinfo['templates']['DNs'] = array();
                        $this->csvinfo['templates']['DNs'][]   = "";

                        /* add found gosaUserTemplate objects */
                        while($result = $ldap->fetch($this->csvinfo['templates']['ldapsearch'])){
                            $index ++;
                            $this->csvinfo['templates']['formfields'][] = $result['sn'][0]." - ".$this->config->idepartments[preg_replace("/^[^,]+,".preg_quote(get_people_ou(), '/')."/i", "", $result['dn'])];
                            $this->csvinfo['templates']['DNs'][] = $result['dn'];
                        }

                        $smarty->assign("templates",$this->csvinfo['templates']['formfields']);

                    }
                    else {
                        $smarty->assign ("LDIFError",TRUE);
                        $smarty->assign ("file_uploaded",FALSE);
                        msg_dialog::display (_ ("Error"),_ ("Cannot find CSV data in the selected file!"),ERROR_DIALOG);
                    }
                }
            }
        }

        /*
         * PHASE 2: After the admin has assigned available user object templates,
         *          we will display the CSV data and allow assigned columns
         *          LDAP attributes to columns in the CSV file.
         *
         *          NOTE: For the sake of better readibility, we turn the CSV
         *                around a 90° angle counter-clockwise.
         */
        elseif (isset ($_POST['phase2'])) {

            /* template handling */
            if (isset ($_POST["template_".$import_account_type]))
            {
                $this->csvinfo['template_main'] = $_POST["template_".$import_account_type];

                if (isset ($_POST["template_".$import_account_type."_aux"]))
                {
                    $this->csvinfo['template_aux'] = $_POST["template_".$import_account_type."_aux"];
                }
                $smarty->assign ("file_uploaded",TRUE);
                $smarty->assign ("templates_selected",TRUE);

                /* initialize $this->csvinfo['attrs_selected'], only do this here and once */
                $tmp_array=array ();
                for ($i=0;$i<33;$i++) {
                    $tmp_array[$i]=$i;
                }
                $this->csvinfo['attrs_selected']=$tmp_array;

                /* student import attributes */
                $smarty->assign ("attrs",$this->csvinfo['attrs']);

                /* per row selected student import attributes */
                $smarty->assign ("attrs_selected",$this->csvinfo['attrs_selected']);

                /* number of CSV columns -> number of rows in 90°-counter-clockwise-rotated table */
                $smarty->assign ("num_rows",$this->csvinfo['num_cols']);

                /* CSV data */
                $smarty->assign ("data",array_slice($this->csvinfo['data'], 0, 5));
            }
        }
        /*
         * PHASE 3: After the admin has assigned LDAP attributes to CSV columns we create
         *          the data structure that will finally be imported into LDAP.
         */
        elseif (isset ($_POST['phase3'])) {

            /* template handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);

            $new_attrs_selected = array();
            for ($i=0; $i<count($this->csvinfo['attrs_selected']);$i++)
            {
                if(isset($_POST["column_head_$i"]))
                {
                    $new_attrs_selected[] = $_POST["column_head_$i"];
                }
                else {
                    $new_attrs_selected[] = $i;
                }
            }
            $this->csvinfo['attrs_selected'] = $new_attrs_selected;

            /* sort the CSV date table according to how it got re-ordered by the webUI admin user */
            $this->csvinfo['data_sorted'] = array();
            foreach ($this->csvinfo['data'] as $data_row) {

                $data_row_sorted= array();

                for ($i=0;$i<count($data_row);$i++)
                {
                        $data_row_sorted[$this->csvinfo['attrs'][$i]]= $data_row[$this->csvinfo['attrs_selected'][$i]];
                }

                $this->csvinfo['data_sorted'][] = $data_row_sorted;
            }

            $this->csvinfo['data_preldap']= $this->prepareLdapImport($this->csvinfo['data_sorted']);

            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            if (is_array($this->csvinfo['data_preldap']) and (!empty($this->csvinfo['data_preldap'])))
            {
                $smarty->assign ("data_sorted",TRUE);
                $smarty->assign ("data",array_slice($this->csvinfo['data_preldap'], 0, 5));
            }
        }

        /*
         * PHASE 4: Finally write everything to LDAP (directly, no futher $_POST['phase4']...)
         */
        elseif (isset ($_POST['phase5'])) {

            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("summary_accepted",TRUE);
        }

        /* Show main page */
        return ($smarty->fetch (get_template_path ('content_manageaccounts.tpl',TRUE)));
    }

    function parseCSV($raw_csv_data)
    {
        $lines=preg_split ("/\n/",$raw_csv_data);
        $num_columns=0;
        $rest=0;
        $data=array ();

        /* check colum count, if it stays zero, we probably don't have a comma separated CSV file */
        if (is_array ($lines))
            foreach ($lines as $line) {

                /* continue if theres a comment */
                if (substr (trim ($line),0,1)=="#") {
                    continue;
                }
                $line=str_replace ("\t","",$line);
                $cells=explode (",",$line);
                if (count ($cells)>$num_columns) {
                    $num_columns=count ($cells);
                }
            }

        /* parse rows and import into $this->csvinfo */
        if ($num_columns > 1) {

            /* Generate array with output info  */
            if (is_array ($lines))
                foreach ($lines as $line) {
                    $rest=0;
                    $cnt=0;

                    /* do not use comments or empty lines */
                    if ( (substr (trim ($line),0,1)=="#")|| (empty ($line))) {
                        continue;
                    }

                    /* replace \t to , */
                    $line=str_replace ("\t",",",$line);

                    /* get all elements  */
                    $cells=explode (",",$line);

                    /* attach all elements to data array */
                    if (is_array ($cells))

                        /* cell count less than num_columns, attach some empty fields */
                        if ( count($cells) <= $num_columns ) {
                            $rest=$num_columns - count($cells);
                            for ($i=0;$i<$rest;$i++) {
                                $cells[]="";
                            }
                        }
                    $data[]=$cells;
                }
            unset ($this->csvinfo['data_sorted']);
            $this->csvinfo['num_cols']=$num_columns;
            $this->csvinfo['data']=$data;
            $this->csvinfo['num_rows']=count($this->csvinfo['data']);
            return TRUE;
        }
        return FALSE;
    }

    function prepareLdapImport($csv_data_sorted)
    {
        /* adapt this in sub-classes!!! */
        return array();
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
