<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2015 Mike Gabriel
 * Copyright (C) 2015 Marius Rasch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


class archiveaccounts extends plugin {

	/* Definitions */
	var $plHeadline = "Archive Accounts";
	var $plDescription = "GOsa2 School Manager Module: Archive Accounts";
	var $access = "";

	/* Array with csv informations */
	var $csvinfo = array ();
	var $archive_type = "";

	/* attribute list for save action */
	var $attributes = array ();
	var $objectclasses = array ();
	var $view_logged = FALSE;
	function __construct(&$config, $dn = NULL) {
		$this->initTime = microtime ( TRUE );

		/* Include config object */
		$this->config = &$config;

		$this->ui = get_userinfo ();

		/* Initialize utils class */
		$this->utils = new schoolmgr_utils($config);

		stats::log ( 'plugin', $class = get_class ( $this ), $category = array (
				$this->acl_category
		), $action = 'open', $amount = 1, $duration = (microtime ( TRUE ) - $this->initTime) );
	}
	function execute() {
		/* Call parent execute */
		plugin::execute ();

		/* Log view */
		if (! $this->view_logged) {
			$this->view_logged = TRUE;
			new log ( "view", "all/" . get_class ( $this ), $this->dn );
		}

		/* initiate smarty */
		$smarty = get_smarty ();

		/* Get the LDAP link, to generate the Export */
		$this->_ldap = $this->config->get_ldap_link ();

		/* initialize CSV Info array */
		if (! is_array ( $this->csvinfo )) {
			$this->csvinfo = array ();
		}

		/* Setup default states for phases */
		/* If PHASE done, set variable to TRUE */
		$smarty->assign ( "chosen_archivetype", FALSE ); // PHASE 0
		$smarty->assign ( "file_uploaded", FALSE ); // PHASE 1
		$smarty->assign ( "archive_configured", FALSE ); // PHASE 2
		$smarty->assign ( "data_sorted", FALSE ); // PHASE 3
		$smarty->assign ( "accounts_reviewed", FALSE ); // PHASE 4
		$smarty->assign ( "accounts_archived", FALSE ); // PHASE 5
		$smarty->assign ( "primgroups_moved", FALSE ); // PHASE 6

		/* reset our failure status from last template view... */
		$this->failure_in_this_phase = FALSE;

		/* Archive types */
		$smarty->assign ( 'available_archive_types', array (
				0 => _ ( "Students" ),
				1 => _ ( "Teachers" )
		) );
		$smarty->assign ( 'preset_archive_type_id', 0 );

		$this->csvinfo ['attrs'] = $this->getAttributes ();
		$this->csvinfo ['attrs'] [] = "---";

		/*
		 * PHASES
		 */
		$result = - 1;
		if (isset ( $_POST ['phase_00'] ) && isset ( $_POST ['chosen_archivetype'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 00
			 * Choose the archive type. (students, teachers..)
			 */
			$result = $this->execute_phase_0 ( $smarty );
		} elseif (isset ( $_POST ['phase_01'] ) && isset ( $_POST ['file_uploaded'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 01
			 * Upload CSV File.
			 */
			$result = $this->execute_phase_1 ( $smarty );
		} elseif (isset ( $_POST ['phase_02'] ) && isset ( $_POST ['archive_configured'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 02
			 * Configure uploaded CSV File.
			 */
			$result = $this->execute_phase_2 ( $smarty );
		} elseif (isset ( $_POST ['phase_03'] ) && isset ( $_POST ['data_sorted'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 03
			 * Configure CSV to LDAP Attributes.
			 */
			$result = $this->execute_phase_3 ( $smarty );
		} elseif (isset ( $_POST ['phase_04'] ) && isset ( $_POST ['accounts_reviewed'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 04
			 * Review and confirm the User objects that will be archived.
			 */
			$result = $this->execute_phase_4 ( $smarty );
		} elseif (isset ( $_POST ['phase_05'] ) && isset ( $_POST ['accounts_archived'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 05
			 * Confirm the archived User objects.
			 */
			$result = $this->execute_phase_5 ( $smarty );
		} elseif (isset ( $_POST ['phase_06'] ) && isset ( $_POST ['primgroups_moved'] ) && (! isset ( $_POST ['cancel_archiving'] ))) {
			/*
			 * PHASE 06
			 * Moving the primgroups
			 */
			$result = $this->execute_phase_6 ( $smarty );
		}

		/*
		 * $result is the return value of the function.
		 * So if somethings fails, we can pass the error code.
		 * If successfull then $result will be TRUE if not then its an integer higher than 0.
		 * It can be usefull for later applications.
		 * The value '1' means that an error msg was shown
		 */
		if ($result < 0) {
			/* No Phase was executed */
		} elseif ($result === 1) {
			/* Error message has already been shown */
		} elseif ($result !== TRUE || $this->failure_in_this_phase === TRUE) {
			msg_dialog::display ( _ ( "Error" ), _ ( "An unknown error occurred! Error code: $result" ), ERROR_DIALOG );
		}
		$this->failure_in_this_phase = FALSE;

		/* Show main page */
		return ($smarty->fetch ( get_template_path ( 'content_archiveaccounts.tpl', TRUE ) ));
	}
	function execute_phase_0($smarty) {
		$returnval = TRUE;

		if (isset ( $_POST ['archivetype_id'] )) {
			$archive_type = $_POST ['archivetype_id'];
			switch ($archive_type) {
				case 0 :
					$archive_type = "students";
					break;
				case 1 :
					$archive_type = "teachers";
					break;
				default :
					$this->failure_in_this_phase = TRUE;
					$returnval = 1;
					msg_dialog::display ( _ ( "Error" ), _ ( "Archive type wasn't an integer or in range! Please try again!" ), ERROR_DIALOG );
					break;
			}
		} else {
			$this->failure_in_this_phase = TRUE;
			$returnval = 1; /* Error msg will be shown -> error code 1 */
			msg_dialog::display ( _ ( "Error" ), _ ( "Archive type did get lost! Please try again!" ), ERROR_DIALOG );
		}

		if ($this->failure_in_this_phase === TRUE) {
			return $returnval;
		}

		$smarty->assign ( "chosen_archivetype", TRUE );
		$this->archive_type = $archive_type;
		$smarty->assign ( "archive_type", $this->archive_type );
		/* Declare delimiters for next phase UI only if phase_0 is successfull */
		$smarty->assign ( 'available_delimiters', array (
				0 => '"," - ' . _ ( "comma separated values" ),
				1 => '";" - ' . _ ( "semicolon separated values" ),
				2 => '<-> - ' . _ ( "tabstop separated values" ),
				3 => '" " - ' . _ ( "blank separated values" )
		) );
		$smarty->assign ( 'preset_delimiter_id', 0 );
		return $returnval;
	}
	function execute_phase_1($smarty) {
		$returnval = TRUE;

		$smarty->assign ( "chosen_archivetype", TRUE );
		$smarty->assign ( "archive_type", $this->archive_type );

		/* Only set default options if phase_1 was successfull */
		if ($this->archive_type === "students")
			$smarty->assign ( "preset_sel_ldap_match_attr_studentid", FALSE );
		$smarty->assign ( "preset_sel_ldap_match_attr_name", TRUE );
		$smarty->assign ( "preset_sel_ldap_match_attr_snname", TRUE );
		$smarty->assign ( "preset_sel_ldap_match_attr_birthday", FALSE );
		$smarty->assign ( "preset_sel_ldap_match_attr_gender", TRUE );

		$file_ok = $this->check_if_file_uploaded ( $smarty );
		if ($file_ok !== TRUE) {
			/* Error msg was already been shown */
			$returnval = 1;
			$smarty->assign ( 'available_delimiters', array (
					0 => '"," - ' . _ ( "comma separated values" ),
					1 => '";" - ' . _ ( "semicolon separated values" ),
					2 => '<-> - ' . _ ( "tabstop separated values" ),
					3 => '" " - ' . _ ( "blank separated values" )
			) );
			$smarty->assign ( 'preset_delimiter_id', 0 );
			return $returnval;
		} else {
			$smarty->assign ( "file_uploaded", TRUE );
		}


		/* Search OUs with objectClass gosaDepartment */
		$this->csvinfo ['ou_tree'] = array ();
		$this->csvinfo ['ou_tree'] ['ldapsearch'] = $this->_ldap->search ( "(objectClass=gosaDepartment)", array (
				"ou",
				"description"
		) );

		/* create arrays for search results */
		$this->csvinfo ['ou_tree'] ['formfields'] = array ();
		$this->csvinfo ['ou_tree'] ['OUs'] = array ();

		/* add found gosaDepartment objects */
		$i = 0;
		$default_ou_users = 0;
		$default_ou_matching_users = 0;
		while ( $result = $this->_ldap->fetch ( $this->csvinfo ['ou_tree'] ['ldapsearch'] ) ) {
			$this->csvinfo ['ou_tree'] ['OUs'] [] = $result ['ou'] [0];
			$this->csvinfo ['ou_tree'] ['formfields'] [] = $result ['ou'] [0] . " - " . $result ['description'] [0];
			$this->csvinfo ['ou_tree'] ['DNs'] [] = $result ['dn'];
			/* Use schoolmanager OU if it exists. */
			if (strcasecmp ( $result ['ou'] [0], "schoolmanager" ) == 0) {
				$default_ou_users = $i;
			} else if (strcasecmp ( $result ['ou'] [0], "students" ) == 0) {
				$default_ou_matching_users = $i;
			}
			$i = $i + 1;
		}

		$smarty->assign ( "ous_available", $this->csvinfo ['ou_tree'] ['formfields'] );
		$this->csvinfo ['ou_users'] = $default_ou_users;
		$this->csvinfo ['ou_matching_users'] = $default_ou_matching_users;
		$smarty->assign ( "preset_ou_users", $this->csvinfo ['ou_users'] );
		$smarty->assign ( "preset_ou_matching_users", $this->csvinfo ['ou_matching_users'] );

		return $returnval;
	}
	function execute_phase_2($smarty) {
		$returnval = TRUE;

		$this->csvinfo ['sel_ldap_match_attr_studentid'] = isset ( $_POST ["sel_ldap_match_attr_studentid"] );
		$this->csvinfo ['sel_ldap_match_attr_name'] = isset ( $_POST ["sel_ldap_match_attr_name"] );
		$this->csvinfo ['sel_ldap_match_attr_snname'] = isset ( $_POST ["sel_ldap_match_attr_snname"] );
		$this->csvinfo ['sel_ldap_match_attr_birthday'] = isset ( $_POST ["sel_ldap_match_attr_birthday"] );
		$this->csvinfo ['sel_ldap_match_attr_gender'] = isset ( $_POST ["sel_ldap_match_attr_gender"] );
		if (isset ( $_POST ['ou_users'] ))
			$this->csvinfo ['ou_users'] = $_POST ['ou_users'];
		if (isset ( $_POST ['ou_matching_users'] ))
			$this->csvinfo ['ou_matching_users'] = $_POST ['ou_matching_users'];

		if ($this->failure_in_this_phase === FALSE) {
			$smarty->assign ( "archive_configured", TRUE );
			$smarty->assign ( "file_uploaded", TRUE );
			$smarty->assign ( "chosen_archivetype", TRUE );

			/* initialize $this->csvinfo['attrs_selected'], only do this here and once */
			$this->csvinfo ['attrs_selected'] = $this->getAttrsPreSelection ( $this->csvinfo ['num_cols'] );

			/* student import attributes */
			$smarty->assign ( "attrs", $this->csvinfo ['attrs'] );

			/* per row selected student import attributes */
			$smarty->assign ( "attrs_selected", $this->csvinfo ['attrs_selected'] );

			/* number of CSV columns -> number of rows in 90°-counter-clockwise-rotated table */
			$smarty->assign ( "num_rows", $this->csvinfo ['num_cols'] );

			/* CSV data */
			$smarty->assign ( "data_size", $this->csvinfo ['num_rows'] );
			$smarty->assign ( "data", array_slice ( $this->csvinfo ['data'], 0, 5 ) );
		}

		return $returnval;
	}
	function execute_phase_3($smarty) {
		$returnval = TRUE;
		$smarty->assign ( "archive_configured", TRUE );
		$smarty->assign ( "file_uploaded", TRUE );
		$smarty->assign ( "chosen_archivetype", TRUE );

		/* sanity checks on LDAP attributes assignments */

		/* read attributes assignments from $_POST */
		$new_attrs_selected = array ();
		for($i = 0; $i < count ( $this->csvinfo ['attrs_selected'] ); $i ++) {
			if (isset ( $_POST ["column_head_$i"] )) {
				$new_attrs_selected [] = $_POST ["column_head_$i"];
			} else {
				$new_attrs_selected [] = $i;
			}
		}
		$this->csvinfo ['attrs_selected'] = $new_attrs_selected;

		/* sort the CSV date table according to how it got re-ordered by the webUI admin user */
		$this->csvinfo ['data_sorted'] = array ();
		$multi_attrs = $this->getMultiAttributes ();
		foreach ( $this->csvinfo ['data'] as $data_row ) {

			$attrs_counter = array ();
			$data_row_sorted = array ();

			for($i = 0; $i < count ( $data_row ); $i ++) {
				$data_in_cell = $data_row [$i];
				$selection_in_cell = $this->csvinfo ['attrs_selected'] [$i];
				$value_of_selection = $this->csvinfo ['attrs'] [$selection_in_cell];
				if ($value_of_selection == "---") {
				} elseif (in_array ( $value_of_selection, $multi_attrs )) {
					if (isset ( $attrs_counter [$value_of_selection] )) {
						$attrs_counter [$value_of_selection] = $attrs_counter [$value_of_selection];
					} else {
						$attrs_counter [$value_of_selection] = 0;
					}
					$data_row_sorted [$value_of_selection . $attrs_counter [$value_of_selection]] = $data_in_cell;
					$attrs_counter [$value_of_selection] ++;
				} elseif (empty ( $attrs_counter [$value_of_selection] )) {
					$data_row_sorted [$value_of_selection] = $data_in_cell;
					$attrs_counter [$value_of_selection] = 1;
				} else {
					$this->failure_in_this_phase = TRUE;
					$smarty->assign ( "LDIFError", TRUE );
					msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "The attribute %s is only allowed to select once!" ), bold ( $value_of_selection ) ), ERROR_DIALOG );
					$returnval = 1;
				}
			}

			$this->csvinfo ['data_sorted'] [] = $data_row_sorted;
		}

		/* transform data_sorted to data_preldap array */
		$this->csvinfo ['data_preldap'] = $this->prepareLdapImport ( $this->csvinfo ['data_sorted'] );

		/* $this->failure_in_this_phase may have been set above or in $this->prepareLdapImport ... */
		if ($this->failure_in_this_phase === FALSE) {
			/* free some memory... */
			unset ( $this->csvinfo ['data_sorted'] );

			$retval = $this->accountStatusCheck ();
			if ($this->failure_in_this_phase === TRUE) {
				if ($retval !== 1)
					msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "accountStatusCheck() failed and returned an unknown Error: %s" ), $retval ), ERROR_DIALOG );
				$returnval = 1; /* Value '1' because a msgbox was already shown */
				return $returnval;
			}

			$smarty->assign ( "data_sorted", TRUE );
			$smarty->assign ( "data", $this->csvinfo ['data_preldap'] );
		} else {
			/* prepare for reloading this phase's web page again */
			$smarty->assign ( "attrs", $this->csvinfo ['attrs'] );
			$smarty->assign ( "attrs_selected", $this->csvinfo ['attrs_selected'] );
			$smarty->assign ( "num_rows", $this->csvinfo ['num_cols'] );
			$smarty->assign ( "data_size", $this->csvinfo ['num_rows'] );
			$smarty->assign ( "data", array_slice ( $this->csvinfo ['data'], 0, 5 ) );
			msg_dialog::display ( "Error", "Something went terribly wrong. You shouldn't see this message.", INFO_DIALOG );
		}

		return $returnval;
	}
	function execute_phase_4($smarty) {
		$returnval = TRUE;
		$smarty->assign ( "data_sorted", TRUE );
		$smarty->assign ( "archive_configured", TRUE );
		$smarty->assign ( "file_uploaded", TRUE );
		$smarty->assign ( "chosen_archivetype", TRUE );

		unset ( $failed_accs );
		foreach ( $this->csvinfo ['data_preldap'] as $idx => $user_data ) {
			if ((isset ( $user_data ['main_account'] ) && (! empty ( $user_data ['main_account'] )))) {
				$return = $this->archiveLDAPUserObject ( $user_data ['main_account'] );
			}

			if ($return === TRUE) {
				/* Now we're going to lock the User account. */
				$this->lockEntry ( $user_data ['main_account'] ['_dn'] );
				$user = new userManagement ();
				$user->lockEntry ();
				/* Setting _status of user to 'archived,acc-locked' */
				$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_status'] [0] = "archived,acc-locked";
				$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_actions'] [0] = "none";

				$expected_primgroup_dn = "cn=" . $this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['uid'] [0] . "," . get_groups_ou () . dn2base ( $this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_dn_ldap'] [0] );

				$this->_ldap->cat ( $expected_primgroup_dn, array (
						'dn'
				) );
				$primgroup = $this->_ldap->fetch ();
				if (! $primgroup) {
					$this->_ldap->search ( "(&(objectClass=posixGroup)(cn=" . $this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['uid'] [0] . "))", array (
							"dn"
					) );
					$primgroup = $this->_ldap->fetch ();
					if ($primgroup) {
						$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_group_actions'] = array (
								'move-primgroup'
						);
					}
				} else {
					$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_group_actions'] = array (
							'move-primgroup'
					);
				}
				$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_ogroup_actions'] = array (
						'none'
				);

				$new_primgroup_base = $user_data ['main_account'] ['_base'] [0];
				$this->check_ou_exists ( $new_primgroup_base );

				$new_primgroup_dn = "cn=archived-" . date ( "Ymd" ) . "-" . $user_data ['main_account'] ['uid'] [0] . ',' . get_groups_ou () . $new_primgroup_base;

				$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_primgroup_dn'] [0] = $new_primgroup_dn;
				$this->csvinfo ['data_preldap'] [$idx] ['main_account'] ['_primgroup_dn_ldap'] [0] = var_export ( $primgroup ['dn'], TRUE );
			} else if ($return !== 1) { // return val 1 -> error msg. was already shown.
				$failed_accs [] = sprintf ( _ ( "%s %s with error code: %s" ), $user_data ['main_account'] ['givenName'] [0], $user_data ['main_account'] ['sn'] [0], $return );
			}
		}

		if (count ( $failed_accs ) >= 1) {
			msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Following users failed to archive: %s" ), var_export ( $failed_accs, TRUE ) ), ERROR_DIALOG );
		}
		$smarty->assign ( "data", $this->csvinfo ['data_preldap'] );
		$smarty->assign ( "accounts_reviewed", TRUE ); // PHASE 4

		return $returnval;
	}
	function execute_phase_5($smarty) {
		$returnval = TRUE;
		$smarty->assign ( "accounts_reviewed", TRUE ); // PHASE 4
		$smarty->assign ( "data_sorted", TRUE ); // PHASE 3
		$smarty->assign ( "archive_configured", TRUE ); // PHASE 2
		$smarty->assign ( "file_uploaded", TRUE ); // PHASE 1
		$smarty->assign ( "chosen_archivetype", TRUE ); // PHASE 0

		foreach ( $this->csvinfo ['data_preldap'] as $idx => $user_data ) {
			if ((isset ( $user_data ['main_account'] ) && (! empty ( $user_data ['main_account'] )))) {
				if (strpos ( $user_data ['main_account'] ['_group_actions'] [0], 'move-primgroup' ) !== FALSE) {
					/* Old UID == Groupname (Hopefully) */
					$group_name = $user_data ['main_account'] ['uid'] [0];
					$new_group_name = ('archived-' . date ( "Ymd" ) . '-' . $group_name);
					$group_data = array (
							'objectClass' => array (
									'posixGroup'
							),
							'cn' => array (
									$new_group_name
							),
							'_dn' => array (
									'cn=' . $new_group_name . ',' . get_groups_ou () . $user_data ['main_account'] ['_base_ldap'] [0]
							),
							'_actions' => array (
									'move'
							)
					);
					$returnval = $this->archiveLDAPGroupObject ( $group_name, $group_data );
				} else {
				}
			}
		}
		$smarty->assign ( "data", $this->csvinfo ['data_preldap'] );
		$smarty->assign ( "accounts_archived", TRUE ); // PHASE 5
		return $returnval;
	}
	function execute_phase_6($smarty) {
		$returnval = TRUE;
		$smarty->assign ( "accounts_archived", TRUE ); // PHASE 5
		$smarty->assign ( "accounts_reviewed", TRUE ); // PHASE 4
		$smarty->assign ( "data_sorted", TRUE ); // PHASE 3
		$smarty->assign ( "archive_configured", TRUE ); // PHASE 2
		$smarty->assign ( "file_uploaded", TRUE ); // PHASE 1
		$smarty->assign ( "chosen_archivetype", TRUE ); // PHASE 0

		$smarty->assign ( "data", $this->csvinfo ['data_preldap'] );
		$smarty->assign ( "primgroups_moved", TRUE ); // PHASE 6
		return $returnval;
	}
	function archiveLDAPUserObject($user_data) {
		/* only perform on this user dataset if _actions does not contain "none"... */
		if ((strpos ( $user_data ['_actions'] [0], 'none' ) !== FALSE) or (strpos ( $user_data ['_actions'] [0], 'ignore' ) !== FALSE)) {
			/* Errorcode 1403 */
                        $returnval = 1403;
                        return $returnval;
		}

		/* Retrieve user object from LDAP */
		$usertab = new usertabs ( $this->config, $this->config->data ['TABS'] ['USERTABS'], $user_data ['_dn_ldap'] [0] );
		$usertab->by_object ['user']->base = $user_data ['_base'] [0];

		$this->failure_messages = array ();
		if (count ( $usertab->check () )) {

			$this->failure_messages [] = $usertab->check ();

			/*
			 * FIXME: collect failure statistics here!!!
			 */

			$this->failure_in_this_phase = TRUE;
			/* Errorcode 3004 */
			$returnval = 3004;
			return $returnval;
		} else {
			/* Moving and renaming user */
			$attr = "uid";
			$old_user_dn = $user_data ['_dn_ldap'];
			/* Extracting new UID from new DN */
			$new_user_dn = $user_data ['_dn'] [0];
			$new_user_uid = explode ( ",", $new_user_dn, 2 ) [0];
			$new_user_uid = substr ( $new_user_uid, strpos ( $new_user_uid, "=" ) + 1 );

			if (isset ( $usertab->$attr )) {
				$usertab->$attr = $new_user_uid;
			}
			foreach ( $usertab->by_object as $pname => $plugin ) {
				if (isset ( $usertab->by_object [$pname]->$attr )) {
					$usertab->by_object [$pname]->$attr = $new_user_uid;
				}
			}
			/* Saving changes made to the user */
			$usertab->save ();
			/* Present a status update in GOsa²'s logging system (/var/log/syslog mostly) */
			new log ( "modify", "users/user", $old_user_dn, array (), "Existing user  (" . $usertab->sn . ", " . $usertab->givenName . ", " . $user_data ['uid'] [0] . ") archived via SchoolManager add-on." );
			
			/* Commiting changes to GOsa² */
			$usertab->by_object ['user']->rename ( $usertab->dn, $new_user_dn );
		}
		return TRUE;
	}
	function archiveLDAPGroupObject($group_name, $group_data) {
		$returnval = "TRUE";

		/* only perform on this user dataset if _actions does not contain "none"... */
		if ((strpos ( $group_data ['_actions'] [0], 'none' ) !== FALSE) or (strpos ( $group_data ['_actions'] [0], 'ignore' ) !== FALSE)) {
			/* Errorcode 1403 */
			$returnval = 1403;
			return $returnval;
		}

		if (in_array ( 'posixGroup', $group_data ['objectClass'] )) {
			$_group_obclass = "posixGroup";
			$_group_tabs = "GROUPTABS";
			$_group_tabs_class = "grouptabs";
			$_group_RDN = get_groups_ou ();
			$_group_obj = "group";
		} else {
			$_group_obclass = "gosaGroupOfNames";
			$_group_tabs = "OGROUPTABS";
			$_group_tabs_class = "ogrouptabs";
			$_group_RDN = get_ou ( "group", "ogroupRDN" );
			$_group_obj = "ogroup";
		}

		if ((strpos ( $group_data ['_actions'] [0], 'create' ) !== FALSE) or (strpos ( $group_data ['_actions'] [0], 'move' ) !== FALSE)) {

			/* Instantiate a new group object via GOsa²'s API */
			$grouptab = new $_group_tabs_class ( $this->config, $this->config->data ['TABS'] [$_group_tabs], 'new' );
			if (isset ( $group_data ['_dn'] [0] )) {
				$grouptab->by_object [$_group_obj]->base = dn2base ( $group_data ['_dn'] [0] );
			}
		} else {
			/* Retrieve group object from LDAP via GOsa²'s API */
			$grouptab = new $_group_tabs_class ( $this->config, $this->config->data ['TABS'] [$_group_tabs], $group_data ['_dn'] [0] );
		}

		if (strpos ( $group_data ['_actions'] [0], 'move' ) !== FALSE) {
			/* do an ldapsearch for the source object and prepare a copy+paste action */

			$_ldapsearch = $this->_ldap->search ( "(&(objectClass=" . $_group_obclass . ")(cn=" . $group_name . "))", array (
					"*"
			) );
			
			/*
			 * There should only be _ONE_ object of this CN in the given LDAP (sub)tree scope.
			 * Thus, only fetching the first object.
			 */
			$source = $this->_ldap->fetch ( $_ldapsearch );

			if (! empty ( $source )) {
				foreach ( $grouptab->by_object as $pname => $plugin ) {
					$grouptab->by_object [$pname]->prepareForCopyPaste ( $source );
				}
				$remove_later_grouptab = new $_group_tabs_class ( $this->config, $this->config->data ['TABS'] [$_group_tabs], $source ['dn'] );
			}
		}

		/* Collect group properties from $group_data */
		$_group_properties = array (
				"cn" => $group_data ['cn'] [0]
		);
		if (((strpos ( $group_data ['_actions'] [0], 'update-description' ) !== FALSE) or (strpos ( $group_data ['_actions'] [0], 'create' ) !== FALSE)) and isset ( $group_data ['description'] [0] )) {
			$_group_properties ["description"] = $group_data ['description'] [0];
		}
		if (((strpos ( $group_data ['_actions'] [0], 'update-mail' ) !== FALSE) or (strpos ( $group_data ['_actions'] [0], 'create' ) !== FALSE)) and isset ( $group_data ['mail'] [0] )) {
			$_group_properties ["mail"] = $group_data ['mail'] [0];
		}

		/*
		 * FIXME: Make mail address creation (from CN) configurable somewhere...
		 */

		/* Populate sub-object attributes for $grouptab with same values */
		foreach ( $_group_properties as $attr => $val ) {
			if (isset ( $grouptab->$attr )) {
				$grouptab->$attr = $val;
			}
			foreach ( $grouptab->by_object as $pname => $plugin ) {
				if (isset ( $grouptab->by_object [$pname]->$attr )) {
					$grouptab->by_object [$pname]->$attr = $val;
				}
			}
		}

		/* if we do import mail properties, make sure the mailgroup object is activated */
		if (isset ( $_group_properties ['mail'] ) && isset ( $grouptab->by_object ['mailgroup'] )) {
			$grouptab->by_object ['mailgroup']->is_account = TRUE;
		}

		/* Run GOsa²'s groups/group | ogroups/ogroup checks */

		if (count ( $grouptab->check () )) {
			msg_dialog::displayChecks ( $grouptab->check () );

			/*
			 * FIXME: collect failure statistics here!!!
			 */

			$this->failure_in_this_phase = TRUE;
			$returnval = 808;
			return $returnval;
		} else {

			if (isset ( $remove_later_grouptab )) {
				$remove_later_grouptab->delete ();
			}
			/* Save group object to LDAP */
			$grouptab->save ();
		}

		/* Present a status update in GOsa²''s logging system (/var/log/syslog mostly) */
		if (strpos ( $group_data ['_actions'] [0], 'create' ) !== FALSE) {
			new log ( "create", "groups/group", $grouptab->dn, array (), "New group created via SchoolManager add-on." );
		} elseif (strpos ( $group_data ['_actions'] [0], 'none' ) !== FALSE) {
			new log ( "modify", "groups/group", $grouptab->dn, array (), "Existing group modified via SchoolManager add-on." );
		}

		return $returnval;
	}
	function check_if_file_uploaded($smarty) {
		/* Check if theres a file uploaded */
		if (! empty ( $_FILES ['userfile'] ['name'] )) {
			$handle = NULL;
			$filename = gosa_file_name ( $_FILES ['userfile'] ['tmp_name'] );
			if (! isset ( $_FILES ['userfile'] ['name'] )) {
				msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Cannot read uploaded file: %s" ), _ ( "file not found" ) ), ERROR_DIALOG );
				$smarty->assign ( "LDIFError", TRUE );
				return FALSE;
			} elseif (! $_FILES ['userfile'] ['size'] > 0) {
				msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Cannot read uploaded file: %s" ), _ ( "file is empty" ) ), ERROR_DIALOG );
				$smarty->assign ( "LDIFError", TRUE );
				return FALSE;
			} /* Is there a tmp file, which we can use ? */
			elseif (! file_exists ( $filename )) {
				msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Cannot read uploaded file: %s" ), _ ( "file not found" ) ), ERROR_DIALOG );
				$smarty->assign ( "LDIFError", TRUE );
				return FALSE;
			} elseif (! $handle = @fopen ( $filename, "r" )) {
				msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Cannot read uploaded file: %s" ), _ ( "file not readable" ) ), ERROR_DIALOG );
				$smarty->assign ( "LDIFError", TRUE );
				return FALSE;
			} else {
				$smarty->assign ( "file_uploaded", TRUE );
				$raw_csv_data = "";

				/* Reading content */
				while ( ! feof ( $handle ) ) {
					$raw_csv_data .= fread ( $handle, 1024 );
				}

				@fclose ( $handle );

				if (! mb_check_encoding ( $raw_csv_data, "UTF-8" )) {
					if (mb_check_encoding ( $raw_csv_data, "iso-8859-1" )) {
						$raw_csv_data = utf8_encode ( $raw_csv_data );
					} else {
						$smarty->assign ( "LDIFError", TRUE );
						$smarty->assign ( "file_uploaded", FALSE );
						msg_dialog::display ( _ ( "Error" ), _ ( "File has an unsupported encoding!" ), ERROR_DIALOG );
						return FALSE;
					}
				}

				// Parse user settings…
				switch ($_POST['delimiter_id']) {
					case 0:
						// Comma separated values
						$delimiter_char = ",";
						break;
					case 1:
						// Semicolon separated values
						$delimiter_char = ";";
						break;
					case 2:
						// Tabstop separated values
						$delimiter_char = "\t";
						break;
					case 3:
						// Blank separated values
						$delimiter_char = " ";
						break;
					default:
						// Default CSV delimiter.
						$delimiter_char = $this->getConfigValue("csv_column_delimiter");
						break;
				}

				if (isset($_POST['csv_with_column_headers'])) {
					$this->csvinfo['skip_first_line'] = TRUE;
				}
				else {
					$this->csvinfo['skip_first_line'] = FALSE;
				}

				// Parse CSV file using utils class.
				$parser_ret = $this->utils->parseCSV($raw_csv_data, $delimiter_char,
													 $this->csvinfo['skip_first_line']);

				if(empty($parser_ret)) {
					$smarty->assign("LDIFError", TRUE);
					$smarty->assign("file_uploaded", FALSE);

					msg_dialog::display(_("Error"), _("Cannot find CSV data in the selected file!"), ERROR_DIALOG);

					return;
				} else {
					$this->csvinfo = $parser_ret;
				}
			}
		} else {
			$smarty->assign ( "LDIFError", TRUE );
			$smarty->assign ( "file_uploaded", FALSE );
			msg_dialog::display ( _ ( "Error" ), _ ( "You need to select a file." ), ERROR_DIALOG );
			return FALSE;
		}
	}

	function getAttributes() {
		$attrs = array ();
		$attrs [] = "no";
		$attrs [] = "uid";
		/* Only Students have a Student ID */
		if ($this->archive_type === "students")
			$attrs [] = "employeeNumber";
		$attrs [] = "sn";
		$attrs [] = "givenName";
		$attrs [] = "dateOfBirth";
		$attrs [] = "gender";

		return $attrs;
	}
	function getAttrsPreSelection($size) {
		$attrs_count = count ( $this->getAttributes () );
		$selection = array ();
		for($i = 0; $i < $attrs_count; $i ++) {
			$selection [] = $i;
		}
		for($i = $attrs_count; $i < $size; $i ++) {
			$selection [] = $attrs_count;
		}
		return $selection;
	}
	function getMultiAttributes() {
		$multiAttrs = array ();
		return $multiAttrs;
	}
	function prepareLdapImport($csv_data_sorted) {
		/* Get elemenet */
		$data_preldap = array ();

		foreach ( $csv_data_sorted as $idx => $row_sorted ) {
			if ($this->CSVRowSanityChecksOk ( $row_sorted, $idx )) {

				/* main account */
				$person = array ();
				if (isset ( $row_sorted ['uid'] ) && $row_sorted ['uid'] && (($row_sorted ['uid'] != "%auto%") && ($row_sorted ['uid'] != "%uid%") && ($row_sorted ['uid'] != "%{uid}"))) {
					$person ['uid'] = array (
							strtolower ( iconv ( 'UTF-8', 'US-ASCII//TRANSLIT', trim ( $row_sorted ['uid'] ) ) )
					);
				} else {
					$person ['uid'] = array (
							'{%uid}'
					);
				}
				$person ['sn'] = array (
						trim ( $row_sorted ['sn'] )
				);
				$person ['givenName'] = array (
						trim ( $row_sorted ['givenName'] )
				);

				$_dateOfBirth = trim ( $row_sorted ['dateOfBirth'] );
				/*
				 * FIXME 1: On a German localized GOsa², this is not required..., but how about other locales???
				 * FIXME 2: If date is formatted like "dd.mm.yy" GOsa² will not use yy as the birthyear but print error
				 */
				if (strpos ( $_dateOfBirth, '.' )) {
					list ( $day, $month, $year ) = explode ( ".", $_dateOfBirth, 3 );
					$_dateOfBirth = $year . "-" . $month . "-" . $day;
					// $_dateOfBirth = $day.".".$month.".20".$year;
				}
				$person ['dateOfBirth'] = array (
						$_dateOfBirth
				);
				$person ['gender'] = array (
						$this->normalizeGenderString ( trim ( $row_sorted ['gender'] ) )
				);
				if (isset ( $row_sorted ['employeeNumber'] ) and $row_sorted ['employeeNumber']) {
					$person ['employeeNumber'] = array (
							$row_sorted ['employeeNumber']
					);
				}
				// /*
				// * Keeping the memory footprint low:
				// *
				// * $student['_template']
				// * is only an int value.
				// *
				// * Retrieve the actual template DN string later like this
				// * $template_dn = $this->csvinfo['templates']['DNs'][$student['_template']];
				// */
				// $person ['_template'] = array (
				// $this->csvinfo ['template_main']
				// );
				$person ['_status'] = array (
						'unchecked'
				);
				$person ['_actions'] = array (
						'none'
				);
				$person ['_group_actions'] = array (
						'none'
				);
			}

			$data_preldap [] = array (
					'main_account' => $person
			);
		}
		return $data_preldap;
	}
	function normalizeGenderString($gender) {
		switch (strtolower ( $gender )) {
			case "male" :
				$gender = "M";
				break;
			case _ ( "male" ) :
				$gender = "M";
				break;
			case "female" :
				$gender = "F";
				break;
			case _ ( "female" ) :
				$gender = "F";
				break;
		}
		return $gender;
	}
	function CSVRowSanityChecksOk($csv_row, $idx) {
		$ok = FALSE;
		if ((empty ( $csv_row ['sn'] )) || (empty ( $csv_row ['givenName'] )) || (! isset ( $csv_row ['sn'] )) || (! isset ( $csv_row ['givenName'] ))) {

			/* Output Error about missing the least set of attributes */
			msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Need at least %s and %s to create users (Check line %d in CSV file)!" ), bold ( "sn" ), bold ( "givenName" ), $idx + 1 ), ERROR_DIALOG );
		} /*
		   *
		   * FIXME: Add plenty of more CSV data sanity checks here!!!!
		   *
		   */

		else {
			$ok = TRUE;
		}

		if (! $ok) {
			$this->failure_in_this_phase = TRUE;
		}

		return $ok;
	}
	function accountStatusCheck() {
		/* If we don't have the $this->csvinfo['data_preldap'] dataset, we are doomed to fail... */
		if (! is_array ( $this->csvinfo ['data_preldap'] ) or (empty ( $this->csvinfo ['data_preldap'] ))) {
			$this->failure_in_this_phase = TRUE;
			msg_dialog::display ( _ ( "Error" ), _ ( "csvinfo['data_preldap'] is empty or not an array! Can't complete accountStatusCheck(). Please try again." ), ERROR_DIALOG );
			return 1; /* We showed an Error box so value '1' (We dont want more boxes) */
		}

		$new_base = "ou=archived-" . date ( "Ymd" ) . "," . $this->csvinfo ['ou_tree'] ['DNs'] [$this->csvinfo ['ou_users']];
		$this->check_ou_exists ( $new_base );

		$ldapsearch = array (
				"uid",
				"sn",
				"givenName",
				"gender",
				"dateOfBirth",
				"mail",
				"departmentNumber",
				"employeeNumber"
		);
		if (isset ( $this->csvinfo ['ou_tree'] ['DNs'] [$this->csvinfo ['ou_matching_users']] )) {
			$this->_ldap->cd ( $this->csvinfo ['ou_tree'] ['DNs'] [$this->csvinfo ['ou_matching_users']] );
			$ldapsearch = $this->_ldap->search ( "(objectClass=gosaAccount)", $ldapsearch );
		} else {
			$this->_ldap->cd ( $this->config->current ['BASE'] );
			$ldapsearch = $this->_ldap->search ( "(objectClass=gosaAccount)", $ldapsearch );
		}

		/* this will probably scale very very badly... Improvement needed. Suggestions? */
		while ( $gosa_account = $this->_ldap->fetch ( $ldapsearch ) ) {
			foreach ( $this->csvinfo ['data_preldap'] as $key => $row ) {
				if (isset ( $row ['main_account'] )) {
					$test_attrs = array ();
					if ($this->csvinfo ['sel_ldap_match_attr_studentid'])
						$test_attrs [] = "employeeNumber";
					if ($this->csvinfo ['sel_ldap_match_attr_snname'])
						$test_attrs [] = "sn";
					if ($this->csvinfo ['sel_ldap_match_attr_name'])
						$test_attrs [] = "givenName";
					if ($this->csvinfo ['sel_ldap_match_attr_gender'])
						$test_attrs [] = "gender";
					if ($this->csvinfo ['sel_ldap_match_attr_birthday'])
						$test_attrs [] = "dateOfBirth";

					/* detect status */
					if ($this->compareObjects ( $row ['main_account'], $gosa_account, $test_attrs, "", TRUE ) === TRUE) {
						if ((! isset ( $row ['main_account'] ['uid'] [0] )) || ($row ['main_account'] ['uid'] [0] === "{%uid}") || ($row ['main_account'] ['uid'] [0] === "%uid")) {
							$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['uid'] = array (
									$gosa_account ['uid'] [0]
							);
							$row ['main_account'] ['uid'] = $this->csvinfo ['data_preldap'] [$key] ['main_account'] ['uid'];
						}
						// $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
						// $expected_account_dn = "uid=" . $row['main_account']['uid'][0] . ',' . get_people_ou() . $expected_base;
						// $expected_account_uid = $row['main_account']['uid'][0];

						/* Explode into array, pick first then get the part after uid= */
						$gosa_account_dn = $gosa_account ['dn'];
						$gosa_account_uid = explode ( ",", $gosa_account_dn, 2 ) [0];
						$gosa_account_uid = substr ( $gosa_account_uid, strpos ( $gosa_account_uid, "=" ) + 1 );

						$gosa_account_base = explode ( ",", $gosa_account_dn, 3 );
						$gosa_account_base = $gosa_account_base [2];

						$new_dn = "uid=archived-" . date ( "Ymd" ) . "-" . $row ['main_account'] ['uid'] [0] . ',' . get_people_ou () . $new_base;

						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_status'] [0] = 'exists';

						/* detect actions */
						$_actions = array ();
						if ($this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_status'] [0] === 'exists') {
							$_actions [] = "archive";
						}

						if (empty ( $_actions )) {
							$_actions [] = 'none';
						}

						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_actions'] [0] = implode ( ",", $_actions );

						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_dn_ldap'] = array (
								$gosa_account_dn
						);
						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_dn'] = array (
								$new_dn
						);
						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_base_ldap'] = array (
								$gosa_account_base // get_people_ou() $expected_base
						);
						$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_base'] = array (
								$new_base
						);
					} // elseif ($this->compareObjects($row['main_account'], $gosa_account, array(
					  // "uid"
					  // ), "", TRUE) === TRUE) {
					  // // The LDAP tree already has an account with this uid, this requires manual introspection

					// $expected_base = $this->genAccountBase($row['main_account'], $row['groups']);
					// $expected_account_dn = "uid=" . $row['main_account']['uid'][0] . ',' . get_people_ou() . $expected_base;
					// $gosa_account_dn = $gosa_account['dn'];

					// if (strtolower($gosa_account_dn) == strtolower($expected_account_dn)) {
					// $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'uid-used-in-same-ou';
					// } else {
					// $this->csvinfo['data_preldap'][$key]['main_account']['_status'][0] = 'uid-used-in-other-ou';
					// }
					// $this->csvinfo['data_preldap'][$key]['main_account']['_actions'][0] = 'ignore,check-manually';
					// }
				}
			}
		}
		foreach ( $this->csvinfo ['data_preldap'] as $key => $row ) {
			if (isset ( $row ['main_account'] )) {
				if ($this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_status'] [0] == 'unchecked') {
					$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_status'] [0] = 'not-found';
					$this->csvinfo ['data_preldap'] [$key] ['main_account'] ['_actions'] [0] = 'ignore,check-manually';
				}
			}
		}
	}
	function compareObjects($object_a, $object_b, $attrs = array("sn", "givenName"), $prefix = "", $ci = FALSE) {
		$unequal = array ();
		foreach ( $attrs as $key => $attr ) {

			$val_a = $val_b = NULL;
			if (isset ( $object_a [$attr] [0] )) {
				$val_a = ($ci === TRUE) ? strtolower ( $object_a [$attr] [0] ) : $object_a [$attr] [0];
			}
			if (isset ( $object_b [$attr] [0] )) {
				$val_b = ($ci === TRUE) ? strtolower ( $object_b [$attr] [0] ) : $object_b [$attr] [0];
			}

			if ($val_a !== $val_b) {
				$unequal [] = $prefix . $attr;
			}
		}
		if (empty ( $unequal )) {
			/* if TRUE is returned, objects regarding the given attributes are identical */

			return TRUE;
		} else {
			/* otherwise return those attribute names that did not give a match */
			return $unequal;
		}
	}
	function genAccountBase($user_data, $groups_data = array()) {
		if ($user_data ['_template'] [0] != 0) {
			$template_dn = $this->csvinfo ['templates'] ['DNs'] [$user_data ['_template'] [0]];
			$template_base = preg_replace ( "/^[^,]+," . preg_quote ( get_people_ou (), '/i' ) . "/", '', $template_dn );
		} else {
			$template_base = $this->config->current ['BASE'];
		}

		/* Put accounts into a sub-OU of the template's base DN if requested */
		if ($this->csvinfo ['accounts_in_class_ou'] === TRUE) {
			$_class_group = "";
			foreach ( $groups_data as $group_key => $group_data ) {

				if (strpos ( $group_data ['cn'] [0], 'class_' ) !== FALSE) {
					$_class_group = 'ou=' . str_replace ( 'class_', '', $group_data ['cn'] [0] ) . ',';
					/*
					 * we take the first class group we find...
					 * presuming that students only have _one_ class group assigned(!)
					 */
					break;
				}
			}
			$template_base = $_class_group . $template_base;
		}
		return $template_base;
	}
	function check_ou_exists($base = NULL) {
		if ($base == NULL) {
			return 2904;
		}

		$this->_ldap->cat ( $base );
		if (! $this->_ldap->fetch ()) {
			msg_dialog::display ( _ ( "Info" ), sprintf ( _ ( "%s doesn't exist! Creating it..." ), $base ), INFO_DIALOG );

			/* change UI "dir" in LDAP to the place where we want to create sub-OUs... */
			$_ui = get_userinfo ();
			$old_ui_dn = $_ui->dn;

			$old_current_main_base = session::global_is_set ( "CurrentMainBase" ) ? session::global_get ( "CurrentMainBase" ) : $_ui->dn;

			/*
			 * if ($user_data ['_template'] [0] != 0) {
			 * change_ui_dn ( $old_ui_dn, dn2base ( $template_dn ) );
			 * session::global_set ( "CurrentMainBase", dn2base ( $template_dn ) );
			 * } else {
			 */

			$_pre_ou = explode ( ",", $base, 2 );
			$_pre_ou = $_pre_ou [1];

			$this->_ldap->cat ( $_pre_ou );
			if (! $this->_ldap->fetch ()) {
				msg_dialog::display ( _ ( "Info" ), sprintf ( _ ( "%s doesn't exist! Creating it..." ), $_pre_ou ), INFO_DIALOG );
				if (TRUE !== $this->check_ou_exists ( $_pre_ou )) {
					msg_dialog::display ( _ ( "Error" ), sprintf ( _ ( "Couldn't create %s" ), $base ), ERROR_DIALOG );
					/* Errocode 2904 */
					$returnval = 2904;
					return $returnval;
				}
			}

			change_ui_dn ( $old_ui_dn, $_pre_ou );
			session::global_set ( "CurrentMainBase", $_pre_ou );



			/*
			 * If _pre_ou is part of the current base then DON'T put _pre_ou infront of current base
			 * $_pre_ou_tmp = explode ( ",", $_pre_ou, 2 );
			 * $_pre_ou_tmp = $_pre_ou_tmp [0];
			 *
			 * if (substr ( $this->config->current ['BASE'], 0, strlen ( $_pre_ou_tmp ) ) !== $_pre_ou_tmp) {
			 * }else {
			 * change_ui_dn ( $old_ui_dn, $_pre_ou . $this->config->current ['BASE']);
			 * session::global_set ( "CurrentMainBase", $_pre_ou . "," . $this->config->current ['BASE'] );
			 * }
			 */

			/* create _base DN as a GOsa² department */
			$_ou = preg_replace ( "/^ou=([^,]+),.*/", "$1", $base );


			$deptab = new deptabs ( $this->config, $this->config->data ['TABS'] ['DEPTABS'], 'new', "deptabs" );
			$deptab->by_object ['department']->ou = $_ou;
			$deptab->by_object ['department']->description = sprintf ( _ ( "Archived: %s" ), $_ou );
			$deptab->save ();

			/* Reload departments */
			$this->config->get_departments ();
			$this->config->make_idepartments ();

			/* change UI dir back to where we came from */
			change_ui_dn ( dn2base ( $_pre_ou . "," . $this->config->current ['BASE'] ), $old_ui_dn );
			session::global_set ( "CurrentMainBase", $old_current_main_base );
		} else {
			msg_dialog::display ( _ ( "Info" ), sprintf ( _ ( "%s already exists. Using it." ), $base ), INFO_DIALOG );
		}
		return TRUE;
	}

	/*
	 * Locks/unlocks the given user(s).
	 */
	function lockEntry($entry, $type = "lock") {
		// Filter out entries we are not allowed to modify
		$disallowed = array ();
		$dns = array ();
		foreach ( $entry as $dn ) {
			if (! preg_match ( "/w/", $this->ui->get_permissions ( $dn, "users/password" ) )) {
				$disallowed [] = $dn;
			} else {
				$allowed [] = $dn;
			}
		}
		if (count ( $disallowed )) {
			msg_dialog::display ( _ ( "Permission" ), msgPool::permDelete ( $disallowed ), INFO_DIALOG );
		}

		// Try to lock/unlock the rest of the entries.
		foreach ( $allowed as $dn ) {
			$this->_ldap->cat ( $dn, array (
					'userPassword'
			) );
			if ($this->_ldap->count () == 1) {

				// We can't lock empty passwords.
				$val = $this->_ldap->fetch ();
				if (! isset ( $val ['userPassword'] )) {
					continue;
				}

				// Detect the password method and try to lock/unlock.
				$pwd = $val ['userPassword'] [0];
				$method = passwordMethod::get_method ( $pwd, $val ['dn'] );
				$success = true;
				if ($method instanceof passwordMethod) {
					if ($type == "toggle") {
						if ($method->is_locked ( $this->config, $val ['dn'] )) {
							$success = $method->unlock_account ( $this->config, $val ['dn'] );
						} else {
							$success = $method->lock_account ( $this->config, $val ['dn'] );
						}
					} elseif ($type == "lock" && ! $method->is_locked ( $this->config, $val ['dn'] )) {
						$success = $method->lock_account ( $this->config, $val ['dn'] );
					} elseif ($type == "unlock" && $method->is_locked ( $this->config, $val ['dn'] )) {
						$success = $method->unlock_account ( $this->config, $val ['dn'] );
					}
					
					// Check if everything went fine.
					if (! $success) {
						$hn = $method->get_hash_name ();
						if (is_array ( $hn )) {
							$hn = $hn [0];
						}
						msg_dialog::display ( _ ( "Account locking" ), sprintf ( _ ( "Password method '%s' does not support locking. Account (%s) has not been locked!" ), $hn, $dn ), WARNING_DIALOG );
					}
				} else {
					// Can't lock unknown methods.
				}
			}
		}
	}
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
