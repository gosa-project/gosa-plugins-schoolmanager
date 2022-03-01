<p>
    {t escape=no}With GOsa2 School Manager's <b>Archive Accounts Module</b> you can archive student user accounts, parent accounts, teacher from multiple CSV files.{/t}
</p>
<hr>

<!--Introduction-->

{if $chosen_archivetype != TRUE}
<input type="hidden" name="phase_00">

<br><h3>{t 1="1/11"}STEP %1: Choose archive type{/t}</h3>
<p>
    {t}Choose archive type. teacher and students are seperate. The students home directory will be deleted directly.{/t}
    {t}The teachers home directory will be removed after a period of time.{/t}
</p>
<table summary="{t}Choose archive type{/t}">
	<tr>
		<td style="vertical-align: middle;">
			<br />
			<LABEL for="archivetype_id">{t}Type of user to archive{/t}</LABEL>
			<br />
            {html_radios name="archivetype_id" options=$available_archive_types selected=$preset_archive_type_id separator='<br />'}
		</td>
	</tr>
</table>


<!-- PHASE 1: Upload CSV File-->

{elseif $file_uploaded != TRUE}
<input type="hidden" name="phase_01">

<br><h3>{t 1="2/11"}STEP %1: Upload CSV File{/t}</h3>

<p>
	{t}The data provided via the uploadable CSV file needs to be of the following data format:{/t}
</p>
<p>
	<table>
        {if $archive_type === "students"}
	        <tr><td>{t}Column{/t} 01</td><td><b>{t}Number{/t}</b></td><td>{t}This is useful for discussing issues with CSV import files, but the number column is normally not used.{/t}</td></tr>
            <tr><td>{t}Column{/t} 02</td><td><b>{t}Login{/t}</b></td><td>{t}The user account's login ID{/t}</td></tr>
            <tr><td>{t}Column{/t} 03</td><td><b>{t}Student ID{/t}</b></td><td>{t}An unequivocal number that a student is associated with in the school's administration software.{/t}</td></tr>
            <tr><td>{t}Column{/t} 04</td><td><b>{t}Last Name(s){/t}</b></td><td>{t}The student's last name(s).{/t}</td></tr>
            <tr><td>{t}Column{/t} 05</td><td><b>{t}First Name(s){/t}</b></td><td>{t}The student's first and middle name(s).{/t}</td></tr>
            <tr><td>{t}Column{/t} 06</td><td><b>{t}Date of Birth{/t}</b></td><td>{t}The student's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td></tr>
            <tr><td>{t}Column{/t} 07</td><td><b>{t}Gender{/t}</b></td><td>{t}The student's gender (male / female / ...).{/t}</td></tr>
        {elseif $archive_type === "teachers"}
            <tr><td>{t}Column{/t} 01</td><td><b>{t}Number{/t}</b></td><td>{t}This is useful for manual discussing issues with CSV import files, but the number column is normally not used.{/t}</td></tr>
	        <tr><td>{t}Column{/t} 02</td><td><b>{t}Login{/t}</b></td><td>{t}The user account's login ID. If left empty, it will be attempted to auto-generated a login ID.{/t}</td></tr>
	        <tr><td>{t}Column{/t} 03</td><td><b>{t}Password{/t}</b></td><td>{t}The user's password. If left empty, a password will be generated during the import.{/t}</td></tr>
	        <tr><td>{t}Column{/t} 04</td><td><b>{t}Last Name{/t}</b></td><td>{t}The teacher's last name(s).{/t}</td></tr>
	        <tr><td>{t}Column{/t} 05</td><td><b>{t}First Name{/t}</b></td><td>{t}The teacher's first (and middle) name(s).{/t}</td></tr>
	        <tr><td>{t}Column{/t} 06</td><td><b>{t}Date of Birth{/t}</b></td><td>{t}The teachers's date of birth. This field is an obligatory field as it is used to distinguish accounts with identical first and last names.{/t}</td></tr>
	        <tr><td>{t}Column{/t} 07</td><td><b>{t}Gender{/t}</b></td><td>{t}The teachers's gender (male / female / ...).{/t}</td></tr>
        {/if}
    </table>
</p>

<table summary="{t}Upload CSV File{/t}">
	<tr>
		<td style="vertical-align: middle;">
            <LABEL for="userfile">{t}Select CSV file with data to import:{/t}</LABEL>
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


<!-- PHASE 2: Configuring archive options -->


{elseif $archive_configured != TRUE}
<input type="hidden" name="phase_02">

<br><h3>{t 1="3/11"}STEP %1: Configure archive options{/t}</h3>

<p>
{t}School Manager's archive functionality is very flexible and can be adapted to your needs. Please configure options of this archiving below.{/t}
</p>

<table summary="{t}Configure archive options{/t}">
	<tr>
		<td colspan="3">
			<hr>
			<p>{t}Where in the LDAP tree should the archived Users go?{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="ou_users">{t}Select OU for archiving users:{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="ou_users" name="ou_users" size="1" title="">
			{html_options options=$ous_available selected=$preset_ou_users}
			</select>
		</td>
	</tr>
	<tr>
		<td colspan="3">
			<hr>
			<p>{t}Where in the LDAP tree should we look for matching Users?{/t}</p>
		</td>
	</tr>
	<tr>
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="ou_matching_users">{t}Select OU for matching users:{/t}</LABEL>
		</td>
		<td style="vertical-align: middle;">
			<select id="ou_matching_users" name="ou_matching_users" size="1" title="">
			{html_options options=$ous_available selected=$preset_ou_matching_users}
			</select>
		</td>
	</tr>
</table>

<table style="min-width:100%">
    <tr>
		<td colspan="3">
			<hr>
			<p>{t}Please select the attributes which will identify users from the CSV file as existing users{/t}</p>
		</td>
	</tr>

<!-- Archiving Students or Teachers. Teachers don't have a Student ID. -->
{if $archive_type === "students"}
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
			<LABEL for="sel_ldap_match_attr_name">{t escape=no}Name{/t}</LABEL>
		</td>
		<td style="float: right;">
{if $preset_sel_ldap_match_attr_name}
			<input type="checkbox" id="sel_ldap_match_attr_name" name="sel_ldap_match_attr_name" checked>
{else}
			<input type="checkbox" id="sel_ldap_match_attr_name" name="sel_ldap_match_attr_name">
{/if}
			{t}(tick this check box, if yes){/t}
		</td>
	</tr>

    <tr style="background-color:whitesmoke; color: black;"> <!-- black on lightgray -->
		<td style="width: 1em;">&nbsp;</td>
		<td style="vertical-align: middle;">
			<LABEL for="sel_ldap_match_attr_snname">{t escape=no}Second Name{/t}</LABEL>
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
			<LABEL for="sel_ldap_match_attr_birthday">{t escape=no}Birthday{/t}</LABEL>
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


</table>


<!-- PHASE 3: CSV data sorting -->


{elseif $data_sorted != TRUE}
<input type="hidden" name="phase_03">

<br><h3>{t 1="4/11"}STEP %1: Check CSV data and assign to LDAP attributes{/t}</h3>

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


<!-- PHASE 4: Reviewing and confirming accounts before archiving -->


{elseif $accounts_reviewed != TRUE}

	{if $accounts_reviewed != TRUE}
		<input type="hidden" name="phase_04">
		<br><h3>{t 1="5/11"}STEP %1: Review user account objects before archiving{/t}</h3>
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
			{if $property != "userPassword" || $data[$key]['main_account'][$property][0]===""}
									{$data[$key]['main_account'][$property][0]}
			{elseif strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
								{t}<keep>{/t}
			{elseif strpos($data[$key]['main_account']['_status'][0],"data-incomplete")!==FALSE }
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

		</table>
		<br>
	{/foreach}


<!-- PHASE 5: Confirming archived user accounts -->


{elseif $accounts_archived != TRUE}
<input type="hidden" name="phase_05">

<br><h3>{t 1="6/11"}STEP %1: Confirm archived user accounts{/t}</h3>

	{foreach from=$data item=row key=key}
		<table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
			<tr>
		{if isset($data[$key]['aux_accounts'])}
				<td bgcolor="#BBBBBB" colspan={2 + count($data[$key]['aux_accounts'])}>
		{else}
				<td bgcolor="#BBBBBB" colspan="2">
		{/if}
		{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}
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
			{if $property != "userPassword" || $data[$key]['main_account'][$property][0]===""}
									{$data[$key]['main_account'][$property][0]}
			{elseif strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
								{t}<keep>{/t}
			{elseif strpos($data[$key]['main_account']['_status'][0],"data-incomplete")!==FALSE }
			{else}
								************
			{/if}
							</td>
						</tr>
		{/foreach}
		</table>
		<br>
	{/foreach}


<!-- PHASE 6: Moving primary groups -->


{elseif $primgroups_moved != TRUE}
<p>PRIMGROUPS MOVING PHASE</p>
{/if}

<!-- SUBMIT: Different buttons depending on the above phases -->


<hr>
<div class="plugin-actions">

    {if $chosen_archivetype != TRUE}
	{t}Continue here, when ready{/t}: <button type='submit' name='chosen_archivetype'>{t 1="2/11"}Upload CSV File (Step %1){/t}</button>

	{elseif $file_uploaded != TRUE}
	{t}Continue here, when ready{/t}: <button type='submit' name='file_uploaded'>{t 1="3/11"}Set archive configuration option (Step %1){/t}</button>

	{elseif $archive_configured != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="archive_configured">{t 1="4/11"}Check and Sort CSV Data (Step %1){/t}</button>

	{elseif $data_sorted != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="data_sorted">{t 1="5/11"}Review Accounts (Step %1){/t}</button>

	{elseif $accounts_reviewed != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="accounts_reviewed">{t 1="6/11"}Archive Accounts (Step %1){/t}</button>

	{elseif $accounts_archived != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="accounts_archived">{t 1="7/11"}Move primary group (Step %1){/t}</button>

	{elseif $primgroups_moved != TRUE}
	{t}Continue here, when ready{/t}: <button type="submit" name="primgroups_moved">{t 1="8/11"}stuff (Step %1){/t}</button>

	{/if}

	{if $file_uploaded == TRUE && $cleanup_completed != TRUE}
	    <button type="submit" name="cancel_archiving">{t}Cancel archiving{/t}</button>
	{/if}
</div>
