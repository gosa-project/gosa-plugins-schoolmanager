<p>
	{if $import_account_type == "studentsandparents"}
	{t escape=no}With GOsa2 School Manager's <b>Import Students and Parents Module</b> you can import student user accounts, parent accounts, class groups and course groups from a single CSV file containing one user's account information per line.{/t}
	{elseif $import_account_type == "teachers"}
	{t escape=no}With GOsa2 School Manager's <b>Import Teachers Module</b> you can import teacher user accounts, class groups, course groups and subject groups from a single CSV file containing one user's account information per line.{/t}
	{elseif $import_account_type == "studentsonly"}
	{t escape=no}With GOsa2 School Manager's <b>Import Students (only) Module</b> you can import student user accounts, class groups and course groups from a single CSV file containing one user's account information per line.{/t}
	{/if}
</p>
<hr>

<!-- PHASE 1: CSV file upload -->

{if $file_uploaded != TRUE}
<input type="hidden" name="phase_01">

<br><h3>{t 1="1/11"}STEP %1: Upload CSV File{/t}</h3>

<p>
	{t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
</p>
<p>
	<table>
	{if $import_account_type == "studentsandparents"}
	<tr><td>{t}Column{/t} 01</td><td><b>{t}Number{/t}</b></td><td>{t}This is useful for discussing issues with CSV import files, but the number column is normally not used.{/t}</td></tr>
	<tr><td>{t}Column{/t} 02</td><td><b>{t}Login{/t}</b></td><td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td></tr>
	<tr><td>{t}Column{/t} 03</td><td><b>{t}Password{/t}</b></td><td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td></tr>
	<tr><td>{t}Column{/t} 04</td><td><b>{t}Student ID{/t}</b></td><td>{t}An unequivocal number that a student is associated with in the school's administration software.{/t}</td></tr>
	<tr><td>{t}Column{/t} 05</td><td><b>{t}Last Name(s){/t}</b></td><td>{t}The student's last name(s).{/t}</td></tr>
	<tr><td>{t}Column{/t} 06</td><td><b>{t}First Name(s){/t}</b></td><td>{t}The student's first and middle name(s).{/t}</td></tr>
	<tr><td>{t}Column{/t} 07</td><td><b>{t}Date of Birth{/t}</b></td><td>{t}The student's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td></tr>
	<tr><td>{t}Column{/t} 08</td><td><b>{t}Gender{/t}</b></td><td>{t}The student's gender (male / female / ...).{/t}</td></tr>
	<tr><td>{t}Column{/t} 09</td><td><b>{t}Class{/t}</b></td><td>{t}The class name / identifier.{/t}</td></tr>
	<tr><td>{t}Column{/t} 10</td><td><b>{t}Last Name (Parent 1){/t}</b></td><td>{t}Last name of parent 1 (e.g. mother).{/t}</td></tr>
	<tr><td>{t}Column{/t} 11</td><td><b>{t}First Name (Parent 1){/t}</b></td><td>{t}First (and middle) name(s) of parent 1.{/t}</td></tr>
	<tr><td>{t}Column{/t} 12</td><td><b>{t}Mail Address (Parent 1){/t}</b></td><td>{t}Mail address of parent 1.{/t}</td></tr>
	<tr><td>{t}Column{/t} 13</td><td><b>{t}Last Name (Parent 2){/t}</b></td><td>{t}Last name of parent 2 (e.g. father).{/t}</td></tr>
	<tr><td>{t}Column{/t} 14</td><td><b>{t}First Name (Parent 2){/t}</b></td><td>{t}First (and middle) name(s) of parent 2.{/t}</td></tr>
	<tr><td>{t}Column{/t} 15</td><td><b>{t}Mail Address (Parent 2){/t}</b></td><td>{t}Mail address of parent 2.{/t}</td></tr>
	<tr><td>{t}Column{/t} 16&ndash;XX</td><td><b>{t}Courses{/t}</b></td><td>{t}Courses a student is signed up for (one per column, use as many columns as needed).{/t}</td></tr>
	{elseif $import_account_type == "teachers"}
	<tr><td>{t}Column{/t} 01</td><td><b>{t}Number{/t}</b></td><td>{t}This is useful for manual discussing issues with CSV import files, but the number column is normally not used.{/t}</td></tr>
	<tr><td>{t}Column{/t} 02</td><td><b>{t}Login{/t}</b></td><td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td></tr>
	<tr><td>{t}Column{/t} 03</td><td><b>{t}Password{/t}</b></td><td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td></tr>
	<tr><td>{t}Column{/t} 04</td><td><b>{t}Last Name{/t}</b></td><td>{t}The teacher's last name(s).{/t}</td></tr>
	<tr><td>{t}Column{/t} 05</td><td><b>{t}First Name{/t}</b></td><td>{t}The teacher's first (and middle) name(s).{/t}</td></tr>
	<tr><td>{t}Column{/t} 06</td><td><b>{t}Date of Birth{/t}</b></td><td>{t}The teachers's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td></tr>
	<tr><td>{t}Column{/t} 07</td><td><b>{t}Gender{/t}</b></td><td>{t}The student's gender (male / female / ...).{/t}</td></tr>
	<tr><td>{t}Column{/t} 08</td><td><b>{t}Subjects{/t}</b></td><td>{t}The subjects a teacher can teach / is trained for.{/t}</td></tr>
	<tr><td>{t}Column{/t} 09</td><td><b>{t}Class{/t}</b></td><td>{t}The primary class a teacher is assigned to. If the teacher is not a class teacher, then leave empty.{/t}</td></tr>
	<tr><td>{t}Column{/t} 10&ndash;XX</td><td><b>{t}Courses{/t}</b></td><td>{t}Courses a teacher teaches (one per column, use as many columns as needed).{/t}</td></tr>
	{elseif $import_account_type == "studentsonly"}
	<tr><td>{t}Column{/t} 01</td><td><b>{t}Number{/t}</b></td><td>{t}This is useful for discussing issues with CSV import files, but the number column is normally not used.{/t}</td></tr>
        <tr><td>{t}Column{/t} 02</td><td><b>{t}Login{/t}</b></td><td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td></tr>
        <tr><td>{t}Column{/t} 03</td><td><b>{t}Password{/t}</b></td><td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td></tr>
        <tr><td>{t}Column{/t} 04</td><td><b>{t}Student ID{/t}</b></td><td>{t}An unequivocal number that a student is associated with in the school's administration software.{/t}</td></tr>
        <tr><td>{t}Column{/t} 05</td><td><b>{t}Last Name(s){/t}</b></td><td>{t}The student's last name(s).{/t}</td></tr>
        <tr><td>{t}Column{/t} 06</td><td><b>{t}First Name(s){/t}</b></td><td>{t}The student's first and middle name(s).{/t}</td></tr>
        <tr><td>{t}Column{/t} 07</td><td><b>{t}Date of Birth{/t}</b></td><td>{t}The student's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td></tr>
        <tr><td>{t}Column{/t} 08</td><td><b>{t}Gender{/t}</b></td><td>{t}The student's gender (male / female / ...).{/t}</td></tr>
        <tr><td>{t}Column{/t} 09</td><td><b>{t}Class{/t}</b></td><td>{t}The class name / identifier.{/t}</td></tr>
        <tr><td>{t}Column{/t} 10&ndash;XX</td><td><b>{t}Courses{/t}</b></td><td>{t}Courses a student is signed up for (one per column, use as many columns as needed).{/t}</td></tr>
	{/if}
	</table>
</p>

<table summary="{t}Upload CSV File{/t}">
	<tr>
		<td style="vertical-align: middle;">
			{if $import_account_type=="studentsandparents"}
				<LABEL for="userfile">{t}Select CSV file with students' and their parents' data to import:{/t}</LABEL>
				{elseif $import_account_type=="teachers"}
				<LABEL for="userfile">{t}Select CSV file with teachers' data to import:{/t}</LABEL>
				{elseif $import_account_type=="studentsonly"}
				<LABEL for="userfile">{t}Select CSV file with students' data to import:{/t}</LABEL>
			{/if}
		</td>
		<td style="vertical-align: middle;">
			<input type="hidden" name="MAX_FILE_SIZE" value="2097152">
			<input id="userfile" name="userfile" type="file" value="{t}Browse{/t}">
		</td>
	</tr>

	<tr>
		<td>
		<h4>{t}CSV format options:{/t}</h4>
		</td>
	</tr>
	<tr>
		<td style=vertical-align: middle;">
			<LABEL for="csv_with_column_headers">{t}First line in CSV file contains column headers (so ignore that first line):{/t}</LABEL>
			<input type="checkbox" id="csv_with_column_headers" name="csv_with_column_headers" checked>
		</td>
	</tr>
	<tr>
		<td style=vertical-align: middle;">
			<br />
			<LABEL for="delimiter_id">{t}Delimiter for column fields in CSV file:{/t}</LABEL>
			<br />
			{html_radios name="delimiter_id" options=$available_delimiters selected=$preset_delimiter_id separator='<br />'}
		</td>
	</tr>
	<tr>
		<td>
		<br />
		<b>{t}Note:{/t}</b> {t}The following filters are automatically applied on CSV file import...{/t}
		<ul>
		<li>{t}Auto-detection of charset encoding (everything gets converted internally to UTF-8){/t}</li>
		<li>{t}Single and double quotes next to the field delimiters get removed{/t}</li>
		<li>{t}MS-DOS / Unix line ending style gets recognized automatically and processed accordingly{/t}</li>
		</ul>
		</td>
	</tr>
</table>


<!-- PHASE 2: Configuring import options -->

{elseif $import_configured != TRUE}
<input type="hidden" name="phase_02">

<br><h3>{t 1="2/11"}STEP %1: Configure import options{/t}</h3>

<p>
{t}School Manager's import functionality is very flexible and can be adapted to your needs. Please configure options of this import below.{/t}
</p>

<table summary="{t}Configure import options{/t}">
{if $import_account_type == 'studentsandparents'}
	<tr>
		<td colspan="3">
			<hr>
			<p>{t}Please choose from the available GOsa2 user object templates what account templates to use for students and for their parents.{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="template_studentsandparents">{t}Select template for student accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_studentsandparents" name="template_studentsandparents"  size="1" title="">
			{html_options options=$templates selected=$preset_template_studentsandparents}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="template_studentsandparents_aux">{t}Select template for parent accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_studentsandparents_aux" name="template_studentsandparents_aux"  size="1" title="">
			{html_options options=$templates selected=$preset_template_studentsandparents_aux}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="accounts_in_class_ou">{t escape=no}Create a sub-OU for each class and place student accounts belonging to the same class into their corresponding class OU?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_accounts_in_class_ou}
			<input type="checkbox" id="accounts_in_class_ou" name="accounts_in_class_ou" checked>
{else}
			<input type="checkbox" id="accounts_in_class_ou" name="accounts_in_class_ou">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="domain_users">{t}Mail domain for new students (if not specified in import file explicitly):{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			@
			<input type="text" id="domain_users" name="domain_users" value="{$preset_domain_users}">
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="try_mail_as_uid">{t escape=no}If CSV data does not contain a user ID (uid) column, use the mail address as user ID instead?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_try_mail_as_uid}
			<input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid" checked>
{else}
			<input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
{/if}
{if $import_account_type == 'teachers'}
	<tr>
		<td colspan="3">
			<hr>
			{t}Please choose from the available GOsa2 user object templates what account template to use for teachers.{/t}
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="template_teachers">{t}Select template for teacher accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_teachers" name="template_teachers" size="1" title="">
			{html_options options=$templates selected=$preset_template_teachers}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="add_course_members_to_class_group">{t escape=no}Make all teachers teaching courses in a class members of that class group?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_add_course_members_to_class_group}
			<input type="checkbox" id="add_course_members_to_class_group" name="add_course_members_to_class_group" checked>
{else}
			<input type="checkbox" id="add_course_members_to_class_group" name="add_course_members_to_class_group">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="domain_users">{t}Mail domain for new teachers (if not specified in import file explicitly):{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			@
			<input type="text" id="domain_users" name="domain_users" value="{$preset_domain_users}">
		</td>
	</tr>

{if $preset_domain_school}
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="aliases_in_schooldomain">{t}Teachers shall have mail aliases in the school's mail domain ({$preset_domain_school}):{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_aliases_in_schooldomain}
			<input type="checkbox" id="aliases_in_schooldomain" name="aliases_in_schooldomain" checked>
{else}
			<input type="checkbox" id="aliases_in_schooldomain" name="aliases_in_schooldomain">
{/if}
		</td>
	</tr>
{/if}

{/if}
{if $import_account_type == 'studentsonly'}
	<tr>
		<td colspan="3">
			<hr>
			<p>{t}Please choose from the available GOsa2 user object templates what account templates to use for students.{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
            <LABEL for="template_studentsonly">{t}Select template for student accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_studentsonly" name="template_studentsonly"  size="1" title="">
			{html_options options=$templates selected=$preset_template_studentsonly}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="accounts_in_class_ou">{t escape=no}Create a sub-OU for each class and place student accounts belonging to the same class into their corresponding class OU?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_accounts_in_class_ou}
			<input type="checkbox" id="accounts_in_class_ou" name="accounts_in_class_ou" checked>
{else}
			<input type="checkbox" id="accounts_in_class_ou" name="accounts_in_class_ou">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="domain_users">{t}Mail domain for new students (if not specified in import file explicitly):{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			@
			<input type="text" id="domain_users" name="domain_users" value="{$preset_domain_users}">
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="try_mail_as_uid">{t escape=no}If CSV data does not contain a user ID (uid) column, use the mail address as user ID instead?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_try_mail_as_uid}
			<input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid" checked>
{else}
			<input type="checkbox" id="try_mail_as_uid" name="try_mail_as_uid">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
{/if}

	<tr>
		<td colspan="3">
			<hr>
			<p>{t}During this import we will probably create several non-primary groups (for classes, courses). Where in the LDAP tree shall these new group objects be created?{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="ou_groups">{t}Select OU for new groups:{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="ou_groups" name="ou_groups" size="1" title="">
			{html_options options=$ous_available selected=$preset_ou_groups}
			</select>
		</td>
	</tr>
	<tr>
		<td colspan="3">
			<p>{t}And what mail domain should these new groups be in?{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="domain_groups">{t}Mail domain for new groups:{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			@
			<input type="text" id="domain_groups" name="domain_groups" value="{$preset_domain_groups}">
		</td>
	</tr>

<tr><td colspan="3"><table style="min-width: 100%;">
        <tr>
		<td colspan="3">
			<hr>
			<p>{t}Please select the attributes which will identify users from the CSV file as existing users{/t}:</p>
		</td>
	</tr>

{if $import_account_type != 'teachers'}
    <tr style="background-color:whitesmoke; color: black;"> <!-- black on lightgray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_studentid">{t escape=no}Student ID{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_studentid}
			<input type="checkbox" id="sel_ldap_match_attr_studentid" name="sel_ldap_match_attr_studentid" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_studentid" name="sel_ldap_match_attr_studentid">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
{/if}

	<tr style="background-color: gainsboro; color: black"> <!-- black on gray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_givenname">{t escape=no}Given Name{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_givenname}
			<input type="checkbox" id="sel_ldap_match_attr_givenname" name="sel_ldap_match_attr_givenname" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_givenname" name="sel_ldap_match_attr_givenname">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>

    <tr style="background-color:whitesmoke; color: black;"> <!-- black on lightgray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_snname">{t escape=no}Surname{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_snname}
			<input type="checkbox" id="sel_ldap_match_attr_snname" name="sel_ldap_match_attr_snname" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_snname" name="sel_ldap_match_attr_snname">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>

    <tr style="background-color: gainsboro; color: black"> <!-- black on gray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_birthday">{t escape=no}Date of Birth{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_birthday}
			<input type="checkbox" id="sel_ldap_match_attr_birthday" name="sel_ldap_match_attr_birthday" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_birthday" name="sel_ldap_match_attr_birthday">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>

    <tr style="background-color: whitesmoke; color: black;"> <!-- black on lightgray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_gender">{t escape=no}Gender{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_gender}
			<input type="checkbox" id="sel_ldap_match_attr_gender" name="sel_ldap_match_attr_gender" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_gender" name="sel_ldap_match_attr_gender">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>


</td></tr></table>
</table>


<!-- PHASE 3: CSV data sorting -->

{elseif $data_sorted != TRUE}
<input type="hidden" name="phase_03">

<br><h3>{t 1="3/11"}STEP %1: Check CSV data and assign to LDAP attributes{/t}</h3>

<p>
{t}Please assign the offered (LDAP) attributes to the CSV data columns. Note that the CSV file has been rotated 90DEGREES counter-clockwise for better readability.{/t}
</p>
<br>
<br>
<table summary="{t}Check CSV data{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
{for $key=0 to $num_rows-1}
	<tr>
		<td bgcolor="#BBBBBB">
			<select name="column_head_{$key}" size="1" title="">
				{html_options options=$attrs selected=$attrs_selected[$key]}
			</select>
		</td>
		{foreach from=$data item=val2 key=key2}
			<td bgcolor="#EEEEEE">
				{$data[$key2][$key]}&nbsp;
			</td>
		{/foreach}
		{if $key == 0 && $data_size > 5}
		<td  style="vertical-align: middle;" bgcolor="#EEEEEE" rowspan={$num_rows}>&nbsp;&nbsp;&nbsp;<b>... {$data_size}</b>&nbsp;&nbsp;&nbsp;</td>
		{/if}
	</tr>
{/for}
</table>

{elseif $accounts_reviewed != TRUE || $accounts_imported != TRUE || ( $groups_reviewed == TRUE && $groups_imported == TRUE && ( $accounts_groupmembers_reviewed != TRUE || $accounts_groupmembers_updated != TRUE ) )  }

{if $accounts_reviewed != TRUE}
<input type="hidden" name="phase_04">
<br><h3>{t 1="4/11"}STEP %1: Review user account objects before LDAP import{/t}</h3>

{elseif $accounts_imported != TRUE}
<input type="hidden" name="phase_05">
<br><h3>{t 1="5/11"}STEP %1  (LDAP import status): User account objects have been imported into LDAP{/t}</h3>

<!-- PHASE 6 and 7 require viewing group properties, see below -->

{elseif $accounts_groupmembers_reviewed != TRUE}
<input type="hidden" name="phase_08">
<br><h3>{t 1="8/11"}STEP %1  Review group memberships before LDAP update{/t}</h3>

{elseif $accounts_groupmembers_updated != TRUE}
<input type="hidden" name="phase_09">
<br><h3>{t 1="9/11"}STEP %1 (LDAP import status): Accounts' group memberships have been updated in LDAP{/t}</h3>

{/if}

{foreach from=$data item=row key=key}
<table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">

	<tr>
{if isset($data[$key]['aux_accounts'])}
		<td bgcolor="#BBBBBB" colspan={2 + count($data[$key]['aux_accounts'])}>
{else}
		<td bgcolor="#BBBBBB" colspan="2">
{/if}
{if $data[$key]['main_account']['_status'][0] === "not-found"}
			{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}New user account: %1, %2{/t}
{else}
			{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}
{/if}
		</td>
	</tr>
{if isset($data[$key]['aux_accounts'])}
	<tr>
		<td>
			<table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}Account group for %1, %2{/t}">
{/if}
{foreach from=$data[$key]['main_account'] item=value key=property}
				<tr>
					<td bgcolor="#EEEEEE">
						<b>{$property}:</b>
					</td>
					<td bgcolor="#F8F8F8">
{if ($property != "userPassword" && $property != "alias") || ( isset($data[$key]['main_account'][$property][0]) && $data[$key]['main_account'][$property][0]==="" )}
						{$data[$key]['main_account'][$property][0]}
{elseif $property == "userPassword" && strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
						{t}<keep>{/t}
{elseif strpos($data[$key]['main_account']['_status'][0],"data-incomplete")!==FALSE }
{elseif $property == "alias" }
{foreach from=$data[$key]['main_account']['alias'] item=alias key=idx}
{if "$idx" != "count" }
						{$alias}<br>
{/if}
{/foreach}
{else}
						************
{/if}
					</td>
				</tr>
{/foreach}
{if $accounts_reviewed && $accounts_imported && !preg_match('/ignore/', $data[$key]['main_account']['_actions'][0])}
				<tr><td colspan="2" style="height: 0.2em;"></td></tr>
				<tr>
					<td bgcolor="#BBBBBB">
					<b>{t}Groups{/t}:</b>
					</td>
					<td bgcolor="#EEEEEE">
					<b>{t}Group membership actions{/t}:</b>
					</td>
				</tr>
{foreach $data[$key]['groups'] item=group key=idx_group}
{if in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_group_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$data[$key]['main_account']['_group_actions'][$group['cn'][0]]}
					</td>
				</tr>
{elseif in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_ogroup_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$data[$key]['main_account']['_ogroup_actions'][$group['cn'][0]]}
					</td>
				</tr>
{/if}
{/foreach}
{if isset($data[$key]['optional_groups'])}
{foreach $data[$key]['optional_groups'] item=group key=idx_group}
{if (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_group_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$data[$key]['main_account']['_group_actions'][$group['cn'][0]]}
					</td>
				</tr>
{elseif (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($data[$key]['main_account']['_ogroup_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$data[$key]['main_account']['_ogroup_actions'][$group['cn'][0]]}
					</td>
				</tr>
{/if}
{/foreach}
{/if}
{/if}
{if isset($data[$key]['aux_accounts'])}
			</table>
		</td>
{foreach $data[$key]['aux_accounts'] item=aux_account key=idx_aux_account}
		<td>
			<table summary="{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
				<tr>
					<td bgcolor="#BBBBBB" colspan="2">
{if $aux_account['_status'][0] === "not-found"}
						{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}New associated account: %1, %2{/t}
{elseif $aux_account['_status'][0] === "data-incomplete"}
						{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}Insufficient account data: %1, %2{/t}
{else}
						{t 1=$aux_account['sn'][0] 2=$aux_account['givenName'][0]}Associated account: %1, %2{/t}
{/if}
					</td>
				</tr>
{foreach from=$aux_account item=value key=property}
				<tr>
					<td bgcolor="#EEEEEE">
						<b>{$property}:</b>
					</td>
					<td bgcolor="#F8F8F8">
{if $property != "userPassword" || ( isset ($aux_account[$property][0]) && $aux_account[$property][0]==="" )}
						{$aux_account[$property][0]}
{elseif strpos($aux_account['_status'][0],"exists")!==FALSE }
						{t}<keep>{/t}
{elseif strpos($aux_account['_status'][0],"data-incomplete")!==FALSE }
{else}
						************
{/if}
					</td>
				</tr>
{/foreach}
{if $accounts_reviewed and $accounts_imported}
{if strpos($aux_account['_status'][0],"data-incomplete")===FALSE}
				<tr><td colspan="2" style="height: 0.2em;"></td></tr>
				<tr>
					<td bgcolor="#BBBBBB">
					<b>{t}Groups{/t}:</b>
					</td>
					<td bgcolor="#EEEEEE">
					<b>{t}Group membership actions{/t}:</b>
					</td>
				</tr>
{foreach $data[$key]['aux_accounts_groups'] item=group key=idx_group}
{if (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($aux_account['_group_actions'][$group['cn'][0]]))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$aux_account['_group_actions'][$group['cn'][0]]}
					</td>
				</tr>
{elseif (strpos($group['_status'][0], 'not-found')===FALSE) and in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($aux_account['_ogroup_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$aux_account['_ogroup_actions'][$group['cn'][0]]}
					</td>
				</tr>
{/if}
{/foreach}
{/if}
{/if}
			</table>
		</td>
{/foreach}
	</tr>
{/if}


</table>
<br>
{/foreach}
{if (!isset($data[$key]['aux_accounts']))}
{/if}

{elseif $groups_reviewed != TRUE || $groups_imported != TRUE}

{if $groups_reviewed != TRUE}
<input type="hidden" name="phase_06">
<br><h3>{t 1="6/11"}STEP %1: Review group objects before LDAP import{/t}</h3>

{elseif $groups_imported != TRUE}
<input type="hidden" name="phase_07">
<br><h3>{t 1="7/11"}STEP %1 (LDAP import status): Group objects have been imported into LDAP{/t}</h3>

{/if}

{if count($data_groups) > 0}
<br>
<b>{t}Standard (POSIX) groups{/t}:</b></p>
<hr>
<br>

{foreach from=$data_groups item=group key=key}
<div style="float:left; width:24em; height:30ex;">
<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
		<tr>
		<td bgcolor="#BBBBBB" colspan="2">
				{t 1=$key}Group object: %1{/t}
		</td>
		<td style="width:1em;">&nbsp;</td>
		</tr>
		{foreach from=$group item=value key=property}
		<tr>
				<td bgcolor="#EEEEEE">
				<b>{$property}:</b>
				</td>
				<td bgcolor="#FEFEFE">
				{$group[$property][0]}
				</td>
				<td style="width:1em;">&nbsp;</td>
		</tr>
		{/foreach}
</table>
<br>
</div>
{/foreach}
<div class="clear"></div>

<br>
<br>
{/if}

{if count($data_ogroups) > 0}
<b>{t}Object groups{/t}:</b>
<hr>
<br>

{foreach from=$data_ogroups item=group key=key}
<div style="float:left; width:24em; height:30ex;">
<table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
		<tr>
		<td bgcolor="#BBBBBB" colspan="2">
				{t 1=$key}Group object: %1{/t}
		</td>
		<td style="width:1em;">&nbsp;</td>
		</tr>
{foreach from=$group item=value key=property}
{if $property!=="member"}
		<tr>
				<td bgcolor="#EEEEEE">
				<b>{$property}:</b>
				</td>
				<td bgcolor="#FEFEFE">
				{$group[$property][0]}
				</td>
				<td style="width:1em;">&nbsp;</td>
		</tr>
{/if}
{/foreach}
</table>
<br>
</div>
{/foreach}
{/if}
<div class="clear"></div>


{elseif $cleanup_completed != TRUE}
<input type="hidden" name="phase_10">

<br><h3>{t 1="10/11"}STEP %1: Check CSV data and assign to LDAP attributes{/t}</h3>

<p>
FIXME: Some nice clean-up introduction text...
</p>

{else}

<br><h3>{t 1="11/11"}STEP %1: LDAP import completed. Statistical overview of all import actions.{/t}</h3>

<p>
{if $error == FALSE}
<b>{t}All entries have been written to the LDAP database successfully.{/t}</b>
{else}
<b style="color:red">{t}There has been at least one error during the import of your data.{/t}</b>
{/if}
</p>

<b>{t}Here is the status report for the import:{/t} </b>
<br>
<br>
FIXME: Todo.
{/if}


<!-- SUBMIT: Different buttons depending on the above phases -->

<hr>
<div class="plugin-actions">

	{if $file_uploaded != TRUE}
	{t}Continue here, when ready{/t}: <button type='submit' name='file_uploaded'>{t 1="2/11"}Set import configuration option (Step %1){/t}</button>

	{elseif $import_configured != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="import_configured">{t 1="3/11"}Check and Sort CSV Data (Step %1){/t}</button>

	{elseif $data_sorted != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="data_sorted">{t 1="4/11"}Review user account objects before LDAP import (Step %1){/t}</button>

	{elseif $accounts_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <button type="submit" name="accounts_reviewed">{t 1="5/11"}Import user account objects into LDAP (Step %1){/t}</button>

	{elseif $accounts_imported != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="accounts_imported">{t 1="6/11"}Review group objects (Step %1){/t}</button>

	{elseif $groups_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <button type="submit" name="groups_reviewed">{t 1="7/11"}Import group objects into LDAP (Step %1){/t}</button>

	{elseif $groups_imported != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <button type="submit" name="groups_imported">{t 1="8/11"}Review accounts' group memberships before LDAP import (Step %1){/t}</button>

	{elseif $accounts_groupmembers_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <button type="submit" name="accounts_groupmembers_reviewed">{t 1="9/11"}Update accounts' group memberships in LDAP (Step %1){/t}</button>

	{elseif $accounts_groupmembers_updated != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="accounts_groupmembers_updated">{t 1="10/11"}Some post-import clean-ups (Step %1){/t}</button>

	{elseif $cleanup_completed != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="cleanup_completed">{t 1="11/11"}Finish LDAP import (Step %1){/t}</button>

	{/if}

	{if $file_uploaded == TRUE && $cleanup_completed != TRUE}
	<button type="submit" name="cancel_import">{t}Cancel Import{/t}</button>
	{/if}
</div>
