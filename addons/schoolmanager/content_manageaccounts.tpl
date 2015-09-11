<p>
	{if $import_account_type == "students"}
	{t escape=no}With GOsa2 School Manager's <b>Manage Students Module</b> you can import student user accounts, parent accounts and course groups from a CSV file containing.{/t}
	{elseif $import_account_type == "teachers"}
	{t escape=no}With GOsa2 School Manager's <b>Manage Teachers Module</b> you can import teacher user accounts and course groups from a CSV file containing.{/t}
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
	FIXME! Todo...
</p>

<table summary="{t}Upload CSV File{/t}">
	<tr>
		<td style="vertical-align: middle;">
			{if $import_account_type=="students"}
				<LABEL for="userfile">{t}Select CSV file with students' data to import:{/t}</LABEL>
				{elseif $import_account_type=="teachers"}
				<LABEL for="userfile">{t}Select CSV file with teachers' data to import:{/t}</LABEL>
			{/if}
		</td>
		<td style="vertical-align: middle;">
			<input type="hidden" name="MAX_FILE_SIZE" value="2097152">
			<input id="userfile" name="userfile" type="file" value="{t}Browse{/t}">
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

FIXMEs:
<ul>
	<li>Provide a checkbox to disable parent imports.
</ul>

<table summary="{t}Select user object templates{/t}">
{if $import_account_type == 'students'}
	<tr>
		<td colspan="3">
			<hr>
			<p>{t}Please choose from the available GOsa2 user object templates what account templates to use for students and for their parents.{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="template_students">{t}Select template for student accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_students" name="template_students"  size="1" title="">
			{html_options options=$templates selected=$preset_template_students}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="template_students_aux">{t}Select template for parent accounts{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="template_students_aux" name="template_students_aux"  size="1" title="">
			{html_options options=$templates selected=$preset_template_students_aux}
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
	<tr>
		<td colspan="3">
			<p>{t}With the start of a new school year, it is advisable to flush group members from all class and course groups? At the beginning of a new school year, we recommend starting with an import of all teachers first and flush all group members during this import.{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="flush_members">{t}Flush members from course and class groups?{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
{if $preset_flush_members}
			<input type="checkbox" id="flush_members" name="flush_members" checked>
{else}
			<input type="checkbox" id="flush_members" name="flush_members">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>
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
{foreach from=$data[0] item=val key=key}
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
		{if $key == 0}

		<!-- FIXME: with very small data sets (up to 6 objects) the below dots should not be printed on screen!!! -->

		<td  style="vertical-align: middle;" bgcolor="#EEEEEE" rowspan={$num_rows}>&nbsp;&nbsp;&nbsp;<b>...</b>&nbsp;&nbsp;&nbsp;</td>
		{/if}
	</tr>
{/foreach}
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
{if $data[$key]['aux_accounts']}
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
{if $data[$key]['aux_accounts']}
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
{if $property != "userPassword" || $data[$key]['main_account'][$property][0]===""}
						{$data[$key]['main_account'][$property][0]}
{elseif strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
						{t}<keep>{/t}
{else}
						************
{/if}
					</td>
				</tr>
{/foreach}
{if $accounts_reviewed && $accounts_imported}
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
{/if}
{if $data[$key]['aux_accounts']}
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
{if $property != "userPassword" || $aux_account[$property][0]===""}
						{$aux_account[$property][0]}
{elseif strpos($aux_account['_status'][0],"exists")!==FALSE }
						{t}<keep>{/t}
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
{if in_array('posixGroup', $group['objectClass']) and in_array($group['cn'][0], array_keys($aux_account['_group_actions']))}
				<tr>
					<td bgcolor="#EEEEEE">
					{$group['cn'][0]}
					</td>
					<td bgcolor="#F8F8F8">
					{$aux_account['_group_actions'][$group['cn'][0]]}
					</td>
				</tr>
{elseif in_array('gosaGroupOfNames', $group['objectClass']) and in_array($group['cn'][0], array_keys($aux_account['_ogroup_actions']))}
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
{if (!$data[$key]['aux_accounts'])}
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
<b>{t}Standard (POSIX) groups{/t}:</p>
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
	{t}Continue here, when ready{/t}: <button type='submit' name='fileup'>{t 1="2/11"}Set import configuration option (Step %1){/t}</button>

	{elseif $import_configured != TRUE}
	{t}Continue here, when ready{/t}: <input name="btn_template_selected" value="{t 1="3/11"}Check and Sort CSV Data (Step %1){/t}" type ="submit">

	{elseif $data_sorted != TRUE}
	{t}Continue here, when ready{/t}: <input name="btn_data_sorted" value="{t 1="4/11"}Review user account objects before LDAP import (Step %1){/t}" type ="submit">

	{elseif $accounts_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <input name="btn_accounts_reviewed" value="{t 1="5/11"}Import user account objects into LDAP (Step %1){/t}" type ="submit">

	{elseif $accounts_imported != TRUE}
	{t}Continue here, when ready{/t}: <input name="btn_data_sorted" value="{t 1="6/11"}Review group objects (Step %1){/t}" type ="submit">

	{elseif $groups_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <input name="btn_data_sorted" value="{t 1="7/11"}Import group objects into LDAP (Step %1){/t}" type ="submit">

	{elseif $groups_imported != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <input name="btn_accounts_reviewed" value="{t 1="8/11"}Review accounts' group memberships before LDAP import (Step %1){/t}" type ="submit">

	{elseif $accounts_groupmembers_reviewed != TRUE}
	{t}Continue here, when ready (this can take a while...){/t}: <input name="btn_accounts_reviewed" value="{t 1="9/11"}Update accounts' group memberships in LDAP (Step %1){/t}" type ="submit">

	{elseif $accounts_groupmembers_updated != TRUE}
	{t}Continue here, when ready{/t}: <input name="btn_accounts_reviewed" value="{t 1="10/11"}Some post-import clean-ups (Step %1){/t}" type ="submit">

	{elseif $cleanup_completed != TRUE}
	{t}Continue here, when ready{/t}: <input name="btn_accounts_reviewed" value="{t 1="11/11"}Finish LDAP import (Step %1){/t}" type ="submit">

	{/if}
</div>
