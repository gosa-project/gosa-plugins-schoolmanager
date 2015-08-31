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

    var $failure_in_last_phase=FALSE;

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
    
    function getMultiAttributes()
    {
        /* adapt this in sub-classes!!! */
        return array();
    }
    function getAttrsPreSelection($size)
    {
        /* adapt this in sub-classes!!! */
        return array();
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

        /* PHASE 4 done, set $groups_reviewed to TRUE  */
        $smarty->assign ("groups_reviewed",0);

        /* PHASE 5 done, set $accounts_reviewed to TRUE  */
        $smarty->assign ("accounts_reviewed",0);

        /* Get the LDAP link, to generate the Export */
        $ldap=$this->config->get_ldap_link ();

        /* initialize CSV Info array */
        if (!is_array ($this->csvinfo)) {
            $this->csvinfo=array ();
        }

        $this->csvinfo['attrs']=$this->getAttributes();
        $this->csvinfo['attrs'][] = "ignore";
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

        /* reset our failure status from last template view... */
        $this->failure_in_last_phase = FALSE;

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
                $this->csvinfo['attrs_selected']=$this->getAttrsPreSelection($this->csvinfo['num_cols']);

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
         *          This data structure will be presented to the admin in two stages:
         *            - groups review (PHASE 4)
         *            - user accounts review (PHASE 5)
         */
        elseif (isset ($_POST['phase3'])) {

            /* sanity checks on LDAP attributes assignments */

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);

            /* read attributes assignments from $_POST */
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
            $multi_attrs=$this->getMultiAttributes();
            foreach ($this->csvinfo['data'] as $data_row) {

                $attrs_counter=array();
                $data_row_sorted= array();

                for ($i=0;$i<count($data_row);$i++)
                {
                        $data_in_cell = $data_row[$i];
                        $selection_in_cell = $this->csvinfo['attrs_selected'][$i];
                        $value_of_selection = $this->csvinfo['attrs'][$selection_in_cell];
                        if($value_of_selection == "ignore")
                        {
                        } elseif(in_array($value_of_selection,$multi_attrs))
                        {
                                $attrs_counter[$value_of_selection]=$attrs_counter[$value_of_selection] ?: 0;
                                $data_row_sorted[$value_of_selection . $attrs_counter[$value_of_selection]] = $data_in_cell;
                                $attrs_counter[$value_of_selection]++;
                        } elseif(empty($attrs_counter[$value_of_selection]))
                        {
                                $data_row_sorted[$value_of_selection] = $data_in_cell;
                                $attrs_counter[$value_of_selection] = 1;
                        } else 
                        {
                                $this->failure_in_last_phase=TRUE;
                                $smarty->assign ("LDIFError",TRUE);
                                msg_dialog::display (_ ("Error"),sprintf (_ ("The attribute %s is only allowed to select once!"),bold ($value_of_selection)),ERROR_DIALOG);
                        }
                }

                $this->csvinfo['data_sorted'][] = $data_row_sorted;
            }

            if($this->failure_in_last_phase)
            {
                $smarty->assign ("attrs",$this->csvinfo['attrs']);
                $smarty->assign ("attrs_selected",$this->csvinfo['attrs_selected']);
                $smarty->assign ("num_rows",$this->csvinfo['num_cols']);
                $smarty->assign ("data",array_slice($this->csvinfo['data'], 0, 5));
            }
            else {

                /* transform data_sorted to data_preldap array */
                $this->csvinfo['data_preldap']= $this->prepareLdapImport($this->csvinfo['data_sorted']);
                /* free some memory... */
                unset ($this->csvinfo['data_sorted']);

                $this->csvinfo['data_preldap_posixgroups'] = array();
                $this->groupStatusCheck();

                $smarty->assign ("data_sorted",TRUE);
                $smarty->assign ("data",$this->csvinfo['data_preldap_posixgroups']);
            }
        }
        /*
         * PHASE 4: Review group names and existing/to-be-created groups
         */
        elseif (isset ($_POST['phase4'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            /* setting groups_reviewed to FALSE for now until we further investigated user input (below) */
            $smarty->assign ("groups_reviewed",FALSE);


            /* TODO: evaluate group review (selections, input, etc.) */


            if ($this->failure_in_last_phase)
            {


                /* TODO: - show error message
                 *       - reset smarty assignments to phase3 variables/values
                 */

            }
            else {

                /* prepare review view for account creation */

                /* Template Handling */
                $smarty->assign ("file_uploaded",TRUE);
                $smarty->assign ("templates_selected",TRUE);
                if (is_array($this->csvinfo['data_preldap']) and (!empty($this->csvinfo['data_preldap'])))
                {
                    $this->accountStatusCheck();

                    /* groups review has been successful */
                    $smarty->assign ("groups_reviewed",TRUE);
                    $smarty->assign ("data",array_slice($this->csvinfo['data_preldap'], 0, 5));
                }
            }
        }
        /*
         * PHASE 5: Finally write everything to LDAP
         */
        elseif (isset ($_POST['phase5'])) {

            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
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

    function compareAccounts($account_a, $account_b, $attrs=array("sn", "givenName"))
    {
        $success = TRUE;
        foreach ($attrs as $key => $attr)
        {
            $val_a = $account_a[$attr][0];
            $val_b = $account_b[$attr][0];

            if ($val_a != $val_b)
            {
                $success = FALSE;
                break;
            }
        }
        return $success;
    }

    function accountStatusCheck()
    {
        $ldap = $this->config->get_ldap_link();

        /*
         * This all may get us in deep trouble on huge LDAP databases
         * when memory limits and/or execution time in PHP are set too low.
         */

        $ldapsearch = $ldap->search("(objectClass=gosaAccount)",array("sn","givenName","gender","dateOfBirth", "mail"));

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        while ($gosa_account = $ldap->fetch($ldapsearch))
        {
            foreach ($this->csvinfo['data_preldap'] as $key => $row)
            {
                if (isset($row['main_account']))
                {
                    if ($this->compareAccounts($row['main_account'], $gosa_account, array("sn", "givenName", "gender", "dateOfBirth")))
                    {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'exists';
                        /*
                         * FIXME: Hard-code a "skip" action here in case we found a matching account in LDAP.
                         *        Later, we need to make actions more configurable...
                         */
                        $this->csvinfo['data_preldap'][$key]['main_account']['_action'][0]= 'skip';

                        /* break out of the inner loop if the $gosa_account has been found */
                        break;
                    }
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $aux_key => $aux_account)
                    {
                        if ($this->compareAccounts($aux_account, $gosa_account, array("sn", "givenName", "mail")))
                        {
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]= 'exists';
                            /*
                             * FIXME: Hard-code a "skip" action here in case we found a matching account in LDAP.
                             *        Later, we need to make actions more configurable...
                             */
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['action'][0]= 'skip';

                            /* break out of the inner loop if the $gosa_account has been found */
                            break;
                        }
                    }
                }
            }
        }
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {
            if (isset($row['main_account']))
            {
                if ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]==='unchecked')
                {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'not-found';
                    $this->csvinfo['data_preldap'][$key]['main_account']['_action'][0]= 'create';
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $aux_key => $aux_account)
                    {
                        if ($this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]==='unchecked')
                        {
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]= 'not-found';
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_action'][0]= 'create';
                        }
                    }
                }
            }
        }
    }

    function groupStatusCheck()
    {
        $ldap = $this->config->get_ldap_link();
        $ldapsearch = $ldap->search("(objectClass=posixGroup)",array("cn","memberUid"));

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        while ($gosa_posix_group = $ldap->fetch($ldapsearch))
        {
            foreach ($this->csvinfo['data_preldap'] as $key => $row)
            {
                if (isset($row['groups']))
                {
                    foreach ($row['groups'] as $group_key => $group)
                    {
                        if ($this->compareAccounts($group, $gosa_posix_group, array("cn")))
                        {
                            $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]= 'exists';
                            /*
                             * FIXME: Hard-code a "skip" action here in case we found a matching account in LDAP.
                             *        Later, we need to make actions more configurable...
                             */
                            $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['action'][0]= 'drop-all-members';

                            /* break out of the inner loop if the $gosa_account has been found */
                            break;
                        }
                    }
                }
            }
        }

        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {
            if (isset($row['groups']))
            {
                foreach ($row['groups'] as $group_key => $group)
                {
                    if ($this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]==='unchecked')
                    {
                        $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]= 'not-found';
                        $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_action'][0]= 'create';
                    }

                    /* grab posix groups (classes, courses) into their own array */
                    $group_name = $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['cn'][0];
                    if (!in_array($group_name, array_keys($this->csvinfo['data_preldap_posixgroups'])))
                    {
                        if (in_array("posixGroup", $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['objectClass']))
                        {
                            $this->csvinfo['data_preldap_posixgroups'][$group_name] = $this->csvinfo['data_preldap'][$key]['groups'][$group_key];
                        }
                    }
                }
            }
        }
        ksort($this->csvinfo['data_preldap_posixgroups']);
    }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
