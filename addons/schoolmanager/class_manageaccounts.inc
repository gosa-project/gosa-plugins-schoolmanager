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
    var $plHeadline= "Manage Accounts";
    var $plDescription= "GOsa2 School Manager super-class for managing accounts";
    var $access="";

    /* Array with csv informations */
    var $csvinfo=array ();

    /* attribute list for save action */
    var $attributes=array ();
    var $objectclasses=array ();
    var $view_logged=FALSE;

    var $failure_in_this_phase=FALSE;

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


        /* PHASE 01 done, set $file_uploaded to TRUE */
        $smarty->assign ("file_uploaded",0);

        /* PHASE 02 done, set $import_configured to TRUE  */
        $smarty->assign ("import_configured",0);

        /* PHASE 03 done, set $data_sorted to TRUE  */
        $smarty->assign ("data_sorted",0);

        /* PHASE 04 done, set $accounts_reviewed to TRUE  */
        $smarty->assign ("accounts_reviewed",0);

        /* PHASE 05 done, set $accounts_imported to TRUE  */
        $smarty->assign ("accounts_imported",0);

        /* PHASE 06 done, set $groups_reviewed to TRUE  */
        $smarty->assign ("groups_reviewed",0);

        /* PHASE 07 done, set $groups_imported to TRUE  */
        $smarty->assign ("groups_imported",0);

        /* PHASE 08 done, set $accounts_groupmembers_reviewed to TRUE */
        $smarty->assign ("accounts_groupmembers_reviewed",0);

        /* PHASE 09 done, set $accounts_groupmembers_updated to TRUE */
        $smarty->assign ("accounts_groupmembers_updated",0);

        /* PHASE 10 done, set $cleanup_completed to TRUE */
        $smarty->assign ("cleanup_completed",0);


        /* Get the LDAP link, to generate the Export */
        $this->_ldap=$this->config->get_ldap_link ();

        /* initialize CSV Info array */
        if (!is_array ($this->csvinfo)) {
            $this->csvinfo=array ();
        }

        $this->csvinfo['attrs']=$this->getAttributes();
        $this->csvinfo['attrs'][] = "---";
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
         */

        /* reset our failure status from last template view... */
        $this->failure_in_this_phase = FALSE;

        /*
         * PHASE 01
         */
        if (isset ($_POST['phase_01'])) {

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

                    if(!mb_check_encoding($raw_csv_data, "UTF-8"))
                    {
                        if(mb_check_encoding($raw_csv_data, "iso-8859-1"))
                        {
                            $raw_csv_data = utf8_encode($raw_csv_data);
                        } else
                        {
                            $smarty->assign ("LDIFError",TRUE);
                            $smarty->assign ("file_uploaded",FALSE);
                            msg_dialog::display (_ ("Error"),_ ("File has an unsupported encoding!"),ERROR_DIALOG);
                        }
                    }

                    $parser_ok = manageaccounts::parseCSV($raw_csv_data);

                    if ($parser_ok!==FALSE)
                    {

                        /* Populate the Template Selectors for PHASE 2 */

                        /* Search all Templates    */
                        $this->csvinfo['templates'] = array();
                        $this->csvinfo['templates']['ldapsearch'] = $this->_ldap->search("(objectClass=gosaUserTemplate)",array("*"));

                        /* add the "None" template first */
                        $this->csvinfo['templates']['formfields'] = array();
                        $this->csvinfo['templates']['formfields'][] = "None";
                        $this->csvinfo['templates']['DNs'] = array();
                        $this->csvinfo['templates']['DNs'][]   = "";

                        /* add found gosaUserTemplate objects */
                        while($result = $this->_ldap->fetch($this->csvinfo['templates']['ldapsearch'])){
                            $this->csvinfo['templates']['formfields'][] = $result['sn'][0]." - ".$this->config->idepartments[preg_replace("/^[^,]+,".preg_quote(get_people_ou(), '/')."/i", "", $result['dn'])];
                            $this->csvinfo['templates']['DNs'][] = $result['dn'];
                        }

                        $smarty->assign("templates",$this->csvinfo['templates']['formfields']);

                        /* Search OUs with objectClass gosaDepartment */
                        $this->csvinfo['ou_tree'] = array();
                        $this->csvinfo['ou_tree']['ldapsearch'] = $this->_ldap->search("(objectClass=gosaDepartment)",array("ou","description"));

                        /* create arrays for search results */
                        $this->csvinfo['ou_tree']['formfields'] = array();
                        $this->csvinfo['ou_tree']['OUs'] = array();

                        /* add found gosaDepartment objects */
                        while($result = $this->_ldap->fetch($this->csvinfo['ou_tree']['ldapsearch'])) {
                            $this->csvinfo['ou_tree']['OUs'][] = $result['ou'][0];
                            $this->csvinfo['ou_tree']['formfields'][] = $result['ou'][0]." - ".$result['description'][0];
                            $this->csvinfo['ou_tree']['DNs'][] = $result['dn'];
                        }

                        $smarty->assign("ous_available",$this->csvinfo['ou_tree']['formfields']);

                        /* import configuration defaults, only set them if PHASE 1 has been successful */
                        $this->csvinfo["template_main"]= 0;
                        $this->csvinfo["template_aux"]= 0;
                        $this->csvinfo['ou_groups']= 0;
                        $this->csvinfo['domain_groups']= "intern";
                        $this->csvinfo['flush_members']= FALSE;
                        $this->csvinfo['accounts_in_class_ou']= FALSE;

                        /* provide pre-set values for account template forms */
                        $smarty->assign("preset_template_".$import_account_type, $this->csvinfo['template_main']);
                        $smarty->assign("preset_template_".$import_account_type."_aux", $this->csvinfo['template_aux']);
                        $smarty->assign("preset_ou_groups", $this->csvinfo['ou_groups']);
                        $smarty->assign("preset_domain_groups", $this->csvinfo['domain_groups']);
                        $smarty->assign("preset_flush_members", $this->csvinfo['flush_members']);
                        $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
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
         * PHASE 2
         */
        elseif (isset ($_POST['phase_02'])) {

            $smarty->assign ("file_uploaded",TRUE);

            /* configure options */
            if (isset ($_POST["template_".$import_account_type]) and isset ($_POST["ou_groups"]) and isset ($_POST["domain_groups"]))
            {
                $this->csvinfo['template_main'] = $_POST["template_".$import_account_type];

                /* Obtain the OU name like this: $this->csvinfo['ou_tree']['OUs'][$this->csvinfo['ou_groups']] */
                $this->csvinfo['ou_groups'] = $_POST["ou_groups"];

                if(empty($_POST["domain_groups"]))
                {
                    $this->csvinfo['domain_groups']= "";
                    $smarty->assign ("LDIFError",TRUE);
                    msg_dialog::display (_ ("Error"),_ ("Domain for new groups is empty!"),ERROR_DIALOG);
                    $this->failure_in_this_phase= TRUE;

                } elseif(substr($_POST["domain_groups"],0,1) == "@")
                {
                    $this->csvinfo['domain_groups'] = substr($_POST["domain_groups"],1,strlen($_POST["domain_groups"]));
                } else
                {
                    $this->csvinfo['domain_groups'] = $_POST["domain_groups"];
                }

                if (isset ($_POST["template_".$import_account_type."_aux"]))
                {
                    $this->csvinfo['template_aux'] = $_POST["template_".$import_account_type."_aux"];
                }
                if (isset ($_POST["flush_members"]))
                {
                    $this->csvinfo['flush_members'] = TRUE;
                }
                else {
                    $this->csvinfo['flush_members'] = FALSE;
                }
                if (isset ($_POST["accounts_in_class_ou"]))
                {
                    $this->csvinfo['accounts_in_class_ou'] = TRUE;
                }
                else {
                    $this->csvinfo['accounts_in_class_ou'] = FALSE;
                }
                if($this->failure_in_this_phase===FALSE)
                {
                    $smarty->assign ("import_configured",TRUE);

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
                else {
                    /* prepare for reloading this phase's web page again */
                    $smarty->assign("templates",$this->csvinfo['templates']['formfields']);
                    $smarty->assign("ous_available",$this->csvinfo['ou_tree']['formfields']);
                    $smarty->assign("preset_template_".$import_account_type, $this->csvinfo['template_main']);
                    $smarty->assign("preset_template_".$import_account_type."_aux", $this->csvinfo['template_aux']);
                    $smarty->assign("preset_ou_groups", $this->csvinfo['ou_groups']);
                    $smarty->assign("preset_domain_groups", $this->csvinfo['domain_groups']);
                    $smarty->assign("preset_flush_members", $this->csvinfo['flush_members']);
                    $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
                }
            }
        }

        /*
         * PHASE 03
         */
        elseif (isset ($_POST['phase_03'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);

            /* sanity checks on LDAP attributes assignments */

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
                        if($value_of_selection == "---")
                        {
                        } elseif(in_array($value_of_selection,$multi_attrs))
                        {
                                if(isset($attrs_counter[$value_of_selection]))
                                {
                                    $attrs_counter[$value_of_selection]=$attrs_counter[$value_of_selection];
                                }
                                else {
                                    $attrs_counter[$value_of_selection]= 0;
                                }
                                $data_row_sorted[$value_of_selection . $attrs_counter[$value_of_selection]] = $data_in_cell;
                                $attrs_counter[$value_of_selection]++;
                        } elseif(empty($attrs_counter[$value_of_selection]))
                        {
                                $data_row_sorted[$value_of_selection] = $data_in_cell;
                                $attrs_counter[$value_of_selection] = 1;
                        } else 
                        {
                                $this->failure_in_this_phase=TRUE;
                                $smarty->assign ("LDIFError",TRUE);
                                msg_dialog::display (_ ("Error"),sprintf (_ ("The attribute %s is only allowed to select once!"),bold ($value_of_selection)),ERROR_DIALOG);
                        }
                }

                $this->csvinfo['data_sorted'][] = $data_row_sorted;
            }

            /* transform data_sorted to data_preldap array */
            $this->csvinfo['data_preldap']= $this->prepareLdapImport($this->csvinfo['data_sorted']);

            /* $this->failure_in_this_phase may have been set above or in $this->prepareLdapImport ... */
            if($this->failure_in_this_phase===FALSE)
            {
                /* free some memory... */
                unset ($this->csvinfo['data_sorted']);

                $this->accountStatusCheck();

                $smarty->assign ("data_sorted",TRUE);
                $smarty->assign ("data",$this->csvinfo['data_preldap']);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $smarty->assign ("attrs",$this->csvinfo['attrs']);
                $smarty->assign ("attrs_selected",$this->csvinfo['attrs_selected']);
                $smarty->assign ("num_rows",$this->csvinfo['num_cols']);
                $smarty->assign ("data",array_slice($this->csvinfo['data'], 0, 5));
            }
        }
        /*
         * PHASE 04
         */
        elseif (isset ($_POST['phase_04'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);

            if($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_reviewed",TRUE);

                /*
                 * Add users to / update users in LDAP
                 */
                foreach($this->csvinfo['data_preldap'] as $idx => $user_data)
                {
                    /* import the user's main account */
                    if((isset($user_data['main_account']) && (!empty($user_data['main_account']))))
                    {

                        $this->importLDAPUserObject($user_data['main_account']);

                    }

                    /* import the user's aux account (e.g., parent accounts) */
                    //if((isset($user_data['aux_accounts']) && (!empty($user_data['aux_accounts']))))
                    //{
                    //    foreach($user_data['aux_accounts'] as $idx_aux => $aux_account_data)
                    //    {
                    //        /* import aux account $aux_account_data */
                    //        $this->importLDAPUserObject($aux_account_data);
                    //    }
                    //}
                }

                /* FIXME: present a message dialog box that informs on the statistics of
                 *        the LDAP group import. (How many groups got created? How many
                 *        groups have been updated? How many groups were not empty and
                 *        required dropping of all members? Etc.
                 */

                $this->accountStatusCheck();
                $smarty->assign("data",$this->csvinfo['data_preldap']);

            }
            else {
                /* prepare for reloading this phase's web page again */
                $smarty->assign("data",$this->csvinfo['data_preldap']);
            }
        }
        /*
         * PHASE 05
         */
        elseif (isset ($_POST['phase_05']))
        {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);

            if($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_imported",TRUE);

                $this->groupStatusCheck();
                $smarty->assign ("data",$this->csvinfo['data_preldap_posixgroups']);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $this->accountStatusCheck();
                $smarty->assign("data",$this->csvinfo['data_preldap']);
            }
        }

        /*
         * PHASE 06
         */
        elseif (isset ($_POST['phase_06']))
        {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);

            $this->groupStatusCheck();

            $smarty->assign ("data",$this->csvinfo['data_preldap_posixgroups']);
            if($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("groups_reviewed",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
            }
        }
        /*
         * PHASE 07
         */
        elseif (isset ($_POST['phase_07'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);

            foreach($this->csvinfo['data_preldap_posixgroups'] as $group_name => $group_data)
            {

                /* only perform on this group if _actions is not "none" */
                if(strpos($group_data['_actions'][0],'none')===FALSE)
                {

                    $this->importLDAPGroupObject($group_name, $group_data);

                }
            }

            /* collect group memberships from user accounts and store the information
             * in separate arrays (one for posixGroups, one for groupOfNames)
             */

            $this->posixgroup_members= array();
            $this->groupofnames_members= array();

            foreach($this->csvinfo['data_preldap'] as $idx => $user_data)
            {

                if((isset($user_data['main_account']) && (!empty($user_data['main_account']))))
                {
                    $_uid= $user_data['main_account']['uid'][0];
                    foreach($user_data['groups'] as $group_data)
                    {
                        $_group_name= $group_data['cn'][0];
                        if (in_array('posixGroup', $group_data['objectClass']))
                        {
                            if (!in_array($_group_name, array_keys($this->posixgroup_members)))
                            {
                                $this->posixgroup_members[$_group_name]= array();
                            }
                            $this->posixgroup_members[$_group_name][]= $_uid;
                        }
                        //if (in_array('groupOfNames', $group_data['objectClass']) or in_array('groupOfUniqueNames', $group_data['objectClass']))
                        //{
                        //    /* FIXME: handle group of (unique) names */
                        //
                        //    if (isset($user_data['main_account']['dn']))
                        //    {
                        //        /* FIXME: store DN in $user_data (in accountStatusCheck()) */
                        //        $this->groupofnames_members[]= $user_data['main_account']['dn'];
                        //    }
                        //}
                    }
                }
            }

            if ($this->failure_in_this_phase===FALSE)
            {
                $this->accountStatusCheck();
                $smarty->assign("data",$this->csvinfo['data_preldap']);
                $smarty->assign ("groups_imported",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $this->groupStatusCheck();
                $smarty->assign ("data",$this->csvinfo['data_preldap_posixgroups']);
            }
        }

        /*
         * PHASE 08
         */
        elseif (isset ($_POST['phase_08'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);
            $smarty->assign ("groups_imported",TRUE);

            $this->addLDAPUserObjectsToGroups($this->posixgroup_members);

            /* FIXME: present a message dialog box that informs on the statistics of
             *        the LDAP group membership updates.
             */

            $this->accountStatusCheck();
            $smarty->assign("data",$this->csvinfo['data_preldap']);
            if ($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_groupmembers_reviewed",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
            }
        }
        /*
         * PHASE 09
         */
        elseif (isset ($_POST['phase_09'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);
            $smarty->assign ("groups_imported",TRUE);
            $smarty->assign ("accounts_groupmembers_reviewed",TRUE);

            if ($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_groupmembers_updated",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
            }
        }
        /*
         * PHASE 10
         */
        elseif (isset ($_POST['phase_10'])) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);
            $smarty->assign ("groups_imported",TRUE);
            $smarty->assign ("accounts_groupmembers_reviewed",TRUE);
            $smarty->assign ("accounts_groupmembers_updated",TRUE);

            if ($this->failure_in_this_phase===FALSE)
            {

                /*
                 * FIXME: If automatic password generation is used, present a download link
                 *        for a generated credentials sheet of paper.
                 */

                /* Render the final SUCCESS page */
                $smarty->assign ("cleanup_completed",TRUE);

            }
            else {

                /* FIXME: no clue what to do on failures here... We will see. */

            }
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

    function compareObjects($object_a, $object_b, $attrs=array("sn", "givenName"), $prefix="")
    {

        $unequal = array();
        foreach ($attrs as $key => $attr)
        {

            $val_a = $val_b = NULL;
            if (isset($object_a[$attr][0]))
            {
                $val_a = $object_a[$attr][0];
            }
            if (isset($object_b[$attr][0]))
            {
                $val_b = $object_b[$attr][0];
            }

            if ($val_a!==$val_b)
            {
                $unequal[] = $prefix.$attr;
            }
        }
        if(empty($unequal))
        {
            /* if TRUE is returned, objects regarding the given attributes are identical */

            return TRUE;
        }
        else {
            /* otherwise return those attribute names that did not give a match */
            return $unequal;
        }
    }

    function accountStatusCheck()
    {
        /* If we don't have the $this->csvinfo['data_preldap'] dataset, we are doomed to fail... */
        if (!is_array($this->csvinfo['data_preldap']) or (empty($this->csvinfo['data_preldap'])))
        {
            $this->failure_in_this_phase= TRUE;
            return;
        }

        $this->_ldap->cd($this->config->current['BASE']);
        $ldapsearch = $this->_ldap->search("(objectClass=gosaAccount)",array("sn","givenName","gender","dateOfBirth", "mail"));

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        while ($gosa_account = $this->_ldap->fetch($ldapsearch))
        {
            foreach ($this->csvinfo['data_preldap'] as $key => $row)
            {
                if (isset($row['main_account']))
                {
                    /* detect status */
                    if ($this->compareObjects($row['main_account'], $gosa_account, array("sn", "givenName", "gender", "dateOfBirth"))===TRUE)
                    {
                        $expected_base= $this->genAccountBase($row['main_account'], $row['groups']);
                        $expected_account_dn= "uid=".$row['main_account']['uid'][0].','.get_people_ou().$expected_base;
                        $gosa_account_dn= $gosa_account['dn'];

                        if(strtolower($gosa_account_dn)==strtolower($expected_account_dn))
                        {
                            $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'exists';
                        }
                        else {
                            $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'exists-elsewhere';
                        }

                        /* detect actions */
                        if($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]==='exists-elsewhere')
                        {
                            $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= 'move';
                        }
                        elseif($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]==='exists')
                        {
                            /*
                             * FIXME: Hard-code a "skip" action here in case we found a matching account in LDAP.
                             *        Later, we need to make actions more configurable...
                             */
                            $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= 'skip';
                        }
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn_ldap']= array($gosa_account_dn);
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn']= array($expected_account_dn);
                        $this->csvinfo['data_preldap'][$key]['main_account']['_base']= array($expected_base);
                    }
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $aux_key => $aux_account)
                    {
                        if ($this->compareObjects($aux_account, $gosa_account, array("sn", "givenName", "mail"))===TRUE)
                        {
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]= 'exists';
                            /*
                             * FIXME: Hard-code a "skip" action here in case we found a matching account in LDAP.
                             *        Later, we need to make actions more configurable...
                             */
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['action'][0]= 'skip';

                            /* break out of the inner loop if the $gosa_account has been found */
                        }
                    }
                }
            }
        }
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {
            if (isset($row['main_account']))
            {
                if ($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]== 'unchecked')
                {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'not-found';
                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= 'create';
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $aux_key => $aux_account)
                    {
                        if ($this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]== 'unchecked')
                        {
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_status'][0]= 'not-found';
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$aux_key]['_actions'][0]= 'create';
                        }
                    }
                }
            }
        }
    }

    function normalizeGenderString($gender)
    {
        switch (strtolower($gender))
        {
            case "male":
                $gender="M";
                break;
            case _("male"):
                $gender="M";
                break;
            case "female":
                $gender="F";
                break;
            case _("female"):
                $gender="F";
                break;
        }
        return $gender;
    }

    function groupStatusCheck()
    {
        $this->_ldap->cd($this->config->current['BASE']);
        $ldapsearch = $this->_ldap->search("(objectClass=posixGroup)",array("cn","memberUid","mail", "description"));

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        while ($gosa_posix_group = $this->_ldap->fetch($ldapsearch))
        {
            foreach ($this->csvinfo['data_preldap'] as $key => $row)
            {
                if (isset($row['groups']))
                {
                    foreach ($row['groups'] as $group_key => $group)
                    {

                        $_break_out_of_inner_loop= FALSE;

                        /* detect status */
                        if ($this->compareObjects($group, $gosa_posix_group, array("cn"))===TRUE)
                        {
                            $expected_group_dn= 'cn='.$group['cn'][0].','.get_groups_ou().$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];
                            $gosa_group_dn= $gosa_posix_group['dn'];
                            if(strtolower($gosa_group_dn)==strtolower($expected_group_dn))
                            {
                                $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]= 'exists';
                            }
                            else {
                                $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]= 'exists-elsewhere';
                            }

                        }

                        /* detect actions */
                        $_actions= array();
                        if($this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]==='exists-elsewhere')
                        {
                            $_actions[]= "move";
                        }
                        if ($this->compareObjects($group, $gosa_posix_group, array("cn"))===TRUE)
                        {

                            /* if group exists, check for to-be-dropped members */
                            if(
                               ($this->csvinfo['flush_members']) and
                               (isset($gosa_posix_group['memberUid']['count']) && ($gosa_posix_group['memberUid']['count'] > 0))
                              )
                            {
                                $_actions[]= "flush-all-members";
                            }
                            $_check_attrs= $this->compareObjects($group, $gosa_posix_group, array("mail","description"), "update-");
                            if ($_check_attrs===TRUE)
                            {
                            }
                            else {
                                $_actions= array_merge($_actions, $_check_attrs);
                            }

                            /* check if the user's main account is in this group */
                            if(in_array('posixGroup', $group['objectClass']) and isset($row['main_account']['uid'][0]))
                            {
                                $_group_name= $group['cn'][0];
                                $_uid= $row['main_account']['uid'][0];
                                $_members= $gosa_posix_group['memberUid'];
                                if(!in_array($_uid, $_members) or (in_array('flush-all-members', $_actions)))
                                {
                                    $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions'][0]= 'update-memberships';
                                    $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions'][$_group_name]= 'add';
                                }
                                else {
                                    $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions'][$_group_name]= 'none';
                                }
                            }

                            /* no action found till here, set the "none" action */
                            if(empty($_actions))
                            {
                                $_actions[]= "none";
                            }
                            $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_actions'][0]= implode(",", $_actions);

                            /* break out of the inner loop if the $gosa_account has been found */
                            break;
                        }
                    }
                }
            }
        }

        $this->csvinfo['data_preldap_posixgroups'] = array();
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {
            if (isset($row['groups']))
            {
                foreach ($row['groups'] as $group_key => $group)
                {
                    if ($this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]== 'unchecked')
                    {
                        $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_status'][0]= 'not-found';
                        $this->csvinfo['data_preldap'][$key]['groups'][$group_key]['_actions'][0]= 'create';
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

    function CSVRowSanityChecksOk($csv_row)
    {
        $ok = FALSE;
        if ( (empty ($csv_row['sn'])) || (empty ($csv_row['givenName']))|| (!isset ($csv_row['sn']))|| (!isset ($csv_row['givenName']))) {

            /* Output Error about missing the least set of attributes */
            msg_dialog::display (_ ("Error"),sprintf (_ ("Need at least %s and %s to create users (Check line %d in CSV file)!"),bold ("sn"),bold ("givenName"), $idx+1),ERROR_DIALOG);

        }

        /*
         *
         * FIXME: Add plenty of more CSV data sanity checks here!!!!
         *
         */

        else {
            $ok = TRUE;
        }

        if(!$ok)
        {
            $this->failure_in_this_phase = TRUE;
        }

        return $ok;
    }

    function importLDAPGroupObject($group_name, $group_data)
    {
        if((strpos($group_data['_actions'][0],'move')!==FALSE) or (strpos($group_data['_actions'][0],'move')!==FALSE))
        {

            /* Instantiate a new group object via GOsa²'s API */

            $grouptab= new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'],'new', "groups" );
            $grouptab->by_object['group']->base= $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];

        }
        else{

            /* Retrieve group object from LDAP via GOsa²'s API */
            $grouptab= new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'],'cn='.$group_name.','.get_groups_ou().$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']], "groups" );

        }

        if(strpos($group_data['_actions'][0],'move')!==FALSE)
        {
            /* do an ldapsearch for the source object and prepare a copy+paste action */

            $_ldapsearch= $this->_ldap->search("(&(objectClass=posixGroup)(cn=".$group_name."))", array("*"));

            /* There should only be _ONE_ object of this CN in the given LDAP (sub)tree scope.
             * Thus, only fetching the first obejct.
             */
            $source= $this->_ldap->fetch($_ldapsearch);

            if(!empty($source))
            {
                foreach($grouptab->by_object as $pname => $plugin)
                {
                    $grouptab->by_object[$pname]->prepareForCopyPaste($source);
                }
                $remove_later_grouptab= new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'],$source['dn'], "groups" );
            }
        }

        /* Collect group properties from $group_data */
        $_group_properties= array(
            "cn" => $group_data['cn'][0],
        );
        if((strpos($group_data['_actions'][0],'update-description')!==FALSE) || (strpos($group_data['_actions'][0],'create')!==FALSE))
        {
            $_group_properties["description"]= $group_data['description'][0];
        }
        if((strpos($group_data['_actions'][0],'update-mail')!==FALSE) || (strpos($group_data['_actions'][0],'create')!==FALSE))
        {
            $_group_properties["mail"]= $group_data['mail'][0];
        }

        /*
         * FIXME: Make mail address creation (from CN) configurable somewhere...
         */

        /* if we do import mail properties, make sure the mailgroup object is activated */
        if ((isset($_group_properties['mail']) && isset($grouptab->by_object['mailgroup'])))
        {
            $grouptab->by_object['mailgroup']->is_account= TRUE;
        }
        /* Populate sub-object attributes for $grouptab with same values */
        foreach($_group_properties as $attr => $val)
        {
            $grouptab->$attr = $val;
            foreach($grouptab->by_object as $pname => $plugin)
            {
                if(isset($grouptab->by_object[$pname]->$attr))
                {
                    $grouptab->by_object[$pname]->$attr = $val;
                }
            }
        }

        /* Drop all group members, if requested */
        if(strpos($group_data['_actions'][0],'flush-all-members')!==FALSE)
        {
            $group_members=$grouptab->by_object['group']->memberUid;
            foreach($group_members as $group_member)
            {
                $grouptab->by_object['group']->removeUser($group_member);
            }
        }

        /* Run GOsa²'s groups/group checks */
        if(count($grouptab->check()))
        {
            msg_dialog::displayChecks($grouptab->check());

            /*
             * FIXME: collect failure statistics here!!!
             */

            $this->failure_in_this_phase= TRUE;

        } else
        {

            if(isset($remove_later_grouptab))
            {
                $remove_later_grouptab->delete();
            }
            /* Save group object to LDAP */
            $grouptab->save();

        }

        /* Present a status update in GOsa²''s logging system (/var/log/syslog mostly) */
        if(strpos($group_data['_actions'][0],'create')!==FALSE)
        {
            new log("create","groups/group",$grouptab->dn, array(), "New group created via SchoolManager add-on.");
        }
        elseif(strpos($group_data['_actions'][0],'none')!==FALSE) {
            new log("modify","groups/group",$grouptab->dn, array(), "Existing group modified via SchoolManager add-on.");
        }
    }


    function importLDAPUserObject($user_data)
    {
        /* only perform on this user dataset if _actions does not contain "skip"... */
        if(strpos($user_data['_actions'][0],'skip')!==FALSE)
        {
            return;
        }
        if((strpos($user_data['_actions'][0],'update')!==FALSE) or (strpos($user_data['_actions'][0],'create')!==FALSE) or (strpos($user_data['_actions'][0],'move')!==FALSE))
        {
            $_user_properties_base= array();
            $_user_properties_extras= array();

            if (strpos($user_data['_actions'][0],'create')!==FALSE)
            {
                /* instantiate a new user object */
                $usertab= new usertabs($this->config, $this->config->data['TABS']['USERTABS'],'new');

                /*
                 * FIXME: If $user_data['uid'][0] is set to "%auto%" or is empty, do uid generation magic here!!!
                 */

                /*
                 * FIXME: If $user_data['userPassword'][0] is set to "%auto%" or is empty, do password generation magic here!!!
                 */

                /* The below attributes must exist and be set with meaningful data once we arrive here!!! */
                $_user_properties_base["uid"]= $user_data['uid'][0];
                $_user_properties_base["sn"]= $user_data['sn'][0];
                $_user_properties_base["givenName"]= $user_data['givenName'][0];
                $_user_properties_extras["gender"]= $user_data['gender'][0];
                $_user_properties_extras["dateOfBirth"]= $user_data['dateOfBirth'][0];
            }
            else {

                /* Retrieve user object from LDAP */
                $usertab= new usertabs($this->config, $this->config->data['TABS']['USERTABS'],$user_data['_dn_ldap'][0]);
            }

            /* on existing account, we allow the below attributes to be updated */

            if(isset($user_data_extras['departmentNumber'])) { $_user_properties["departmentNumber"]= $user_data['departmentNumber'][0]; }
            if(isset($user_data_extras['mail'])) { $_user_properties["mail"]= $user_data['mail'][0]; }

            /* test if base DN exists, this is only relevant for actions "create" and "move" but does not hurt for updates, either */
            $this->_ldap->cat($user_data['_base'][0]);
            if(!$this->_ldap->fetch())
            {

                /* change LDAP "dir" to the place where we want to create sub-OUs... */
                if($user_data['_template'][0] != 0)
                {
                    $template_dn=  $this->csvinfo['templates']['DNs'][$user_data['_template'][0]];
                    $this->_ldap->cd(dn2base($template_dn));
                }
                else {
                    $this->_ldap->cd($this->config->current['BASE']);
                }
                /* create base DN as a GOsa² department, presuming the the base DN's parent DN already exists */
                $_ou= preg_replace("/^ou=([^,]+),.*/", "$1", $user_data['_base'][0]);
                $deptab= new deptabs($this->config, $this->config->data['TABS']['DEPTABS'],'new', "deptabs" );
                $deptab->by_object['department']->ou= $_ou;
                $deptab->by_object['department']->description= sprintf(_("Class: %s"), $_ou);
                $deptab->save();
            }

            if (
                (strpos($user_data['_actions'][0],'create')!==FALSE) and
                ($user_data['_template'][0] != 0)
               )
            {
                /* adapt new user object from a GOsa user object template */
                $usertab->adapt_from_template($template_dn, array("uid","cn","givenName","sn"));

                $usertab->by_object['user']->base= $user_data['_base'][0];
            }
            /* Populate sub-object attributes for $usertab with same values */
            foreach($_user_properties_extras as $attr => $val)
            {
                $usertab->$attr = $val;
                foreach($usertab->by_object as $pname => $plugin)
                {
                    if(isset($usertab->by_object[$pname]->$attr))
                    {
                        $usertab->by_object[$pname]->$attr = $val;
                    }
                }
            }

            /* Insert into the LDAP tree */
            if(count($usertab->check())) {

              msg_dialog::displayChecks($usertab->check());

              /*
               * FIXME: collect failure statistics here!!!
               */

              $this->failure_in_this_phase= TRUE;

            } else {

                $usertab->save();

                if (strpos($user_data['_status'][0],'exists')===FALSE)
                {
                    if(!change_password($usertab->dn,$user_data['userPassword'][0],FALSE,'','', $message)){
                        msg_dialog::displayChecks(array($message));
                    }
                }

                if(strpos($user_data['_actions'][0],'move')!==FALSE)
                {
                    $usertab->by_object['user']->rename($usertab->dn, $user_data['_dn'][0]);
                }

                /*
                 * FIXME: collect success statistics here!!!
                 */

            }

            /* Present a status update in GOsa²''s logging system (/var/log/syslog mostly) */
            if(strpos($user_data['_actions'][0],'create')!==FALSE)
            {
                new log("create","users/user",$usertab->dn, array(), "New user created via SchoolManager add-on.");
            }
            elseif(strpos($user_data['_actions'][0],'none')!==FALSE) {
                new log("modify","users/user",$usertab->dn, array(), "Existing user modified via SchoolManager add-on.");
            }
        }
    }

    function addLDAPUserObjectsToGroups($group_members_data)
    {
        foreach($group_members_data as $group_name => $new_members)
        {
            $grouptab= new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'],'cn='.$group_name.','.get_groups_ou().$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']], "groups" );

            $current_members=$grouptab->by_object['group']->memberUid;
            foreach($new_members as $new_member)
            {
                if(!in_array($new_member, $current_members))
                {
                    $grouptab->by_object['group']->addUser($new_member);
                }
            }
            /* Run GOsa²'s groups/group checks */
            if(count($grouptab->check()))
            {
                msg_dialog::displayChecks($grouptab->check());

                /*
                 * FIXME: collect failure statistics here!!!
                 */

                $this->failure_in_this_phase= TRUE;

            } else
            {
                /* Save group object to LDAP */
                $grouptab->save();
            }
        }
    }

    function genAccountBase($user_data, $groups_data)
    {
        if($user_data['_template'][0]!=0)
        {
            $template_dn=  $this->csvinfo['templates']['DNs'][$user_data['_template'][0]];
            $template_base= preg_replace("/^[^,]+,".preg_quote(get_people_ou(), '/i')."/", '', $template_dn);
        }
        else {
            $template_base= $this->config->current['BASE'];
        }

        /* Put accounts into a sub-OU of the template's base DN if requested */
        if($this->csvinfo['accounts_in_class_ou']===TRUE)
        {
            $_class_group= "";
            foreach($groups_data as $group_key => $group_data)
            {

                if(strpos($group_data['cn'][0], 'class_')!==FALSE)
                {
                    $_class_group= 'ou='.str_replace('class_', '', $group_data['cn'][0]).',';
                    /* we take the first class group we find...
                     * presuming that students only have _one_ class group assigned(!)
                     */
                    break;
                }
            }
            $template_base= $_class_group.$template_base;
        }
        return $template_base;
    }

    function validGroupName($group_name)
    {
        /* FIXME: This can be better done with regular expressions!!!
         */
       return str_replace('.','', str_replace(':', '-', str_replace(' ', '_',strtolower (trim($group_name)))));
    }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
