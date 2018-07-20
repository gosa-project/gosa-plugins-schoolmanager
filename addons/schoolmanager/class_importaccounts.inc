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
class importaccounts extends plugin{

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

    var $default_template_main = "";
    var $default_template_aux = "";

    /* constructor */
    function __construct(&$config,$dn=NULL) {

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

        $smarty->assign('available_delimiters', array(
                               0 => '"," - '._("komma separated values"),
                               1 => '";" - '._("semicolon separated values"),
                               2 => '<-> - '._("tabstop separated values"),
                               3 => '" " - '._("blank separated values")));
        $smarty->assign('preset_delimiter_id', 0);

        $this->csvinfo['attrs']=$this->getAttributes();
        $this->csvinfo['attrs'][] = "---";
        /* Check permissions for import */
        $acl=$this->ui->get_permissions ($this->config->current['BASE'],"all/all");
        if (!preg_match ("/w/",$acl)) {
            if (isset ($_POST['file_uploaded']) ||
                isset ($_POST['import_configured']) ||
                isset ($_POST['data_sorted']) ||
                isset ($_POST['accounts_reviewed']) ||
                isset ($_POST['accounts_imported']) ||
                isset ($_POST['groups_reviewed']) ||
                isset ($_POST['groups_imported']) ||
                isset ($_POST['accounts_groupmembers_reviewed']) ||
                isset ($_POST['accounts_groupmembers_updated']) ||
                isset ($_POST['cleanup_completed'])) {
                msg_dialog::display (_ ("Permission error"),_ ("You've no permission to import user data!"),ERROR_DIALOG);
            }
            return ($smarty->fetch (get_template_path ('content_importaccounts.tpl',TRUE)));
        }

        /*
         * PHASES
         */

        /* reset our failure status from last template view... */
        $this->failure_in_this_phase = FALSE;

        /*
         * PHASE 01
         */
        if (isset ($_POST['phase_01']) && isset($_POST['file_uploaded']) && (!isset($_POST['cancel_import']))) {

            /* Check if theres a file uploaded */
            if (!empty ($_FILES['userfile']['name'])) {
                $handle=NULL;
                $filename=gosa_file_name ($_FILES['userfile']['tmp_name']);
                if ( !isset ($_FILES['userfile']['name'])) {
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

                    $parser_ok = importaccounts::parseCSV($raw_csv_data);

                    if ($parser_ok!==FALSE)
                    {

                        $this->csvinfo["template_main"]= 0;
                        $this->csvinfo["template_aux"]= 0;

                        /* Populate the Template Selectors for PHASE 2 */

                        /* Search all Templates    */
                        $this->csvinfo['templates'] = array();
                        $this->csvinfo['templates']['ldapsearch'] = $this->_ldap->search("(objectClass=gosaUserTemplate)",array("*"));

                        /* add the "None" template first */
                        $this->csvinfo['templates']['formfields'] = array();
                        $this->csvinfo['templates']['formfields'][] = "None";
                        $this->csvinfo['templates']['DNs'] = array();
                        $this->csvinfo['templates']['DNs'][]   = "";

                        # Set the default mail domain for group accounts here...
                        $this->csvinfo['domain_groups']= "intern";

                        /* add found gosaUserTemplate objects */
                        $i = 0;
                        while($result = $this->_ldap->fetch($this->csvinfo['templates']['ldapsearch'])){

                            $this->csvinfo['templates']['formfields'][] = $result['sn'][0]." - ".$this->config->idepartments[preg_replace("/^[^,]+,".preg_quote(get_people_ou(), '/')."/i", "", $result['dn'])];
                            $this->csvinfo['templates']['DNs'][] = $result['dn'];

                            $i = $i + 1;

                            if ( ($this->default_template_main != "") && (preg_match ( "/".$this->default_template_main."/i", $result['sn'][0] ) ) ) {
                                $this->csvinfo["template_main"]= $i;

                                # obtain mail domain from pre-selected main user template, if that is available
                                if (strpos($result['mail'][0], '@')) {
                                    $this->csvinfo["domain_groups"] = preg_split('/@/', $result['mail'][0],2)[1];
                                }
                            }
                            if ( ($this->default_template_aux != "") && (preg_match ( "/".$this->default_template_aux."/i", $result['sn'][0] ) ) ) {
                                $this->csvinfo["template_aux"]= $i;
                            }

                        }

                        $smarty->assign("templates",$this->csvinfo['templates']['formfields']);

                        /* Search OUs with objectClass gosaDepartment */
                        $this->csvinfo['ou_tree'] = array();
                        $this->csvinfo['ou_tree']['ldapsearch'] = $this->_ldap->search("(objectClass=gosaDepartment)",array("ou","description"));

                        /* create arrays for search results */
                        $this->csvinfo['ou_tree']['formfields'] = array();
                        $this->csvinfo['ou_tree']['OUs'] = array();

                        /* add found gosaDepartment objects */
                        $i = 0;
                        $default_ou_groups = 0;
                        while($result = $this->_ldap->fetch($this->csvinfo['ou_tree']['ldapsearch'])) {
                            $this->csvinfo['ou_tree']['OUs'][] = $result['ou'][0];
                            $this->csvinfo['ou_tree']['formfields'][] = $result['ou'][0]." - ".$result['description'][0];
                            $this->csvinfo['ou_tree']['DNs'][] = $result['dn'];
                            if (strcasecmp($result['ou'][0], "schoolmanager") == 0) {
                                $default_ou_groups = $i;
                            }
                            $i = $i + 1;
                        }

                        $smarty->assign("ous_available",$this->csvinfo['ou_tree']['formfields']);

                        /* import configuration defaults, only set them if PHASE 1 has been successful */
                        $this->csvinfo['ou_groups']= $default_ou_groups;
                        $this->csvinfo['accounts_in_class_ou']= TRUE;
                        $this->csvinfo['try_mail_as_uid']= TRUE;
                        $this->csvinfo['add_course_members_to_class_group']= TRUE;

                        /* provide pre-set values for account template forms */
                        $smarty->assign("preset_template_".$import_account_type, $this->csvinfo['template_main']);
                        $smarty->assign("preset_template_".$import_account_type."_aux", $this->csvinfo['template_aux']);
                        $smarty->assign("preset_ou_groups", $this->csvinfo['ou_groups']);
                        $smarty->assign("preset_domain_groups", $this->csvinfo['domain_groups']);
                        $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
                        $smarty->assign("preset_try_mail_as_uid", $this->csvinfo['try_mail_as_uid']);
                        $smarty->assign("preset_add_course_members_to_class_group", $this->csvinfo['add_course_members_to_class_group']);
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
        elseif (isset ($_POST['phase_02']) && isset($_POST['import_configured']) && (!isset($_POST['cancel_import']))) {

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
                if (isset ($_POST["accounts_in_class_ou"]))
                {
                    $this->csvinfo['accounts_in_class_ou'] = TRUE;
                }
                else {
                    $this->csvinfo['accounts_in_class_ou'] = FALSE;
                }
                if (isset ($_POST["try_mail_as_uid"]))
                {
                    $this->csvinfo['try_mail_as_uid'] = TRUE;
                }
                else {
                    $this->csvinfo['try_mail_as_uid'] = FALSE;
                }
                if (isset ($_POST["add_course_members_to_class_group"]))
                {
                    $this->csvinfo['add_course_members_to_class_group'] = TRUE;
                }
                else {
                    $this->csvinfo['add_course_members_to_class_group'] = FALSE;
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
                    $smarty->assign ("data_size",$this->csvinfo['num_rows']);
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
                    $smarty->assign("preset_accounts_in_class_ou", $this->csvinfo['accounts_in_class_ou']);
                    $smarty->assign("preset_try_mail_as_uid", $this->csvinfo['try_mail_as_uid']);
                    $smarty->assign("preset_add_course_members_to_class_group", $this->csvinfo['add_course_members_to_class_group']);
                }
            }
        }

        /*
         * PHASE 03
         */
        elseif (isset ($_POST['phase_03']) && isset($_POST['data_sorted']) && (!isset($_POST['cancel_import']))) {

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
        elseif (isset ($_POST['phase_04']) && isset($_POST['accounts_reviewed']) && (!isset($_POST['cancel_import']))) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);

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
                if((isset($user_data['aux_accounts']) && (!empty($user_data['aux_accounts']))))
                {
                    foreach($user_data['aux_accounts'] as $idx_aux => $aux_account)
                    {
                        /* import aux account $aux_account_data */
                        $this->importLDAPUserObject($aux_account);
                    }
                }
            }

            if($this->failure_in_this_phase===FALSE)
            {
                /* FIXME: present a message dialog box that informs on the statistics of
                 *        the LDAP group import. (How many groups got created? How many
                 *        groups have been updated? How many groups were not empty and
                 *        required dropping of all members? Etc.
                 */

                $smarty->assign ("accounts_reviewed",TRUE);

                $this->accountStatusCheck();
                $smarty->assign("data",$this->csvinfo['data_preldap']);

            }
            else {

                /* show failure messages */
                foreach($this->failure_messages as $msg)
                {
                    msg_dialog::displayChecks($msg);
                }

                /* prepare for reloading this phase's web page again */
                $this->accountStatusCheck();
                $smarty->assign("data",$this->csvinfo['data_preldap']);
            }
        }
        /*
         * PHASE 05
         */
        elseif (isset ($_POST['phase_05']) && isset($_POST['accounts_imported']) && (!isset($_POST['cancel_import'])))
        {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);

            if($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_imported",TRUE);

                foreach($this->csvinfo['data_preldap'] as $idx => $user_data)
                {
                    if((isset($user_data['main_account']) && (!empty($user_data['main_account']))))
                    {
                        $_uid= $user_data['main_account']['uid'][0];

                        $_all_groups= array();
                        if(isset($user_data['groups'])) { $_all_groups= array_merge($_all_groups, $user_data['groups']); }
                        if(isset($user_data['aux_accounts_groups'])) { $_all_groups= array_merge($_all_groups, $user_data['aux_accounts_groups']); }

                        foreach($_all_groups as $group_key => $group_data)
                        {

                            if(isset($group_data['_key'][0])) { $_group_type_key= $group_data['_key'][0]; }
                            else { $_group_type_key= 'groups'; }

                            /* find+replace {%uid} in group CN with the user's uid string */
                            $_group_name= $group_data['cn'][0];
                            if(preg_match('/\{%uid\}/', $_group_name))
                            {
                                $this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key]['cn'][0]= str_replace('{%uid}', $_uid, $_group_name);
                            }
                            if(preg_match('/\{%uid\}/', $group_key))
                            {
                                $this->csvinfo['data_preldap'][$idx][$_group_type_key][str_replace('{%uid}', $_uid, $group_key)]= $this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key];
                                unset($this->csvinfo['data_preldap'][$idx][$_group_type_key][$group_key]);
                            }
                        }
                    }
                }

                $this->groupStatusCheck();

                $smarty->assign ("data_groups", $this->csvinfo['data_preldap_posixgroups']);
                $smarty->assign ("data_ogroups", $this->csvinfo['data_preldap_ogroups']);
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
        elseif (isset ($_POST['phase_06']) && isset($_POST['groups_reviewed']) && (!isset($_POST['cancel_import'])))
        {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);

            foreach($this->csvinfo['data_preldap_posixgroups'] as $group_name => $group_data)
            {

                /* only perform on this group if _actions is not "none" */
                if(strpos($group_data['_actions'][0],'none')===FALSE)
                {

                    $this->importLDAPGroupObject($group_name, $group_data);

                }
            }

            foreach($this->csvinfo['data_preldap_ogroups'] as $group_name => $group_data)
            {

                /* only perform on this group if _actions is not "none" */
                if(strpos($group_data['_actions'][0],'none')===FALSE)
                {

                    $this->importLDAPGroupObject($group_name, $group_data);

                }
            }

            $this->groupStatusCheck();

            $smarty->assign ("data_groups", $this->csvinfo['data_preldap_posixgroups']);
            $smarty->assign ("data_ogroups", $this->csvinfo['data_preldap_ogroups']);

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
        elseif (isset ($_POST['phase_07']) && isset($_POST['groups_imported']) && (!isset($_POST['cancel_import']))) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);

            $this->accountStatusCheck();
            $this->groupStatusCheck();
            $this->prepareGroupMemberships();
            $smarty->assign("data",$this->csvinfo['data_preldap']);

            if ($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("groups_imported",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $this->groupStatusCheck();
                $smarty->assign ("data_groups", $this->csvinfo['data_preldap_posixgroups']);
                $smarty->assign ("data_ogroups", $this->csvinfo['data_preldap_ogroups']);
            }
        }

        /*
         * PHASE 08
         */
        elseif (isset ($_POST['phase_08']) && isset($_POST['accounts_groupmembers_reviewed']) && (!isset($_POST['cancel_import']))) {

            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("import_configured",TRUE);
            $smarty->assign ("data_sorted",TRUE);
            $smarty->assign ("accounts_reviewed",TRUE);
            $smarty->assign ("accounts_imported",TRUE);
            $smarty->assign ("groups_reviewed",TRUE);
            $smarty->assign ("groups_imported",TRUE);

            $this->addLDAPUserObjectsToGroups($this->posixgroup_members);
            $this->addLDAPUserObjectsToOGroups($this->groupofnames_members);
            $this->moveLDAPPrimGroupObjects();

            /* FIXME: present a message dialog box that informs on the statistics of
             *        the LDAP group membership updates.
             */

            $this->accountStatusCheck();
            $this->groupStatusCheck();
            $smarty->assign("data",$this->csvinfo['data_preldap']);
            if ($this->failure_in_this_phase===FALSE)
            {
                $smarty->assign ("accounts_groupmembers_reviewed",TRUE);
            }
            else {
                /* prepare for reloading this phase's web page again */
                $this->prepareGroupMemberships();
            }
        }
        /*
         * PHASE 09
         */
        elseif (isset ($_POST['phase_09']) && isset($_POST['accounts_groupmembers_updated']) && (!isset($_POST['cancel_import']))) {

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
        elseif (isset ($_POST['phase_10']) && isset($_POST['cleanup_completed']) && (!isset($_POST['cancel_import']))) {

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
        return ($smarty->fetch (get_template_path ('content_importaccounts.tpl',TRUE)));
    }

    function parseCSV($raw_csv_data)
    {

        $this->csvinfo['delimiter_id'] = $_POST['delimiter_id'];
        $this->csvinfo['skip_first_line'] = $_POST['csv_with_column_headers'];

        $raw_csv_data = str_replace('\r', '', $raw_csv_data);

        $lines=preg_split ("/\n/",$raw_csv_data);
        $num_columns=0;
        $rest=0;
        $data=array ();

        if ($this->csvinfo['skip_first_line']) {
                array_shift ($lines);
        }

        /* check column count, if it stays zero, we probably don't have a comma separated CSV file */
        if (is_array ($lines)) {
            foreach ($lines as $line) {

                $line = trim ($line);

                /* ignore empty lines */
                if (!$line) {
                    continue;
                }

                /* continue if theres a comment */
                if (substr ($line, 0, 1)=="#") {
                    continue;
                }

                $cells = array();

                switch ($this->csvinfo['delimiter_id'])
                {
                    case 0:
                        /* komma separated values */
                        $delimiter_char = ",";
                        break;
                    case 1:
                        /* semikolon separated values */
                        $delimiter_char = ";";
                        break;
                    case 2:
                        /* tabstop separated values */
                        $delimiter_char = "\t";
                        break;
                    case 3:
                        /* blank separated values */
                        $delimiter_char = " ";
                        break;
                }

                $raw_cells=explode ($delimiter_char,$line);

                $concatenating = FALSE;
                $concat_cell = "";
                $concat_quote = "";

                foreach ($raw_cells as $cell) {

                    if ((!$concatenating) && (substr($cell, 0, 1) == '"')) {
                        $concat_cell = substr($cell, 1);
                        $concatenating = TRUE;
                        $concat_quote = '"';
                    }
                    elseif ((!$concatenating) && (substr($cell, 0, 1) == "'")) {
                        $concat_cell = substr($cell, 1);
                        $concatenating = TRUE;
                        $concat_quote = '"';
                    }
                    elseif (($concatenating) && (substr($cell, -1) == $concat_quote)) {
                        $concat_cell = $concat_cell.$delimiter_char.substr($cell, 0, -1);
                        $cells[] = $concat_cell;
                        $concatenating = FALSE;
                        $concat_cell = "";
                        $concat_quote = "";
                    }
                    elseif ($concatenating)
                    {
                        $concat_cell = $concat_cell.$delimiter_char.$cell;
                    }
                    else {
                        $cells[] = $cell;
                    }
                }

                if (count ($cells)>$num_columns) {
                    $num_columns=count ($cells);
                }

                $data[] = $cells;

            }

        }

        /* parse rows and import into $this->csvinfo */
        if ($num_columns >= 1) {

            /* Generate array with output info  */

            foreach ($data as $row) {

                /* fill up rows if less cells then num_columns */
                if (is_array ($row)) {

                    /* cell count less than num_columns, attach some empty fields */
                    if ( count($row) <= $num_columns ) {
                        $rest=$num_columns - count($row);
                        for ($i=0;$i<$rest;$i++) {
                            $row[]="";
                        }
                    }

                }

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

    function compareObjects($object_a, $object_b, $attrs=array("sn", "givenName"), $prefix="", $ci=FALSE)
    {

        $unequal = array();
        foreach ($attrs as $key => $attr)
        {

            $val_a = $val_b = NULL;
            if (isset($object_a[$attr][0]))
            {
                $val_a = ($ci===TRUE) ? strtolower($object_a[$attr][0]) : $object_a[$attr][0];
            }
            if (isset($object_b[$attr][0]))
            {
                $val_b = ($ci===TRUE) ? strtolower($object_b[$attr][0]) : $object_b[$attr][0];
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
        $ldapsearch = $this->_ldap->search("(objectClass=gosaAccount)",array("uid", "sn","givenName","gender","dateOfBirth", "mail", "departmentNumber", "employeeNumber"));

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        while ($gosa_account = $this->_ldap->fetch($ldapsearch))
        {
            foreach ($this->csvinfo['data_preldap'] as $key => $row)
            {
                if (isset($row['main_account']))
                {
                    /* detect status */
                    if ($this->compareObjects($row['main_account'], $gosa_account, array("sn", "givenName", "gender", "dateOfBirth"), "", TRUE)===TRUE)
                    {
                        if((!isset($row['main_account']['uid'][0])) || ($row['main_account']['uid'][0]==="{%uid}"))
                        {
                            $this->csvinfo['data_preldap'][$key]['main_account']['uid']= array($gosa_account['uid'][0]);
                            $row['main_account']['uid']= $this->csvinfo['data_preldap'][$key]['main_account']['uid'];
                        }
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
                        $_actions= array();
                        if($this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]==='exists-elsewhere')
                        {
                            $_actions[]= "move";
                        }

                        /* only update attributes that are already set in the main account dataset */
                        $_updatable_attrs= array();
                        foreach(array("mail","employeeNumber","departmentNumber") as $_attr)
                        {
                            if(isset($row['main_account'][$_attr]))
                            {
                                $_updatable_attrs[]= $_attr;
                            }
                        }

                        $_check_attrs= $this->compareObjects($row['main_account'], $gosa_account, $_updatable_attrs, "update-");
                        if ($_check_attrs===TRUE)
                        {
                        }
                        else {
                            $_actions= array_merge($_actions, $_check_attrs);
                        }

                        if(empty($_actions))
                        {
                            $_actions[]= 'none';
                        }

                        $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= implode(",", $_actions);

                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn_ldap']= array($gosa_account_dn);
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn']= array($expected_account_dn);
                        $this->csvinfo['data_preldap'][$key]['main_account']['_base']= array($expected_base);
                    }
                    elseif ($this->compareObjects($row['main_account'], $gosa_account, array("uid"), "", TRUE)===TRUE)
                    {
                        # The LDAP tree already has an account with this uid, this requires manual introspection

                        $expected_base= $this->genAccountBase($row['main_account'], $row['groups']);
                        $expected_account_dn= "uid=".$row['main_account']['uid'][0].','.get_people_ou().$expected_base;
                        $gosa_account_dn= $gosa_account['dn'];

                        if(strtolower($gosa_account_dn)==strtolower($expected_account_dn))
                        {
                            $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'uid-used-in-same-ou';
                        }
                        else
                        {
                            $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0]= 'uid-used-in-other-ou';
                        }
                        $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= 'ignore,check-manually';
                    }
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $idx_aux => $aux_account)
                    {
                        if ($this->compareObjects($aux_account, $gosa_account, array("sn", "givenName", "mail"), "", TRUE)===TRUE)
                        {
                            $expected_base= $this->genAccountBase($aux_account);
                            $gosa_account_dn= $gosa_account['dn'];

                            /* detect status */

                            /*
                             * FIXME: Support automatic moving of AUX accounts!!!
                             */

                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0]= 'exists';

                            $_actions= array();

                            /*
                             * FIXME: Hard-code a "none" action here in case we found a matching account in LDAP.
                             *        Later, we need to make actions more configurable...
                             */

                            /* only update attributes that are already set in the main account dataset */
                            $_updatable_attrs= array();
                            foreach(array("mail") as $_attr)
                            {
                                if(isset($row['main_account'][$_attr]))
                                {
                                    $_updatable_attrs[]= $_attr;
                                }
                            }

                            $_check_attrs= $this->compareObjects($aux_account, $gosa_account, $_updatable_attrs, "update-");
                            if ($_check_attrs===TRUE)
                            {
                            }
                            else {
                                $_actions= array_merge($_actions, $_check_attrs);
                            }

                            if(empty($_actions))
                            {
                                $_actions[]= 'none';
                            }

                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0]= implode(",", $_actions);

                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_dn_ldap']= array($gosa_account_dn);

                            /* FIXME: We need a check that DN generation for aux accounts will actually work, probably when
                             *        constructing $this->csvinfo['data_preldap'].
                             */
                            $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_base']= array($expected_base);
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
                    $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0]= 'create,set-password';

                    /* generate passwords for main accounts (if not provided) */
                    if((!isset($row['main_account']['userPassword'][0])) or ($row['main_account']['userPassword'][0]==="") or ($row['main_account']['userPassword'][0]==='{%userPassword}'))
                    {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0].= ",password-generated";
                        $this->csvinfo['data_preldap'][$key]['main_account']['userPassword'][0]= $this->randomPassword(12);
                    }

                    /* set the main account's base for object creation */
                    $expected_base= $this->genAccountBase($row['main_account'], $row['groups']);
                    if(isset($row['main_account']['uid'][0]))
                    {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn']= array("uid=".$row['main_account']['uid'][0].",".$expected_base);
                    }
                    else {
                        $this->csvinfo['data_preldap'][$key]['main_account']['_dn']= array("new");
                    }
                    $this->csvinfo['data_preldap'][$key]['main_account']['_base']= array($expected_base);
                }
                if (isset($row['aux_accounts']))
                {
                    foreach ($row['aux_accounts'] as $idx_aux => $aux_account)
                    {
                        if ($this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0]== 'unchecked')
                        {
                            if(isset($aux_account['sn']) and isset($aux_account['mail']) and isset($aux_account['givenName']))
                            {
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0]= 'not-found';
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0]= 'create,set-password';
                                /* set the main account's base for object creation */
                                $expected_base= $this->genAccountBase($aux_account);
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_base']= array($expected_base);
                            }
                            else {
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0]= 'data-incomplete';
                                $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0]= 'ignore';
                            }

                            /* generate passwords for aux accounts (if not provided) */
                            if((!isset($aux_account['userPassword'][0])) or ($aux_account['userPassword'][0]==="") or ($aux_account['userPassword'][0]==='{%userPassword}'))
                            {
                                if(strpos($this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_actions'][0], 'create')!==FALSE)
                                {
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0].= ",password-generated";
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['userPassword'][0]= $this->randomPassword(12);
                                }
                                else {
                                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['userPassword']= array("<keep>");
                                }
                            }
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

        /* initialize group actions for each main account and each aux account */
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {

            if(strpos($row['main_account']['_actions'][0],'ignore')!==FALSE)
            {
                continue;
            }

            /* check DN (i.e., location in the LDAP DIT) of primary group object */
            $expected_primgroup_dn= "cn=".$row['main_account']['uid'][0].",".get_groups_ou().dn2base($row['main_account']['_dn'][0]);

            $this->_ldap->cat($expected_primgroup_dn, array('dn'));
            if(!$this->_ldap->fetch())
            {
                $this->_ldap->search("(&(objectClass=posixGroup)(cn=".$row['main_account']['uid'][0]."))",array("dn"));
                if($this->_ldap->fetch())
                {
                    $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions']= array('move-primgroup');
                }
                else {
                    /* FIXME: test if LDAP account's primary group exists (it may be a non-per-user primary group) */
                    $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions']= array('create-primgroup');
                }
            }
            else {
                $this->csvinfo['data_preldap'][$key]['main_account']['_group_actions']= array('none');
            }
            $this->csvinfo['data_preldap'][$key]['main_account']['_ogroup_actions']= array('none');

            /* no primgroups for aux accounts */
            if(isset($row['aux_accounts']))
            {
                foreach($row['aux_accounts'] as $idx_aux => $aux_account)
                {
                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_group_actions']= array('none');
                    $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_ogroup_actions']= array('none');
                }
            }
        }

        $this->_ldap->cd($this->config->current['BASE']);

        $this->csvinfo['data_preldap_posixgroups'] = array();
        $this->csvinfo['data_preldap_ogroups'] = array();

        /* this will probably scale very very badly... Improvement needed. Suggestions? */
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {

            if(strpos($row['main_account']['_actions'][0],'ignore')!==FALSE)
            {
                continue;
            }

            $_all_groups= array();
            if(isset($row['groups'])) { $_all_groups= array_merge($_all_groups, $row['groups']); }
            if(isset($row['aux_accounts_groups'])) { $_all_groups= array_merge($_all_groups, $row['aux_accounts_groups']); }
            if(isset($row['optional_groups']))
            {
                foreach($row['optional_groups'] as $opt_group)
                {
                    if(
                       isset($opt_group['cn'][0]) and
                       (!in_array($opt_group['cn'][0],array_keys($_all_groups)))
                      )
                    {
                        $_all_groups[$opt_group['cn'][0]]= $opt_group;
                    }
                }
            }

            foreach ($_all_groups as $group_key => $group)
            {

                $_cn= "";
                if(isset($group['cn'][0]))
                {
                    $_cn=$group['cn'][0];
                }
                else {
                    /* no CN available, ignore this group */
                    continue;
                }

                if(in_array('posixGroup', $group['objectClass']))
                {
                    $memberAttr= "memberUid";
                    $group_type= "group";
                    $ldapsearch = $this->_ldap->search("(&(objectClass=posixGroup)(cn=".$_cn."))",array("cn",$memberAttr,"mail", "description"));
                }
                elseif(in_array('gosaGroupOfNames', $group['objectClass']))
                {
                    $memberAttr= "member";
                    $group_type= "ogroup";
                    $ldapsearch = $this->_ldap->search("(&(objectClass=gosaGroupOfNames)(cn=".$_cn."))",array("cn",$memberAttr,"mail", "description"));
                }
                else {
                    /* neither gosaGroupOfNames nor posixGroup import, no clue what to do... */
                    continue;
                }

                if(isset($group['_key'][0])) { $_group_type_key= $group['_key'][0]; }
                else { $_group_type_key= 'groups'; }

                if($gosa_group = $this->_ldap->fetch($ldapsearch))
                {
                    /* detect status */
                    if ($this->compareObjects($group, $gosa_group, array("cn"))===TRUE)
                    {
                        if(in_array('posixGroup', $group['objectClass']))
                        {
                            $expected_group_dn= 'cn='.$group['cn'][0].','.get_groups_ou().$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];
                        }
                        elseif(in_array('gosaGroupOfNames', $group['objectClass']))
                        {
                             $expected_group_dn= 'cn='.$group['cn'][0].','.get_ou('group','ogroupRDN').$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];
                        }
                        $gosa_group_dn= $gosa_group['dn'];
                        if(strtolower($gosa_group_dn)==strtolower($expected_group_dn) or ($_group_type_key==="optional_groups"))
                        {
                            $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_status'][0]= 'exists';
                        }
                        else {
                            $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_status'][0]= 'exists-elsewhere';
                        }
                    }
                }
                else {
                    if ($this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_status'][0]== 'unchecked')
                    {
                        $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_status'][0]= 'not-found';
                        if($_group_type_key==="optional_groups")
                        {
                            $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_actions'][0]= 'ignore';
                        }
                        else {
                            $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_actions'][0]= 'create';
                        }
                    }
                }

                /* detect actions */
                $_actions= array();
                if($this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_status'][0]==='exists-elsewhere')
                {
                    $_actions[]= "move";
                }
                if ($this->compareObjects($group, $gosa_group, array("cn"))===TRUE)
                {
                    if($_group_type_key!=='optional_groups')
                    {
                        $_check_attrs= $this->compareObjects($group, $gosa_group, array("mail","description"), "update-");
                        if ($_check_attrs===TRUE)
                        {
                        }
                        else {
                            $_actions= array_merge($_actions, $_check_attrs);
                        }
                    }

                    if($_group_type_key!=="aux_accounts_groups")
                    {

                        $_g_actions= explode(",", $this->csvinfo['data_preldap'][$key]['main_account']['_'.$group_type.'_actions'][0]);
                        if($_g_actions[0]==="none") { $_g_actions=array(); }

                        /* check if the user's main account is in this group */
                        if(isset($row['main_account']['uid'][0]))
                        {
                            $_group_name= $group['cn'][0];
                            $_uid= $row['main_account']['uid'][0];
                            $_members= isset($gosa_group['memberUid']) ? $gosa_group['memberUid'] : array();
                            if(!in_array($_uid, $_members))
                            {
                                if(!in_array('update-memberships', $_g_actions))
                                {
                                    $_g_actions[]= 'update-memberships';
                                }
                                $this->csvinfo['data_preldap'][$key]['main_account']['_'.$group_type.'_actions'][$_group_name]= 'add';
                            }
                            else {
                                $this->csvinfo['data_preldap'][$key]['main_account']['_'.$group_type.'_actions'][$_group_name]= 'none';
                            }
                        }
                        /* re-add the none action if no group action got detected above */
                        if(empty($_g_actions))
                        {
                            $_g_actions[]= 'none';
                        }
                        $this->csvinfo['data_preldap'][$key]['main_account']['_'.$group_type.'_actions'][0]= implode(",", $_g_actions);
                    }
                    else {

                        /* FIXME: The below code block needs to be adapted, in case there will be imports with AUX GROUPS being
                         *        POSIX groups.
                         */

                        /* check if the aux accounts are in this group */
                        if(isset($row['aux_accounts']) and (!empty($row['aux_accounts'])))
                        {
                            foreach($row['aux_accounts'] as $idx_aux => $aux_account)
                            {

                                /* if the object's DN is not known by now, let's skip this object */
                                if(!isset($aux_account['_dn_ldap'])) { continue; }

                                $_group_name= $group['cn'][0];
                                $_dn= $aux_account['_dn_ldap'][0];
                                $_members= isset($gosa_group[$memberAttr]) ? $gosa_group[$memberAttr] : array();

                                if($this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_status'][0]!=="data-incomplete")
                                {
                                    if(!in_array($_dn, $_members))
                                    {
                                        $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_'.$group_type.'_actions'][0]= 'update-memberships';
                                        $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_'.$group_type.'_actions'][$_group_name]= 'add';
                                    }
                                    else {
                                        $this->csvinfo['data_preldap'][$key]['aux_accounts'][$idx_aux]['_'.$group_type.'_actions'][$_group_name]= 'none';
                                    }
                                }
                            }
                        }
                    }

                    /* no action found till here, set the "none" action */
                    if(empty($_actions))
                    {
                        $_actions[]= "none";
                    }
                    $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['_actions'][0]= implode(",", $_actions);

                }

                /* grab groups (classes, courses) into their own array */
                $group_name = $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key]['cn'][0];
                if (!in_array($group_name, array_keys($this->csvinfo['data_preldap_posixgroups'])) and ($_group_type_key!=="optional_groups"))
                {
                    if(in_array('posixGroup', $group['objectClass']))
                    {
                        $this->csvinfo['data_preldap_posixgroups'][$group_name] = $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key];
                    }
                    elseif(in_array('gosaGroupOfNames', $group['objectClass']))
                    {
                        $this->csvinfo['data_preldap_ogroups'][$group_name] = $this->csvinfo['data_preldap'][$key][$_group_type_key][$group_key];
                    }
                }
            }
        }

        /* finally add optional groups, if not already explicitly listed above */
        foreach ($this->csvinfo['data_preldap'] as $key => $row)
        {

            if(strpos($row['main_account']['_actions'][0],'ignore')!==FALSE)
            {
                continue;
            }

            if(isset($row['optional_groups']))
            {
                $_opt_groups= $row['optional_groups'];
                ksort($_opt_groups);
                foreach($_opt_groups as $opt_group)
                {
                    if(in_array('posixGroup', $opt_group['objectClass']))
                    {
                        if(
                           isset($opt_group['cn'][0]) and
                           (!in_array($opt_group['cn'][0],array_keys($this->csvinfo['data_preldap_posixgroups'])))
                          )
                        {
                            $this->csvinfo['data_preldap_posixgroups'][$opt_group['cn'][0]]= $opt_group;
                        }
                    }
                    elseif(in_array('gosaGroupOfNames', $opt_group['objectClass']))
                    {
                        if(
                           isset($opt_group['cn'][0]) and
                           (!in_array($opt_group['cn'][0],array_keys($this->csvinfo['data_preldap_ogroups'])))
                          )
                        {
                            $this->csvinfo['data_preldap_ogroups'][$opt_group['cn'][0]]= $opt_group;
                        }
                    }
                    else {
                        /* neither gosaGroupOfNames nor posixGroup import, no clue what to do... */
                        continue;
                    }
                }
            }
        }

        ksort($this->csvinfo['data_preldap_posixgroups']);
        ksort($this->csvinfo['data_preldap_ogroups']);

    }

    function CSVRowSanityChecksOk($csv_row, $idx)
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

    function moveLDAPPrimGroupObjects()
    {
        foreach($this->csvinfo['data_preldap'] as $idx => $row)
        {

            /* we should get here, but just in case... */
            if(strpos($row['main_account']['_actions'][0],'ignore')!==FALSE)
            {
                continue;
            }

            $user_data= $row['main_account'];

            if(strpos($user_data['_group_actions'][0], '-primgroup')===FALSE)
            {
                /* neither create-primgroup nor move-primgroup given, nothing to do for this user account */
                continue;
            }

            /* create a group_data object that we can pass on to importLDAPGroupObject */
            $group_name= $user_data['uid'][0];
            $group_data= array(
                'objectClass' => array('posixGroup'),
                'cn' => array($group_name),
                '_dn' => array("cn=".$row['main_account']['uid'][0].",".get_groups_ou().dn2base($row['main_account']['_dn'][0])),
            );

            /* set the action for group_data, derived from the acount's _group_actions */
            if(strpos($user_data['_group_actions'][0], 'create-primgroup')!==FALSE)
            {
                $group_data['_actions']= array('create');

                # FIXME: we need to update the user accounts gidNumber attribute description here

            }
            elseif (strpos($user_data['_group_actions'][0], 'move-primgroup')!==FALSE)
            {
                $group_data['_actions']= array('move');
            }
            else {
                /* we should never come here, leaving this continue statement here anyway, just in case */
                continue;
            }

            /* create or move account's primary group */
            $this->importLDAPGroupObject($group_name, $group_data);
        }
    }


    function importLDAPGroupObject($group_name, $group_data)
    {

        /* only perform on this user dataset if _actions does not contain "none"... */
        if((strpos($group_data['_actions'][0],'none')!==FALSE) or (strpos($group_data['_actions'][0],'ignore')!==FALSE))
        {
            return;
        }

        if(in_array('posixGroup', $group_data['objectClass']))
        {
            $_group_obclass= "posixGroup";
            $_group_tabs= "GROUPTABS";
            $_group_tabs_class= "grouptabs";
            $_group_RDN= get_groups_ou();
            $_group_obj= "group";
        }
        else {
            $_group_obclass= "gosaGroupOfNames";
            $_group_tabs= "OGROUPTABS";
            $_group_tabs_class="ogrouptabs";
            $_group_RDN= get_ou("group", "ogroupRDN");
            $_group_obj= "ogroup";
        }

        if((strpos($group_data['_actions'][0],'create')!==FALSE) or (strpos($group_data['_actions'][0],'move')!==FALSE))
        {

            /* Instantiate a new group object via GOsa²'s API */

            $grouptab= new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs],'new');
            if(isset($group_data['_dn'][0]))
            {
                $grouptab->by_object[$_group_obj]->base= dn2base($group_data['_dn'][0]);
            }
            else {
                $grouptab->by_object[$_group_obj]->base= $this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']];
            }

        }
        else{

            /* Retrieve group object from LDAP via GOsa²'s API */
            $grouptab= new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs],'cn='.$group_name.','.$_group_RDN.$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']]);

        }

        if(strpos($group_data['_actions'][0],'move')!==FALSE)
        {
            /* do an ldapsearch for the source object and prepare a copy+paste action */

            $_ldapsearch= $this->_ldap->search("(&(objectClass=".$_group_obclass.")(cn=".$group_name."))", array("*"));

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
                $remove_later_grouptab= new $_group_tabs_class($this->config, $this->config->data['TABS'][$_group_tabs],$source['dn']);
            }
        }

        /* Collect group properties from $group_data */
        $_group_properties= array(
            "cn" => $group_data['cn'][0],
        );
        if(((strpos($group_data['_actions'][0],'update-description')!==FALSE) or (strpos($group_data['_actions'][0],'create')!==FALSE)) and isset($group_data['description'][0]))
        {
            $_group_properties["description"]= $group_data['description'][0];
        }
        if(((strpos($group_data['_actions'][0],'update-mail')!==FALSE) or (strpos($group_data['_actions'][0],'create')!==FALSE)) and isset($group_data['mail'][0]))
        {
            $_group_properties["mail"]= $group_data['mail'][0];
        }

        /*
         * FIXME: Make mail address creation (from CN) configurable somewhere...
         */

        /* Populate sub-object attributes for $grouptab with same values */
        foreach($_group_properties as $attr => $val)
        {
            if(isset($grouptab->$attr))
            {
                $grouptab->$attr = $val;
            }
            foreach($grouptab->by_object as $pname => $plugin)
            {
                if(isset($grouptab->by_object[$pname]->$attr))
                {
                    $grouptab->by_object[$pname]->$attr = $val;
                }
            }
        }
        /* if we do import mail properties, make sure the mailgroup object is activated */
        if (isset($_group_properties['mail']) && isset($grouptab->by_object['mailgroup']))
        {
            $grouptab->by_object['mailgroup']->is_account= TRUE;
        }

        /* Run GOsa²'s groups/group | ogroups/ogroup checks */
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
        /* only perform on this user dataset if _actions does not contain "none"... */
        if((strpos($user_data['_actions'][0],'none')!==FALSE) or (strpos($user_data['_actions'][0],'ignore')!==FALSE))
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

                $_user_properties_base["sn"]= $user_data['sn'][0];
                $_user_properties_base["givenName"]= $user_data['givenName'][0];

                /* The below attributes must exist and be set with meaningful data once we arrive here!!! */
                if(isset($user_data['uid'][0]) and $user_data['uid'][0]!=="{%uid}") {
                    $_user_properties_base["uid"]= $user_data['uid'][0];
                }
                elseif(isset($user_data['sn'][0]) and isset($user_data['givenName'][0]))
                {
                    if(isset($user_data['uid'][0]) and ($user_data['uid'][0]==="{%uid}" or $user_data['uid'][0]===""))
                    {
                        if ($this->config->get_cfg_value("core","idGenerator") != "")
                        {
                            $_genStr= $this->config->get_cfg_value("core","idGenerator");
                            $_attributes= array(
                                'sn' => $user_data['sn'][0],
                                'givenName' => $user_data['givenName'][0]
                            );
                            $_user_properties_base['uid']= gen_uids($_genStr,$_attributes)[0];
                        }
                        else {

                            /* FIXME: We can only auto-generate uid strings if the idGenerator is configured
                             *        Report this as an error dialog box.
                             */

                        }
                    }
                    elseif(
                       ($this->csvinfo['try_mail_as_uid']===TRUE) and
                       (isset($user_data['mail'][0]) and $user_data['mail'][0])
                      )
                    {
                        $_user_properties_base['uid']= strtolower($user_data['mail'][0]);
                    }
                    else {
                        log ("ERROR","No exact UIDs given, no UID column in the CSV sheet, permission to use the MAIL attribute as UID", $this->dn);
                        msg_dialog::display("ERROR", "No exact UIDs given, no UID column in the CSV sheet, permission to use the MAIL attribute as UID",ERROR_DIALOG);
                    }
                }
                else {
                    /* FIXME: error message, if uid is not provided and sn+givenName is not provided or incomplete,
                     *        we cannot go on...
                     */
                    log ("ERROR","if uid is not provided and sn+givenName is not provided or incomplete, we cannot go on...", $this->dn);
                    msg_dialog::display("ERROR", "if uid is not provided and sn+givenName is not provided or incomplete, we cannot go on...",ERROR_DIALOG);
                }
                $_user_properties_base["sn"]= $user_data['sn'][0];
                $_user_properties_base["givenName"]= $user_data['givenName'][0];

                if(isset($user_data['gender'][0])) { $_user_properties_extras["gender"]= $user_data['gender'][0]; }
                if(isset($user_data['dateOfBirth'][0])) { $_user_properties_extras["dateOfBirth"]= $user_data['dateOfBirth'][0]; }

                foreach($_user_properties_base as $attr => $val)
                {
                    if(isset($usertab->$attr))
                    {
                        $usertab->$attr = $val;
                    }
                    foreach($usertab->by_object as $pname => $plugin)
                    {
                        if(isset($usertab->by_object[$pname]->$attr))
                        {
                            $usertab->by_object[$pname]->$attr = $val;
                        }
                    }
                }
            }
            else {

                /* Retrieve user object from LDAP */
                $usertab= new usertabs($this->config, $this->config->data['TABS']['USERTABS'],$user_data['_dn_ldap'][0]);

            }

            /* on existing account, we allow the below attributes to be updated */
            if(isset($user_data['employeeNumber'][0]) and ((strpos($user_data['_actions'][0],'update-employeeNumber')!==FALSE) or (strpos($user_data['_actions'][0],'create')!==FALSE)))
            {
                $_user_properties_extras["employeeNumber"]= $user_data['employeeNumber'][0];
            }
            if(isset($user_data['departmentNumber'][0]) and ((strpos($user_data['_actions'][0],'update-departmentNumber')!==FALSE) or (strpos($user_data['_actions'][0],'create')!==FALSE)))
            {
                $_user_properties_extras["departmentNumber"]= $user_data['departmentNumber'][0];
            }
            if(isset($user_data['mail'][0]) and ((strpos($user_data['_actions'][0],'update-mail')!==FALSE) or (strpos($user_data['_actions'][0],'create')!==FALSE)))
            {
                $_user_properties_extras["mail"]= $user_data['mail'][0];
            }

            if($user_data['_template'][0] != 0)
            {
                $template_dn=  $this->csvinfo['templates']['DNs'][$user_data['_template'][0]];
            }

            /* test if base DN exists, this is only relevant for actions "create" and "move" but does not hurt for updates, either */
            $this->_ldap->cat($user_data['_base'][0]);
            if(!$this->_ldap->fetch())
            {

                /* change UI "dir" in LDAP to the place where we want to create sub-OUs... */
                $_ui= get_userinfo();
                $old_ui_dn= $_ui->dn;
                $old_current_main_base= session::global_is_set("CurrentMainBase")?session::global_get("CurrentMainBase"):$_ui->dn;
                if($user_data['_template'][0] != 0)
                {
                    change_ui_dn($old_ui_dn, dn2base($template_dn));
                    session::global_set("CurrentMainBase", dn2base($template_dn));
                }
                else {
                    change_ui_dn($old_ui_dn, $this->config->current['BASE']);
                    session::global_set("CurrentMainBase", $this->config->current['BASE']);
                }

                /* create _base DN as a GOsa² department */
                $_ou= preg_replace("/^ou=([^,]+),.*/", "$1", $user_data['_base'][0]);
                $deptab= new deptabs($this->config, $this->config->data['TABS']['DEPTABS'],'new', "deptabs" );
                $deptab->by_object['department']->ou= $_ou;
                $deptab->by_object['department']->description= sprintf(_("Class: %s"), $_ou);
                $deptab->save();

                /* Reload departments */
                $this->config->get_departments();
                $this->config->make_idepartments();

                /* change UI dir back to where we came from */
                change_ui_dn(dn2base($template_dn), $old_ui_dn);
                session::global_set("CurrentMainBase", $old_current_main_base);
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

            if(isset($_user_properties_extras["mail"]) and isset($usertab->by_object['mailAccount'])) {
                /* FIXME: check if a mailAccount plugin is available at all */
                $usertab->by_object['mailAccount']->is_account= TRUE;
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
            $this->failure_messages= array();
            if(count($usertab->check())) {

                $this->failure_messages[]= $usertab->check();

                /*
                 * FIXME: collect failure statistics here!!!
                 */

                $this->failure_in_this_phase= TRUE;
            } else {

                $usertab->save();

                /* Present a status update in GOsa²''s logging system (/var/log/syslog mostly) */
                if(strpos($user_data['_actions'][0],'create')!==FALSE)
                {
                    new log("create","users/user",$usertab->dn, array(), "New user (".$usertab->sn.", ".$usertab->givenName.") created via SchoolManager add-on.");
                }
                elseif(strpos($user_data['_actions'][0],'none')!==FALSE) {
                    new log("modify","users/user",$usertab->dn, array(), "Existing user  (".$usertab->sn.", ".$usertab->givenName.") modified via SchoolManager add-on.");
                }

                if (strpos($user_data['_actions'][0],'set-password')!==FALSE)
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

        }
    }

    function addLDAPUserObjectsToGroups($group_members_data)
    {

        foreach($group_members_data as $group_name => $new_members)
        {
            if(empty($new_members))
            {
                continue;
            }
            else {

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
    }

    function addLDAPUserObjectsToOGroups($ogroup_members_data)
    {
        foreach($ogroup_members_data as $ogroup_name => $new_members)
        {

            if(empty($new_members))
            {
                continue;
            }
            else {

                $ogrouptab= new ogrouptabs($this->config, $this->config->data['TABS']['OGROUPTABS'],'cn='.$ogroup_name.','.get_ou('group', 'ogroupRDN').$this->csvinfo['ou_tree']['DNs'][$this->csvinfo['ou_groups']], "ogroups" );

                $current_members=$ogrouptab->by_object['ogroup']->member;
                foreach($new_members as $new_member)
                {
                    if(!in_array($new_member, $current_members))
                    {

                        $ogrouptab->by_object['ogroup']->member[$new_member]= $new_member;
                    }
                }

                /* Update the object type in gosaGroupObjects at the end of adding new members */
                $obj_types= preg_replace('/[\[\]]/', '', $ogrouptab->by_object['ogroup']->gosaGroupObjects);
                if(strpos($obj_types, 'U')===FALSE)
                {
                    $obj_types.= 'U';
                    $ogrouptab->by_object['ogroup']->gosaGroupObjects= '['.$obj_types.']';
                }

                /* Run GOsa²'s ogroups/ogroup checks */
                if(count($ogrouptab->check()))
                {

                    msg_dialog::displayChecks($ogrouptab->check());

                    /*
                     * FIXME: collect failure statistics here!!!
                     */

                    $this->failure_in_this_phase= TRUE;

                } else
                {
                    /* Save group object to LDAP */
                    $ogrouptab->save();
                }
            }
        }
    }

    function genAccountBase($user_data, $groups_data=array())
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

    function prepareGroupMemberships()
    {
        /* collect group memberships from user accounts and store the information
         * in separate arrays (one for posixGroups, one for gosaGroupOfNames)
         */

        $this->posixgroup_members= array();
        $this->groupofnames_members= array();

        foreach($this->csvinfo['data_preldap'] as $idx => $user_data)
        {
            if((isset($user_data['main_account']) && (!empty($user_data['main_account']))))
            {
                $_groups= $user_data['groups'];

                /* process memberships in optional groups, only for groups that exist */
                if(isset($user_data['optional_groups']))
                {
                    foreach($user_data['optional_groups'] as $opt_group)
                    {
                        if(
                           isset($opt_group['cn'][0]) and
                           (strpos($user_data['optional_groups'][$opt_group['cn'][0]]['_status'][0],'exists')!==FALSE)
                          )
                        {
                            $_groups[$opt_group['cn'][0]]= $opt_group;
                        }
                    }
                }

                foreach($_groups as $group_key => $group_data)
                {
                    if (in_array('posixGroup', $group_data['objectClass']) and
                        (strpos($user_data['main_account']['_group_actions'][0], 'update-memberships')!==FALSE) and
                        ($user_data['main_account']['_group_actions'][$group_key]==="add")
                       )
                    {
                        $_uid= $user_data['main_account']['uid'][0];
                        if (!in_array($group_key, array_keys($this->posixgroup_members)))
                        {
                            $this->posixgroup_members[$group_key]= array();
                        }
                        $this->posixgroup_members[$group_key][]= $_uid;
                    }

                    if (in_array('gosaGroupOfNames', $group_data['objectClass']) and
                        (strpos($user_data['main_account']['_ogroup_actions'][0], 'update-memberships')!==FALSE) and
                        ($user_data['main_account']['_ogroup_actions'][$group_key]==="add")
                       )
                    {
                        if (isset($user_data['main_account']['_dn_ldap']))
                        {
                            $this->groupofnames_members[$group_key][]= $user_data['main_account']['_dn_ldap'];
                        }
                    }
                }
            }
            if(
               (isset($user_data['aux_accounts'])) and (!empty($user_data['aux_accounts'])) and
               (isset($user_data['aux_accounts_groups'])) and (!empty($user_data['aux_accounts_groups']))
              )
            {
                foreach($user_data['aux_accounts_groups'] as $group_key => $group_data)
                {
                    foreach($user_data['aux_accounts'] as $aux_account)
                    {
                        if (in_array('posixGroup', $group_data['objectClass']) and
                            (strpos($aux_account['_group_actions'][0], 'update-memberships')!==FALSE) and
                            ($aux_account['_group_actions'][$group_key]==="add")
                           )
                            {
                            $_uid= $aux_account['uid'][0];
                            if (!in_array($group_key, array_keys($this->posixgroup_members)))
                            {
                                $this->posixgroup_members[$group_key]= array();
                            }
                            $this->posixgroup_members[$group_key][]= $_uid;
                        }
                        if (in_array('gosaGroupOfNames', $group_data['objectClass']) and
                            (strpos($aux_account['_ogroup_actions'][0], 'update-memberships')!==FALSE) and
                            ($aux_account['_ogroup_actions'][$group_key]==="add")
                           )
                        {
                            if (!in_array($group_key, array_keys($this->groupofnames_members)))
                            {
                                $this->groupofnames_members[$group_key]= array();
                            }
                            if (isset($aux_account['_dn_ldap']))
                            {

                                /* FIXME: DN memberships are case insensitive, we need to convert
                                 *        $groupofnames_members[$group_key] items into lower case and then
                                 *        compare with strtolower($aux_account['_dn_ldap'][0]).
                                 *        Only if there is no case-insensitive match, we may add
                                 *        another member DN to this group.
                                 */

                                $this->groupofnames_members[$group_key][]= $aux_account['_dn_ldap'][0];
                            }
                        }
                    }
                }
            }
        }
    }

    function validGroupName($group_name)
    {

        /* FIXME 1: This can be better done with regular expressions!!!
         * FIXME 2: The transliteration part is locale-dependent. We tried iconv and it did not work...
         */

        $_grp = str_replace('ä', 'ae',
                 str_replace('ö', 'oe',
                  str_replace('ü', 'ue',
                   str_replace('ß', 'ss',
                    str_replace('.','',
                     str_replace(':', '-',
                      str_replace('(', '_',
                       str_replace(')', '',
                        str_replace(' ', '_',
                         strtolower (trim($group_name)))))))))));
         if (preg_match ('/^[a-z][-a-z0-9_]*[a-z0-9]$/', $_grp)) {
             return $_grp;
         } else {
             return "";
         }
    }

    function randomPassword($length)
    {
        $alphabet = "abcdefghijklmnopqrstuwxyzABCDEFGHIJKLMNOPQRSTUWXYZ0123456789";
        $pass = array(); //remember to declare $pass as an array
        $alphaLength = strlen($alphabet) - 1; //put the length -1 in cache
        for ($i = 0; $i < $length; $i++) {
            $n = rand(0, $alphaLength);
            $pass[] = $alphabet[$n];
        }
        return implode($pass); //turn the array into a string
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
