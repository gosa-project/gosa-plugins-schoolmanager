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
class managestudents extends plugin {

    /* Definitions */
    var $plHeadline="Manage Students";
    var $plDescription="This does something";
    var $access="";

    /* Array with csv informations */
    var $csvinfo=array ();

    /* attribute list for save action */
    var $attributes=array ();
    var $objectclasses=array ();
    var $view_logged=FALSE;

    /* constructor */
    function managestudents (&$config,$dn=NULL) {

        $this->initTime=microtime (TRUE);

        /* Include config object */
        $this->config=&$config;
        $this->ui=get_userinfo ();
        stats::log ('plugin',$class=get_class ($this),$category=array ($this->acl_category),$action='open',$amount=1,$duration= (microtime (TRUE)-$this->initTime));
    }

    function execute () {

        /* Call parent execute */
        plugin::execute ();

        /* Log view */
        if (!$this->view_logged) {
            $this->view_logged=TRUE;
            new log ("view","all/".get_class ($this),$this->dn);
        }

        /* initiate smarty */
        $smarty=get_smarty ();
        $smarty->assign ("selectedattrs",array (0=>""));
        $smarty->assign ("data",array (0=>""));
        $smarty->assign ("head",array (0=>""));


        $smarty->assign ("file_uploaded",0);

        /* PHASE 1 */
        $smarty->assign ("templates_selected",0);

        /* PHASE 2 */
        $smarty->assign ("data_sorted",0);

        /* PHASE 3 */
        /* View Summary */

        /* Get the LDAP link, to generate the Export */
        $ldap=$this->config->get_ldap_link ();

        /* initialize CSV Info array */
        if (!is_array ($this->csvinfo)) {
            $this->csvinfo=array ();
        }

        /* CSV columns required for import */
        $new=array ();
        $new[0]="no";
        $new[1]="id";
        $new[2]="sn";
        $new[3]="givenName";
        $new[4]="dateOfBirth";
        $new[5]="gender";
        $new[6]="class";
        $new[7]="mother_sn";
        $new[8]="mother_givenName";
        $new[9]="mother_mail";
        $new[10]="father_sn";
        $new[11]="father_givenName";
        $new[12]="father_mail";
        for ($i=13;$i<33;$i++) {
            $new[$i]="subjectclass". ($i-12);
        }
        $this->csvinfo['attr']=$new;
        $tmp=array ();
        for ($i=0;$i<33;$i++) {
            $tmp[$i]=$i;
        }
        $this->csvinfo['arr_selected']=$tmp;
        $smarty->assign ("attrs",$this->csvinfo['attr']);

        /* Check permissions for import */
        $acl=$this->ui->get_permissions ($this->config->current['BASE'],"all/all");
        if (!preg_match ("/w/",$acl)) {
            if (isset ($_POST['userfile'])||isset ($_POST['sorted'])||isset ($_POST['fileup'])) {
                msg_dialog::display (_ ("Permission error"),_ ("You've no permission to import CSV files!"),ERROR_DIALOG);
            }
            return ($smarty->fetch (get_template_path ('content_managestudents.tpl',TRUE)));
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
         * PHASE 1: CSV has been selected for upload, we now upload the file and select user object template for the to be import user accounts
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

                    $parser_ok = managestudents::parseCSV($raw_csv_data);

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
         * PHASE 2: After the admin has assigned LDAP attributes to CSV columns we create
         *          the data structure that will finally be imported into LDAP.
         */
        elseif (isset ($_POST['phase2'])) {

            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);

            /* Temp var 1 */
            $smarty->assign ("ia",1);

            /* Temp var 2 */
            $smarty->assign ("i",0);

            /* Num rows    */
            $smarty->assign ("rows",$this->csvinfo['shownrowscount']);
            for ($i=0;$i<$anz;$i++)
                $this->csvinfo['arr_selected'][]="0";
            $smarty->assign ("selectedattrs",$this->csvinfo['arr_selected']);

            /* Entrie count5 */
            $smarty->assign ("anz",$this->csvinfo['cols']);

            /* Array with data */
            $smarty->assign ("data",$tmp2);

            for ($i=0;$i<$this->csvinfo['rows'];$i++) {

                /* Preparing given Data */
                $this->csvinfo['sorteddata'][$i]=array ();

                /* Go through every col */
                for ($a=0;$a<= ($this->csvinfo['cols']);$a++) {

                    /* Append a status col at last, to show possibly errors */
                    if ($a== ($this->csvinfo['cols'])) {
                        $this->csvinfo['sorteddata'][$i]["status"]="-";
                    }
                    else {
                        $cc= ($i* ($this->csvinfo['cols']))+$a;

                        /* If theres a Attribut given for this col */
                        if (!empty ($_POST[ ('row'. ($a))])) {
                            $this->csvinfo['sorteddata'][$i][$this->csvinfo['attr'][$_POST[ ('row'. ($a))]]]=$this->csvinfo['data'][ ($cc)];
                        }
                    }
                }
            }

            /* at least one entry */
            $rowcnt=0;
            if (isset ($this->csvinfo['sorteddata'])) {
                $rowcnt++;

                /* Set the col count */
                $tblcolcount=count ($this->csvinfo['cols']);

                /* entrie count */
                $tbl_entries=count ($this->csvinfo['count']);

                /* row count */
                $tbl_rows=$tbl_entries/$tblcolcount;

                /* No error occurred */
                $failing=FALSE;

                /* For every row */
                for ($i=0;$i<$this->csvinfo['rows'];$i++) {

                    /* Get elemenet */
                    $dat=$this->csvinfo['sorteddata'][$i];

                    /* If not all needed vars set .... */
                    if ( (empty ($dat['sn']))|| (empty ($dat['givenName']))|| (!isset ($dat['sn']))|| (!isset ($dat['givenName']))) {

                        /* Output Error */
                        msg_dialog::display (_ ("Error"),sprintf (_ ("Need at least %s, %s and %s to create users!"),bold ("sn"),bold ("gn"),bold ("uid")),ERROR_DIALOG);
                    }
                    else {
                        $tmpclassparts=explode (".",strtolower ($dat['class']));
                        $tmpclassparts[0]=trim ($tmpclassparts[0]);
                        $tmpclassparts[1]=trim ($tmpclassparts[1]);
                        $tmpclassparts[1]=substr ($tmpclassparts[1],1);
                        $dat['class']=$tmpclassparts[0].".".$tmpclassparts[1];
                        foreach ($dat as $key=>$value) {
                            if (substr ($key,0,12)==="subjectclass") {
                                $dat[$key]=strtolower ($value);
                            }
                        }
                        $student=array ();
                        $student['sn']=array ($dat['sn']);
                        $student['givenName']=array ($dat['givenName']);
                        $student['birthday']=array ($dat['dateOfBirth']);
                        $student['gender']=array ($dat['gender']);
                        $mother=array ();
                        $mother['sn']=array ($dat['mother_sn']);
                        $mother['givenName']=array ($dat['mother_givenName']);
                        $mother['mail']=array ($dat['mother_mail']);
                        $father=array ();
                        $father['sn']=array ($dat['father_sn']);
                        $father['givenName']=array ($dat['father_givenName']);
                        $father['mail']=array ($dat['father_mail']);
                        var_dump ($student,$mother,$father);
                        $this->csvinfo['sorteddata'][$i]=$dat;
                    }
                }
                $pointsbefore=FALSE;
                $pointsafter=FALSE;

                /* Get Attributs for Tableheadline  */
                $this->csvinfo['colsstr']=array ();
                for ($i=0;$i<=$this->csvinfo['cols'];$i++) {
                    if (!empty ($_POST[ ('row'.$i)])) {
                        $this->csvinfo['colsstr'][$i]=$this->csvinfo['attr'][$_POST[ ('row'.$i)]];
                    }
                }

                /*Create array with data for the template */
                if ($failing==FALSE) {
                    $dataouts=array ();
                    for ($i=1;$i<=$this->csvinfo['shownrowscount'];$i++) {
                        if (is_array ($this->csvinfo['sorteddata'][ ($i-1)])) {
                            foreach ($this->csvinfo['sorteddata'][ ($i-1)] as $dat) {
                                $dataouts[]=$dat;
                            }
                        }
                    }
                    if ($this->csvinfo['rows']>$this->csvinfo['shownrowscount']) {
                        $pointsafter=TRUE;
                    }
                }
                else {
                    $pointsbefore=TRUE;
                    $pointsafter=TRUE;
                    $begin=$failing-3;
                    if ($begin<0) {
                        $begin=0;
                        $pointsbefore=FALSE;
                    }
                    $end=$failing+2;
                    if ($end>$this->csvinfo['rows']) {
                        $end=$this->csvinfo['rows']+1;
                        $pointsafter=FALSE;
                    }
                    $this->csvinfo['shownrowscount']=$end-$begin;
                    if ($this->csvinfo['shownrowscount']>$this->csvinfo['rows'])
                        $this->csvinfo['shownrowscount']=$this->csvinfo['rows'];
                    $dataouts=array ();
                    for ($i=$begin;$i<=$end;$i++) {
                        if (is_array ($this->csvinfo['sorteddata'][ ($i-1)])) {
                            foreach ($this->csvinfo['sorteddata'][ ($i-1)] as $dat) {
                                $dataouts[]=$dat;
                            }
                        }
                    }
                }
                $tmparr2=$this->csvinfo['colsstr'];
                $tmparr2[]=_ ("status");

                /* Error? */
                if ($failing) {
                    msg_dialog::display (_ ("Error"),sprintf (_ ("Cannot add entry %s!"),bold ($failing)),ERROR_DIALOG);
                }
                $smarty->assign ("error",$failing);

                /* Set vars to smarty */
                $smarty->assign ("cols",count ($tmparr2));
                $smarty->assign ("anz",$this->csvinfo['count']);
                $smarty->assign ("rows",$this->csvinfo['shownrowscount']);
                $cnt=0;
                $tmp2=$tmp3=array ();
                if (is_array ($dataouts))
                    foreach ($dataouts as $tmp) {
                        $tmp2[]=$tmp;
                    $cnt++;
                    if ($cnt>=count ($tmparr2)) {
                        $tmp3[]=$tmp2;
                        $tmp2=array ();
                        $cnt=0;
                    }
                }
                $smarty->assign ("head",$tmparr2);
                $smarty->assign ("data",$tmp3);

                /* Set other vars  */
                $smarty->assign ("i",1);
                $smarty->assign ("ie",0);
                $smarty->assign ("pointsafter",$pointsafter);
                $smarty->assign ("pointsbefore",$pointsbefore);
            }
            else {
                msg_dialog::display (_ ("Information"),_ ("Nothing to import!"),INFO_DIALOG);
            }

            /* If theres a File uploaded */
        }

        /*
         * PHASE 3: The user object templates have been selected, now we create the use objects from templates.
         */
        elseif (isset ($_POST['phase3'])) {

            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            $smarty->assign ("data_sorted",TRUE);

            /*
             * PHASE 4: Finally write everything to LDAP (directly, no futher $_POST['phase4']...)
             */
            /* Template Handling */
            $smarty->assign ("file_uploaded",TRUE);
            $smarty->assign ("templates_selected",TRUE);
            $smarty->assign ("data_sorted",TRUE);
        }

        /* Show main page */
        return ($smarty->fetch (get_template_path ('content_managestudents.tpl',TRUE)));
    }

    function parseCSV($raw_csv_data)
    {
        $lines=preg_split ("/\n/",$raw_csv_data);
        $anz=0;
        $rest=0;
        $data=array ();

        /* check colum count */
        if (is_array ($lines))
            foreach ($lines as $line) {

                /* continue if theres a comment */
                if (substr (trim ($line),0,1)=="#") {
                    continue;
                }
                $line=str_replace ("\t","",$line);
                $cells=explode (",",$line);
                if (count ($cells)>$anz) {
                    $anz=count ($cells);
                }
            }

        /* At least one entry */
        if ($anz>1) {

            /* Generate array with outpu info  */
            if (is_array ($lines))
                foreach ($lines as $line) {
                    $rest=0;
                    $cnt=0;

                    /* dont use comments or empty lines */
                    if ( (substr (trim ($line),0,1)=="#")|| (empty ($line))) {
                        continue;
                    }

                /* replace \t to , */
                $line=str_replace ("\t",",",$line);

                /* get all elements  */
                $cells=explode (",",$line);

                /* attach all elements to data array */
                if (is_array ($cells))
                    foreach ($cells as $cell) {
                        $cnt++;
                        $data[]=trim ($cell);
                }

                /* cell count less than anz, attach some empty fields */
                if ( ($cnt!=$anz)&& (!empty ($cnt))) {
                    $rest=$anz-$cnt;
                    for ($i=0;$i<$rest;$i++) {
                        $data[]=" ";
                    }
                }
            }
            unset ($this->csvinfo['sorteddata']);
            unset ($this->csvinfo['colsstr']);
            unset ($this->csvinfo['sorteddata']);
            $this->csvinfo['cols']=$anz;
            $this->csvinfo['data']=array ();
            $this->csvinfo['data']=$data;
            $this->csvinfo['count']=count ($this->csvinfo['data']);
            if ($this->csvinfo['count']> (6*$this->csvinfo['cols'])) {

                /* only show 6 rows in ouptuttablei */
                $datouttemp=array_chunk ($this->csvinfo['data'], (6*$this->csvinfo['rows']));
                $this->csvinfo['dataout']=$datouttemp[0];
                $this->csvinfo['showncolscount']=6;
            }
            else {
                $this->csvinfo['showncolscount']= (count ($this->csvinfo['data']))/$this->csvinfo['rows'];
                $this->csvinfo['dataout']=$this->csvinfo['data'];
            }
            $this->csvinfo['rows']= (count ($this->csvinfo['data'])/$anz);
            $i=0;
            $tmp=$tmp2=array ();
            if (is_array ($this->csvinfo['dataout']))
                foreach ($this->csvinfo['dataout'] as $dat) {
                    $tmp[]=$dat;
                $i++;
                if ($i>=$this->csvinfo['cols']) {
                    $i=0;
                    $tmp2[]=$tmp;
                    $tmp=array ();
                }
            }
            return TRUE;
        }
        return FALSE;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
