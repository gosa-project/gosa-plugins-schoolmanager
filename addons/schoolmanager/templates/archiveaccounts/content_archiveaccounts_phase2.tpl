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
